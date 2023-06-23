/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <typeinfo>
#ifndef WIN32
#include <cxxabi.h>
#endif
#include "stdafx.h"

#include "Debug.h"
#include "Exceptions.h"
#include "Sugar.h"
#include "Environment.h"
#include "InterventionsContainer.h"
#include "IIndividualHuman.h"
#include "IIndividualHumanContext.h"
#include "NodeEventContext.h"
#include "INodeContext.h"
#include "Properties.h"
#include "EventTrigger.h"

SETUP_LOGGING( "InterventionsContainer" )


namespace Kernel
{
    QueryResult
    InterventionsContainer::QueryInterface( iid_t iid, void** ppinstance )
    {
        assert(ppinstance); // todo: add a real message: "QueryInterface requires a non-NULL destination!");

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        GET_IID(IIndividualHumanInterventionsContext);
        iid_t retVal = GET_IID(IIndividualHumanInterventionsContext);
        if (iid == retVal)
            foundInterface = static_cast<IIndividualHumanInterventionsContext*>(this);
        else if (iid == GET_IID(IInterventionConsumer))
            foundInterface = static_cast<IInterventionConsumer*>(this);
        else if (iid == GET_IID(IVaccineConsumer))
            foundInterface = static_cast<IVaccineConsumer*>(this);
        else
            foundInterface = nullptr;

        QueryResult status;
        if ( !foundInterface )
        {
            status = e_NOINTERFACE;
            // if we had a base class we would QI down into it, but we don't.
        }
        else
        {
            //foundInterface->AddRef();           // not implementing this yet!
            status = s_OK;
        }

        *ppinstance = foundInterface;
        return status;
    }

    InterventionsContainer::~InterventionsContainer()
    {
        for (auto intervention : interventions)
        {
            delete intervention;
        }
    }

    void InterventionsContainer::PropagateContextToDependents()
    {
        IIndividualHumanContext *context = GetParent();
        for (auto intervention : interventions)
        {
            intervention->SetContextTo(context);
        }
    }

    std::list<IDistributableIntervention*> InterventionsContainer::GetInterventionsByType(const std::string& type_name)
    {
        std::list<IDistributableIntervention*> interventions_of_type;
        LOG_DEBUG_F( "Looking for intervention of type %s\n", type_name.c_str() );
        for (auto intervention : interventions)
        {
            std::string cur_iv_type_name = typeid( *intervention ).name();
            if( cur_iv_type_name == type_name )
            {
                interventions_of_type.push_back( intervention );
            }
        }

        return interventions_of_type;
    }

    std::list<IDistributableIntervention*> InterventionsContainer::GetInterventionsByName(const std::string& intervention_name)
    {
        std::list<IDistributableIntervention*> interventions_list;
        LOG_DEBUG_F( "Looking for interventions with name %s\n", intervention_name.c_str() );
        for (auto intervention : interventions)
        {
            if( intervention->GetName() == intervention_name )
            {
                interventions_list.push_back( intervention );
            }
        }

        return interventions_list;
    }

    void InterventionsContainer::PurgeExisting( const std::string& iv_name )
    {
        std::list<IDistributableIntervention*> iv_list = GetInterventionsByType(iv_name);

        for( auto iv_ptr : iv_list )
        {
            LOG_DEBUG_F("Found an existing intervention by that name (%s) which we are purging\n", iv_name.c_str());
            interventions.remove( iv_ptr );
            delete iv_ptr;
        }
    }

    void InterventionsContainer::PurgeExistingByName( const std::string& iv_name )
    {
        std::list<IDistributableIntervention*> iv_list = GetInterventionsByName(iv_name);

        for( auto iv_ptr : iv_list )
        {
            LOG_DEBUG_F("Found an existing intervention by that name (%s) which we are purging\n", iv_name.c_str());
            interventions.remove( iv_ptr );
            delete iv_ptr;
        }
    }

    bool InterventionsContainer::ContainsExisting( const std::string& iv_name )
    {
        for( auto intervention : interventions )
        {
            if( typeid(*intervention).name() == iv_name )
            {
                return true;
            }
        }
        return false;
    }

    bool InterventionsContainer::ContainsExistingByName( const std::string& iv_name )
    {
        for( auto intervention : interventions )
        {
            if( intervention->GetName() == iv_name )
            {
                return true;
            }
        }
        return false;
    }

    std::list<IDrug*> InterventionsContainer::GetDrugInterventions()
    {
        std::list<IDrug*> drug_list;
        for (auto intervention : interventions)
        {
            IDrug* p_drug = intervention->GetDrug();
            if ( p_drug )
            {
                drug_list.push_back(p_drug);
            }
        }

        return drug_list;
    }

