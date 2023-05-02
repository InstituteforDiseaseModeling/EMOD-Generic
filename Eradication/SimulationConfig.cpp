/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#ifdef WIN32
#include "windows.h"
#endif

#include "SimulationConfig.h"

// These headers are included here mostly for 
// many constants defines which eventually in SS4 will be
// moved back to where they belong so that we don't have to 
// include all these header files
#include "Common.h"
#include "Debug.h"
#include "Configure.h"
#include "InterventionEnums.h"
#include "Log.h"
#include "Sugar.h"
#include "NoCrtWarnings.h"

#ifndef DISABLE_VECTOR
#include "VectorParameters.h"
#include "VectorSpeciesParameters.h"
#endif

#ifndef DISABLE_MALARIA
#include "GenomeMarkers.h"
#include "MalariaParameters.h"
#include "MalariaDrugTypeParameters.h"
#endif

#include "Infection.h"

SETUP_LOGGING( "SimulationConfig" )

namespace Kernel
{

ISimulationConfigFactory * SimulationConfigFactory::_instance = nullptr;
static const char* malaria_drug_placeholder_string = "<malaria_drug_name_goes_here>";

ISimulationConfigFactory * SimulationConfigFactory::getInstance()
{
    if( _instance == nullptr )
    {
        _instance = new SimulationConfigFactory();
    }
    return _instance;
}

SimulationConfig* SimulationConfigFactory::CreateInstance(Configuration * config)
{
    SimulationConfig *SimConfig =  _new_ SimulationConfig();
    release_assert(SimConfig);
    if (SimConfig)
    {           
        if (SimConfig->Configure(config))
        {
            SimConfig->AddRef();
        }
        else
        {
            SimConfig->Release();
            SimConfig = nullptr;
        }
    }
    return SimConfig;
}

void SimulationConfigFactory::Register(string classname, instantiator_function_t _if)
{  
    getRegisteredClasses()[classname] = _if;
}

support_spec_map_t& SimulationConfigFactory::getRegisteredClasses()
{ 
    static support_spec_map_t registered_classes; 
    return registered_classes; 
}

IMPLEMENT_FACTORY_REGISTERED(SimulationConfig)

BEGIN_QUERY_INTERFACE_BODY(SimulationConfig)
     HANDLE_INTERFACE(IConfigurable)
END_QUERY_INTERFACE_BODY(SimulationConfig)

SimulationConfig::~SimulationConfig()
{
}

bool SimulationConfig::Configure(const Configuration * inputJson)
{
    LOG_DEBUG( "Configure\n" );

    release_assert(inputJson);

    m_jsonConfig = inputJson;

    //vector enums
    if (MatchesDependency(inputJson, "Simulation_Type", "VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"))
    {
        VectorInitConfig( inputJson );
    }
    if (MatchesDependency(inputJson, "Simulation_Type", "MALARIA_SIM"))
    {
        MalariaInitConfig( inputJson );
    }

    // --------------------------------------
    // --- Read the parameters from the JSON
    // --------------------------------------
    LOG_DEBUG_F( "Calling main Configure..., use_defaults = %d\n", JsonConfigurable::_useDefaults );

    bool ret = JsonConfigurable::Configure( inputJson );

    // ---------------------------------------
    // --- Return if just generating schema
    // ---------------------------------------
    if( JsonConfigurable::_dryrun == true )
    {
        return true;
    }

    // ------------------------------------------------------------------
    // --- Update, check, and read other parameters given the input data
    // ------------------------------------------------------------------
    if (MatchesDependency(inputJson, "Simulation_Type", "VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"))
    {
        VectorCheckConfig( inputJson );
        if (MatchesDependency(inputJson, "Simulation_Type", "MALARIA_SIM"))
        {
            MalariaCheckConfig( inputJson );
        }
    }

    return ret;
}

QuickBuilder SimulationConfig::GetSchema()
{
    LOG_DEBUG( "GetSchema\n" );
    json::Object * job = new json::Object( GetSchemaBase() );
    json::QuickBuilder retJson( *job );

#if !defined(_DLLS_)
    VectorAddSchema( retJson );
    MalariaAddSchema( retJson );
#endif

    return retJson;
}

SimulationConfig::SimulationConfig()
    : m_jsonConfig(nullptr)
    , vector_params(nullptr)
    , malaria_params(nullptr)
{
#ifndef DISABLE_VECTOR
    vector_params = new VectorParameters();
#endif
#ifndef DISABLE_MALARIA
    malaria_params = new MalariaParameters();
    malaria_params->pGenomeMarkers = new GenomeMarkers();
#endif
}

// ----------------------------------------------------------------------------
// --- VectorParameters
// ----------------------------------------------------------------------------

void SimulationConfig::VectorInitConfig( const Configuration* inputJson )
{
#ifndef DISABLE_VECTOR
    initConfig( "Egg_Hatch_Delay_Distribution",     vector_params->egg_hatch_delay_dist,             inputJson, MetadataDescriptor::Enum(Egg_Hatch_Delay_Distribution_DESC_TEXT,     Egg_Hatch_Delay_Distribution_DESC_TEXT,     MDD_ENUM_ARGS(EggHatchDelayDist))       ); // vector pop only
    initConfig( "Egg_Saturation_At_Oviposition",    vector_params->egg_saturation,                   inputJson, MetadataDescriptor::Enum(Egg_Saturation_At_Oviposition_DESC_TEXT,    Egg_Saturation_At_Oviposition_DESC_TEXT,    MDD_ENUM_ARGS(EggSaturation))           ); // vector pop only
    initConfig( "Larval_Density_Dependence",        vector_params->larval_density_dependence,        inputJson, MetadataDescriptor::Enum(Larval_Density_Dependence_DESC_TEXT,        Larval_Density_Dependence_DESC_TEXT,        MDD_ENUM_ARGS(LarvalDensityDependence)) ); // vector pop only
    initConfig( "Egg_Hatch_Density_Dependence",     vector_params->egg_hatch_density_dependence,     inputJson, MetadataDescriptor::Enum(Egg_Hatch_Density_Dependence_DESC_TEXT,     Egg_Hatch_Density_Dependence_DESC_TEXT,     MDD_ENUM_ARGS(EggHatchDensityDependence)) ); // vector pop only
    initConfig( "Vector_Sampling_Type",             vector_params->vector_sampling_type,             inputJson, MetadataDescriptor::Enum("Vector_Sampling_Type",                     Vector_Sampling_Type_DESC_TEXT,             MDD_ENUM_ARGS(VectorSamplingType))      );

    initConfig( "Vector_Sugar_Feeding_Frequency",   vector_params->vector_sugar_feeding,             inputJson, MetadataDescriptor::Enum(Vector_Sugar_Feeding_Frequency_DESC_TEXT,   Vector_Sugar_Feeding_Frequency_DESC_TEXT,   MDD_ENUM_ARGS(VectorSugarFeeding)), "Vector_Sampling_Type", "TRACK_ALL_VECTORS,SAMPLE_IND_VECTORS"      ); // vector pop individual only
    initConfig( "Vector_Larval_Rainfall_Mortality", vector_params->vector_larval_rainfall_mortality, inputJson, MetadataDescriptor::Enum(Vector_Larval_Rainfall_Mortality_DESC_TEXT, Vector_Larval_Rainfall_Mortality_DESC_TEXT, MDD_ENUM_ARGS(VectorRainfallMortality)) );
    // get the c50 value if there is rainfall mortality
    initConfigTypeMap( "Larval_Rainfall_Mortality_Threshold", &(vector_params->larval_rainfall_mortality_threshold), Larval_Rainfall_Mortality_Threshold_DESC_TEXT, 0.01f, 1000.0f, 100.0f, "Vector_Larval_Rainfall_Mortality", "SIGMOID,SIGMOID_HABITAT_SHIFTING" );

    //Q added stuff
    initConfigTypeMap( "Enable_Temperature_Dependent_Egg_Hatching", &(vector_params->temperature_dependent_egg_hatching), Enable_Temperature_Dependent_Egg_Hatching_DESC_TEXT, false );
    initConfigTypeMap( "Egg_Arrhenius1", &(vector_params->eggarrhenius1), Egg_Arrhenius1_DESC_TEXT, 0.0f, 10000000000.0f, 61599956.864f, "Enable_Temperature_Dependent_Egg_Hatching" );
    initConfigTypeMap( "Egg_Arrhenius2", &(vector_params->eggarrhenius2), Egg_Arrhenius2_DESC_TEXT, 0.0f, 10000000000.0f, 5754.033f, "Enable_Temperature_Dependent_Egg_Hatching" );
    initConfigTypeMap( "Enable_Egg_Mortality", &(vector_params->egg_mortality), Enable_Egg_Mortality_DESC_TEXT, false ); // not hooked up yet
    initConfigTypeMap( "Enable_Drought_Egg_Hatch_Delay", &(vector_params->delayed_hatching_when_habitat_dries_up), Enable_Drought_Egg_Hatch_Delay_DESC_TEXT, false );
    initConfigTypeMap( "Drought_Egg_Hatch_Delay", &(vector_params->droughtEggHatchDelay), Drought_Egg_Hatch_Delay_DESC_TEXT, 0.0f, 1.0f, 0.33f, "Enable_Drought_Egg_Hatch_Delay" );

    // get the larval density dependence parameters 
    initConfigTypeMap( "Larval_Density_Mortality_Scalar", &(vector_params->larvalDensityMortalityScalar), Larval_Density_Mortality_Scalar_DESC_TEXT,  0.01f, 1000.0f, 10.0f, "Larval_Density_Dependence", "GRADUAL_INSTAR_SPECIFIC,LARVAL_AGE_DENSITY_DEPENDENT_MORTALITY_ONLY" );
    initConfigTypeMap( "Larval_Density_Mortality_Offset", &(vector_params->larvalDensityMortalityOffset), Larval_Density_Mortality_Offset_DESC_TEXT, 0.0001f, 1000.0f, 0.1f, "Larval_Density_Dependence", "GRADUAL_INSTAR_SPECIFIC,LARVAL_AGE_DENSITY_DEPENDENT_MORTALITY_ONLY" );

    initConfig( "HEG_Model", vector_params->heg_model, inputJson, MetadataDescriptor::Enum(HEG_Model_DESC_TEXT, HEG_Model_DESC_TEXT, MDD_ENUM_ARGS(HEGModel)) );
    // get the larval density dependence parameters
    initConfigTypeMap( "HEG_Homing_Rate",        &(vector_params->HEGhomingRate),        HEG_Homing_Rate_DESC_TEXT,        0, 1, 0, "HEG_Model", "GERMLINE_HOMING,EGG_HOMING,DUAL_GERMLINE_HOMING,DRIVING_Y" );
    initConfigTypeMap( "HEG_Fecundity_Limiting", &(vector_params->HEGfecundityLimiting), HEG_Fecundity_Limiting_DESC_TEXT, 0, 1, 0, "HEG_Model", "GERMLINE_HOMING,EGG_HOMING,DUAL_GERMLINE_HOMING,DRIVING_Y" );

    // Value of lloffset is one-half of Node_Grid_Size ; store directly in lloffset and correct after configuration
    initConfigTypeMap( "Node_Grid_Size",                      &(vector_params->lloffset),                            Node_Grid_Size_DESC_TEXT, 0.004167f, 90.0f, 0.004167f );
    initConfigTypeMap( "Mosquito_Weight",                     &(vector_params->mosquito_weight),                     Mosquito_Weight_DESC_TEXT,        1, 10000,         1, "Vector_Sampling_Type", "SAMPLE_IND_VECTORS");

    initConfigTypeMap( "Enable_Vector_Aging",                 &(vector_params->vector_aging),                        Enable_Vector_Aging_DESC_TEXT, false );
    initConfig( "Temperature_Dependent_Feeding_Cycle", vector_params->temperature_dependent_feeding_cycle, inputJson, MetadataDescriptor::Enum(Temperature_Dependent_Feeding_Cycle_DESC_TEXT, Temperature_Dependent_Feeding_Cycle_DESC_TEXT, MDD_ENUM_ARGS(TemperatureDependentFeedingCycle)) ); // vector pop only

    initConfigTypeMap( "Mean_Egg_Hatch_Delay",                       &(vector_params->meanEggHatchDelay),                   Mean_Egg_Hatch_Delay_DESC_TEXT, 0, 120, 0, "Egg_Hatch_Delay_Distribution", "EXPONENTIAL_DISTRIBUTION" );
    initConfigTypeMap( "Wolbachia_Mortality_Modification",           &(vector_params->WolbachiaMortalityModification),      Wolbachia_Mortality_Modification_DESC_TEXT, 0, 100, 1 );
    initConfigTypeMap( "Wolbachia_Infection_Modification",           &(vector_params->WolbachiaInfectionModification),      Wolbachia_Infection_Modification_DESC_TEXT, 0, 100, 1 );

    initConfigTypeMap( "Human_Feeding_Mortality", &(vector_params->human_feeding_mortality), Human_Feeding_Mortality_DESC_TEXT, 0.0f, 1.0f, 0.1f );

    initConfigTypeMap( "Temporary_Habitat_Decay_Factor",   &(vector_params->tempHabitatDecayScalar),        Temporary_Habitat_Decay_Factor_DESC_TEXT,   0.001f,    100.0f, 0.05f );
    initConfigTypeMap( "Semipermanent_Habitat_Decay_Rate", &(vector_params->semipermanentHabitatDecayRate), Semipermanent_Habitat_Decay_Rate_DESC_TEXT, 0.0001f,   100.0f, 0.01f );
    initConfigTypeMap( "Rainfall_In_mm_To_Fill_Swamp",     &(vector_params->mmRainfallToFillSwamp),         Rainfall_In_mm_To_Fill_Swamp_DESC_TEXT,     1.0f,    10000.0f, 1000.0f, "Simulation_Type", "VECTOR_SIM, MALARIA_SIM, DENGUE_SIM" );

    // This is a key parameter for the mosquito ecology and can vary quite a lot
    initConfigTypeMap( "x_Larval_Habitats", &(vector_params->x_LarvalHabitats), x_Larval_Habitats_DESC_TEXT, 0.0f, 10000.0f, 1.0f );

    vector_params->vector_species_names.value_source = "Vector_Species_Params.*";
    initConfigTypeMap( "Vector_Species_Names", &(vector_params->vector_species_names), Vector_Species_Names_DESC_TEXT );

    if( JsonConfigurable::_dryrun )
    {
#if !defined(_DLLS_)
        // for the schema
        std::string arab( "vector_species_name_goes_here" );
        VectorSpeciesParameters * vsp = VectorSpeciesParameters::CreateVectorSpeciesParameters( inputJson, arab );
        vsp->Configure( inputJson );
        vector_params->vspMap[ arab ] = vsp;
#endif
    }

#endif // DISABLE_VECTOR
}

void SimulationConfig::VectorCheckConfig( const Configuration* inputJson )
{
#ifndef DISABLE_VECTOR
    if( vector_params->vector_species_names.empty() )
    {
        LOG_WARN("The simulation is being run without any mosquitoes!  Unless this was intentional, please specify the name of one or more vector species in the 'Vector_Species_Names' array and their associated vector species parameters.\n\n                     ,-.\n         `._        /  |        ,\n            `--._  ,   '    _,-'\n     _       __  `.|  / ,--'\n      `-._,-'  `-. \\ : /\n           ,--.-.-`'.'.-.,_-\n         _ `--'-'-;.'.'-'`--\n     _,-' `-.__,-' / : \\\n                _,'|  \\ `--._\n           _,--'   '   .     `-.\n         ,'         \\  |        `\n                     `-'\n\n");
    }
    for (const auto& vector_species_name : vector_params->vector_species_names)
    {
        // vspMap only in SimConfig now. No more static map in VSP.
        vector_params->vspMap[ vector_species_name ] = VectorSpeciesParameters::CreateVectorSpeciesParameters( inputJson,
                                                                                                               vector_species_name );
    }

    // Correct lloffset; value read from config equal to Node_Grid_Size
    vector_params->lloffset *= 0.5f;
#endif // DISABLE_VECTOR
}

void SimulationConfig::VectorAddSchema( json::QuickBuilder& retJson )
{
#ifndef DISABLE_VECTOR
    for (auto& entry : vector_params->vspMap)
    {
        json::QuickBuilder foo = json::QuickBuilder( vector_params->vspMap[ entry.first ]->GetSchema() );
        const std::string& species_key = entry.first;
        retJson["Vector_Species_Params"][ species_key ] = foo.As<Object>();
    }
#endif // DISABLE_VECTOR
}

// ----------------------------------------------------------------------------
// --- MalariaParameters
// ----------------------------------------------------------------------------

void SimulationConfig::MalariaInitConfig( const Configuration* inputJson )
{
#ifndef DISABLE_MALARIA
    initConfig( "PKPD_Model", malaria_params->PKPD_model, inputJson, MetadataDescriptor::Enum(PKPD_Model_DESC_TEXT, PKPD_Model_DESC_TEXT, MDD_ENUM_ARGS(PKPDModel)) ); // special case: intervention (anti-malarial drug) only

    initConfigTypeMap( "Falciparum_MSP_Variants",      &(malaria_params->falciparumMSPVars),       Falciparum_MSP_Variants_DESC_TEXT,      0, 1e3, DEFAULT_MSP_VARIANTS ); // malaria
    initConfigTypeMap( "Falciparum_Nonspecific_Types", &(malaria_params->falciparumNonSpecTypes),  Falciparum_Nonspecific_Types_DESC_TEXT, 0, 1e3, DEFAULT_NONSPECIFIC_TYPES ); // malaria
    initConfigTypeMap( "Falciparum_PfEMP1_Variants",   &(malaria_params->falciparumPfEMP1Vars),    Falciparum_PfEMP1_Variants_DESC_TEXT,   0, 1e5, DEFAULT_PFEMP1_VARIANTS ); // malaria
    initConfigTypeMap( "Fever_Detection_Threshold",    &(malaria_params->feverDetectionThreshold), Fever_Detection_Threshold_DESC_TEXT,    0.5f, 5.0f, 1.0f );

    initConfigTypeMap( "Parasite_Smear_Sensitivity", &(malaria_params->parasiteSmearSensitivity), Parasite_Smear_Sensitivity_DESC_TEXT, 0.0001f, 100.0f, 0.1f ); // malaria
    initConfigTypeMap( "New_Diagnostic_Sensitivity", &(malaria_params->newDiagnosticSensitivity), New_Diagnostic_Sensitivity_DESC_TEXT, 0.0001f, 100000.0f, 0.01f ); // malaria

    initConfigTypeMap( "Genome_Markers", &(malaria_params->genome_marker_names), Genome_Markers_DESC_TEXT );

    // for schema?
    if( JsonConfigurable::_dryrun )
    {
        std::string drug_name( malaria_drug_placeholder_string );
        MalariaDrugTypeParameters * mdtp = MalariaDrugTypeParameters::CreateMalariaDrugTypeParameters( inputJson, drug_name, *(malaria_params->pGenomeMarkers) );
        mdtp->Configure( inputJson );
        malaria_params->MalariaDrugMap[ drug_name ] = mdtp;
    }
#endif //DISABLE_MALARIA
}

void SimulationConfig::MalariaCheckConfig( const Configuration* inputJson )
{
#ifndef DISABLE_MALARIA
    // for each key in Malaria_Drug_Params, create/configure MalariaDrugTypeParameters object and add to static map
    try
    {
        if( (malaria_params->genome_marker_names.size() > 0) && MatchesDependency(inputJson, "Vector_Sampling_Type", "VECTOR_COMPARTMENTS_NUMBER,VECTOR_COMPARTMENTS_PERCENT") )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                                                    "Vector_Sampling_Type", "VECTOR_COMPARTMENTS_NUMBER or VECTOR_COMPARTMENTS_PERCENT",
                                                    "Genome_Markers", "<not empty>",
                                                    "Genome_Markers can only be used with individual vectors (i.e. TRACK_ALL_VECTORS or SAMPLE_IND_VECTORS)." );
        }

