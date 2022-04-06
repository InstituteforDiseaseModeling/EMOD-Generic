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
#include "ReportEnvironmental.h"
#include "PropertyReportEnvironmental.h"
#include "ProgVersion.h"

using namespace Kernel;

#pragma warning(disable : 4996)

SETUP_LOGGING( "SimulationEnvironmental" )

namespace Kernel
{
    SimulationEnvironmental::SimulationEnvironmental() : Simulation()
    {
        reportClassCreator = ReportEnvironmental::CreateReport;
        propertiesReportClassCreator = PropertyReportEnvironmental::CreateReport;
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

        IndividualHumanEnvironmentalConfig   env_individual_config_obj;
        SusceptibilityEnvironmentalConfig    env_susceptibility_config_obj;
        InfectionEnvironmentalConfig         env_infection_config_obj;

        env_individual_config_obj.Configure( config );
        env_susceptibility_config_obj.Configure( config );
        env_infection_config_obj.Configure( config );
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
        // TODO: any disease-specific validation goes here.

        return Simulation::ValidateConfiguration(config);
    }

    void SimulationEnvironmental::addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                              suids::suid node_suid,
                                                              NodeDemographicsFactory *nodedemographics_factory,
                                                              ClimateFactory *climate_factory,
                                                              bool white_list_enabled )
    {
        NodeEnvironmental *node = NodeEnvironmental::CreateNode( this, externalNodeId, node_suid );

        addNode_internal( node, nodedemographics_factory, climate_factory, white_list_enabled );
    }

    void SimulationEnvironmental::InitializeFlags( const ::Configuration *config )
    {
    }
}

#endif // ENABLE_POLIO
