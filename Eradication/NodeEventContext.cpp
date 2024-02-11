/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <typeinfo>

#include "ISimulationContext.h"
#include "NodeEventContext.h"
#include "NodeEventContextHost.h"
#include "Debug.h"
#include "NodeDemographics.h"
#include "Node.h"
#include "IIndividualHuman.h"
#include "IIndividualHumanContext.h"
#include "EventTrigger.h"
#include "StrainIdentity.h"
#include "IdmDateTime.h"
#include "Log.h"
#include "Exceptions.h"
#include "TransmissionGroupsBase.h"

SETUP_LOGGING( "NodeEventContext" )

namespace Kernel
{
    NodeEventContextHost::NodeEventContextHost()
        : node(nullptr)
        , arrival_distribution_sources()
        , departure_distribution_sources()
        , node_interventions()
        , broadcaster_impl()
        , birthrate_multiplier(1.0f)
        , connection_multiplier_inbound(1.0f)
        , connection_multiplier_outbound(1.0f)
        , inf_mult_by_route()
    {
        arrival_distribution_sources.clear();
    }

    NodeEventContextHost::NodeEventContextHost(Node* _node)
        : node(_node)
        , arrival_distribution_sources()
        , departure_distribution_sources()
        , node_interventions()
        , broadcaster_impl()
        , birthrate_multiplier(1.0f)
        , connection_multiplier_inbound(1.0f)
        , connection_multiplier_outbound(1.0f)
        , inf_mult_by_route()
    {
        arrival_distribution_sources.clear();
    }

    // This was done with macros, but prefer actual code.
    Kernel::QueryResult NodeEventContextHost::QueryInterface( iid_t iid, void** ppinstance )
    {
        release_assert(ppinstance); // todo: add a real message: "QueryInterface requires a non-NULL destination!");

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        if ( iid == GET_IID(INodeEventContext)) 
            foundInterface = static_cast<INodeEventContext*>(this);
        else if (iid == GET_IID(INodeInterventionConsumer))
            foundInterface = static_cast<INodeInterventionConsumer*>(this);
        else if (iid == GET_IID(ICampaignCostObserver))
            foundInterface = static_cast<ICampaignCostObserver*>(this);
        else if (iid == GET_IID( IIndividualEventBroadcaster ))
            foundInterface = static_cast<IIndividualEventBroadcaster*>(this);
        else if (iid == GET_IID(INodeContext))
            foundInterface = (INodeContext*)node;
        else
            foundInterface = nullptr;

        QueryResult status = e_NOINTERFACE;
        if ( foundInterface )
        {
            foundInterface->AddRef();
            status = s_OK;
            //status = InterventionsContainer::QueryInterface(iid, (void**)&foundInterface);
        }

        *ppinstance = foundInterface;
        return status;
    }

    void NodeEventContextHost::SetContextTo( INodeContext* context )
    {
        PropagateContextToDependents();
    }

    void NodeEventContextHost::SetupTxRoutes()
    {
        for( auto & tx_route: node->GetTransmissionRoutes() )
        {
            inf_mult_by_route[tx_route] = 1.0f;
        }
    }

    // method 1 for VisitIndividuals uses a functor/lambda function
    void NodeEventContextHost::VisitIndividuals( individual_visit_function_t func )
    {
        for( auto individual : node->individualHumans)
        {
            // TODO: I would like this function to return bool so we can keep a total of 'successful' calls, but
            // compiler threw up on me when I tried that! :(
            func(individual->GetEventContext());
        }
    }

    // method 2 for VisitIndividuals uses an interface
    int NodeEventContextHost::VisitIndividuals(IVisitIndividual* pEventCoordinator)
    {
        int   retTotal    = 0;
        int   retMax      = pEventCoordinator->GetMaxEvents();
        float unusedVal = 0.0f;
        
        std::vector<IIndividualHuman*>::iterator iter01, iterBegin, iterMid, iterEnd;
        iterBegin = node->individualHumans.begin();
        iterMid   = iterBegin;
        iterEnd   = node->individualHumans.end();

        // If using max distributions less than number of agents; start at random position in vector
        if(retMax < node->individualHumans.size())
        {
            iterMid += node->GetRng()->uniformZeroToN32(node->individualHumans.size());
        }

        // First pass; start somewhere in vector
        for(iter01 = iterMid; iter01 < iterEnd; iter01++)
        {
            if(retTotal >= retMax)
            {
                break;
            }
            if(pEventCoordinator->visitIndividualCallback((*iter01)->GetEventContext(), unusedVal, this))
            {
                retTotal++;
            }
        }

        // Second pass; start at begining, only check through previous start
        for(iter01 = iterBegin; iter01 < iterMid; iter01++)
        {
            if(retTotal >= retMax)
            {
                break;
            }
            if(pEventCoordinator->visitIndividualCallback((*iter01)->GetEventContext(), unusedVal, this))
            {
                retTotal++;
            }
        }

        return retTotal;
    }