    void InterventionsContainer::PreInfectivityUpdate( float dt )
    {
        // If group membership is affected by IP transitions, those transitions need to happen before
        // infectivity is accumulated. Occurs at the start of individual->UpdateInfectiousness
        for( auto intervention : interventions )
        {
            if( intervention->NeedsPreInfectivityUpdate() )
            {
                intervention->Update( dt );
            }
        }
    }

    void InterventionsContainer::InfectiousLoopUpdate( float dt )
    {
        // --- ------------------------------------------------------------------------------------
        // --- The purpose of this method is to update the interventions that need to be updated
        // --- in the infectious update loop.  These interventions can be updated multiple times
        // --- per time step and affect the person's infections.  Drugs are typically interventions
        // --- that need to be in this loop.
        // --- ------------------------------------------------------------------------------------
        std::fill(reduced_acquire.begin(),   reduced_acquire.end(),   1.0f);
        std::fill(reduced_transmit.begin(),  reduced_transmit.end(),  1.0f);
        std::fill(reduced_mortality.begin(), reduced_mortality.end(), 1.0f);

        for( auto intervention : interventions )
        {
            if( intervention->NeedsInfectiousLoopUpdate() )
            {
                intervention->Update( dt );
            }
        }
    }

    void InterventionsContainer::Update( float dt )
    {
        if( !interventions.empty() )
        {
            // -----------------------------------------------------------------------
            // --- The "orig_num" check below is done below so that we update
            // --- interventions that were distributed by the existing interventions.
            // --- We need to update both types of new interventions.
            // -----------------------------------------------------------------------
            int orig_num = interventions.size();
            int i = 0;
            for( auto intervention : interventions )
            {
                if( (i >= orig_num) || ( !intervention->NeedsInfectiousLoopUpdate() &&
                                         !intervention->NeedsPreInfectivityUpdate()   ) )
                {
                    intervention->Update( dt );
                }
                ++i;
            }

            // TODO: appears that it might be more efficient to remove on the fly
            std::vector<IDistributableIntervention*> dead_ivs;
            for( auto intervention : interventions )
            {
                // ------------------------------------------------------------------------------
                // --- BEWARE: Some interventions distribute other interventions or fire events
                // --- (which can cause interventions to be added) during the call to Expired().
                // ------------------------------------------------------------------------------
                if( intervention->Expired() )
                {
                    LOG_DEBUG("Found an expired intervention\n");
                    intervention->OnExpiration();
                    dead_ivs.push_back( intervention );
                }
            }

            // Remove any expired interventions (e.g., calendars). Don't want these things accumulating forever.
            for (auto intervention : dead_ivs)
            {
                LOG_DEBUG("Destroying an expired intervention.\n");
                interventions.remove( intervention );
                delete intervention;
            }
        }
    }

    InterventionsContainer::InterventionsContainer()
        : parent(nullptr)
        , reduced_acquire(IVRoute::pairs::count(), 1.0f)
        , reduced_transmit(IVRoute::pairs::count(), 1.0f)
        , reduced_mortality(IVRoute::pairs::count(), 1.0f)
        , interventions()
    {
    }

    bool InterventionsContainer::GiveIntervention( IDistributableIntervention* iv )
    {
        interventions.push_back( iv );
        // We need to increase the reference counter here to represent fact that interventions container
        // is keeping a pointer to the intervention. (Otherwise when event coordinator calls Release,
        // and ref counter is decremented, the intervention object will delete itself.)
        iv->AddRef();
        iv->SetContextTo( parent );
        LOG_DEBUG_F("InterventionsContainer (individual %d) has %d interventions now\n", GetParent()->GetSuid().data, interventions.size());
        return true;
    }

    void InterventionsContainer::UpdateIVAcquireRate(float acq, IVRoute::Enum vax_route)
    {
        if(vax_route == IVRoute::ALL)
        {
            for(auto iv_route : IVRoute::pairs::get_values())
            {
                reduced_acquire[iv_route]   *= (1.0f - acq);
            }
        }
        else
        {
            reduced_acquire[vax_route] *= (1.0f - acq);
        }
    }

    void InterventionsContainer::UpdateIVTransmitRate(float xmit, IVRoute::Enum vax_route)
    {
        if(vax_route == IVRoute::ALL)
        {
            for(auto iv_route : IVRoute::pairs::get_values())
            {
                reduced_transmit[iv_route]  *= (1.0f - xmit);
            }
        }
        else
        {
            reduced_transmit[vax_route]  *= (1.0f - xmit);
        }
    }

