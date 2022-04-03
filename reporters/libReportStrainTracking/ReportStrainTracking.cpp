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
#include "IdmMpi.h"
#include "INodeContext.h"
#include "IIndividualHuman.h"

//******************************************************************************

#define REP_VERSION            ("1.01")
#define NUM_COLUMNS            ("8")
#define DEFAULT_REP_NAME       ("ReportStrainTracking.json")
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
    , m_num_rows(0.0f)
    , m_num_rows_tot(0)
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
    // Turn off header newline
    BaseTextReport::AddHeaderNewline(false);

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
    return "";
  }


  // Evaluates for each node
  void ReportStrainTracking::LogNodeData(INodeContext* node)
  {
    if(m_all_done || node->GetTime().time < m_time_start)
    {
      return;
    }

    // Create data map for new infections
    std::string                  uID;
    int                          clade, genome;
    float                        mc_weight;
    std::map<std::string, float> smap_new_inf;

    std::map<std::string, int>                 smap_clade;
    std::map<std::string, int>                 smap_genome;
    std::map<std::string, std::vector<float>>  smap_data;
    
    for (auto human : node->GetHumans())
    {
      mc_weight  = human->GetMonteCarloWeight();
      for (auto infection : human->GetInfections())
      {
        // New infections have a total duration of zero at end-timestep
        if(infection->GetDuration() == 0.0f)
        {
          clade   = infection->GetStrain()->GetCladeID();
          genome  = infection->GetStrain()->GetGeneticID();
          for (const auto &uIDval : node->GetStrainData())
          {
            uID         = uIDval.first;
            smap_clade  = node->GetStrainClades();
            smap_genome = node->GetStrainGenomes();
            if(smap_clade[uID] == clade && smap_genome[uID] == genome)
             {
                 break;
             }
          }
          if(smap_new_inf.count(uID) == 0)
          {
            smap_new_inf[uID] = 0.0f;
          }
          smap_new_inf[uID] += mc_weight;;
        }
      }
    }

    // Report summary vector for each strain in node
    std::stringstream local_buffer;
    local_buffer.precision(6);
    for (const auto &uIDval : node->GetStrainData())
    {
      uID         = uIDval.first;
      smap_clade  = node->GetStrainClades();
      smap_genome = node->GetStrainGenomes();
      smap_data   = node->GetStrainData();
      local_buffer << "{";
      local_buffer << "\"time\":"      << node->GetTime().time                << ",";
      local_buffer << "\"node\":"      << node->GetExternalID()               << ",";
      local_buffer << "\"clade\":"     << smap_clade[uID]                     << ",";
      local_buffer << "\"genome\":"    << smap_genome[uID]                    << ",";
      local_buffer << "\"tot_inf\":"   << smap_data[uID][INDEX_RST_TOT_INF]   << ",";
      local_buffer << "\"con_inf\":"   << smap_data[uID][INDEX_RST_CON_INF]   << ",";
      local_buffer << "\"contagion\":" << smap_data[uID][INDEX_RST_CONTAGION] << ",";
      local_buffer << "\"new_inf\":"   << smap_new_inf[uID]                   << "},";
    }

    // If data, remove trailing comma and add to vector of strings
    m_num_rows += static_cast<float>(smap_data.size());
    if(smap_data.size() > 0)
    {
      std::string tmp_val = local_buffer.str();
      tmp_val.pop_back();
      m_nodedict_vec.push_back(tmp_val);
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

    if(currentTime-dt > m_time_end)
    {
      m_all_done = true;
      return;
    }

    // Collect all node reports from processor and create local summary
    std::stringstream local_buffer;
    for(int k1 = 0; k1 < static_cast<int>(m_nodedict_vec.size()); k1++)
    {
      local_buffer << m_nodedict_vec[k1] << ",";
    }

    // Unify all summaries into timestep report
    float       unified_rows;
    std::string unified_string;
    EnvPtr->MPI.p_idm_mpi->Reduce(&m_num_rows,&unified_rows,1);
    EnvPtr->MPI.p_idm_mpi->GatherToRoot(local_buffer.str(), unified_string);

    m_num_rows = 0.0f;
    m_nodedict_vec.clear();

    // If data, remove trailing comma and add to vector of strings
    if(unified_string.length() > 0)
    {
      unified_string.pop_back();
      if(EnvPtr->MPI.Rank == 0)
      {
        m_num_rows_tot += static_cast<int>(unified_rows);
        m_timestep_vec.push_back(unified_string);
      }
    }

    return;
  }


  // End of simulation
  void ReportStrainTracking::Finalize()
  {
    // Write summary vectors for each timestep
    if( EnvPtr->MPI.Rank == 0)
    {
      std::stringstream local_buffer;
      local_buffer << "{\"header\":{"
                   << "\"report_type\": "    << "\"strain_tracking\"" << ","
                   << "\"report_version\": " << REP_VERSION           << ","
                   << "\"number_columns\": " << NUM_COLUMNS           << ","
                   << "\"number_rows\": "    << m_num_rows_tot        << "},";

      local_buffer << "\"data\":[";
      for(int k1 = 0; k1 < static_cast<int>(m_timestep_vec.size())-1; k1++)
      {
         local_buffer << m_timestep_vec[k1] << ",";
      }
      if(m_timestep_vec.size() > 0)
      {
        local_buffer << m_timestep_vec.back();
      }
      local_buffer << "]}";
      WriteData(local_buffer.str());
      m_timestep_vec.clear();
    }

    // Call to parent finalize; closes file
    BaseTextReport::Finalize();

    return;
  }
}

//******************************************************************************
