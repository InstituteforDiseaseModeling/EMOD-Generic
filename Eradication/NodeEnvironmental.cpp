/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#if defined(ENABLE_ENVIRONMENTAL)

#include "IdmDateTime.h"
#include "NodeEnvironmental.h"
#include "ConfigParams.h"
#include "IndividualEnvironmental.h"
#include "TransmissionGroupsFactory.h"
#include "INodeContext.h"
#include "NodeEventContextHost.h"
#include "ISimulationContext.h"
#include "SimulationEventContext.h"
#include "EventTriggerNode.h"
#include "Infection.h"

SETUP_LOGGING( "NodeEnvironmental" )

namespace Kernel
{
    NodeEnvironmental::NodeEnvironmental()
        : Node()
    { }

    NodeEnvironmental::NodeEnvironmental(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : Node(_parent_sim, externalNodeId, node_suid)
    { }

    NodeEnvironmental::~NodeEnvironmental(void)
    { }

    NodeEnvironmental *NodeEnvironmental::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodeEnvironmental *newnode = _new_ NodeEnvironmental(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    IIndividualHuman* NodeEnvironmental::createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanEnvironmental::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    void NodeEnvironmental::updateInfectivity(float dt)
    {
        Node::updateInfectivity(dt);

        // Incorporate multiplicative infectivity
        float infectivity_multiplication = event_context_host->GetInfectivityMultiplier(TransmissionRoute::ENVIRONMENTAL);
        txEnvironment->EndUpdate(infectivity_multiplication);
    }

    ITransmissionGroups* NodeEnvironmental::CreateTransmissionGroups()
    {
        txEnvironment = TransmissionGroupsFactory::CreateNodeGroups( TransmissionGroupType::StrainAwareGroups, GetRng() );
        txEnvironment->UseGroupPopulationForNormalization();
        txEnvironment->SetTag( "environmental" );
        return TransmissionGroupsFactory::CreateNodeGroups( TransmissionGroupType::StrainAwareGroups, GetRng() );
    }

    void NodeEnvironmental::BuildTransmissionRoutes( float contagionDecayRate )
    {
        transmissionGroups->Build(contagionDecayRate,                    GetParams()->number_clades, GetTotalGenomes());
        txEnvironment->Build(GetParams()->node_contagion_decay_fraction, GetParams()->number_clades, GetTotalGenomes());
    }

    void NodeEnvironmental::SetupIntranodeTransmission()
    {
        transmissionGroups = CreateTransmissionGroups();

        if( IPFactory::GetInstance() && IPFactory::GetInstance()->HasIPs() && GetParams()->enable_hint )
        {
            for( auto p_ip : IPFactory::GetInstance()->GetIPList() )
            {
                auto hint = p_ip->GetIntraNodeTransmission( GetExternalID() );
                auto matrix = hint.GetMatrix();

                if ( matrix.size() > 0 )
                {
                    TransmissionRoute::Enum routeName = hint.GetRouteName();
                    AddRoute(routeName);
                    ITransmissionGroups* txGroups = (routeName == TransmissionRoute::CONTACT) ? transmissionGroups : txEnvironment;
                    txGroups->AddProperty( p_ip->GetKeyAsString(),
                                           p_ip->GetValues<IPKeyValueContainer>().GetValuesToList(),
                                           matrix );
                }
                else if ( hint.GetRouteToMatrixMap().size() > 0 )
                {
                    for (auto entry : hint.GetRouteToMatrixMap())
                    {
                        TransmissionRoute::Enum routeName = entry.first;
                        auto& matrix = entry.second;
                        AddRoute(routeName);
                        ITransmissionGroups* txGroups = (routeName == TransmissionRoute::CONTACT) ? transmissionGroups : txEnvironment;
                        txGroups->AddProperty( p_ip->GetKeyAsString(),
                                               p_ip->GetValues<IPKeyValueContainer>().GetValuesToList(),
                                               matrix );
                    }
                }
                else //HINT is enabled, but no transmission matrix is detected
                {
                    // This is okay. We don't need every IP to participate in HINT. 
                }
            }
        }
        else
        {
            // Default with no custom HINT
            AddRoute(TransmissionRoute::CONTACT);
            AddRoute(TransmissionRoute::ENVIRONMENTAL);
        }

        event_context_host->SetupTxRoutes();
        BuildTransmissionRoutes( 1.0f );
    }

    void NodeEnvironmental::DepositFromIndividual( const IStrainIdentity& strain_IDs, float contagion_quantity, TransmissionGroupMembership_t individual, TransmissionRoute::Enum route )
    {
        LOG_DEBUG_F( "deposit from individual: clade index =%d, genome index = %d, quantity = %f, route = %d\n", strain_IDs.GetCladeID(), strain_IDs.GetGeneticID(), contagion_quantity, uint32_t(route) );

        switch (route)
        {
            case TransmissionRoute::CONTACT:
                transmissionGroups->DepositContagion( strain_IDs, contagion_quantity, individual );
                break;

            case TransmissionRoute::ENVIRONMENTAL:
                txEnvironment->DepositContagion( strain_IDs, contagion_quantity, individual );
                break;

            default:
                // TODO - try to get proper string from route enum
                throw new BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "route", uint32_t(route), "???" );
        }
    }

    float NodeEnvironmental::GetContagionByRouteAndProperty( TransmissionRoute::Enum route, const IPKeyValue& property_value )
    {
        ITransmissionGroups* txGroups = (route == TransmissionRoute::CONTACT) ? transmissionGroups : txEnvironment;

        return txGroups->GetContagionByProperty( property_value );
    }

    void NodeEnvironmental::GetGroupMembershipForIndividual(TransmissionRoute::Enum route, const tProperties& properties, TransmissionGroupMembership_t& transmissionGroupMembership)
    {
        LOG_DEBUG_F( "Calling %s.\n", __FUNCTION__ );
        ITransmissionGroups* txGroups = (route == TransmissionRoute::CONTACT) ? transmissionGroups : txEnvironment;
        txGroups->GetGroupMembershipForProperties( properties, transmissionGroupMembership );
    }

    void NodeEnvironmental::ExposeIndividual(IInfectable* candidate, TransmissionGroupMembership_t individual, float dt, TransmissionRoute::Enum route)
    {
        switch (route)
        {
            case TransmissionRoute::CONTACT:
                Node::ExposeIndividual( candidate, individual, dt, route);
                break;

            case TransmissionRoute::ENVIRONMENTAL:
                txEnvironment->ExposeToContagion(candidate, individual, dt, route);
                break;

            default:
                // TODO - try to get proper string from route enum
                throw new BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "route", uint32_t(route), "???" );
        }
    }

    std::map<TransmissionRoute::Enum, float> NodeEnvironmental::GetContagionByRoute() const
    {
        std::map<TransmissionRoute::Enum, float> returnThis;

        returnThis.insert( std::make_pair(TransmissionRoute::CONTACT,       transmissionGroups->GetTotalContagion() ) );
        returnThis.insert( std::make_pair(TransmissionRoute::ENVIRONMENTAL, txEnvironment->GetTotalContagion() ) );

        LOG_VALID_F( "contact: %f, environmental: %f\n", returnThis[TransmissionRoute::CONTACT], returnThis[TransmissionRoute::ENVIRONMENTAL] );

        return returnThis;
    }

    REGISTER_SERIALIZABLE(NodeEnvironmental);

    void NodeEnvironmental::serialize(IArchive& ar, NodeEnvironmental* obj)
    {
        Node::serialize(ar, obj);
        NodeEnvironmental& node = *obj;
    }
}

#endif // ENABLE_ENVIRONMENTAL
