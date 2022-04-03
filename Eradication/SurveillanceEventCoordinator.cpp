/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "SurveillanceEventCoordinator.h"
#include "SimulationEventContext.h"
#include "InterventionFactory.h"
#include "IncidenceCounterSurveillance.h"
#include "ReportStatsByIP.h"

SETUP_LOGGING("SurveillanceEventCoordinator")

namespace Kernel
{
    // ------------------------------------------------------------------------
    // --- ResponderSurveillance
    // ------------------------------------------------------------------------

    ResponderSurveillance::ResponderSurveillance()
        : Responder()
        , m_RespondedEvent()
        , m_PercentageEventsToCount()
    {
    }

    ResponderSurveillance::~ResponderSurveillance()
    {
    }

#define SEC_Percentage_Events_To_Count_DESC_TEXT "TBD"

    bool ResponderSurveillance::Configure( const Configuration * inputJson )
    {
        initConfig( "Responded_Event", m_RespondedEvent, inputJson, MetadataDescriptor::Enum("Responded_Event", SEC_Responded_Event_DESC_TEXT , MDD_ENUM_ARGS( EventTrigger ) ) );

        initConfigTypeMap( "Percentage_Events_To_Count",
                           &m_PercentageEventsToCount,
                           SEC_Percentage_Events_To_Count_DESC_TEXT,
                           nullptr,
                           JsonConfigurable::empty_set,
                           "Threshold_Type",
                           "PERCENTAGE_EVENTS" );

        bool ret = Responder::Configure( inputJson );
        return ret;
    }

    void ResponderSurveillance::CheckConfiguration( const Configuration * inputJson )
    {
        //empty, no checks needed
    }

    bool ResponderSurveillance::Distribute( const std::vector<INodeEventContext*>& nodes, uint32_t numIncidences, uint32_t qualifyingPopulation )
    {
        bool distributed = Responder::Distribute( nodes, numIncidences, qualifyingPopulation );
        if( distributed && m_RespondedEvent != EventTrigger::NoTrigger )
        {
            m_sim->GetCoordinatorEventBroadcaster()->TriggerObservers( m_Parent, m_RespondedEvent );
        }
        return distributed;
    }

    // ------------------------------------------------------------------------
    // --- SurveillanceEventCoordinator
    // ------------------------------------------------------------------------

    IMPLEMENT_FACTORY_REGISTERED(SurveillanceEventCoordinator)

    BEGIN_QUERY_INTERFACE_DERIVED( SurveillanceEventCoordinator, IncidenceEventCoordinator )
        HANDLE_INTERFACE( IEventCoordinatorEventContext )
        HANDLE_INTERFACE( ISurveillanceReporting )
    END_QUERY_INTERFACE_DERIVED( SurveillanceEventCoordinator, IncidenceEventCoordinator )

    SurveillanceEventCoordinator::SurveillanceEventCoordinator()
        : IncidenceEventCoordinator( new IncidenceCounterSurveillance(), new ResponderSurveillance() )
        , m_StopTriggerConditionList()
        , m_StartTriggerConditionList()
        , m_IsActive( false )
        , m_IsStarting( false )
        , m_IsStopping( false )
        , m_Duration( -1.0f )
        , m_DurationExpired ( false )
        , m_CoordinatorName("SurveillanceEventCoordinator")
    {
        initSimTypes( 1, "*" );
    }

    SurveillanceEventCoordinator::~SurveillanceEventCoordinator()
    {
    }


    bool SurveillanceEventCoordinator::Configure( const Configuration * inputJson )
    {
        initVectorConfig( "Start_Trigger_Condition_List",
                          m_StartTriggerConditionList,
                          inputJson,
                          MetadataDescriptor::Enum(
                            "Start_Trigger_Condition_List",
                            SEC_Start_Trigger_Condition_List_DESC_TEXT ,
                            MDD_ENUM_ARGS(EventTrigger) ) );
        initVectorConfig( "Stop_Trigger_Condition_List",
                          m_StopTriggerConditionList,
                          inputJson,
                          MetadataDescriptor::Enum(
                            "Stop_Trigger_Condition_List",
                            SEC_Stop_Trigger_Condition_List_DESC_TEXT ,
                            MDD_ENUM_ARGS(EventTrigger)));
        initConfigTypeMap( "Duration", &m_Duration, SEC_Duration_DESC_TEXT, -1.0f, FLT_MAX, -1.0f );
        initConfigTypeMap( "Coordinator_Name", &m_CoordinatorName, SEC_Coordinator_Name_DESC_TEXT, "SurveillanceEventCoordinator" );

        bool retValue = IncidenceEventCoordinator::Configure( inputJson );

        if( retValue && !JsonConfigurable::_dryrun )
        {
            CheckConfigurationTriggers();
            IncidenceCounterSurveillance* p_ics = static_cast<IncidenceCounterSurveillance*>(m_pIncidenceCounter);
            ResponderSurveillance* p_rs = static_cast<ResponderSurveillance*>(m_pResponder);
            p_ics->SetPercentageEventsToCount( p_rs->GetPercentageEventsToCount() );
        }

        return retValue;
    }

