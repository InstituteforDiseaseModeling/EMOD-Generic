/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"

namespace Kernel
{
    ClimateParams    ClimateConfig::climate_params;
    NodeParams       NodeConfig::node_params;
    MigrationParams  MigrationConfig::migration_params;
    SimParams        SimConfig::sim_params;

    ClimateParams::ClimateParams()
        : climate_structure(ClimateStructure::CLIMATE_OFF)
        , climate_update_resolution(ClimateUpdateResolution::CLIMATE_UPDATE_YEAR)
        , climate_airtemperature_filename()
        , climate_koppen_filename()
        , climate_landtemperature_filename()
        , climate_rainfall_filename()
        , climate_relativehumidity_filename()
        , enable_climate_stochasticity(false)
        , rainfall_variance_enabled(false)
        , airtemperature_offset(0.0f)
        , airtemperature_variance(0.0f)
        , base_airtemperature(0.0f)
        , base_humidity(0.0f)
        , base_landtemperature(0.0f)
        , base_rainfall(0.0f)
        , humidity_scale_factor(0.0f)
        , humidity_variance(0.0f)
        , landtemperature_offset(0.0f)
        , landtemperature_variance(0.0f)
        , rainfall_scale_factor(0.0f)
    {}

    NodeParams::NodeParams()
        : ind_sampling_type(IndSamplingType::TRACK_ALL)
        , enable_hint(false)
        , enable_initial_prevalence(false)
        , susceptibility_scaling(false)
        , vector_mortality(false)
        , base_sample_rate(0.0f)
        , environmental_cutoff_days(0.0f)
        , environmental_peak_start(0.0f)
        , environmental_ramp_down_duration(0.0f)
        , environmental_ramp_up_duration(0.0f)
        , immune_downsample_min_age(0.0f)
        , immune_threshold_for_downsampling(0.0f)
        , max_sampling_cell_pop(0.0f)
        , node_contagion_decay_fraction(0.0f)
        , population_scaling_factor(0.0f)
        , rel_sample_rate_immune(0.0f)
        , sample_rate_0_18mo(0.0f)
        , sample_rate_10_14(0.0f)
        , sample_rate_15_19(0.0f)
        , sample_rate_18mo_4yr(0.0f)
        , sample_rate_20_plus(0.0f)
        , sample_rate_5_9(0.0f)
        , sample_rate_birth(0.0f)
        , susceptibility_scaling_rate(0.0f)
        , mosquito_weight(0)
    {}

    MigrationParams::MigrationParams()
        : migration_pattern(MigrationPattern::RANDOM_WALK_DIFFUSION)
        , migration_structure(MigrationStructure::NO_MIGRATION)
        , vec_mod_equ(ModiferEquationType::NOT_DEFINED)
        , mig_file_air()
        , mig_file_family()
        , mig_file_local()
        , mig_file_regional()
        , mig_file_sea()
        , mig_file_vec_local()
        , mig_file_vec_regional()
        , enable_mig_air(false)
        , enable_mig_family(false)
        , enable_mig_local(false)
        , enable_mig_regional(false)
        , enable_mig_sea(false)
        , enable_mig_vec(false)
        , enable_mig_vec_local(false)
        , enable_mig_vec_regional(false)
        , air_roundtrip_duration(0.0f)
        , air_roundtrip_prob(0.0f)
        , family_roundtrip_duration(0.0f)
        , local_roundtrip_duration(0.0f)
        , local_roundtrip_prob(0.0f)
        , mig_mult_air(0.0f)
        , mig_mult_family(0.0f)
        , mig_mult_local(0.0f)
        , mig_mult_regional(0.0f)
        , mig_mult_sea(0.0f)
        , mig_mult_vec_local(0.0f)
        , mig_mult_vec_regional(0.0f)
        , region_roundtrip_duration(0.0f)
        , region_roundtrip_prob(0.0f)
        , sea_roundtrip_duration(0.0f)
        , sea_roundtrip_prob(0.0f)
        , vec_mod_food(0.0f)
        , vec_mod_habitat(0.0f)
        , vec_mod_stayput(0.0f)
        , roundtrip_waypoints(0)
    {}

