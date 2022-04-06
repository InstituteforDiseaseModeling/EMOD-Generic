/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportHumanMigrationTracking.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "ISimulationContext.h"
#include "NodeEventContext.h"
#include "Individual.h"
#include "IdmDateTime.h"
#include "INodeContext.h"

//******************************************************************************

#define ADULT_AGE_YRS (15.0f)

//******************************************************************************

SETUP_LOGGING( "ReportHumanMigrationTracking" )

static const char* _sim_types[] = { "*", nullptr };

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
__cdecl GetReportInstantiator()
{
    return new Kernel::ReportHumanMigrationTracking();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ReportHumanMigrationTracking Methods
// ----------------------------------------

namespace Kernel
{

    ReportHumanMigrationTracking::ReportHumanMigrationTracking()
        : BaseTextReportEvents( "ReportHumanMigrationTracking.csv" )
        , m_EndTime(0.0)
        , m_MigrationDataMap()
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();
    }

    ReportHumanMigrationTracking::~ReportHumanMigrationTracking()
    {
    }

    bool ReportHumanMigrationTracking::Configure( const Configuration * inputJson )
    {

        bool ret = BaseTextReportEvents::Configure( inputJson );

        // Manually push required events into the eventTriggerList
        eventTriggerList.push_back( EventTrigger::Emigrating       );
        eventTriggerList.push_back( EventTrigger::Immigrating      );
        eventTriggerList.push_back( EventTrigger::NonDiseaseDeaths );
        eventTriggerList.push_back( EventTrigger::DiseaseDeaths    );
        
        return ret;
    }

    void ReportHumanMigrationTracking::UpdateEventRegistration( float currentTime, 
                                                                float dt, 
                                                                std::vector<INodeEventContext*>& rNodeEventContextList,
                                                                ISimulationEventContext* pSimEventContext )
    {
        BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList, pSimEventContext );
    }


    std::string ReportHumanMigrationTracking::GetHeader() const
    {
        std::stringstream header ;
        header << "Time"          << ", "
               << "IndividualID"  << ", "
               << "AgeYears"      << ", "
               << "Gender"        << ", "
               << "IsAdult"       << ", "
               << "Home_NodeID"   << ", "
               << "From_NodeID"   << ", "
               << "To_NodeID"     << ", "
               << "MigrationType" << ", "
               << "Event";

        return header.str();
    }

    bool ReportHumanMigrationTracking::notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger )
    {
        IIndividualHuman* p_ih = nullptr;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHuman), (void**)&p_ih) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHuman", "IIndividualHumanEventContext");
        }
        IMigrate * im = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IMigrate), (void**)&im) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IMigrate", "IIndividualHumanEventContext");
        }

        ISimulationContext* p_sim = context->GetNodeEventContext()->GetNodeContext()->GetParent();

        float        time           = context->GetNodeEventContext()->GetTime().time;
        long         individual_id  = context->GetSuid().data;
        float        age_years      = p_ih->GetAge() / DAYSPERYEAR;
        char         is_adult       = (age_years >= ADULT_AGE_YRS) ? 'T' : 'F';
        char         gender         = (p_ih->GetGender() == 0) ? 'M' : 'F';
        uint32_t     home_node_id   = p_sim->GetNodeExternalID( p_ih->GetHomeNodeId() );
        uint32_t     from_node_id   = p_sim->GetNodeExternalID( context->GetNodeEventContext()->GetId() );
        uint32_t     to_node_id     = from_node_id;
        std::string  mig_type_str   = "local";
        bool         is_emigrating  = (trigger == EventTrigger::Emigrating);
        bool         is_immigrating = (trigger == EventTrigger::Immigrating);
        bool         is_dead        = p_ih->IsDead();

        if( is_immigrating && !is_dead)
        {
            if( p_ih->AtHome() )
            {
                if( m_MigrationDataMap.count( individual_id ) == 0 )
                {
                    std::ostringstream msg ;
                    msg << "Individual=" << individual_id << " is not in the map yet." ;
                    throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str());
                }
                m_MigrationDataMap[ individual_id ].node_id = to_node_id ;
            }
        }
        else if( is_emigrating  && !is_dead)
        {
            to_node_id =  p_sim->GetNodeExternalID( im->GetMigrationDestination() ) ;

            int mig_type = im->GetMigrationType();
            if( p_ih->IsOnFamilyTrip() )
                mig_type_str = "family" ;
            else if( mig_type == MigrationType::LOCAL_MIGRATION )
                mig_type_str = "local" ;
            else if( mig_type == MigrationType::AIR_MIGRATION )
                mig_type_str = "air" ;
            else if( mig_type == MigrationType::REGIONAL_MIGRATION )
                mig_type_str = "regional" ;
            else if( mig_type == MigrationType::SEA_MIGRATION )
                mig_type_str = "sea" ;
            else if( mig_type == MigrationType::INTERVENTION_MIGRATION )
                mig_type_str = "intervention" ;
            else
                release_assert( false );

            // only keep track if the person is on the core that their home node is
            if( p_sim->GetNodeRank( p_ih->GetHomeNodeId() ) == EnvPtr->MPI.Rank )
            {
                if( m_MigrationDataMap.count( individual_id ) == 0 )
                {
                    std::ostringstream msg ;
                    msg << "Individual=" << individual_id << " is not in the map yet." ;
                    throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str());
                }
                m_MigrationDataMap[ individual_id ].age_years          = age_years ;
                m_MigrationDataMap[ individual_id ].is_adult           = (age_years >= ADULT_AGE_YRS) ;
                m_MigrationDataMap[ individual_id ].node_id            = to_node_id ;
                m_MigrationDataMap[ individual_id ].migration_type_str = mig_type_str ;
            }
        }
        else if(is_dead)
        {
            m_MigrationDataMap.erase( individual_id );
        }
        else
        {
            release_assert( false );
        }

        if( is_emigrating && !is_dead )
        {
            GetOutputStream() << time
                       << "," << individual_id 
                       << "," << age_years 
                       << "," << gender 
                       << "," << is_adult 
                       << "," << home_node_id 
                       << "," << from_node_id 
                       << "," << to_node_id 
                       << "," << mig_type_str 
                       << "," << EventTrigger::pairs::lookup_key( trigger )
                       << endl;
        }
        return true;
    }

    void ReportHumanMigrationTracking::LogIndividualData( IIndividualHuman* individual ) 
    {
        long individual_id = individual->GetSuid().data ;

        ISimulationContext* p_sim = individual->GetEventContext()->GetNodeEventContext()->GetNodeContext()->GetParent();

        MigrationData md ;
        md.age_years          = individual->GetAge() / DAYSPERYEAR ;
        md.gender             = individual->GetGender();
        md.is_adult           = (md.age_years >= ADULT_AGE_YRS);
        md.home_node_id       = p_sim->GetNodeExternalID( individual->GetHomeNodeId() ) ;
        md.node_id            = p_sim->GetNodeExternalID( individual->GetEventContext()->GetNodeEventContext()->GetId() ) ;

        if( m_MigrationDataMap.count( individual_id ) == 0 )
        {
            m_MigrationDataMap.insert( std::make_pair( individual_id, md ) );
        }
        else
        {
            m_MigrationDataMap[ individual_id ].age_years = md.age_years ;
            m_MigrationDataMap[ individual_id ].is_adult  = md.is_adult ;
            m_MigrationDataMap[ individual_id ].node_id   = md.node_id ;
        }
    }

    bool ReportHumanMigrationTracking::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return true ;
    }

    void ReportHumanMigrationTracking::EndTimestep( float currentTime, float dt )
    {
        m_EndTime = currentTime ;
        BaseTextReportEvents::EndTimestep( currentTime, dt );
    }

    void ReportHumanMigrationTracking::Reduce()
    {
        for( auto entry : m_MigrationDataMap )
        {
            std::string mig_type_str = entry.second.migration_type_str ;
            if( entry.second.node_id == entry.second.home_node_id )
            {
                mig_type_str = "home" ;
            }

            GetOutputStream() << m_EndTime
                       << "," << entry.first 
                       << "," << entry.second.age_years 
                       << "," << ((entry.second.gender == 0) ? 'M' : 'F') 
                       << "," << (entry.second.is_adult ? 'T' : 'F')
                       << "," << entry.second.home_node_id 
                       << "," << entry.second.node_id 
                       << "," << entry.second.node_id 
                       << "," << mig_type_str
                       << "," << "SimulationEnd"
                       << endl;
        }
        BaseTextReportEvents::EndTimestep( m_EndTime, 1.0 );
    }
}