    void NodeEventContextHost::notifyCampaignExpenseIncurred(float expenseIncurred, const IIndividualHumanEventContext * pIndiv)
    {
        release_assert( node );
        if( expenseIncurred > 0 )
        {
            LOG_DEBUG("Campaign expense was incurred\n");
        }
        float cost = expenseIncurred;
        if( pIndiv != nullptr )
        {
            cost *= float( pIndiv->GetMonteCarloWeight() );
        }
        IncrementCampaignCost( cost );
    }

    // First cut of this function writes the intervention report to stdout. It is an abbreviated,
    // 1-line-per-intervention report.
    void NodeEventContextHost::notifyCampaignEventOccurred(
        /*const*/ ISupports * pDistributedIntervention,
        /*const*/ ISupports * pDistributor,// for tb, cool to know parent intervention that 'gave', including tendency if it's an HSB.
        /*const*/ IIndividualHumanContext * pDistributeeIndividual
    )
    {
        if( Environment::getInstance()->Log->CheckLogLevel(Logger::DEBUG, "EEL") )
        {
            // intervention recipient
            std::stringstream msg;
            float recipientAge = float(pDistributeeIndividual->GetEventContext()->GetAge());
            msg << "hum_id="
                << pDistributeeIndividual->GetSuid().data
                << ",ra="
                << recipientAge
                << std::endl;
            Environment::getInstance()->Log->Log(Logger::DEBUG, "EEL","%s\n", msg.str().c_str() );
        }
    }

    const suids::suid& NodeEventContextHost::GetId() const
    {
        return node->suid;
    }

    void NodeEventContextHost::ProcessArrivingIndividual( IIndividualHuman* ih )
    {
        for (auto& entry : arrival_distribution_sources)
        {
            for (int k = 0; k < entry.second; k++)
                entry.first->ProcessArriving(ih->GetEventContext());
        }
    }

    void NodeEventContextHost::ProcessDepartingIndividual( IIndividualHuman* ih )
    {
        //LOG_DEBUG( "ProcessDepartingIndividual\n" );
        for (auto& entry : departure_distribution_sources)
        {
            LOG_DEBUG( "ProcessDepartingIndividual for event coordinator\n" );
            for (int k = 0; k < entry.second; k++)
            {
                LOG_DEBUG( "ProcessDepartingIndividual: Calling ProcessDeparting for individual\n" );
                entry.first->ProcessDeparting(ih->GetEventContext());
            }
        }
    }

    void NodeEventContextHost::RegisterTravelDistributionSource( ITravelLinkedDistributionSource *tlds, TravelEventType type )
    {
        travel_distribution_source_map_t *sources = sourcesMapForType(type);        
        if (sources->find(tlds) != sources->end())
            (*sources)[tlds]++;
        else
            (*sources)[tlds] = 1;

        tlds->AddRef();
    }

    void NodeEventContextHost::UnregisterTravelDistributionSource( ITravelLinkedDistributionSource *tlds, TravelEventType type )
    {
        travel_distribution_source_map_t *sources = sourcesMapForType(type);        
        if (sources->find(tlds) != sources->end())
        {
            (*sources)[tlds]--;
            tlds->Release();

            if ((*sources)[tlds] == 0)
            {
                sources->erase(tlds);
            }
        } else
        {
            // unregistering something that wasnt registered...probably an error
        }
    }

    void NodeEventContextHost::RegisterObserver(
        IIndividualEventObserver * pObserver,
        const EventTrigger::Enum& trigger
    )
    { 
        broadcaster_impl.RegisterObserver( pObserver, trigger );
    }

    void NodeEventContextHost::UnregisterObserver(
        IIndividualEventObserver * pObserver,
        const EventTrigger::Enum& trigger
    )
    {
        //LOG_INFO( "[UnregisterIndividualEventObserver] Putting individual event observer into the disposed observers list .\n" );
        broadcaster_impl.UnregisterObserver( pObserver, trigger );
        //LOG_INFO( "[UnregisterIndividualEventObserver] Putting individual event observer into the disposed observers list .\n" );
    }

