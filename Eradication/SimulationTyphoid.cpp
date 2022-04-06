/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TYPHOID

#include "SimulationTyphoid.h"
#include "NodeTyphoid.h"
#include "IndividualTyphoid.h"
#include "InfectionTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "suids.hpp"
#include "ReportTyphoid.h"
#include "TyphoidReportEventRecorder.h"
#include "SpatialReportTyphoid.h"
#include "ReportTyphoidByAgeAndGender.h"
#include "PropertyReportTyphoid.h"
#include "ProgVersion.h"

#pragma warning(disable : 4996)

SETUP_LOGGING( "SimulationTyphoid" )

#ifdef _TYPHOID_DLL

// Note: _diseaseType has to match the Simulation_Type name in config.json
static std::string _diseaseType = "TYPHOID_SIM";

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif


//
// This is the interface function from the DTK.
//
DTK_DLLEXPORT Kernel::ISimulation* __cdecl
CreateSimulation(
    const Environment * pEnv
)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
    LOG_INFO("CreateSimulation called for \n");
    return Kernel::SimulationTyphoid::CreateSimulation( EnvPtr->Config );
}

DTK_DLLEXPORT
const char *
__cdecl
GetDiseaseType()
{
    LOG_INFO("GetDiseaseType called for \n");
    return _diseaseType.c_str();
}

DTK_DLLEXPORT
char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
    ProgDllVersion pv;
    LOG_INFO_F("GetEModuleVersion called with ver=%s\n", pv.getVersion());
    if (sVer) strcpy(sVer, pv.getVersion());
    return sVer;
}


DTK_DLLEXPORT
const char* __cdecl
GetSchema()
{
    LOG_DEBUG_F("GetSchema called for %s: map has size %d\n", _module, Kernel::JsonConfigurable::get_registration_map().size() );
    
    json::Object configSchemaAll;
    std::ostringstream schemaJsonString;
    for (auto& entry : Kernel::JsonConfigurable::get_registration_map())
    {
        const std::string classname = entry.first;
        LOG_DEBUG_F( "classname = %s\n", classname.c_str() );
        json::QuickBuilder config_schema = ((*(entry.second))());
        configSchemaAll[classname] = config_schema;
    }

    json::Writer::Write( configSchemaAll, schemaJsonString );

    putenv( ( std::string( "GET_SCHEMA_RESULT=" ) + schemaJsonString.str().c_str() ).c_str() );
    return schemaJsonString.str().c_str();
}
#ifdef __cplusplus
}
#endif

#endif

namespace Kernel
{
    SimulationTyphoid::SimulationTyphoid() : SimulationEnvironmental()
    {
        reportClassCreator = ReportTyphoid::CreateReport;
        eventReportClassCreator = TyphoidReportEventRecorder::CreateReport;
        spatialReportClassCreator = SpatialReportTyphoid::CreateReport;
        propertiesReportClassCreator = PropertyReportTyphoid::CreateReport;
    }

    void SimulationTyphoid::Initialize()
    {
        SimulationEnvironmental::Initialize();
    }

    void SimulationTyphoid::Initialize(const ::Configuration *config)
    {
        SimulationEnvironmental::Initialize(config);

        IndividualHumanTyphoidConfig   typ_individual_config_obj;
        SusceptibilityTyphoidConfig    typ_susceptibility_config_obj;
        InfectionTyphoidConfig         typ_infection_config_obj;

        typ_individual_config_obj.Configure( config );
        typ_susceptibility_config_obj.Configure( config );
        typ_infection_config_obj.Configure( config );
    }

    SimulationTyphoid *SimulationTyphoid::CreateSimulation()
    {
        SimulationTyphoid *newsimulation = _new_ SimulationTyphoid();
        newsimulation->Initialize();

        return newsimulation;
    }

    SimulationTyphoid *SimulationTyphoid::CreateSimulation(const ::Configuration *config)
    {
       SimulationTyphoid *newsimulation = _new_ SimulationTyphoid();

       if (newsimulation)
       {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!newsimulation->ValidateConfiguration(config))
            {
                delete newsimulation;
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "TYPHOID_SIM requested with invalid configuration." );
            }
       }

        return newsimulation;
    }

    bool SimulationTyphoid::ValidateConfiguration(const ::Configuration *config)
    {
        // TODO: any disease-specific validation goes here.

        return SimulationEnvironmental::ValidateConfiguration(config);
    }

    // called by demographic file Populate()
    void SimulationTyphoid::addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                        suids::suid node_suid,
                                                        NodeDemographicsFactory *nodedemographics_factory, 
                                                        ClimateFactory *climate_factory,
                                                        bool white_list_enabled )
    {
        NodeTyphoid *node = NodeTyphoid::CreateNode(this, externalNodeId, node_suid);

        addNode_internal( node, nodedemographics_factory, climate_factory, white_list_enabled );
    }

    void SimulationTyphoid::resolveMigration()
    {
    }

    void SimulationTyphoid::Reports_CreateBuiltIn()
    {
        reports.push_back(ReportTyphoidByAgeAndGender::Create(this,DAYSPERYEAR));
        return SimulationEnvironmental::Reports_CreateBuiltIn();
    }
}

#endif // ENABLE_TYPHOID
