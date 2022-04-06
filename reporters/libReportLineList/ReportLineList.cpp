//******************************************************************************
//
// Reporter for generating line-list style outputs
//
//******************************************************************************

#pragma once

#include "stdafx.h"

#include "ReportLineList.h"

#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "IdmDateTime.h"
#include "Exceptions.h"

#include "INodeContext.h"
#include "IIndividualHuman.h"


//******************************************************************************

#define REP_VERSION            ("1.00")
#define NUM_COLUMNS            ("6")
#define PER_100K               (0.00001f)
#define PER_YEAR               (0.00273973f)
#define DEFAULT_REP_NAME       ("ReportLineList.csv")
#define DEFAULT_REP_TYPE       ("NOT_SPECIFIED")
#define RTYPE_INFT             ("INFECTION")
#define RTYPE_MORT             ("MORTALITY")
#define DESC_TEXT_REPORT_NAME  ("Output file name.")
#define DESC_TEXT_REPORT_TYPE  ("Events to list: INFECTION of MORTALITY")
#define DESC_TEXT_TIME_START   ("No output prior to this timestep.")
#define DESC_TEXT_TIME_END     ("No output after this timestep.")
#define DESC_TEXT_NI_RATE      ("Rate of non-infected per 100k population.")

//******************************************************************************

SETUP_LOGGING("ReportLineList")
static const char* _sim_types[]            = { "*", nullptr };

Kernel::report_instantiator_function_t rif = 
             [](){return (Kernel::IReport*)(new Kernel::ReportLineList());};

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
  ReportLineList::ReportLineList()
    : BaseTextReport(DEFAULT_REP_NAME, false)
    , m_all_done(false)
    , m_time_start(0.0f)
    , m_time_end(FLT_MAX)
    , m_rep_type()
    , m_ni_rate(0.0f)
  {
    // Nothing
  }


  // Destructor
  ReportLineList::~ReportLineList()
  {
    // Nothing
  }


  // Retrieves values from reporter-specific config file;
  bool ReportLineList::Configure(const Configuration* inputJson)
  {
    // Turn off header newline
    BaseTextReport::AddHeaderNewline(true);

    // Configure optional parameters
    JsonConfigurable::_useDefaults = true;
    std::string fName;
    initConfigTypeMap("Report_Name",       &fName,        DESC_TEXT_REPORT_NAME, DEFAULT_REP_NAME);
    initConfigTypeMap("Report_Type",       &m_rep_type,   DESC_TEXT_REPORT_TYPE, DEFAULT_REP_TYPE);
    initConfigTypeMap("Time_Start",        &m_time_start, DESC_TEXT_TIME_START,  0.0f, FLT_MAX, 0.0f);
    initConfigTypeMap("Time_End",          &m_time_end,   DESC_TEXT_TIME_END,    0.0f, FLT_MAX, FLT_MAX);
    initConfigTypeMap("Non_Infected_Rate", &m_ni_rate,    DESC_TEXT_NI_RATE,     0.0f, FLT_MAX, 0.0f,    "Report_Type", RTYPE_INFT);
    bool retVal = JsonConfigurable::Configure( inputJson );

    // Update report name
    BaseTextReport::SetReportName(fName);

    // Validate reporter type
    if( (m_rep_type != RTYPE_INFT) && (m_rep_type != RTYPE_MORT) )
    {
        std::ostringstream msg;
        msg << m_rep_type << " is not a supported option.";
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
    }

    return retVal;
  }


  // Provides header line; called by BaseTextReport
  std::string ReportLineList::GetHeader() const
  {
      return "TIME,NODE,AGE,INFECTED,MCWEIGHT,MODACQUIRE";
  }


  // Indicator for LogIndividualData();
  bool ReportLineList::IsCollectingIndividualData(float cTime, float dt) const
  {
    return true;
  }


  // Evaluates for each agent;
  void ReportLineList::LogIndividualData(IIndividualHuman* individual)
  {
    if(m_all_done || individual->GetParent()->GetTime().time < m_time_start)
    {
      return;
    }

    // Line-list infection output
    if(m_rep_type == RTYPE_INFT)
    {
      // Report new infections
      if(individual->GetNewInfectionState() == NewInfectionState::NewInfection   ||
         individual->GetNewInfectionState() == NewInfectionState::NewAndDetected   )
      {
        OutputLine(individual);
      }
    }

    // Line-list mortality output
    else if(m_rep_type == RTYPE_MORT)
    {
      // Report new death
      if(individual->GetStateChange() == HumanStateChange::DiedFromNaturalCauses ||
         individual->GetStateChange() == HumanStateChange::KilledByInfection       )
      {
        OutputLine(individual);
      }
    }

    // Should never get here
    else
    {
        //release_assert(false)
    }

    return;
  }


  // Evaluates for each node
  void ReportLineList::LogNodeData(INodeContext* node)
  {
    if(m_all_done || node->GetTime().time < m_time_start)
    {
      return;
    }

    // Report some non-infections
    std::vector<IIndividualHuman*> human_list = node->GetHumans();
    int num_discard = node->GetRng()->Poisson(m_ni_rate*human_list.size()*PER_100K*
                                              node->GetTime().GetTimeDelta()*PER_YEAR);

    for(int k1 = 0; k1 < num_discard; k1++)
    {
      IIndividualHuman* human = human_list[node->GetRng()->uniformZeroToN32(human_list.size())];
      if(!human->IsInfected())
      {
        OutputLine(human);
      }
    }

    return;
  }

  void ReportLineList::OutputLine(const IIndividualHuman* agent)
  {
    GetOutputStream() << agent->GetParent()->GetTime().time            << ","
                      << agent->GetParent()->GetExternalID()           << ","
                      << agent->GetAge()                               << ","
                      << agent->IsInfected()                           << ","
                      << agent->GetMonteCarloWeight()                  << ","
                      << agent->GetImmunityReducedAcquire()*
                         agent->GetInterventionReducedAcquire()        << std::endl;
  }

  // End of timestep outputs
  void ReportLineList::EndTimestep(float currentTime, float dt)
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