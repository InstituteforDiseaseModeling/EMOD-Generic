//******************************************************************************
//
// Reporter for serosurveys
//
//******************************************************************************

#pragma once

#include "stdafx.h"

#include "ReportSerosurvey.h"

#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "IdmDateTime.h"
#include "INodeContext.h"
#include "IIndividualHuman.h"
#include "InterventionsContainer.h"

//******************************************************************************

#define DEFAULT_REP_NAME       ("ReportSerosurvey.csv")
#define NULL_PROP              ("")

#define DESC_TEXT_AGE_BINS     ("List of ages in days defining the upper value on age bins.")
#define DESC_TEXT_REPORT_NAME  ("Output file name.")
#define DESC_TEXT_SUS_THRESH   ("Susceptibility level that must be exceeded to qualify as susceptible.")
#define DESC_TEXT_TARG_PROP    ("A key:value label for target agents. Empty string targets all agents.")
#define DESC_TEXT_TIME_STAMPS  ("List of timesteps to log serosurvy data.")

//******************************************************************************

SETUP_LOGGING( "ReportSerosurvey" )

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
    return new Kernel::ReportSerosurvey();
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
    ReportSerosurvey::ReportSerosurvey()
        : BaseTextReport(DEFAULT_REP_NAME, false)
        , m_sus_thresh(0.0f)
        , m_targ_prop()
        , m_age_bins()
        , m_time_stamps()
    { }


    // Destructor
    ReportSerosurvey::~ReportSerosurvey()
    { }


    // Retrieves values from reporter-specific config file
    bool ReportSerosurvey::Configure(const Configuration* inputJson)
    {
        std::string fName;

        initConfigTypeMap("Report_Name",               &fName,          DESC_TEXT_REPORT_NAME, DEFAULT_REP_NAME);
        initConfigTypeMap("Target_Property",           &m_targ_prop,    DESC_TEXT_TARG_PROP,   NULL_PROP);

        initConfigTypeMap("Susceptibility_Threshold",  &m_sus_thresh,   DESC_TEXT_SUS_THRESH,  0.0f,  1.0f,  0.0f);

        initConfigTypeMap("Age_Bins",                  &m_age_bins,     DESC_TEXT_AGE_BINS,    0.0f,  FLT_MAX,  true);
        initConfigTypeMap("Time_Stamps",               &m_time_stamps,  DESC_TEXT_TIME_STAMPS, 0.0f,  FLT_MAX,  true);

        bool retVal = JsonConfigurable::Configure( inputJson );

        BaseTextReport::SetReportName(fName);

        std::reverse(m_time_stamps.begin(), m_time_stamps.end());

        return retVal;
    }


    // Provides header line; called by BaseTextReport
    std::string ReportSerosurvey::GetHeader() const
    {
        std::stringstream hline;

        hline << "TIME,NODE,";
        for(int k1 = 0; k1 < m_age_bins.size(); k1++)
        {
            std::string bval = std::to_string(static_cast<int>(m_age_bins[k1]));
            hline << "SUS_" << bval << "," << "TOT_" << bval << ",";
        }

        std::string ret_val = hline.str();
        ret_val.pop_back();

        return ret_val;
    }


    // Evaluates for each node
    void ReportSerosurvey::LogNodeData(INodeContext* node)
    {
        if(m_time_stamps.empty() || node->GetTime().time < m_time_stamps.back())
        {
            return;
        }

        // Accumulate data
        std::vector<float> bin_sus(m_age_bins.size(), 0.0f);
        std::vector<float> bin_tot(m_age_bins.size(), 0.0f);

        for (auto human : node->GetHumans())
        {
            // Skip agent if property restriction exists and doesn't match
            if(m_targ_prop != NULL_PROP && !human->GetProperties()->Contains(m_targ_prop))
            {
                continue;
            }

            int abin_id = 0;
            for(; abin_id < m_age_bins.size(); abin_id++)
            {
                if(human->GetAge() < m_age_bins[abin_id])
                {
                    break;
                }
            }

            if(abin_id < m_age_bins.size())
            {
                bin_tot[abin_id] += human->GetMonteCarloWeight();
                // Adding route aware IVs required querying based on route of infection; CONTACT route is default
                if(human->GetSusceptibilityContext()->getModAcquire() *
                   human->GetVaccineContext()->GetInterventionReducedAcquire(TransmissionRoute::CONTACT) > m_sus_thresh)
                {
                    bin_sus[abin_id] += human->GetMonteCarloWeight();
                }
            }
        }

        // Log data
        std::stringstream datline;

        datline << node->GetTime().time   << ","
                << node->GetExternalID()  << ",";

        int abin_id = 0;
        for(; abin_id < m_age_bins.size(); abin_id++)
        {
            datline << bin_sus[abin_id] << ","
                    << bin_tot[abin_id] << ",";
        }

        std::string ret_val = datline.str();
        ret_val.pop_back();

        GetOutputStream() << ret_val << "\n";

        return;
    }


    // End of timestep operations; NOTE - time has already been incremented
    void ReportSerosurvey::EndTimestep(float currentTime, float dt)
    {
        if(m_time_stamps.empty() || currentTime-dt < m_time_stamps.back())
        {
            return;
        }

        m_time_stamps.pop_back();

        // Call to parent end timestep
        BaseTextReport::EndTimestep(currentTime, dt);

        return;
    }

}

//******************************************************************************