    SimParams::SimParams()
        : enable_interventions(false)
    {}



    // ConfigParams Methods
    bool ConfigParams::Configure(const Configuration* config)
    {
        bool bRet = true;

        ClimateConfig      climate_config_obj;
        NodeConfig         node_config_obj;
        MigrationConfig    migration_config_obj;
        SimConfig          sim_config_obj;

        bRet &= climate_config_obj.Configure(config);
        bRet &= node_config_obj.Configure(config);
        bRet &= migration_config_obj.Configure(config);
        bRet &= sim_config_obj.Configure(config);

        return bRet;
    }



    // ClimateConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(ClimateConfig,ClimateConfig)
    BEGIN_QUERY_INTERFACE_BODY(ClimateConfig)
    END_QUERY_INTERFACE_BODY(ClimateConfig)

    bool ClimateConfig::Configure(const Configuration* config)
    {
        // Climate parameter dependencies
        const std::map<std::string, std::string> dset_clim01 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"}};
        const std::map<std::string, std::string> dset_clim02 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"},{"Climate_Model","CLIMATE_CONSTANT,CLIMATE_BY_DATA,CLIMATE_KOPPEN"}};
        const std::map<std::string, std::string> dset_clim03 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"},{"Climate_Model","CLIMATE_CONSTANT,CLIMATE_BY_DATA"}};
        const std::map<std::string, std::string> dset_clim04 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"},{"Climate_Model","CLIMATE_CONSTANT,CLIMATE_BY_DATA"},{"Enable_Climate_Stochasticity","1"}};
        const std::map<std::string, std::string> dset_clim05 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"},{"Climate_Model","CLIMATE_KOPPEN"}};
        const std::map<std::string, std::string> dset_clim06 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"},{"Climate_Model","CLIMATE_BY_DATA"}};
        const std::map<std::string, std::string> dset_clim07 {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,POLIO_SIM,AIRBORNE_SIM"},{"Climate_Model","CLIMATE_CONSTANT"}};

        // Climate parameters
        initConfig("Climate_Model",              climate_params.climate_structure,          config,  MetadataDescriptor::Enum("Climate_Model",             Climate_Model_DESC_TEXT,             MDD_ENUM_ARGS(ClimateStructure)),        nullptr, nullptr, &dset_clim01);
        initConfig("Climate_Update_Resolution",  climate_params.climate_update_resolution,  config,  MetadataDescriptor::Enum("Climate_Update_Resolution", Climate_Update_Resolution_DESC_TEXT, MDD_ENUM_ARGS(ClimateUpdateResolution)), nullptr, nullptr, &dset_clim02);

        initConfigTypeMap("Enable_Climate_Stochasticity",   &climate_params.enable_climate_stochasticity,  Enable_Climate_Stochasticity_DESC_TEXT,   false, nullptr, nullptr, &dset_clim03);
        initConfigTypeMap("Enable_Rainfall_Stochasticity",  &climate_params.rainfall_variance_enabled,     Enable_Rainfall_Stochasticity_DESC_TEXT,  true,  nullptr, nullptr, &dset_clim04);

        initConfigTypeMap("Air_Temperature_Variance",       &climate_params.airtemperature_variance,       Air_Temperature_Variance_DESC_TEXT,    0.0f, 5.0f,  2.0f,   nullptr, nullptr, &dset_clim04);
        initConfigTypeMap("Land_Temperature_Variance",      &climate_params.landtemperature_variance,      Land_Temperature_Variance_DESC_TEXT,   0.0f, 7.0f,  2.0f,   nullptr, nullptr, &dset_clim04);
        initConfigTypeMap("Relative_Humidity_Variance",     &climate_params.humidity_variance,             Relative_Humidity_Variance_DESC_TEXT,  0.0f, 0.12f, 0.05f,  nullptr, nullptr, &dset_clim04);

        initConfigTypeMap("Koppen_Filename",             &climate_params.climate_koppen_filename, Koppen_Filename_DESC_TEXT,  "",  nullptr, nullptr, &dset_clim05);

        initConfigTypeMap("Air_Temperature_Offset",         &climate_params.airtemperature_offset,   Air_Temperature_Offset_DESC_TEXT,         -20.0f, 20.0f, 0.0f,  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Land_Temperature_Offset",        &climate_params.landtemperature_offset,  Land_Temperature_Offset_DESC_TEXT,        -20.0f, 20.0f, 0.0f,  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Rainfall_Scale_Factor",          &climate_params.rainfall_scale_factor,   Rainfall_Scale_Factor_DESC_TEXT,            0.1f, 10.0f, 1.0f,  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Relative_Humidity_Scale_Factor", &climate_params.humidity_scale_factor,   Relative_Humidity_Scale_Factor_DESC_TEXT,   0.1f, 10.0f, 1.0f,  nullptr, nullptr, &dset_clim06);

        initConfigTypeMap("Air_Temperature_Filename",    &climate_params.climate_airtemperature_filename,    Air_Temperature_Filename_DESC_TEXT,     "",  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Land_Temperature_Filename",   &climate_params.climate_landtemperature_filename,   Land_Temperature_Filename_DESC_TEXT,    "",  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Rainfall_Filename",           &climate_params.climate_rainfall_filename,          Rainfall_Filename_DESC_TEXT,            "",  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Relative_Humidity_Filename",  &climate_params.climate_relativehumidity_filename,  Relative_Humidity_Filename_DESC_TEXT,   "",  nullptr, nullptr, &dset_clim06);

        initConfigTypeMap("Base_Air_Temperature",    &climate_params.base_airtemperature,   Base_Air_Temperature_DESC_TEXT,    -55.0f,  45.0f, 22.0f,  nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Base_Land_Temperature",   &climate_params.base_landtemperature,  Base_Land_Temperature_DESC_TEXT,   -55.0f,  60.0f, 26.0f,  nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Base_Rainfall",           &climate_params.base_rainfall,         Base_Rainfall_DESC_TEXT,             0.0f, 150.0f, 10.0f,  nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Base_Relative_Humidity",  &climate_params.base_humidity,         Base_Relative_Humidity_DESC_TEXT,    0.0f,   1.0f,  0.75f, nullptr, nullptr, &dset_clim07);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        // Normalize rainfall; config param in mm, need value in m for calculations
        climate_params.base_rainfall /= MILLIMETERS_PER_METER;


        return bRet;
    }

    const ClimateParams* ClimateConfig::GetClimateParams()
    {
        return &climate_params;
    }



    // NodeConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(NodeConfig,NodeConfig)
    BEGIN_QUERY_INTERFACE_BODY(NodeConfig)
    END_QUERY_INTERFACE_BODY(NodeConfig)

    bool NodeConfig::Configure(const Configuration* config)
    {
        // Node parameter dependencies
        const std::map<std::string, std::string> dset_hint     {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","GENERIC_SIM,STI_SIM,HIV_SIM,AIRBORNE_SIM,TBHIV_SIM,PY_SIM,ENVIRONMENTAL_SIM,TYPHOID_SIM,POLIO_SIM"}};
        const std::map<std::string, std::string> dset_iprev    {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","GENERIC_SIM,VECTOR_SIM,STI_SIM,ENVIRONMENTAL_SIM,MALARIA_SIM"}};

        const std::map<std::string, std::string> dset_env01    {{"Simulation_Type","ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM"}};

        const std::map<std::string, std::string> dset_polio01  {{"Simulation_Type","POLIO_SIM"}};
        const std::map<std::string, std::string> dset_polio02  {{"Simulation_Type","POLIO_SIM"},{"Enable_Susceptibility_Scaling", "1"}};

        const std::map<std::string, std::string> dset_vec01    {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"}};
        const std::map<std::string, std::string> dset_vec02    {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"},{"Vector_Sampling_Type", "SAMPLE_IND_VECTORS"}};

        const std::map<std::string, std::string> dset_samp01   {{"Individual_Sampling_Type","FIXED_SAMPLING,ADAPTED_SAMPLING_BY_IMMUNE_STATE"}};
        const std::map<std::string, std::string> dset_samp02   {{"Individual_Sampling_Type","ADAPTED_SAMPLING_BY_IMMUNE_STATE"}};
        const std::map<std::string, std::string> dset_samp03   {{"Individual_Sampling_Type","ADAPTED_SAMPLING_BY_AGE_GROUP,ADAPTED_SAMPLING_BY_AGE_GROUP_AND_POP_SIZE"}};
        const std::map<std::string, std::string> dset_samp04   {{"Individual_Sampling_Type","ADAPTED_SAMPLING_BY_POPULATION_SIZE,ADAPTED_SAMPLING_BY_AGE_GROUP_AND_POP_SIZE"}};

        // Node parameters
        initConfig("Individual_Sampling_Type", node_params.ind_sampling_type, config, MetadataDescriptor::Enum("Individual_Sampling_Type", Individual_Sampling_Type_DESC_TEXT, MDD_ENUM_ARGS(IndSamplingType)));

        initConfigTypeMap("Enable_Heterogeneous_Intranode_Transmission",  &node_params.enable_hint,                Enable_Heterogeneous_Intranode_Transmission_DESC_TEXT,  false,  nullptr,  nullptr,  &dset_hint);
        initConfigTypeMap("Enable_Initial_Prevalence",                    &node_params.enable_initial_prevalence,  Enable_Initial_Prevalence_DESC_TEXT,                    false,  nullptr,  nullptr,  &dset_iprev);

        initConfigTypeMap("Enable_Susceptibility_Scaling",                &node_params.susceptibility_scaling,     Enable_Susceptibility_Scaling_DESC_TEXT,                false,  nullptr,  nullptr,  &dset_polio01);

        initConfigTypeMap("Enable_Vector_Mortality",                      &node_params.vector_mortality,           Enable_Vector_Mortality_DESC_TEXT,                       true,  nullptr,  nullptr,  &dset_vec01);

        initConfigTypeMap("Environmental_Cutoff_Days",          &node_params.environmental_cutoff_days,         Environmental_Cutoff_Days_DESC_TEXT,            0.0f,  DAYSPERYEAR,     2.0f,  nullptr,  nullptr,  &dset_env01);
        initConfigTypeMap("Environmental_Peak_Start",           &node_params.environmental_peak_start,          Environmental_Peak_Start_DESC_TEXT,             0.0f,       500.0f,     2.0f,  nullptr,  nullptr,  &dset_env01);
        initConfigTypeMap("Environmental_Ramp_Down_Duration",   &node_params.environmental_ramp_down_duration,  Environmental_Ramp_Down_Duration_DESC_TEXT,  FLT_MIN,  DAYSPERYEAR,     2.0f,  nullptr,  nullptr,  &dset_env01);
        initConfigTypeMap("Environmental_Ramp_Up_Duration",     &node_params.environmental_ramp_up_duration,    Environmental_Ramp_Up_Duration_DESC_TEXT,    FLT_MIN,  DAYSPERYEAR,     2.0f,  nullptr,  nullptr,  &dset_env01);
        initConfigTypeMap("Node_Contagion_Decay_Rate",          &node_params.node_contagion_decay_fraction,     Node_Contagion_Decay_Rate_DESC_TEXT,            0.0f,         1.0f,     1.0f,  nullptr,  nullptr,  &dset_env01);

        initConfigTypeMap("Susceptibility_Scaling_Rate",        &node_params.susceptibility_scaling_rate,       Susceptibility_Scaling_Rate_DESC_TEXT,          0.0f,      FLT_MAX,     0.0f,  nullptr,  nullptr,  &dset_polio02);

        initConfigTypeMap( "Base_Individual_Sample_Rate",       &node_params.base_sample_rate,                  Base_Individual_Sample_Rate_DESC_TEXT,          0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp01);
        initConfigTypeMap( "Relative_Sample_Rate_Immune",       &node_params.rel_sample_rate_immune,            Relative_Sample_Rate_Immune_DESC_TEXT,          0.001f,       1.0f,     0.1f,  nullptr,  nullptr,  &dset_samp02);
        initConfigTypeMap( "Immune_Threshold_For_Downsampling", &node_params.immune_threshold_for_downsampling, Immune_Threshold_For_Downsampling_DESC_TEXT,    0.0f,         1.0f,     0.0f,  nullptr,  nullptr,  &dset_samp02);
        initConfigTypeMap( "Immune_Downsample_Min_Age",         &node_params.immune_downsample_min_age,         Immune_Downsample_Min_Age_DESC_TEXT,            0.0f,      FLT_MAX,     0.0f,  nullptr,  nullptr,  &dset_samp02);
        initConfigTypeMap( "Sample_Rate_Birth",                 &node_params.sample_rate_birth,                 Sample_Rate_Birth_DESC_TEXT,                    0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Sample_Rate_0_18mo",                &node_params.sample_rate_0_18mo,                Sample_Rate_0_18mo_DESC_TEXT,                   0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Sample_Rate_18mo_4yr",              &node_params.sample_rate_18mo_4yr,              Sample_Rate_18mo_4yr_DESC_TEXT,                 0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Sample_Rate_5_9",                   &node_params.sample_rate_5_9,                   Sample_Rate_5_9_DESC_TEXT,                      0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Sample_Rate_10_14",                 &node_params.sample_rate_10_14,                 Sample_Rate_10_14_DESC_TEXT,                    0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Sample_Rate_15_19",                 &node_params.sample_rate_15_19,                 Sample_Rate_15_19_DESC_TEXT,                    0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Sample_Rate_20_Plus",               &node_params.sample_rate_20_plus,               Sample_Rate_20_plus_DESC_TEXT,                  0.001f,       1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap( "Max_Node_Population_Samples",       &node_params.max_sampling_cell_pop,             Max_Node_Population_Samples_DESC_TEXT,          1.0f,      FLT_MAX,    30.0f,  nullptr,  nullptr,  &dset_samp04);

        initConfigTypeMap( "x_Base_Population",                 &node_params.population_scaling_factor,         x_Base_Population_DESC_TEXT,                    0.0f,      FLT_MAX,     1.0f); 

        initConfigTypeMap("Mosquito_Weight",           &node_params.mosquito_weight,        Mosquito_Weight_DESC_TEXT,     1, 10000,     1,  nullptr,  nullptr,  &dset_vec02);

        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        return bRet;
    }

    const NodeParams* NodeConfig::GetNodeParams()
    {
        return &node_params;
    }



    // MigrationConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(MigrationConfig,MigrationConfig)
    BEGIN_QUERY_INTERFACE_BODY(MigrationConfig)
    END_QUERY_INTERFACE_BODY(MigrationConfig)

    bool MigrationConfig::Configure(const Configuration* config)
    {
        // Migration parameter dependencies
        const std::map<std::string, std::string> dset_mig00      {{"Migration_Model","FIXED_RATE_MIGRATION"}};
        const std::map<std::string, std::string> dset_mig01a     {{"Migration_Model","FIXED_RATE_MIGRATION"}};
        const std::map<std::string, std::string> dset_mig01b     {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"}};
        const std::map<std::string, std::string> dset_mig02a     {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Local_Migration","1"}};
        const std::map<std::string, std::string> dset_mig02b     {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Enable_Local_Migration","1"}};
        const std::map<std::string, std::string> dset_mig03      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Enable_Air_Migration","1"}};
        const std::map<std::string, std::string> dset_mig04      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Enable_Regional_Migration","1"}};
        const std::map<std::string, std::string> dset_mig05      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Enable_Sea_Migration","1"}};
        const std::map<std::string, std::string> dset_mig06      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Enable_Family_Migration","1"}};
        const std::map<std::string, std::string> dset_mig07      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Migration_Pattern", "SINGLE_ROUND_TRIPS"},{"Enable_Local_Migration","1"}};
        const std::map<std::string, std::string> dset_mig08      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Migration_Pattern","SINGLE_ROUND_TRIPS"},{"Enable_Air_Migration","1"}};
        const std::map<std::string, std::string> dset_mig09      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Migration_Pattern","SINGLE_ROUND_TRIPS"},{"Enable_Regional_Migration","1"}};
        const std::map<std::string, std::string> dset_mig10      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Migration_Pattern","SINGLE_ROUND_TRIPS"},{"Enable_Sea_Migration","1"}};
        const std::map<std::string, std::string> dset_mig11      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Enable_Demographics_Builtin","0"},{"Migration_Pattern","SINGLE_ROUND_TRIPS"},{"Enable_Family_Migration","1"}};
        const std::map<std::string, std::string> dset_mig12      {{"Migration_Model","FIXED_RATE_MIGRATION"},{"Migration_Pattern", "WAYPOINTS_HOME"}};

        const std::map<std::string, std::string> dset_mig_vec01  {{"Simulation_Type","VECTOR_SIM,DENGUE_SIM,MALARIA_SIM"}};
        const std::map<std::string, std::string> dset_mig_vec02a {{"Simulation_Type","VECTOR_SIM,DENGUE_SIM,MALARIA_SIM"},{"Enable_Vector_Migration","1"}};
        const std::map<std::string, std::string> dset_mig_vec02b {{"Simulation_Type","VECTOR_SIM,DENGUE_SIM,MALARIA_SIM"},{"Enable_Demographics_Builtin","0"},{"Enable_Vector_Migration","1"}};
        const std::map<std::string, std::string> dset_mig_vec03a {{"Simulation_Type","VECTOR_SIM,DENGUE_SIM,MALARIA_SIM"},{"Enable_Vector_Migration","1"},{"Enable_Vector_Migration_Local","1"}};
        const std::map<std::string, std::string> dset_mig_vec03b {{"Simulation_Type","VECTOR_SIM,DENGUE_SIM,MALARIA_SIM"},{"Enable_Demographics_Builtin","0"},{"Enable_Vector_Migration","1"},{"Enable_Vector_Migration_Local","1"}};
        const std::map<std::string, std::string> dset_mig_vec04  {{"Simulation_Type","VECTOR_SIM,DENGUE_SIM,MALARIA_SIM"},{"Enable_Demographics_Builtin","0"},{"Enable_Vector_Migration","1"},{"Enable_Vector_Migration_Regional","1"}};

        // Migration parameters
        initConfig("Migration_Model",   migration_params.migration_structure,  config, MetadataDescriptor::Enum("Migration_Model",   Migration_Model_DESC_TEXT,   MDD_ENUM_ARGS(MigrationStructure)));
        initConfig("Migration_Pattern", migration_params.migration_pattern,    config, MetadataDescriptor::Enum("Migration_Pattern", Migration_Pattern_DESC_TEXT, MDD_ENUM_ARGS(MigrationPattern)), nullptr, nullptr, &dset_mig00);

        initConfig("Vector_Migration_Modifier_Equation", migration_params.vec_mod_equ, config, MetadataDescriptor::Enum("Vector_Migration_Modifier_Equation", Vector_Migration_Modifier_Equation_DESC_TEXT, MDD_ENUM_ARGS(ModiferEquationType)), nullptr, nullptr, &dset_mig_vec02b); 

        initConfigTypeMap("Roundtrip_Waypoints", &migration_params.roundtrip_waypoints, Roundtrip_Waypoints_DESC_TEXT, 0, 1000, 10, nullptr, nullptr, &dset_mig12);

        initConfigTypeMap("Local_Migration_Roundtrip_Probability",     &migration_params.local_roundtrip_prob,   Local_Migration_Roundtrip_Probability_DESC_TEXT,     0.0f, 1.0f, 0.95f, nullptr, nullptr, &dset_mig07);
        initConfigTypeMap("Air_Migration_Roundtrip_Probability",       &migration_params.air_roundtrip_prob,     Air_Migration_Roundtrip_Probability_DESC_TEXT,       0.0f, 1.0f, 0.80f, nullptr, nullptr, &dset_mig08);
        initConfigTypeMap("Regional_Migration_Roundtrip_Probability",  &migration_params.region_roundtrip_prob,  Regional_Migration_Roundtrip_Probability_DESC_TEXT,  0.0f, 1.0f, 0.10f, nullptr, nullptr, &dset_mig09);
        initConfigTypeMap("Sea_Migration_Roundtrip_Probability",       &migration_params.sea_roundtrip_prob,     Sea_Migration_Roundtrip_Probability_DESC_TEXT,       0.0f, 1.0f, 0.25f, nullptr, nullptr, &dset_mig10);

        initConfigTypeMap("Local_Migration_Roundtrip_Duration",     &migration_params.local_roundtrip_duration,   Local_Migration_Roundtrip_Duration_DESC_TEXT,     0.0f, 10000.0f, 1.0f, nullptr, nullptr, &dset_mig07);
        initConfigTypeMap("Air_Migration_Roundtrip_Duration",       &migration_params.air_roundtrip_duration,     Air_Migration_Roundtrip_Duration_DESC_TEXT,       0.0f, 10000.0f, 1.0f, nullptr, nullptr, &dset_mig08);
        initConfigTypeMap("Regional_Migration_Roundtrip_Duration",  &migration_params.region_roundtrip_duration,  Regional_Migration_Roundtrip_Duration_DESC_TEXT,  0.0f, 10000.0f, 1.0f, nullptr, nullptr, &dset_mig09);
        initConfigTypeMap("Sea_Migration_Roundtrip_Duration",       &migration_params.sea_roundtrip_duration,     Sea_Migration_Roundtrip_Duration_DESC_TEXT,       0.0f, 10000.0f, 1.0f, nullptr, nullptr, &dset_mig10);
        initConfigTypeMap("Family_Migration_Roundtrip_Duration",    &migration_params.family_roundtrip_duration,  Family_Migration_Roundtrip_Duration_DESC_TEXT,    0.0f, 10000.0f, 1.0f, nullptr, nullptr, &dset_mig11);

        initConfigTypeMap("Enable_Local_Migration",             &migration_params.enable_mig_local,        Enable_Local_Migration_DESC_TEXT,           false, nullptr, nullptr, &dset_mig01a);
        initConfigTypeMap("Enable_Air_Migration",               &migration_params.enable_mig_air,          Enable_Air_Migration_DESC_TEXT,             false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Regional_Migration",          &migration_params.enable_mig_regional,     Enable_Regional_Migration_DESC_TEXT,        false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Sea_Migration",               &migration_params.enable_mig_sea,          Enable_Sea_Migration_DESC_TEXT,             false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Family_Migration",            &migration_params.enable_mig_family,       Enable_Family_Migration_DESC_TEXT,          false, nullptr, nullptr, &dset_mig01b);

        initConfigTypeMap("Enable_Vector_Migration",            &migration_params.enable_mig_vec,          Enable_Vector_Migration_DESC_TEXT,          false, nullptr, nullptr, &dset_mig_vec01);
        initConfigTypeMap("Enable_Vector_Migration_Local",      &migration_params.enable_mig_vec_local,    Enable_Vector_Migration_Local_DESC_TEXT,    false, nullptr, nullptr, &dset_mig_vec02a);
        initConfigTypeMap("Enable_Vector_Migration_Regional",   &migration_params.enable_mig_vec_regional, Enable_Vector_Migration_Regional_DESC_TEXT, false, nullptr, nullptr, &dset_mig_vec02b); 

        initConfigTypeMap("Local_Migration_Filename",           &migration_params.mig_file_local,        Local_Migration_Filename_DESC_TEXT,           "", nullptr, nullptr, &dset_mig02b);
        initConfigTypeMap("Air_Migration_Filename",             &migration_params.mig_file_air,          Air_Migration_Filename_DESC_TEXT,             "", nullptr, nullptr, &dset_mig03);
        initConfigTypeMap("Regional_Migration_Filename",        &migration_params.mig_file_regional,     Regional_Migration_Filename_DESC_TEXT,        "", nullptr, nullptr, &dset_mig04);
        initConfigTypeMap("Sea_Migration_Filename",             &migration_params.mig_file_sea,          Sea_Migration_Filename_DESC_TEXT,             "", nullptr, nullptr, &dset_mig05);
        initConfigTypeMap("Family_Migration_Filename",          &migration_params.mig_file_family,       Family_Migration_Filename_DESC_TEXT,          "", nullptr, nullptr, &dset_mig06);

        initConfigTypeMap("Vector_Migration_Filename_Local",    &migration_params.mig_file_vec_local,    Vector_Migration_Filename_Local_DESC_TEXT,    "", nullptr, nullptr, &dset_mig_vec03b);
        initConfigTypeMap("Vector_Migration_Filename_Regional", &migration_params.mig_file_vec_regional, Vector_Migration_Filename_Regional_DESC_TEXT, "", nullptr, nullptr, &dset_mig_vec04);

        initConfigTypeMap("x_Local_Migration",            &migration_params.mig_mult_local,        x_Local_Migration_DESC_TEXT,           0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig02a);
        initConfigTypeMap("x_Air_Migration",              &migration_params.mig_mult_air,          x_Air_Migration_DESC_TEXT,             0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig03);
        initConfigTypeMap("x_Regional_Migration",         &migration_params.mig_mult_regional,     x_Regional_Migration_DESC_TEXT,        0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig04);
        initConfigTypeMap("x_Sea_Migration",              &migration_params.mig_mult_sea,          x_Sea_Migration_DESC_TEXT,             0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig05);
        initConfigTypeMap("x_Family_Migration",           &migration_params.mig_mult_family,       x_Family_Migration_DESC_TEXT,          0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig06);

        initConfigTypeMap("x_Vector_Migration_Local",     &migration_params.mig_mult_vec_local,    x_Vector_Migration_Local_DESC_TEXT,    0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig_vec03a);
        initConfigTypeMap("x_Vector_Migration_Regional",  &migration_params.mig_mult_vec_regional, x_Vector_Migration_Regional_DESC_TEXT, 0.0f, FLT_MAX, 1.0f, nullptr, nullptr, &dset_mig_vec04);

        initConfigTypeMap("Vector_Migration_Habitat_Modifier",  &migration_params.vec_mod_habitat, Vector_Migration_Habitat_Modifier_DESC_TEXT,  0.0f, FLT_MAX, 0.0f, nullptr, nullptr, &dset_mig_vec02b);
        initConfigTypeMap("Vector_Migration_Food_Modifier",     &migration_params.vec_mod_food,    Vector_Migration_Food_Modifier_DESC_TEXT,     0.0f, FLT_MAX, 0.0f, nullptr, nullptr, &dset_mig_vec02b);
        initConfigTypeMap("Vector_Migration_Stay_Put_Modifier", &migration_params.vec_mod_stayput, Vector_Migration_Stay_Put_Modifier_DESC_TEXT, 0.0f, FLT_MAX, 0.0f, nullptr, nullptr, &dset_mig_vec02b);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        // Set fixed values
        if(migration_params.migration_pattern == MigrationPattern::RANDOM_WALK_DIFFUSION)
        {
            migration_params.local_roundtrip_prob      = 0.0f;
            migration_params.air_roundtrip_prob        = 0.0f;
            migration_params.region_roundtrip_prob     = 0.0f;
            migration_params.sea_roundtrip_prob        = 0.0f;
        }

        if(migration_params.migration_pattern == MigrationPattern::SINGLE_ROUND_TRIPS)
        {
            migration_params.roundtrip_waypoints = 1;
        }

        if(migration_params.migration_pattern == MigrationPattern::WAYPOINTS_HOME)
        {
            migration_params.local_roundtrip_prob      = 1.0f;
            migration_params.air_roundtrip_prob        = 1.0f;
            migration_params.region_roundtrip_prob     = 1.0f;
            migration_params.sea_roundtrip_prob        = 1.0f;
        }


        return bRet;
    }

    const MigrationParams* MigrationConfig::GetMigrationParams()
    {
        return &migration_params;
    }



    // SimConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(SimConfig,SimConfig)
    BEGIN_QUERY_INTERFACE_BODY(SimConfig)
    END_QUERY_INTERFACE_BODY(SimConfig)

    bool SimConfig::Configure(const Configuration* config)
    {
        // Sim parameter dependencies

        // Sim parameters
        initConfigTypeMap("Enable_Interventions",                        &sim_params.enable_interventions,   Enable_Interventions_DESC_TEXT,                        false);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        return bRet;
    }

    const SimParams* SimConfig::GetSimParams()
    {
        return &sim_params;
    }

}
