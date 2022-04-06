/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "Exceptions.h"
#include "SimulationTBHIV.h"
#include "NodeTBHIV.h"

// TBHIV
#include "ReportTBHIV.h"

// TB
#include "PropertyReportTB.h"
#include "SpatialReportTB.h"
#include "BinnedReportTB.h"


SETUP_LOGGING( "SimulationTBHIV" )

namespace Kernel
{
    SimulationTBHIV::~SimulationTBHIV(void) { }
    SimulationTBHIV::SimulationTBHIV()
    {
        //TBHIV Reports
        reportClassCreator = ReportTBHIV::CreateReport;
        
        //TB Reports
        //reportClassCreator = ReportTB::CreateReport;
        binnedReportClassCreator = BinnedReportTB::CreateReport;
        spatialReportClassCreator = SpatialReportTB::CreateReport;
        propertiesReportClassCreator = PropertyReportTB::CreateReport;
    }

    SimulationTBHIV *SimulationTBHIV::CreateSimulation()
    {
        SimulationTBHIV *newsimulation = _new_ SimulationTBHIV();
        newsimulation->Initialize();

        return newsimulation;
    }

    SimulationTBHIV *SimulationTBHIV::CreateSimulation(const ::Configuration *config)
    {
        SimulationTBHIV *newsimulation = _new_ SimulationTBHIV();

        if (newsimulation)
        {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!newsimulation->ValidateConfiguration(config))
            {
                delete newsimulation;
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "TBHIV_SIM requested with invalid configuration." );
            }
        }

        return newsimulation;
    }

    bool SimulationTBHIV::ValidateConfiguration(const ::Configuration *config)
    {
        // TODO: any disease-specific validation goes here.

        return SimulationAirborne::ValidateConfiguration(config);
    }

    void SimulationTBHIV::Initialize()
    {
        SimulationAirborne::Initialize();
    }

    void SimulationTBHIV::Initialize( const ::Configuration *config )
    {
        SimulationAirborne::Initialize( config );

        IndividualHumanCoInfectionConfig   coi_individual_config_obj;
        SusceptibilityTBConfig             coi_susceptibility_config_obj;
        InfectionTBConfig                  coi_infection_config_obj;

        SusceptibilityHIVConfig            hiv_susceptibility_config_obj;
        InfectionHIVConfig                 hiv_infection_config_obj;

        coi_individual_config_obj.Configure( config );
        coi_susceptibility_config_obj.Configure( config );
        coi_infection_config_obj.Configure( config );

        if( IndividualHumanCoInfectionConfig::enable_coinfection )
        {
            // Create static, constant map between CD4 and factor for increased reactivation rate
            coi_individual_config_obj.SetCD4Map( coi_infection_config_obj.GetCD4Map() );

            hiv_susceptibility_config_obj.Configure( config );
            hiv_infection_config_obj.Configure( config );
        }
    }

    void SimulationTBHIV::addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                      suids::suid node_suid,
                                                      NodeDemographicsFactory *nodedemographics_factory,
                                                      ClimateFactory *climate_factory )
    {
        NodeTBHIV *node = NodeTBHIV::CreateNode(this, externalNodeId, node_suid);
        addNode_internal( node, nodedemographics_factory, climate_factory );
    }


    REGISTER_SERIALIZABLE(SimulationTBHIV);

    void SimulationTBHIV::serialize(IArchive& ar, SimulationTBHIV* obj)
    {
        SimulationAirborne::serialize(ar, obj);
        // Nothing to do here
    }

}

