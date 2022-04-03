/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "Report_TBHIV_Basic.h"
#include "DllInterfaceHelper.h"

#include "TBContexts.h"
#include "IndividualCoInfection.h"
#include "TBInterventionsContainer.h"
#include "MasterInterventionsContainer.h"
#include "Drugs.h"
#include "AntiTBDrug.h"
#include "InfectionTB.h"

#include "INodeContext.h"
#include "NodeEventContext.h"
#include "IIndividualHuman.h"
#include "IIndividualHumanHIV.h"
#include "IHIVInterventionsContainer.h"
#include "SusceptibilityHIV.h"
#include "SusceptibilityTB.h"
#include "FactorySupport.h"
#include "IdmDateTime.h"

// TODO: 
// --> Start_Year
// --> Every 6 months
// --> Strings for CD4 stage and care stage
// --> Functions computing cd4_stage and care_stage
// --> Function for counter reset

// BASE_YEAR is temporary until Year() is fixed!
#define BASE_YEAR (0)
#define FIFTEEN_YEARS (15.0f * DAYSPERYEAR)
#define SIX_MONTHS (0.5f * DAYSPERYEAR)

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Name for logging, CustomReport.json, and DLL GetType()
SETUP_LOGGING( "Report_TBHIV_Basic" ) // <<< Name of this file

namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = { "TBHIV_SIM", nullptr };// <<< Types of simulation the report is to be used with

report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new Report_TBHIV_Basic()); // <<< Report to create
};

DllInterfaceHelper DLL_HELPER( _module, _sim_types, rif );

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// ------------------------------
// --- DLL Interface Methods
// ---
// --- The DTK will use these methods to establish communication with the DLL.
// ------------------------------

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

DTK_DLLEXPORT char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char * __cdecl
GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT void __cdecl
GetReportInstantiator( Kernel::report_instantiator_function_t* pif )
{
    DLL_HELPER.GetReportInstantiator( pif );
}

#ifdef __cplusplus
}
#endif