    void NodeEventContextHost::TriggerObservers(
        IIndividualHumanEventContext *pIndiv,
        const EventTrigger::Enum& trigger
    )
    {
        broadcaster_impl.TriggerObservers( pIndiv, trigger );
    }

    void NodeEventContextHost::PropagateContextToDependents()
    {
        for (auto intervention : node_interventions)
        {
            intervention->SetContextTo( this );
        }
    }

    void NodeEventContextHost::IncrementCampaignCost(float cost)
    {
        node->Campaign_Cost += cost;
    }

    void NodeEventContextHost::UpdateBirthRateMultiplier(float mult_val)
    {
        birthrate_multiplier           *= mult_val;
    }

    void NodeEventContextHost::UpdateConnectionModifiers(float inbound, float outbound)
    {
        connection_multiplier_inbound  *= inbound;
        connection_multiplier_outbound *= outbound;
    }

    void NodeEventContextHost::UpdateInfectivityMultiplier(float mult_val, TransmissionRoute::Enum tx_route)
    {
        inf_mult_by_route.at(tx_route) *= mult_val;
    }

    void NodeEventContextHost::UpdateInterventions(float dt)
    {
        birthrate_multiplier            = 1.0f;
        connection_multiplier_inbound   = 1.0f;
        connection_multiplier_outbound  = 1.0f;
        for( auto & route_mult: inf_mult_by_route )
        {
            route_mult.second = 1.0f;
        }

        std::vector<INodeDistributableIntervention*> expired_list;
        for( auto intervention : node_interventions )
        {
            intervention->Update( dt );
            if( intervention->Expired() )
            {
                expired_list.push_back( intervention );
            }
        }

        for( auto intven : expired_list )
        {
            node_interventions.remove( intven );
            delete intven;
        }

        broadcaster_impl.DisposeOfUnregisteredObservers();
    }

    bool NodeEventContextHost::GiveIntervention( INodeDistributableIntervention* iv )
    {
        node_interventions.push_back( iv );
        // We need to increase the reference counter here to represent fact that interventions container
        // is keeping a pointer to the intervention. (Otherwise when event coordinator calls Release,
        // and ref counter is decremented, the intervention object will delete itself.)
        iv->AddRef();
        iv->SetContextTo( this );
        return true;
    }

    std::list<INodeDistributableIntervention*> NodeEventContextHost::GetInterventionsByType(const std::string& type_name)
    {
        std::list<INodeDistributableIntervention*> interventions_of_type;
        LOG_DEBUG_F( "Looking for intervention of type %s\n", type_name.c_str() );
        for (auto intervention : node_interventions)
        {
            std::string cur_iv_type_name = typeid( *intervention ).name();
            if( cur_iv_type_name == type_name )
            {
                LOG_DEBUG("Found one...\n");
                interventions_of_type.push_back( intervention );
            }
        }

        return interventions_of_type;
    }

    std::list<INodeDistributableIntervention*> NodeEventContextHost::GetInterventionsByName(const std::string& intervention_name)
    {
        std::list<INodeDistributableIntervention*> interventions_list;
        LOG_DEBUG_F( "Looking for interventions with name %s\n", intervention_name.c_str() );
        for (auto intervention : node_interventions)
        {
            if( intervention->GetName() == intervention_name )
            {
                interventions_list.push_back( intervention );
            }
        }

        return interventions_list;
    }

    void NodeEventContextHost::PurgeExisting( const std::string& iv_name )
    {
        std::list<INodeDistributableIntervention*> iv_list = GetInterventionsByType(iv_name);

        for( auto iv_ptr : iv_list )
        {
            LOG_DEBUG_F("Found an existing intervention by that name (%s) which we are purging\n", iv_name.c_str());
            node_interventions.remove( iv_ptr );
            delete iv_ptr;
        }
    }

    void NodeEventContextHost::PurgeExistingByName( const std::string& iv_name )
    {
        std::list<INodeDistributableIntervention*> iv_list = GetInterventionsByName(iv_name);

        for( auto iv_ptr : iv_list )
        {
            LOG_DEBUG_F("Found an existing intervention by that name (%s) which we are purging\n", iv_name.c_str());
            node_interventions.remove( iv_ptr );
            delete iv_ptr;
        }
    }

