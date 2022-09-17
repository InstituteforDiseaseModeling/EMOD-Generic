/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"

#include "NodeTB.h"
#include "TransmissionGroupsFactory.h" //for SetupIntranodeTransmission
#include "NodeEventContextHost.h" //for node level trigger
#include "IndividualCoInfection.h"
#include "EventTrigger.h"

SETUP_LOGGING( "NodeTB" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodeTB, NodeAirborne)
        HANDLE_INTERFACE(INodeTB)
    END_QUERY_INTERFACE_DERIVED(NodeTB, NodeAirborne)


    NodeTB::~NodeTB(void)
    {
    }

    NodeTB::NodeTB()
        : NodeAirborne()
    {
    }

    NodeTB::NodeTB(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : NodeAirborne(_parent_sim, externalNodeId, node_suid)
    {
    }

    NodeTB *NodeTB::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodeTB *newnode = _new_ NodeTB(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    void NodeTB::Initialize()
    {
        NodeAirborne::Initialize();
    }

    IIndividualHuman* NodeTB::createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanCoInfection::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    uint64_t NodeTB::GetTotalGenomes() const
    {
        // TB infections use genome for drug resistance with the TBInfectionDrugResistance ENUM type.
        return 2;
    }

    void NodeTB::resetNodeStateCounters(void)
    {
        NodeAirborne::resetNodeStateCounters();
        incident_counter = 0.0f;
        MDR_incident_counter = 0.0f;
        MDR_evolved_incident_counter = 0.0f;
        MDR_fast_incident_counter = 0.0f;
    }

    IIndividualHuman* NodeTB::addNewIndividual( float monte_carlo_weight, float initial_age, int gender, int initial_infection_count, float immparam, float riskparam)
    {
        auto tempind = NodeAirborne::addNewIndividual(monte_carlo_weight, initial_age, gender, initial_infection_count, immparam, riskparam);

        return tempind;
    }

    float NodeTB::GetIncidentCounter() const 
    {
        return incident_counter;
    }

    float NodeTB::GetMDRIncidentCounter() const 
    {
        return MDR_incident_counter;
    }

    float NodeTB::GetMDREvolvedIncidentCounter() const 
    {
        return MDR_evolved_incident_counter;
    }

    float NodeTB::GetMDRFastIncidentCounter() const 
    {
        return MDR_fast_incident_counter;
    }

    REGISTER_SERIALIZABLE(NodeTB);

    void NodeTB::serialize(IArchive& ar, NodeTB* obj)
    {
        NodeAirborne::serialize(ar, obj);
        NodeTB& node = *obj;
        ar.labelElement("incident_counter") & node.incident_counter;
        ar.labelElement("MDR_incident_counter") & node.MDR_incident_counter;
        ar.labelElement("MDR_evolved_incident_counter") & node.MDR_evolved_incident_counter;
        ar.labelElement("MDR_fast_incident_counter") & node.MDR_fast_incident_counter;
    }
}

