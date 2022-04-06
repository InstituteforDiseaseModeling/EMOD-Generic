/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifndef DISABLE_AIRBORNE

#include "NodeAirborne.h"

#include "IndividualAirborne.h"

SETUP_LOGGING( "NodeAirborne" )

namespace Kernel
{
    NodeAirborne::NodeAirborne(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    : Node(_parent_sim, externalNodeId, node_suid)
    {
    }

    NodeAirborne::NodeAirborne() : Node()
    {
    }

    NodeAirborne::~NodeAirborne(void)
    {
    }

    NodeAirborne *NodeAirborne::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodeAirborne *newnode = _new_ NodeAirborne(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    IIndividualHuman* NodeAirborne::createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanAirborne::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    REGISTER_SERIALIZABLE(NodeAirborne);

    void NodeAirborne::serialize(IArchive& ar, NodeAirborne* obj)
    {
        Node::serialize(ar, obj);
    }
}

#endif // DISABLE_AIRBORNE
