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

    REGISTER_SERIALIZABLE(NodeEnvironmental);

    void NodeEnvironmental::serialize(IArchive& ar, NodeEnvironmental* obj)
    {
        Node::serialize(ar, obj);
        NodeEnvironmental& node = *obj;
    }
}

#endif // ENABLE_ENVIRONMENTAL
