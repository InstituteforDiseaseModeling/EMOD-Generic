/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#pragma warning(disable:4996)

#ifdef ENABLE_PYTHON

#include <math.h>
#include <numeric> // for std::accumulate
#include <functional> // why not algorithm?
#include "Sugar.h"
#include "Exceptions.h"
#include "NodePy.h"
#include "ConfigParams.h"
#include "IndividualPy.h"
#include "TransmissionGroupsFactory.h"
#include "PythonSupport.h"

using namespace Kernel;

SETUP_LOGGING( "NodePy" )

#define ENABLE_PYTHON_FEVER 1
namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodePy, Node)
        HANDLE_INTERFACE(INodePy)
    END_QUERY_INTERFACE_DERIVED(NodePy, Node)


    NodePy::NodePy() : Node() { }

    NodePy::NodePy(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : Node(_parent_sim, externalNodeId, node_suid)
    {
    }

    void NodePy::Initialize()
    {
        Node::Initialize();
    }

    NodePy *NodePy::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodePy *newnode = _new_ NodePy(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    NodePy::~NodePy(void)
    {
    }

    void NodePy::resetNodeStateCounters(void)
    {
        // This is a chance to do a single call into PYTHON_FEVER at start of timestep
#ifdef ENABLE_PYTHON_FEVER
        PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_PYTHON_FEVER, "start_timestep" ));
        if( pFunc )
        {
            PyObject* retval = PyObject_CallObject( pFunc, nullptr );
            Py_XDECREF(retval);
        }
#endif

        Node::resetNodeStateCounters();
    }

    void NodePy::updateNodeStateCounters(IndividualHuman *ih)
    {
        float mc_weight                = float(ih->GetMonteCarloWeight());
        IIndividualHumanPy *tempind2 = NULL;
        if( ih->QueryInterface( GET_IID( IIndividualHumanPy ), (void**)&tempind2 ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "tempind2", "IndividualHumanPy", "IndividualHuman" );
        }

        Node::updateNodeStateCounters(ih);
    }

    IndividualHuman* NodePy::createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanPy::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    NodePyTest* NodePyTest::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        auto *newnode = _new_ NodePyTest(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    NodePyTest::NodePyTest(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        parent = _parent_sim;
        auto newPerson = configureAndAddNewIndividual(1.0f /*mc*/, 0 /*age*/, 0.0f /*prev*/, 0.5f /*gender*/, 1.0f /*mod_acquire*/, 1.0f /*risk_mod*/); // N.B. temp_prevalence=0 without maternal_transmission flag

        for (auto pIndividual : individualHumans)
        {
             // Nothing to do at the moment.
        }
    }
}
#endif // ENABLE_PYTHON
