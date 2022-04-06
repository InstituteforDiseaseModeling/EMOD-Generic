//******************************************************************************
//
// Reporter for strain tracking
//
//******************************************************************************

#pragma once

#include "stdafx.h"

#include "ReportStrainTracking.h"

#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "IdmDateTime.h"
#include "INodeContext.h"
#include "IIndividualHuman.h"

//******************************************************************************

#define DEFAULT_REP_NAME       ("ReportStrainTracking.csv")
#define DESC_TEXT_REPORT_NAME  ("Output file name.")
#define DESC_TEXT_TIME_START   ("No output prior to this timestep.")
#define DESC_TEXT_TIME_END     ("No output after this timestep.")

//******************************************************************************

SETUP_LOGGING( "ReportStrainTracking" )

static const char* _sim_types[]            = { "*", nullptr };

Kernel::DllInterfaceHelper DLL_HELPER( _module, _sim_types );

//******************************************************************************
// DLL Methods
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

DTK_DLLEXPORT char*
__cdecl GetEModuleVersion(char* sVer, const Environment* pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void
__cdecl GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char*
__cdecl GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT Kernel::IReport*
__cdecl GetReportInstantiator(Kernel::report_instantiator_function_t* pif)
{
    return new Kernel::ReportStrainTracking();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************
// Class Methods
//******************************************************************************

namespace Kernel
{
    // Constructor
    ReportStrainTracking::ReportStrainTracking()
        : BaseTextReport(DEFAULT_REP_NAME, false)
        , m_all_done(false)
        , m_time_start(0.0f)
        , m_time_end(FLT_MAX)
    { }


    // Destructor
    ReportStrainTracking::~ReportStrainTracking()
    { }


    // Retrieves values from reporter-specific config file
    bool ReportStrainTracking::Configure(const Configuration* inputJson)
    {
        std::string fName;

        initConfigTypeMap("Report_Name", &fName,        DESC_TEXT_REPORT_NAME, DEFAULT_REP_NAME);
        initConfigTypeMap("Time_Start",  &m_time_start, DESC_TEXT_TIME_START,  0.0f, FLT_MAX, 0.0f);
        initConfigTypeMap("Time_End",    &m_time_end,   DESC_TEXT_TIME_END,    0.0f, FLT_MAX, FLT_MAX);

        bool retVal = JsonConfigurable::Configure( inputJson );

        BaseTextReport::SetReportName(fName);

        return retVal;
    }


    // Provides header line; called by BaseTextReport
    std::string ReportStrainTracking::GetHeader() const
    {
        return "TIME,NODE,CLADE,GENOME,TOT_INF,CON_INF,CONTAGION,NEW_INF,LABEL";
    }


    // Evaluates for each node
    void ReportStrainTracking::LogNodeData(INodeContext* node)
    {
        if(m_all_done || node->GetTime().time < m_time_start)
        {
            return;
        }

        // Add data for new infections;
        // new infections are identified by total duration of zero at end of timestep
        for (auto human : node->GetHumans())
        {
            for (auto infection : human->GetInfections())
            {
                if(infection->GetDuration() == 0.0f)
                {
                    auto uID = infection->GetStrain()->GetStrainName();

                    // Initialize map if not present
                    if(node->GetStrainData().count(uID) == 0)
                    {
                        node->GetStrainData()[uID] = std::vector<float> {0.0f, 0.0f, 0.0f, 0.0f};
                    }

                    // Record data on new infections
                    node->GetStrainData()[uID][INDEX_RST_NEW_INF] += human->GetMonteCarloWeight();
                }
            }
        }

        // Report summary vector for each strain in node
        for (const auto &uIDval : node->GetStrainData())
        {
            std::pair<uint32_t,uint64_t> uID = uIDval.first;

            GetOutputStream() << node->GetTime().time                            << ","         // Time
                              << node->GetExternalID()                           << ","         // NodeID
                              << (std::get<0>(uID))                              << ","         // Clade
                              << (std::get<1>(uID) &  MAX_24BIT)                 << ","         // Genome
                              << node->GetStrainData()[uID][INDEX_RST_TOT_INF]   << ","         // Total Infections
                              << node->GetStrainData()[uID][INDEX_RST_CON_INF]   << ","         // Contagious Infections
                              << node->GetStrainData()[uID][INDEX_RST_CONTAGION] << ","         // Total Contagion
                              << node->GetStrainData()[uID][INDEX_RST_NEW_INF]   << ","         // Newly added infections
                              << (std::get<1>(uID) >> SHIFT_BIT)                 << std::endl;  // Infection label
        }

        return;
    }


    // End of timestep operations; NOTE - time has already been incremented
    void ReportStrainTracking::EndTimestep(float currentTime, float dt)
    {
        if(m_all_done || currentTime-dt < m_time_start)
        {
            return;
        }

        // Call to parent end timestep
        BaseTextReport::EndTimestep(currentTime, dt);

        if(currentTime-dt >= m_time_end)
        {
            m_all_done = true;
        }

        return;
    }

}

//******************************************************************************
