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
        , txEnvironment(nullptr)
    { }

    NodeEnvironmental::NodeEnvironmental(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : Node(_parent_sim, externalNodeId, node_suid)
        , txEnvironment(nullptr)
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
        txEnvironment->EndUpdate(getSeasonalAmplitude());
        GetParent()->GetSimulationEventContext()->GetNodeEventBroadcaster()->TriggerObservers( GetEventContext(), EventTriggerNode::SheddingComplete );
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

    float NodeEnvironmental::getSeasonalAmplitude() const
    {
        float amplification = 0.0f;

        float peak_amplification = 1.0f;
        float ramp_down_days = GetParams()->environmental_ramp_down_duration;
        float ramp_up_days   = GetParams()->environmental_ramp_up_duration;
        float cutoff_days    = GetParams()->environmental_cutoff_days;
        float peak_start_day = floor(GetParams()->environmental_peak_start);
        if (peak_start_day > DAYSPERYEAR)
        {
            peak_start_day = peak_start_day - DAYSPERYEAR;
        }
        //this is mostly for calibtool purposes
        float peak_days = (DAYSPERYEAR - cutoff_days) - (ramp_down_days + ramp_up_days);
        float peak_end_day = peak_start_day + peak_days;
        if (peak_end_day > DAYSPERYEAR)
        {
            peak_end_day = peak_end_day - DAYSPERYEAR;
        }

        float slope_up = peak_amplification / ramp_up_days;
        float slope_down = peak_amplification / ramp_down_days;

        int SimDay = (int)GetTime().time; // is this the date of the simulated year?
        int nDayOfYear = SimDay % DAYSPERYEAR;
#define OFFSET (0.0f) // (0.5f)
        if (peak_start_day - ramp_up_days > 0)
        {
            if ((nDayOfYear >= peak_start_day-ramp_up_days) && ( nDayOfYear < peak_start_day))
            { // beginning of wastewater irrigation
                amplification=((nDayOfYear- (peak_start_day-ramp_up_days))+ OFFSET )*(slope_up);
                LOG_DEBUG_F( "Seasonal Amplification 1 stage A: ramping up.\n" );
            }
            else if ((peak_start_day - peak_end_day >= 0) && ((nDayOfYear >= peak_start_day)  || (nDayOfYear<=peak_end_day)))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
                LOG_DEBUG_F( "Seasonal Amplification 1 stage B: peak amp plateau.\n" );
            }
            else if ((peak_start_day - peak_end_day < 0) && (nDayOfYear >= peak_start_day) && (nDayOfYear <= peak_end_day))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
                LOG_DEBUG_F( "Seasonal Amplification 1 stage C: peak amp plateau.\n" );
            }
            else if ((peak_end_day + ramp_down_days < DAYSPERYEAR) && (nDayOfYear > peak_end_day) && (nDayOfYear <= (peak_end_day+ramp_down_days)))
            {
                amplification= peak_amplification-(((nDayOfYear-peak_end_day)- OFFSET )*slope_down);
                LOG_DEBUG_F( "Seasonal Amplification 1 stage D: slope down.\n" );
            }
            else if ((peak_end_day + ramp_down_days >= DAYSPERYEAR) && ((nDayOfYear > peak_end_day) || (nDayOfYear < ramp_down_days - (DAYSPERYEAR- peak_end_day))))
            {
                // end of wastewater irrigation
                if (nDayOfYear > peak_end_day)
                {
                    amplification = peak_amplification-(((nDayOfYear-peak_end_day)- OFFSET )*slope_down);
                    LOG_DEBUG_F( "Seasonal Amplification 1 stage E(i): slope down.\n" );
                }
                if (nDayOfYear < ramp_down_days - (DAYSPERYEAR - peak_end_day))
                {
                    amplification = peak_amplification - (((DAYSPERYEAR-peak_end_day)+nDayOfYear- OFFSET )*slope_down);
                    LOG_DEBUG_F( "Seasonal Amplification 1 stage E(ii): slope down.\n" );
                }
            }
            else
            {
                LOG_DEBUG_F( "Seasonal Amplification 1 stage F: zero amp plateau.\n" );
            }
        }
        else if (peak_start_day - ramp_up_days <= 0)
        {
            if ((nDayOfYear >= peak_start_day-ramp_up_days+DAYSPERYEAR) || ( nDayOfYear < peak_start_day))
            { // beginning of wastewater irrigation
                if (nDayOfYear >= peak_start_day-ramp_up_days+DAYSPERYEAR)
                {
                    amplification= (nDayOfYear - (peak_start_day-ramp_up_days+DAYSPERYEAR)+ OFFSET )*(slope_up);
                    LOG_DEBUG_F( "Seasonal Amplification 2 stage A(i): ramping up.\n" );
                }
                else if (nDayOfYear < peak_start_day)
                {
                    amplification= (((ramp_up_days-peak_start_day) + nDayOfYear)  +  OFFSET )*(slope_up);
                    LOG_DEBUG_F( "Seasonal Amplification 2 stage A(ii): ramping up.\n" );
                }
            }
            if ((peak_start_day - peak_end_day > 0) && ((nDayOfYear >= peak_start_day)  || (nDayOfYear<=peak_end_day)))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
                LOG_DEBUG_F( "Seasonal Amplification 2 stage B: peak plateau.\n" );
            }
            else if ((peak_start_day - peak_end_day < 0) && (nDayOfYear >= peak_start_day) && (nDayOfYear <= peak_end_day))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
                LOG_DEBUG_F( "Seasonal Amplification 2 stage C: peak plateau.\n" );
            }
            else if ((nDayOfYear > peak_end_day) && (nDayOfYear <= (peak_end_day+ramp_down_days)) )
            { // end of wastewater irrigation
                amplification= peak_amplification-(((nDayOfYear-peak_end_day)- OFFSET )*slope_down);
                LOG_DEBUG_F( "Seasonal Amplification 2 stage D: bottom plateau.\n" );
            }
        }

        LOG_VALID_F("amplification calculated as %f: day of year=%d, start=%f, end=%f, ramp_up=%f, ramp_down=%f, cutoff=%f.\n", amplification, nDayOfYear, peak_start_day, peak_end_day, ramp_up_days, ramp_down_days, cutoff_days );
        return amplification;
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

    void NodeEnvironmental::UpdateTransmissionGroupPopulation(const tProperties& properties, float size_delta, float mc_weight)
    {
        TransmissionGroupMembership_t contact;
        transmissionGroups->GetGroupMembershipForProperties( properties, contact );
        transmissionGroups->UpdatePopulationSize(contact, size_delta, mc_weight);

        TransmissionGroupMembership_t environmental;
        txEnvironment->GetGroupMembershipForProperties( properties, environmental );
        txEnvironment->UpdatePopulationSize(environmental, size_delta, mc_weight);
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

    uint64_t NodeEnvironmental::GetTotalGenomes() const
    {
        // Environmental infections use genome to represent transmission route
        return 3;
    }

    REGISTER_SERIALIZABLE(NodeEnvironmental);

    void NodeEnvironmental::serialize(IArchive& ar, NodeEnvironmental* obj)
    {
        Node::serialize(ar, obj);
        NodeEnvironmental& node = *obj;
    }
}

#endif // ENABLE_ENVIRONMENTAL
