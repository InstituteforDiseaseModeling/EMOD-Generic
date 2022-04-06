/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "NodeHIV.h"

#include "IndividualHIV.h"

SETUP_LOGGING( "NodeHIV" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodeHIV, NodeSTI)
        HANDLE_INTERFACE(INodeHIV)
    END_QUERY_INTERFACE_DERIVED(NodeHIV, NodeSTI)

    NodeHIV::NodeHIV(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : NodeSTI(_parent_sim, externalNodeId, node_suid)
    { }

    NodeHIV::NodeHIV()
        : NodeSTI()
    { }

    NodeHIV::~NodeHIV(void)
    { }

    NodeHIV *NodeHIV::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodeHIV *newnode = _new_ NodeHIV(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    IIndividualHuman* NodeHIV::createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanHIV::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    REGISTER_SERIALIZABLE(NodeHIV);

    void NodeHIV::serialize(IArchive& ar, NodeHIV* obj)
    {
        NodeSTI::serialize(ar, obj);
        // clorton TODO
    }
}
