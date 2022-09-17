/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#if defined(ENABLE_ENVIRONMENTAL)

#include "SimulationEnvironmental.h"
#include "NodeEnvironmental.h"
#include "InfectionEnvironmental.h"
#include "SusceptibilityEnvironmental.h"
#include "ProgVersion.h"

using namespace Kernel;

#pragma warning(disable : 4996)

SETUP_LOGGING( "SimulationEnvironmental" )

namespace Kernel
{
    SimulationEnvironmental::SimulationEnvironmental()
        : Simulation()
    {
    }

    SimulationEnvironmental::~SimulationEnvironmental(void)
    {
    }

    void SimulationEnvironmental::Initialize()
    {
        Simulation::Initialize();
    }

    void SimulationEnvironmental::Initialize(const ::Configuration *config)
    {
        Simulation::Initialize(config);
    }

    SimulationEnvironmental *SimulationEnvironmental::CreateSimulation()
    {
        SimulationEnvironmental *newsimulation = _new_ SimulationEnvironmental();
        newsimulation->Initialize();

        return newsimulation;
    }

    SimulationEnvironmental *SimulationEnvironmental::CreateSimulation(const ::Configuration *config)
    {
        SimulationEnvironmental *newsimulation = _new_ SimulationEnvironmental();

        if (newsimulation)
        {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!newsimulation->ValidateConfiguration(config))
            {
                delete newsimulation;
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "ENVIRONMENTAL_SIM requested with invalid configuration." );
            }
        }

        return newsimulation;
    }

    bool SimulationEnvironmental::ValidateConfiguration(const ::Configuration *config)
    {
        return Simulation::ValidateConfiguration(config);
    }

    void SimulationEnvironmental::addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                              suids::suid node_suid,
                                                              NodeDemographicsFactory *nodedemographics_factory,
                                                              ClimateFactory *climate_factory )
    {
        NodeEnvironmental *node = NodeEnvironmental::CreateNode( this, externalNodeId, node_suid );

        addNode_internal( node, nodedemographics_factory, climate_factory );
    }
}

#endif // ENABLE_ENVIRONMENTAL