        malaria_params->pGenomeMarkers->Initialize( malaria_params->genome_marker_names );

        json::Object mdp = (*EnvPtr->Config)["Malaria_Drug_Params"].As<Object>();
        json::Object::const_iterator itMdp;
        for (itMdp = mdp.Begin(); itMdp != mdp.End(); ++itMdp)
        {
            std::string drug_name( itMdp->name );
            auto * mdtp = MalariaDrugTypeParameters::CreateMalariaDrugTypeParameters( inputJson, drug_name, *(malaria_params->pGenomeMarkers) );
            release_assert( mdtp );
            malaria_params->MalariaDrugMap[ drug_name ] = mdtp;
        }
    }
    catch(json::Exception &e)
    {
        // Exception casting Malaria_Drug_Params to json::Object
        throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, e.what() ); 
    }
#endif //DISABLE_MALARIA
}

void SimulationConfig::MalariaAddSchema( json::QuickBuilder& retJson )
{
#ifndef DISABLE_MALARIA
    if( malaria_params->MalariaDrugMap.count( malaria_drug_placeholder_string ) > 0 )
    {
        retJson["Malaria_Drug_Params"][ malaria_drug_placeholder_string ] = malaria_params->MalariaDrugMap[ malaria_drug_placeholder_string ]->GetSchema().As<Object>();
    }
#endif // DISABLE_MALARIA
}

}
