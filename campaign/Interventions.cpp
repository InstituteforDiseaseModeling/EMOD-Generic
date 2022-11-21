/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <stdafx.h>
#ifndef WIN32
#include <cxxabi.h>
#endif

#include "Interventions.h"
#include "InterventionFactory.h"
#include "INodeContext.h"
#include "NodeEventContext.h"
#include "IIndividualHumanContext.h"
#include "Exceptions.h"
#include "EventTrigger.h"

SETUP_LOGGING( "Interventions" )

//unsigned int total_intervention_counter = 0;

namespace Kernel
{
    BaseIntervention::BaseIntervention()
        : parent(nullptr)
        , name()
        , cost_per_unit(0.0f)
        , expired(false)
        , dont_allow_duplicates(false)
        , enable_iv_replacement(false)
        , first_time(true)
        , disqualifying_properties()
        , status_property()
        , event_trigger_distributed()
        , event_trigger_expired()
    {
        initSimTypes( 1, "*" );
    }

    BaseIntervention::BaseIntervention( const BaseIntervention& original )
        : JsonConfigurable()
        , parent(nullptr)
        , name(original.name)
        , cost_per_unit(original.cost_per_unit)
        , expired(original.expired)
        , dont_allow_duplicates(original.dont_allow_duplicates)
        , enable_iv_replacement(original.enable_iv_replacement)
        , first_time(original.first_time)
        , disqualifying_properties(original.disqualifying_properties)
        , status_property(original.status_property)
        , event_trigger_distributed(original.event_trigger_distributed)
        , event_trigger_expired(original.event_trigger_expired)
    {
    }

    BaseIntervention::~BaseIntervention()
    {
    }

