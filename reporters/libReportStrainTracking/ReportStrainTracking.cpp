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

#define REP_VERSION            ("1.02")
#define NUM_COLUMNS            ("8")
#define DEFAULT_REP_NAME       ("ReportStrainTracking.csv")
#define DESC_TEXT_REPORT_NAME  ("Output file name.")
#define DESC_TEXT_TIME_START   ("No output prior to this timestep.")
#define DESC_TEXT_TIME_END     ("No output after this timestep.")

//******************************************************************************

SETUP_LOGGING("ReportStrainTracking")
static const char* _sim_types[]            = { "*", nullptr };

Kernel::report_instantiator_function_t rif = 
          [](){return (Kernel::IReport*)(new Kernel::ReportStrainTracking());};

Kernel::DllInterfaceHelper DLL_HELPER( _module, _sim_types, rif );

//******************************************************************************
// DLL Methods
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

DTK_DLLEXPORT char* __cdecl 
GetEModuleVersion(char* sVer, const Environment* pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char* __cdecl
GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT void __cdecl
GetReportInstantiator(Kernel::report_instantiator_function_t* pif)
{
    DLL_HELPER.GetReportInstantiator( pif );
}

#ifdef __cplusplus
}
#endif

//******************************************************************************
// Class Methods
//******************************************************************************

namespace Kernel
{
  // Constructor; needs default_name string and report_every_timestep boolean
  ReportStrainTracking::ReportStrainTracking()
    : BaseTextReport(DEFAULT_REP_NAME, false)
    , m_all_done(false)
    , m_time_start(0.0f)
    , m_time_end(FLT_MAX)
  {
    // Nothing
  }


  // Destructor
  ReportStrainTracking::~ReportStrainTracking()
  {
    // Nothing
  }


  // Retrieves values from reporter-specific config file
  bool ReportStrainTracking::Configure(const Configuration* inputJson)
  {
    // Configure optional parameters
    JsonConfigurable::_useDefaults = true;
    std::string fName;
    initConfigTypeMap( "Report_Name", &fName,        DESC_TEXT_REPORT_NAME, DEFAULT_REP_NAME);
    initConfigTypeMap( "Time_Start",  &m_time_start, DESC_TEXT_TIME_START,  0.0f, FLT_MAX, 0.0f);
    initConfigTypeMap( "Time_End",    &m_time_end,   DESC_TEXT_TIME_END,    0.0f, FLT_MAX, FLT_MAX);
    bool retVal = JsonConfigurable::Configure( inputJson );

    // Update report name
    BaseTextReport::SetReportName(fName);

    return retVal;
  }


  // Provides header line; called by BaseTextReport
  std::string ReportStrainTracking::GetHeader() const
  {
    return "TIME,NODE,CLADE,GENOME,TOT_INF,CON_INF,CONTAGION,NEW_INF";
  }


  // Evaluates for each node
  void ReportStrainTracking::LogNodeData(INodeContext* node)
  {
    if(m_all_done || node->GetTime().time < m_time_start)
    {
      return;
    }

    std::string                  uID;

    std::map<std::string, int>                 smap_clade;
    std::map<std::string, int>                 smap_genome;
    std::map<std::string, float>               smap_new_inf;
    std::map<std::string, std::vector<float>>  smap_data;

    smap_clade  = node->GetStrainClades();
    smap_genome = node->GetStrainGenomes();
    smap_data   = node->GetStrainData();

    // Create data map for new infections; new infections are identified by 
    // having total duration of zero at end-timestep
    for (auto human : node->GetHumans())
    {
      for (auto infection : human->GetInfections())
      {
        if(infection->GetDuration() == 0.0f)
        {
          uID     = infection->GetStrain()->GetName();

          // Initialize maps if not present
          if(smap_clade.count(uID) == 0)
          {
            smap_clade[uID] = infection->GetStrain()->GetCladeID();
          }
          if(smap_genome.count(uID) == 0)
          {
            smap_genome[uID] = infection->GetStrain()->GetGeneticID();
          }
          if(smap_new_inf.count(uID) == 0)
          {
            smap_new_inf[uID] = 0.0f;
          }
          if(smap_data.count(uID) == 0)
          {
            smap_data[uID] = std::vector<float> {0.0f, 0.0f, 0.0f};
          }

          // Record data
          smap_new_inf[uID] += human->GetMonteCarloWeight();
        }
      }
    }

    // Report summary vector for each strain in node
    for (const auto &uIDval : node->GetStrainData())
    {
      uID         = uIDval.first;

      GetOutputStream() << node->GetTime().time                << ","
                        << node->GetExternalID()               << ","
                        << smap_clade[uID]                     << ","
                        << smap_genome[uID]                    << ","
                        << smap_data[uID][INDEX_RST_TOT_INF]   << ","
                        << smap_data[uID][INDEX_RST_CON_INF]   << ","
                        << smap_data[uID][INDEX_RST_CONTAGION] << ","
                        << smap_new_inf[uID]                   << std::endl;
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