    void SurveillanceEventCoordinator::CheckConfigurationTriggers()
    {
        //check if the same trigger is defined as start and stop condition
        for( EventTriggerCoordinator& etc : m_StopTriggerConditionList )
        {
            auto found = find( m_StartTriggerConditionList.begin(), m_StartTriggerConditionList.end(), etc );
            if( found != m_StartTriggerConditionList.end() )
            {
                std::ostringstream msg;
                msg << "In Coordinator \'" << m_CoordinatorName << "\' stop trigger \'" << EventTrigger::pairs::lookup_key( etc ) << "\' is already defined in Start_Trigger_Condition_List.";
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
        }
    }

    void SurveillanceEventCoordinator::SetContextTo( ISimulationEventContext *isec )
    {
        IncidenceEventCoordinator::SetContextTo( isec );
        m_pResponder->SetContextTo( m_Parent, this );

        Register();
    }

    void SurveillanceEventCoordinator::Update( float dt )
    {        
        /*
        Methods are called in the following order:
        1) Update()         - Set state of object based on notifications from previous time step
        2) notifyOnEvent()  - check for new events
        3) UpdateNodes()    - execute actions like e.g. responing
        */
        if( m_Duration != -1.0f )
        {
            if( m_Duration <= 0  )
            {
                m_DurationExpired = true;
            }
            m_Duration -= dt;
        }
        if( m_IsStarting )
        {
            m_IsStarting = false;
            m_IsActive = true;
            m_pIncidenceCounter->StartCounting();
        }
        if( m_IsStopping )
        {
            m_IsStopping = false;
            m_IsActive = false;
        }

        if( !m_IsActive ) return;
        m_pIncidenceCounter->Update( dt );
    }

    void SurveillanceEventCoordinator::Register()
    {
        //Register Start and Stop events
        for( EventTriggerCoordinator& etc : m_StartTriggerConditionList)
        {
            m_Parent->GetCoordinatorEventBroadcaster()->RegisterObserver( this, etc );
            LOG_INFO_F( "%s: registered Start_Trigger: %s\n", m_CoordinatorName.c_str(), EventTrigger::pairs::lookup_key( etc ) );
        }
        for( EventTriggerCoordinator& etc : m_StopTriggerConditionList)
        {
            m_Parent->GetCoordinatorEventBroadcaster()->RegisterObserver( this, etc );
            LOG_INFO_F( "%s: registered Stop_Trigger: %s\n", m_CoordinatorName.c_str(), EventTrigger::pairs::lookup_key( etc ) );
        }
    }

    bool SurveillanceEventCoordinator::notifyOnEvent( IEventCoordinatorEventContext *pEntity, const EventTriggerCoordinator& trigger )
    {
        auto it_start = find( m_StartTriggerConditionList.begin(), m_StartTriggerConditionList.end(), trigger );
        if ( it_start != m_StartTriggerConditionList.end() )
        {
            LOG_INFO_F( "%s: notifyOnEvent received Start: %s\n", m_CoordinatorName.c_str(), EventTrigger::pairs::lookup_key( trigger ) );
            m_IsStarting = true;           
        }
        else
        {
            LOG_INFO_F( "%s: notifyOnEvent received Stop: %s\n", m_CoordinatorName.c_str(), EventTrigger::pairs::lookup_key( trigger ) );
            m_IsStopping = true;
        }
        return true;
    }

    void SurveillanceEventCoordinator::Unregister()
    {
        for ( EventTriggerCoordinator& etc : m_StartTriggerConditionList )
        {
            m_Parent->GetCoordinatorEventBroadcaster()->UnregisterObserver( this, etc );
            LOG_INFO_F( "%s: Unregistered Start_Trigger: %s\n", m_CoordinatorName.c_str(), EventTrigger::pairs::lookup_key( etc ) );
        }
        for ( EventTriggerCoordinator& etc : m_StopTriggerConditionList )
        {
            m_Parent->GetCoordinatorEventBroadcaster()->UnregisterObserver( this, etc );
            LOG_INFO_F( "%s: Unregistered Stop_Trigger: %s\n", m_CoordinatorName.c_str(), EventTrigger::pairs::lookup_key( etc ) );
        }
    }

    bool SurveillanceEventCoordinator::IsFinished()
    {
        if( m_DurationExpired )
        {
            Unregister();
            m_IsExpired = true;
            IncidenceEventCoordinator::IsFinished();
        }
        return m_DurationExpired;
    }

    void SurveillanceEventCoordinator::ConsiderResponding()
    {
        if (m_pIncidenceCounter->IsDoneCounting())
        {
            uint32_t num_incidences = m_pIncidenceCounter->GetCount();
            uint32_t num_qualifying_pop = m_pIncidenceCounter->GetCountOfQualifyingPopulation( m_CachedNodes );
            m_pResponder->Distribute( m_CachedNodes, num_incidences, num_qualifying_pop );
            m_pIncidenceCounter->StartCounting();
        }
    }

    void SurveillanceEventCoordinator::UpdateNodes(float dt)
    {
        if( !m_IsActive ) return; 
        ConsiderResponding();
    }

    const std::string& SurveillanceEventCoordinator::GetName() const
    {
        return m_CoordinatorName;
    }

    const IdmDateTime& SurveillanceEventCoordinator::GetTime() const
    {
        return m_Parent->GetSimulationTime();
    }

    void SurveillanceEventCoordinator::CollectStats( ReportStatsByIP& rStats )
    {
        for( auto p_nec : m_CachedNodes )
        {
            if( m_pIncidenceCounter->IsNodeQualified( p_nec ) )
            {
                rStats.CollectDataFromNode( p_nec,
                                            m_pIncidenceCounter->GetIndividualQualifiedFunction() );
            }
        }
    }

    uint32_t SurveillanceEventCoordinator::GetNumCounted() const
    {
        return m_pIncidenceCounter->GetCount();
    }

    float SurveillanceEventCoordinator::GetCurrentActionThreshold() const
    {
        return m_pResponder->GetCurrentAction()->GetThreshold();
    }
}