    bool BaseIntervention::Configure(const Configuration * inputJson)
    {
        // ------------------------------------------------------------------
        // --- Must calculate default name in Configure(). You can't do it
        // --- in the constructor because the pointer doesn't know what object
        // --- it is yet.
        // ------------------------------------------------------------------
        name = typeid(*this).name();
#ifdef WIN32
        name = name.substr( 14 ); // remove "class Kernel::"
#else
        name = abi::__cxa_demangle( name.c_str(), 0, 0, nullptr );
        name = name.substr( 8 ); // remove "Kernel::"
#endif

        std::string default_name = name;

        initConfig( "Event_Trigger_Distributed",  event_trigger_distributed,  inputJson, MetadataDescriptor::Enum("Event_Trigger_Distributed", Event_Trigger_Distributed_DESC_TEXT, MDD_ENUM_ARGS( EventTrigger ) ) );
        initConfig( "Event_Trigger_Expired",      event_trigger_expired,      inputJson, MetadataDescriptor::Enum("Event_Trigger_Expired",     Event_Trigger_Expired_DESC_TEXT,     MDD_ENUM_ARGS( EventTrigger ) ) );

        initConfigTypeMap( "Intervention_Name", &name, Intervention_Name_DESC_TEXT, default_name );

        initConfigTypeMap( "Dont_Allow_Duplicates",           &dont_allow_duplicates,   Dont_Allow_Duplicates_DESC_TEXT,           false );
        initConfigTypeMap( "Enable_Intervention_Replacement", &enable_iv_replacement,   Enable_Intervention_Replacement_DESC_TEXT, false, "Dont_Allow_Duplicates" );

        jsonConfigurable::tDynamicStringSet tmp_disqualifying_properties;
        initConfigTypeMap("Disqualifying_Properties", &tmp_disqualifying_properties, IP_Disqualifying_Properties_DESC_TEXT );
        initConfigTypeMap("New_Property_Value", &status_property, IP_New_Property_Value_DESC_TEXT );

        bool ret = JsonConfigurable::Configure(inputJson);
        if( ret && !JsonConfigurable::_dryrun )
        {
            // transfer the strings into the IPKeyValueContainer
            for( auto state : tmp_disqualifying_properties )
            {
                IPKeyValue kv( state );
                disqualifying_properties.Add( kv );
            }

            // error if the intervention_state is an invalid_state
            if( status_property.IsValid() && disqualifying_properties.Contains( status_property ) )
            {
                std::string abort_state_list ;
                for( auto state : tmp_disqualifying_properties )
                {
                    abort_state_list += "'" + state + "', " ;
                }
                if( tmp_disqualifying_properties.size() > 0 )
                {
                    abort_state_list = abort_state_list.substr( 0, abort_state_list.length() - 2 );
                }
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                                                        "New_Property_Value", status_property.ToString().c_str(),
                                                        "Disqualifying_Properties", abort_state_list.c_str(),
                                                        "The New_Property_Value cannot be one of the Disqualifying_Properties." );
            }
        }
        return ret ;
    }

    bool BaseIntervention::Expired()
    {
        return expired;
    }

    void BaseIntervention::SetExpired( bool isExpired )
    {
        expired = isExpired;
    }

    void BaseIntervention::OnExpiration()
    {
        if(event_trigger_expired != EventTrigger::NoTrigger)
        {
            parent->GetEventContext()->GetNodeEventContext()->GetIndividualEventBroadcaster()->TriggerObservers(parent->GetEventContext(), event_trigger_expired);
        }
    }

    bool BaseIntervention::Distribute( IIndividualHumanInterventionsContext* context, ICampaignCostObserver* const pICCO )
    {
        if( dont_allow_duplicates && context->ContainsExistingByName(name) )
        {
            if( enable_iv_replacement )
            {
                context->PurgeExistingByName( name );
            }
            else
            {
                return false ;
            }
        }

        if( AbortDueToDisqualifyingInterventionStatus( context->GetParent() ) )
        {
            return false;
        }

        bool wasDistributed=false;

        if( context->GetInterventionConsumer()->GiveIntervention(this) )
        {
            // Need to get Individual pointer from interventions container pointer. Try parent.
            IIndividualHumanEventContext * pIndiv = (context->GetParent())->GetEventContext();
            if( pICCO )
            {
                pICCO->notifyCampaignExpenseIncurred( cost_per_unit, pIndiv );
            }
            wasDistributed = true;
            if(event_trigger_distributed != EventTrigger::NoTrigger)
            {
                pIndiv->GetNodeEventContext()->GetIndividualEventBroadcaster()->TriggerObservers(pIndiv, event_trigger_distributed);
            }
        }

        return wasDistributed;
    }

    bool BaseIntervention::AbortDueToDisqualifyingInterventionStatus( IIndividualHumanContext* pHuman )
    {
        const IPKeyValueContainer* props = pHuman->GetEventContext()->GetProperties();

        IPKeyValue found = props->FindFirst( disqualifying_properties );
        if( found.IsValid() )
        {
            LOG_DEBUG_F( "The property \"%s\" for intervention is one of the Disqualifying_Properties.  Expiring '%s' for individual %d.\n",
                            found.ToString().c_str(), name.c_str(), pHuman->GetSuid().data );

            IIndividualEventBroadcaster* broadcaster = pHuman->GetEventContext()->GetNodeEventContext()->GetIndividualEventBroadcaster();

            broadcaster->TriggerObservers( pHuman->GetEventContext(), EventTrigger::InterventionDisqualified );
            expired = true;

            return true;
        }
        return false;
    }

    bool BaseIntervention::UpdateIndividualsInterventionStatus()
    {
        if( AbortDueToDisqualifyingInterventionStatus( parent ) )
        {
            return false;
        }
        // is this the first time through?  if so, update the intervention state.
        if( first_time && status_property.IsValid() )
        {
            LOG_DEBUG_F( "Setting Intervention Status to %s for individual %d.\n", status_property.ToString().c_str(), parent->GetSuid().data );
            parent->GetInterventionsContext()->ChangeProperty( status_property.GetKeyAsString().c_str(), status_property.GetValueAsString().c_str() );
            first_time = false;
        }
        return true;
    }

    void BaseIntervention::SetContextTo( IIndividualHumanContext *context )
    {
        parent = context;
    }

    void BaseIntervention::serialize( IArchive& ar, BaseIntervention* obj )
    {
        BaseIntervention& intervention = *obj;
        ar.labelElement( "name" )                       & intervention.name;
        ar.labelElement( "cost_per_unit" )              & intervention.cost_per_unit;
        ar.labelElement( "expired" )                    & intervention.expired;
        ar.labelElement( "dont_allow_duplicates" )      & intervention.dont_allow_duplicates;
        ar.labelElement( "enable_iv_replacement" )      & intervention.enable_iv_replacement;
        ar.labelElement( "first_time" )                 & intervention.first_time;
        ar.labelElement( "disqualifying_properties" )   & intervention.disqualifying_properties;
        ar.labelElement( "status_property" )            & intervention.status_property;
        ar.labelElement( "event_trigger_distributed" )  & (uint32_t&)intervention.event_trigger_distributed;
        ar.labelElement( "event_trigger_expired" )      & (uint32_t&)intervention.event_trigger_expired;
    }

    // ------------------------------------------------------------------------
    // --- BaseNodeIntervention
    // ------------------------------------------------------------------------

    BaseNodeIntervention::BaseNodeIntervention()
        : parent(nullptr)
        , name()
        , cost_per_unit(0.0f)
        , expired(false)
        , dont_allow_duplicates(false)
        , enable_iv_replacement(false)
        , first_time(true)
        , disqualifying_properties()
        , status_property()
    {
        initSimTypes( 1, "*" );
    }

    BaseNodeIntervention::BaseNodeIntervention( const BaseNodeIntervention& original )
        : JsonConfigurable()
        , parent(nullptr)
        , name(original.name)
        , cost_per_unit(original.cost_per_unit)
        , expired(original.expired)
        , dont_allow_duplicates(original.dont_allow_duplicates)
        , enable_iv_replacement(original.enable_iv_replacement)
        , first_time(original.first_time)
        , disqualifying_properties(original.disqualifying_properties)
        , status_property(original.status_property)
    {
    }

    BaseNodeIntervention::~BaseNodeIntervention()
    {
    }

    bool BaseNodeIntervention::Configure( const Configuration * inputJson )
    {
        // ------------------------------------------------------------------
        // --- Must calculate default name in Configure(). You can't do it
        // --- in the constructor because the pointer doesn't know what object
        // --- it is yet.
        // ------------------------------------------------------------------
        name = typeid(*this).name();
#ifdef WIN32
        name = name.substr( 14 ); // remove "class Kernel::"
#else
        name = abi::__cxa_demangle( name.c_str(), 0, 0, nullptr );
        name = name.substr( 8 ); // remove "Kernel::"
#endif
        std::string default_name = name;

        initConfigTypeMap( "Intervention_Name", &name, Intervention_Name_DESC_TEXT, default_name );

        initConfigTypeMap( "Dont_Allow_Duplicates",           &dont_allow_duplicates,   Dont_Allow_Duplicates_DESC_TEXT,           false );
        initConfigTypeMap( "Enable_Intervention_Replacement", &enable_iv_replacement,   Enable_Intervention_Replacement_DESC_TEXT, false, "Dont_Allow_Duplicates" );

        jsonConfigurable::tDynamicStringSet tmp_disqualifying_properties;
        initConfigTypeMap( "Disqualifying_Properties", &tmp_disqualifying_properties, NP_Disqualifying_Properties_DESC_TEXT );
        initConfigTypeMap( "New_Property_Value", &status_property, NP_New_Property_Value_DESC_TEXT );

        bool ret = JsonConfigurable::Configure( inputJson );
        if( ret && !JsonConfigurable::_dryrun )
        {
            // transfer the strings into the NPKeyValueContainer
            for( auto state : tmp_disqualifying_properties )
            {
                NPKeyValue kv( state );
                disqualifying_properties.Add( kv );
            }

            // error if the intervention_state is an invalid_state
            if( status_property.IsValid() && disqualifying_properties.Contains( status_property ) )
            {
                std::string abort_state_list ;
                for( auto state : tmp_disqualifying_properties )
                {
                    abort_state_list += "'" + state + "', " ;
                }
                if( tmp_disqualifying_properties.size() > 0 )
                {
                    abort_state_list = abort_state_list.substr( 0, abort_state_list.length() - 2 );
                }
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                                                        "New_Property_Value", status_property.ToString().c_str(),
                                                        "Disqualifying_Properties", abort_state_list.c_str(),
                                                        "The New_Property_Value cannot be one of the Disqualifying_Properties." );
            }
        }
        return ret ;
    }

    bool BaseNodeIntervention::Expired()
    {
        return expired;
    }

    void BaseNodeIntervention::SetExpired( bool isExpired )
    {
        expired = isExpired;
    }

    void BaseNodeIntervention::OnExpiration()
    {
    }

    bool BaseNodeIntervention::Distribute( INodeEventContext *context, IEventCoordinator2 * ec )
    {
       if( dont_allow_duplicates && context->ContainsExistingByName(name) )
        {
            if( enable_iv_replacement )
            {
                context->PurgeExistingByName( name );
            }
            else
            {
                return false ;
            }
        }

        if( AbortDueToDisqualifyingInterventionStatus( context ) )
        {
            return false;
        }

        bool wasDistributed=false;

        if( context->GetNodeInterventionConsumer()->GiveIntervention(this) )
        {
            wasDistributed = true;
            context->IncrementCampaignCost(cost_per_unit);
        }

        return wasDistributed;
    }

    void BaseNodeIntervention::SetContextTo( INodeEventContext *context )
    {
        parent = context;
    }
    bool BaseNodeIntervention::AbortDueToDisqualifyingInterventionStatus( INodeEventContext* context )
    {
        NPKeyValueContainer& node_propeties = context->GetNodeContext()->GetNodeProperties();

        NPKeyValue found = node_propeties.FindFirst( disqualifying_properties );
        if( found.IsValid() )
        {
            LOG_DEBUG_F( "The property \"%s\" is one of the Disqualifying_Properties.  Expiring the '%s' for node %d.\n",
                found.ToString().c_str(), name.c_str(), context->GetExternalId() );

            IIndividualEventBroadcaster* broadcaster = context->GetIndividualEventBroadcaster();
            broadcaster->TriggerObservers( nullptr, EventTrigger::InterventionDisqualified );
            expired = true;

            return true;
        }
        return false;
    }

    bool BaseNodeIntervention::UpdateNodesInterventionStatus()
    {
        if( AbortDueToDisqualifyingInterventionStatus( parent ) )
        {
            return false;
        }
        // is this the first time through?  if so, update the intervention state.
        if( first_time && status_property.IsValid() )
        {
            LOG_DEBUG_F( "Setting Intervention Status to %s for node %d.\n", status_property.ToString().c_str(), parent->GetExternalId() );
            parent->GetNodeContext()->GetNodeProperties().Set( status_property );
            first_time = false;
        }
        return true;
    }

    void BaseNodeIntervention::serialize( IArchive& ar, BaseNodeIntervention* obj )
    {
        BaseNodeIntervention& intervention = *obj;
        ar.labelElement( "name" )                       & intervention.name;
        ar.labelElement( "cost_per_unit" )              & intervention.cost_per_unit;
        ar.labelElement( "expired" )                    & intervention.expired;
        ar.labelElement( "dont_allow_duplicates" )      & intervention.dont_allow_duplicates;
        ar.labelElement( "enable_iv_replacement" )      & intervention.enable_iv_replacement;
        ar.labelElement( "first_time" )                 & intervention.first_time;
        ar.labelElement( "disqualifying_properties" )   & intervention.disqualifying_properties;
        ar.labelElement( "status_property" )            & intervention.status_property;
    }
}