// ----------------------------------------
// --- Report_HIV_WHO2015 Methods
// ----------------------------------------

    Report_TBHIV_Basic::Report_TBHIV_Basic()
        : BaseTextReportEvents( "Report_TBHIV_Basic_Adult.csv" )
        , report_tbhiv_half_period( DAYSPERYEAR / 2.0f )
        , next_report_time(report_tbhiv_half_period)
        , doReport( false )
        , startYear(0.0)
        , stopYear(FLT_MAX)
        , is_collecting_data(false)
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();

        // Add events to event trigger list

        // Call a reset function
        ZERO_ARRAY( Population );
        NonDiseaseDeaths = 0;
        ZERO_ARRAY( DiseaseDeaths );
        ZERO_ARRAY( ART_Initiations );
        ZERO_ARRAY(New_TBDiagnoses);
        ZERO_ARRAY(New_Activations);
        ZERO_ARRAY(ARTDropouts);
    }

    Report_TBHIV_Basic::~Report_TBHIV_Basic()
    {
    }

    bool Report_TBHIV_Basic::Configure( const Configuration * inputJson )
    {
        initConfigTypeMap( "Start_Year", &startYear, "Year to start collecting data ", BASE_YEAR, MAX_YEAR, BASE_YEAR );
        initConfigTypeMap( "Stop_Year",  &stopYear,  "Year to stop collecting data",  BASE_YEAR, MAX_YEAR, BASE_YEAR );

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret ) {
            if( startYear < BASE_YEAR ) //IdmDateTime::_base_year
            {
                startYear = BASE_YEAR;  //IdmDateTime::_base_year ;
            }
            if( startYear >= stopYear )
            {
                 throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Start_Year", startYear, "Stop_Year", stopYear );
            }
        }

        // Manually push required events into the eventTriggerList
        eventTriggerList.push_back( EventTrigger::TBTestPositive    );
        eventTriggerList.push_back( EventTrigger::StartedART            );
        eventTriggerList.push_back( EventTrigger::StoppedART            );
        //eventTriggerList.push_back( EventTrigger("HCTUptakePostDebut9") );
        eventTriggerList.push_back( EventTrigger::TBActivation     );
        eventTriggerList.push_back( EventTrigger::DiseaseDeaths         );
        eventTriggerList.push_back( EventTrigger::NonDiseaseDeaths      );
        
        return ret;
    }

    void Report_TBHIV_Basic::Initialize( unsigned int nrmSize )
    {
        BaseTextReportEvents::Initialize( nrmSize );

        // has to be done if Initialize() since it is called after the demographics is read
       // IndividualProperty* p_ip = IPFactory::GetInstance()->GetIP( "InterventionStatus", "", false );
       // if( p_ip != nullptr )
      //  {
         //   m_InterventionStatusKey = p_ip->GetKey<IPKey>();
       // }
    }

    void Report_TBHIV_Basic::UpdateEventRegistration(  float currentTime,
                                                       float dt, 
                                                       std::vector<INodeEventContext*>& rNodeEventContextList,
                                                      ISimulationEventContext* pSimEventContext )
    {
        // not enforcing simulation to be not null in constructor so one can create schema with it null

        release_assert( !rNodeEventContextList.empty() );

        // BASE_YEAR is TEMPORARY HERE!!!
        float current_year = BASE_YEAR + rNodeEventContextList.front()->GetTime().Year();

        if( !is_collecting_data && (startYear <= current_year) && (current_year < stopYear) )
        {
            BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList, pSimEventContext );
            is_collecting_data = true ;

            // ------------------------------------------------------------------------
            // --- The idea here is to ensure that as we increase the startYear from the 
            // --- base_year we get the same report times as when startYear = base_year
            // --- The difference between start and base year gives us the number of days
            // --- into the simulation that we think we should be.  It ignores issues with
            // --- the size of dt not ending exactly on integer years.  We subtract the
            // --- dt/2 to deal with rounding errors.  For example, if the half period was 182.5,
            // --- start_year == _base_year, and dt = 30, then next_report_time = 167.5 and
            // --- data would be collected at 180.  However, for the next update
            // --- next_report_time would be 350 and the update would occur at 360.
            // ------------------------------------------------------------------------
            next_report_time = DAYSPERYEAR*(startYear - BASE_YEAR) + report_tbhiv_half_period - dt / 2.0f ;
        }
        else if( is_collecting_data && (current_year >= stopYear) )
        {
            //UnregisterAllBroadcasters();
            is_collecting_data = false ;
        }

        if( is_collecting_data )
        {
            // Figure out when to set doReport to true.  doReport is true for those
            // timesteps where we take a snapshot, i.e., as a function of the
            // half-year offset and full-year periodicity.
            doReport = false;

            if( currentTime >= next_report_time ) 
            {
                next_report_time += report_tbhiv_half_period;

                LOG_DEBUG_F( "Setting doReport to true .\n" );
                doReport = true;
            }
        }
    }

    CD4_Stage::Enum Report_TBHIV_Basic::ComputeCD4Stage(IIndividualHumanEventContext *context)
    {
        CD4_Stage::Enum cd4_stage = CD4_Stage::HIV_NEGATIVE;

        IIndividualHumanHIV* hiv_individual = NULL;
        if( context->QueryInterface( GET_IID( IIndividualHumanHIV ), (void**)&hiv_individual ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman" );
        }


        // CD4 Stage
        if( hiv_individual->HasHIV() ) {
            float cd4 = hiv_individual->GetHIVSusceptibility()->GetCD4count();
            if( cd4 < 200 )
                cd4_stage = CD4_Stage::CD4_UNDER_200;
            else if( cd4 < 350 )
                cd4_stage = CD4_Stage::CD4_200_TO_350;
            else if( cd4 < 500 )
                cd4_stage = CD4_Stage::CD4_350_TO_500;
            else
                cd4_stage = CD4_Stage::CD4_ABOVE_500;
        }

        return cd4_stage;
    }

    ARTStatusLocal::Enum Report_TBHIV_Basic::ComputeARTStatus(IIndividualHumanEventContext* context)
    {
        ARTStatusLocal::Enum art_status = ARTStatusLocal::NA;

        IIndividualHumanHIV* hiv_individual = NULL;
        if (context->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&hiv_individual) != s_OK)
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman");
        }

        if (hiv_individual->HasHIV())
        {

            if (hiv_individual->GetHIVInterventionsContainer()->OnArtQuery())
            {
                art_status = ARTStatusLocal::ONART;
            }
            else
            {
                art_status = ARTStatusLocal::OFFART;
            }

        }

        return art_status;
    }

    TB_State::Enum Report_TBHIV_Basic::ComputeTBState(IIndividualHumanEventContext *context)
    {
        TB_State::Enum tb_state = TB_State::Negative;

        IIndividualHumanHIV* hiv_individual = NULL;
        if (context->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&hiv_individual) != s_OK)
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman");
        }

        IIndividualHumanCoInfection* coinf_individual = NULL;
        if (context->QueryInterface(GET_IID(IIndividualHumanCoInfection), (void**)&coinf_individual) != s_OK)
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman");
        }

        IIndividualHumanTB* tb_individual = NULL;
        if (context->QueryInterface(GET_IID(IIndividualHumanTB), (void**)&tb_individual) != s_OK)
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman");
        }

        if (!coinf_individual->HasTB())
            return tb_state;     // Care_Stage::Enum::NA

        if (tb_individual->HasLatentInfection())
        {
            tb_state = TB_State::Latent;
        }

        if (tb_individual->HasActiveInfection())
        {
            if (tb_individual->HasActivePresymptomaticInfection())
            {
                tb_state = TB_State::ActivePreSymptomatic;
            }
            if (tb_individual->IsExtrapulmonary())
            {
                tb_state = TB_State::ActiveExtraPulm;
            }
            else if (tb_individual->IsSmearPositive())
            {
                tb_state = TB_State::ActiveSmearPos;
            }
            else
            {
                tb_state = TB_State::ActiveSmearNeg;
            }

        }
        return tb_state;
    }
      
    MDR_State::Enum Report_TBHIV_Basic::ComputeMDRState(IIndividualHumanEventContext *context)
    {
        MDR_State::Enum mdr_state = MDR_State::NA;


        IIndividualHumanCoInfection* coinf_individual = NULL;
        if (context->QueryInterface(GET_IID(IIndividualHumanCoInfection), (void**)&coinf_individual) != s_OK)
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman");
        }

        IIndividualHumanTB* tb_individual = NULL;
        if (context->QueryInterface(GET_IID(IIndividualHumanTB), (void**)&tb_individual) != s_OK)
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman");
        }

        if (!coinf_individual->HasTB())
        {
            return mdr_state;     // mdr state NA
        }
        else if (tb_individual->IsMDR())
        {
            mdr_state = MDR_State::MDR;
        }
        else
        {
            mdr_state = MDR_State::Negative;
        }
        
         return mdr_state;
    
    }

    
    std::string Report_TBHIV_Basic::GetHeader() const
    {
        std::stringstream header ;
        header << "Year"             << ", "
               << "NodeID"           << ", "
               << "TBState"          << ", "
               << "ARTStatus"        << ", "
               << "CD4_Stage"        << ", "
               << "MDRState"         << ", "
               << "Population"       << ", "
               << "DiseaseDeaths"    << ", "
               << "NonDiseaseDeaths" << ", "
               << "New_Activations"  << ", "
               << "New_TBDiagnoses"  << ", "
               << "ART_Initiations"  << ", "
               << "ART_Dropouts";

        return header.str();
    }

    void Report_TBHIV_Basic::LogNodeData(Kernel::INodeContext* pNC)
    {
        if ((is_collecting_data == false) || (doReport == false))
        {
            return;
        }
        LOG_DEBUG_F("%s: doReport = %d\n", __FUNCTION__, doReport);

        // BASE_YEAR is TEMPORARY HERE!
        float year = BASE_YEAR + pNC->GetTime().Year();
        int nodeId = pNC->GetExternalID();

        for (int cd4stage_idx = 0; cd4stage_idx < CD4_Stage::Enum::COUNT; cd4stage_idx++)
        {
            for (int tbstate_idx = 0; tbstate_idx < TB_State::Enum::COUNT; tbstate_idx++)

            {
                for (int mdrstate_idx = 0; mdrstate_idx < MDR_State::Enum::COUNT; mdrstate_idx++)
                {
                    for (int artstatus_idx = 0; artstatus_idx < ARTStatusLocal::Enum::COUNT; artstatus_idx++)
                    {
                        GetOutputStream() << year
                            << "," << nodeId
                            << "," << TB_State::pairs::lookup_key(tbstate_idx)
                            << "," << ARTStatusLocal::pairs::lookup_key(artstatus_idx)
                            << "," << CD4_Stage::pairs::lookup_key(cd4stage_idx)
                            << "," << MDR_State::pairs::lookup_key(mdrstate_idx)
                            << "," << Population[tbstate_idx][cd4stage_idx][mdrstate_idx][artstatus_idx] 
                            << "," << DiseaseDeaths[tbstate_idx][cd4stage_idx][mdrstate_idx][artstatus_idx]
                            << "," << (artstatus_idx == (int)ARTStatusLocal::Enum::NA && cd4stage_idx == (int)CD4_Stage::Enum::NA && mdrstate_idx == (int)MDR_State::Enum::NA && tbstate_idx == (int)TB_State::Enum::NA ? NonDiseaseDeaths : 0.0f)
                            << "," << (tbstate_idx == (int)TB_State::Enum::NA ? New_Activations[cd4stage_idx][mdrstate_idx][artstatus_idx] : 0.0f)
                            << "," << (tbstate_idx == (int)TB_State::Enum::NA ? New_TBDiagnoses[cd4stage_idx][mdrstate_idx][artstatus_idx] : 0.0f)
                            << "," << (artstatus_idx == (int)ARTStatusLocal::Enum::NA && tbstate_idx == (int)TB_State::Enum::NA && mdrstate_idx == (int)MDR_State::Enum::NA ? ART_Initiations[cd4stage_idx] : 0.0f)
                            << "," << (artstatus_idx == (int)ARTStatusLocal::Enum::NA && tbstate_idx == (int)TB_State::Enum::NA && mdrstate_idx == (int)MDR_State::Enum::NA ? ARTDropouts[cd4stage_idx] : 0.0f)

                            << endl;
                    }
                }
            }
        }


        // Call a reset function
        ZERO_ARRAY( Population );
        ZERO_ARRAY( DiseaseDeaths );
        NonDiseaseDeaths = 0;
        ZERO_ARRAY(New_Activations);
        ZERO_ARRAY(New_TBDiagnoses);
        ZERO_ARRAY( ART_Initiations );
        ZERO_ARRAY(ARTDropouts);
      
    }

    bool Report_TBHIV_Basic::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return is_collecting_data && doReport; 
    }

    void Report_TBHIV_Basic::LogIndividualData( Kernel::IIndividualHuman* individual )
    {
        if( individual->GetAge() < FIFTEEN_YEARS )
            return;

        float mc_weight = individual->GetMonteCarloWeight();

        CD4_Stage::Enum cd4_stage = ComputeCD4Stage(individual->GetEventContext());
        TB_State::Enum tb_state = ComputeTBState(individual->GetEventContext());
        MDR_State::Enum mdr_state = ComputeMDRState(individual->GetEventContext());
        ARTStatusLocal::Enum art_status = ComputeARTStatus(individual->GetEventContext());

        // Now increment prevalence counters
        Population[tb_state][cd4_stage][mdr_state][art_status] += mc_weight;
    }

    bool Report_TBHIV_Basic::notifyOnEvent( IIndividualHumanEventContext *context, 
                                            const EventTrigger::Enum& trigger )
    {
        if( context->GetAge() < FIFTEEN_YEARS )
            return true;

        // iindividual context for suid
        IIndividualHumanContext * iindividual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanContext), (void**)&iindividual) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanContext", "IIndividualHumanEventContext");
        }

        float mc_weight = context->GetMonteCarloWeight();

        CD4_Stage::Enum cd4_stage = ComputeCD4Stage(context);
        TB_State::Enum tb_state = ComputeTBState(context);
        MDR_State::Enum mdr_state = ComputeMDRState(context);
        ARTStatusLocal::Enum art_status = ComputeARTStatus(context);

        if( trigger == EventTrigger::TBActivation )
        {
            New_Activations[cd4_stage][mdr_state][art_status] += mc_weight;
        }
        else if( trigger == EventTrigger::StartedART )
        {
            ART_Initiations[cd4_stage] += mc_weight;
        }
        else if( trigger == EventTrigger::StoppedART )
        {
            ARTDropouts[cd4_stage] += mc_weight;
        }
        else if( trigger == EventTrigger::TBTestPositive )
        {
            New_TBDiagnoses[cd4_stage][mdr_state][art_status] += mc_weight;
        }
        else if( trigger == EventTrigger::DiseaseDeaths )
        {
            DiseaseDeaths[tb_state][cd4_stage][mdr_state][art_status] += mc_weight;
        }
        else if( trigger == EventTrigger::NonDiseaseDeaths )
        {
            NonDiseaseDeaths += mc_weight;
        }

        return true;
    }

}