    bool NodeEventContextHost::ContainsExisting( const std::string& iv_name )
    {
        for( auto intervention : node_interventions )
        {
            if( typeid(*intervention).name() == iv_name )
            {
                return true;
            }
        }
        return false;
    }

    bool NodeEventContextHost::ContainsExistingByName( const std::string& iv_name )
    {
        for( auto intervention : node_interventions )
        {
            if( intervention->GetName() == iv_name )
            {
                return true;
            }
        }
        return false;
    }

    NodeEventContextHost::~NodeEventContextHost()
    {
        cleanupDistributionSourceMap(arrival_distribution_sources);
        cleanupDistributionSourceMap(departure_distribution_sources);

        for (auto intervention : node_interventions)
        {
            delete intervention;
        }
    }

    NodeEventContextHost::travel_distribution_source_map_t* NodeEventContextHost::sourcesMapForType( INodeEventContext::TravelEventType type )
    {
        travel_distribution_source_map_t *sources = nullptr;
        switch (type)
        {
        case Arrival: sources = &arrival_distribution_sources; break;
        case Departure: sources = &departure_distribution_sources; break;
        default: break;
        }
        return sources;
    }

    void NodeEventContextHost::cleanupDistributionSourceMap( travel_distribution_source_map_t &sources )
    {
        for (auto& entry : sources)
        {
            for (int k = 0; k < entry.second; k++)
                entry.first->Release();
        }

        sources.clear();
    }

    void NodeEventContextHost::AddImportCases( const StrainIdentity* outbreak_strainID, float import_age, int num_cases_per_node, float inf_prob, float female_prob, float mc_weight )
    {
        for (int i = 0; i < num_cases_per_node; i++)
        {
            // Add individual (init_prev = 0.0f; init_mod_acquire = 1.0f; risk_mod = 1.0f)
            IIndividualHuman* new_individual = node->configureAndAddNewIndividual(mc_weight, import_age, 0.0f, female_prob, 1.0f, 1.0f);

            // Start as infectious (incubation_period = 0)
            if(GetRng()->SmartDraw(inf_prob))
            {
                new_individual->AcquireNewInfection( outbreak_strainID, TransmissionRoute::OUTBREAK, 0.0f );
            }
        }
    }

    // These are all pass-throughs in order to get rid of GetNode() as an interface method.
    bool NodeEventContextHost::IsInPolygon(float* vertex_coords, int numcoords)
    {
        return node->IsInPolygon( vertex_coords, numcoords );
    }

    bool NodeEventContextHost::IsInPolygon( const json::Array &poly )
    {
        return node->IsInPolygon( poly );
    }

    bool NodeEventContextHost::IsInExternalIdSet( const std::list<ExternalNodeId_t>& nodelist )
    {
        return node->IsInExternalIdSet( nodelist );
    }

    const IdmDateTime& NodeEventContextHost::GetTime() const
    {
        return node->GetTime();
    }

    RANDOMBASE*  NodeEventContextHost::GetRng()
    {
        return node->GetRng();
    } 

    INodeContext* NodeEventContextHost::GetNodeContext()
    {
        return static_cast<INodeContext*>(node);
    }

    int NodeEventContextHost::GetIndividualHumanCount() const
    {
        size_t nodePop = node->individualHumans.size();
        return int(nodePop);
    }

    ExternalNodeId_t NodeEventContextHost::GetExternalId() const
    {
        ExternalNodeId_t nodeId = node->GetExternalID();
        return nodeId;
    }

    INodeInterventionConsumer* NodeEventContextHost::GetNodeInterventionConsumer()
    {
        return static_cast<INodeInterventionConsumer*>(this);
    }

    ICampaignCostObserver* NodeEventContextHost::GetCampaignCostObserver()
    {
        return static_cast<ICampaignCostObserver*>(this);
    }

    IIndividualEventBroadcaster* NodeEventContextHost::GetIndividualEventBroadcaster()
    {
        return static_cast<IIndividualEventBroadcaster*>(this);
    }

    float NodeEventContextHost::GetBirthRateMultiplier() const
    {
        return birthrate_multiplier;
    }

    float NodeEventContextHost::GetInboundConnectionModifier() const
    {
        return connection_multiplier_inbound;
    }

    float NodeEventContextHost::GetOutboundConnectionModifier() const
    {
        return connection_multiplier_outbound;
    }

    float NodeEventContextHost::GetInfectivityMultiplier(TransmissionRoute::Enum tx_route) const
    {
        return inf_mult_by_route.at(tx_route);
    }
}