    void InterventionsContainer::UpdateIVMortalityRate(float mort, IVRoute::Enum vax_route)
    {
        if(vax_route == IVRoute::ALL)
        {
            for(auto iv_route : IVRoute::pairs::get_values())
            {
                reduced_mortality[iv_route] *= (1.0f - mort);
            }
        }
        else
        {
            reduced_mortality[vax_route] *= (1.0f - mort);
        }
    }

    void InterventionsContainer::ChangeProperty(const char* property, const char* new_value)
    {
        // Get parent property (remove need for casts)
        IPKeyValueContainer* pProps = parent->GetEventContext()->GetProperties();

        // Check that property exists, except Age_Bins which are special case. We bootstrap individuals into age_bins at t=1,
        // with no prior existing age bin property.
        IPKey key( property );
        if( ( std::string( property ) != "Age_Bin" ) && !pProps->Contains( key ) )
        {
            throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "properties", property );
        }

        IPKeyValue new_kv( property, new_value );

        if( !pProps->Contains( new_kv ) )
        {
            IPKeyValue old_kv = pProps->Get( key );
            LOG_DEBUG_F( "Moving individual (%lu) property %s from %s to %s\n", parent->GetSuid().data, property, old_kv.GetValueAsString().c_str(), new_value );
            parent->UpdateGroupPopulation(-1.0f);
            pProps->Set( new_kv );
            parent->UpdateGroupMembership();
            parent->UpdateGroupPopulation(1.0f);
            parent->SetPropertyReportString("");

            //broadcast that the individual changed properties
            IIndividualEventBroadcaster* broadcaster = parent->GetEventContext()->GetNodeEventContext()->GetIndividualEventBroadcaster();
            LOG_DEBUG_F( "Individual %d changed property, broadcasting PropertyChange \n", parent->GetSuid().data );
            broadcaster->TriggerObservers( parent->GetEventContext(), EventTrigger::PropertyChange );
        }
    }

    void InterventionsContainer::SetContextTo(IIndividualHumanContext* context)
    {
        parent = context;
        if (parent)
        {
            PropagateContextToDependents();
        }
    }

    IIndividualHumanContext* InterventionsContainer::GetParent()
    {
        return parent;
    }

    IInterventionConsumer* InterventionsContainer::GetInterventionConsumer()
    {
        return static_cast<IInterventionConsumer*>(this);
    }

    float InterventionsContainer::GetInterventionReducedAcquire(TransmissionRoute::Enum tx_route) const
    {
        if(tx_route == TransmissionRoute::CONTACT)
        {
            return GetIVReducedAcquire(IVRoute::CONTACT);
        }
        else if(tx_route == TransmissionRoute::ENVIRONMENTAL)
        {
            return GetIVReducedAcquire(IVRoute::ENVIRONMENTAL);
        }
        else
        {
            return GetIVReducedAcquire(IVRoute::ALL);
        }
    }

    float InterventionsContainer::GetInterventionReducedTransmit(TransmissionRoute::Enum tx_route) const
    {
        if(tx_route == TransmissionRoute::CONTACT)
        {
            return GetIVReducedTransmit(IVRoute::CONTACT);
        }
        else if(tx_route == TransmissionRoute::ENVIRONMENTAL)
        {
            return GetIVReducedTransmit(IVRoute::ENVIRONMENTAL);
        }
        else
        {
            return GetIVReducedTransmit(IVRoute::ALL);
        }
    }

    float InterventionsContainer::GetInterventionReducedMortality(TransmissionRoute::Enum tx_route) const
    {
        if(tx_route == TransmissionRoute::CONTACT)
        {
            return GetIVReducedMortality(IVRoute::CONTACT);
        }
        else if(tx_route == TransmissionRoute::ENVIRONMENTAL)
        {
            return GetIVReducedMortality(IVRoute::ENVIRONMENTAL);
        }
        else
        {
            return GetIVReducedMortality(IVRoute::ALL);
        }
    }

    float InterventionsContainer::GetIVReducedAcquire(IVRoute::Enum iv_route) const
    {
        return reduced_acquire[iv_route];
    }

    float InterventionsContainer::GetIVReducedTransmit(IVRoute::Enum iv_route) const
    { 
        return reduced_transmit[iv_route];
    }

    float InterventionsContainer::GetIVReducedMortality(IVRoute::Enum iv_route) const
    {
        return reduced_mortality[iv_route];
    }

    REGISTER_SERIALIZABLE(InterventionsContainer);

    void InterventionsContainer::serialize(IArchive& ar, InterventionsContainer* obj)
    {
        InterventionsContainer& container = *obj;
        ar.labelElement("interventions") & container.interventions;

        ar.labelElement("reduced_acquire" )     & container.reduced_acquire;
        ar.labelElement("reduced_transmit" )    & container.reduced_transmit;
        ar.labelElement("reduced_mortality" )   & container.reduced_mortality;
    }
}
