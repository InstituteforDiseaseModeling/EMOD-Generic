/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Common.h"
#include "Configure.h"
#include "SimulationEnums.h"


namespace Kernel
{
    struct ClimateParams
    {
    public:
        ClimateParams();

        ClimateStructure::Enum          climate_structure;
        ClimateUpdateResolution::Enum   climate_update_resolution;

        std::string climate_airtemperature_filename;
        std::string climate_koppen_filename;
        std::string climate_landtemperature_filename;
        std::string climate_rainfall_filename;
        std::string climate_relativehumidity_filename;

        bool enable_climate_stochasticity;
        bool rainfall_variance_enabled;

        float airtemperature_offset;
        float airtemperature_variance;
        float base_airtemperature;
        float base_humidity;
        float base_landtemperature;
        float base_rainfall;
        float humidity_scale_factor;
        float humidity_variance;
        float landtemperature_offset;
        float landtemperature_variance;
        float rainfall_scale_factor;
    };

    struct NodeParams
    {
    public:
        NodeParams();

        IndSamplingType::Enum   ind_sampling_type;

        bool enable_hint;
        bool enable_initial_prevalence;
        bool susceptibility_scaling;
        bool vector_mortality;

        float base_sample_rate;
        float environmental_cutoff_days;
        float environmental_peak_start;
        float environmental_ramp_down_duration;
        float environmental_ramp_up_duration;
        float immune_downsample_min_age;
        float immune_threshold_for_downsampling;
        float max_sampling_cell_pop;
        float node_contagion_decay_fraction;
        float population_scaling_factor;
        float rel_sample_rate_immune;
        float sample_rate_0_18mo;
        float sample_rate_10_14;
        float sample_rate_15_19;
        float sample_rate_18mo_4yr;
        float sample_rate_20_plus;
        float sample_rate_5_9;
        float sample_rate_birth;
        float susceptibility_scaling_rate;

        int32_t mosquito_weight;
    };

    struct MigrationParams
    {
    public:
        MigrationParams();

        MigrationPattern::Enum      migration_pattern;
        MigrationStructure::Enum    migration_structure;
        ModiferEquationType::Enum   vec_mod_equ;

        std::string mig_file_air;
        std::string mig_file_family;
        std::string mig_file_local;
        std::string mig_file_regional;
        std::string mig_file_sea;
        std::string mig_file_vec_local;
        std::string mig_file_vec_regional;

        bool enable_mig_air;
        bool enable_mig_family;
        bool enable_mig_local;
        bool enable_mig_regional;
        bool enable_mig_sea;
        bool enable_mig_vec;
        bool enable_mig_vec_local;
        bool enable_mig_vec_regional;

        float air_roundtrip_duration;
        float air_roundtrip_prob;
        float local_roundtrip_duration;
        float local_roundtrip_prob;
        float family_roundtrip_duration;
        float mig_mult_air;
        float mig_mult_family;
        float mig_mult_local;
        float mig_mult_regional;
        float mig_mult_sea;
        float mig_mult_vec_local;
        float mig_mult_vec_regional;
        float region_roundtrip_duration;
        float region_roundtrip_prob;
        float sea_roundtrip_duration;
        float sea_roundtrip_prob;
        float vec_mod_habitat;
        float vec_mod_food;
        float vec_mod_stayput;

        int roundtrip_waypoints;
    };

    struct SimParams
    {
    public:
        SimParams();

        bool enable_interventions;
    };



    class ConfigParams
    {
    public:
        bool Configure(const Configuration* config);
    };



    class ClimateConfig : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(ClimateConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const ClimateParams*     GetClimateParams();

    protected:
        static       ClimateParams      climate_params;
    };



    class NodeConfig : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(NodeConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const NodeParams*        GetNodeParams();

    protected:
        static       NodeParams         node_params;
    };



    class MigrationConfig : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(MigrationConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const MigrationParams*   GetMigrationParams();

    protected:
        static       MigrationParams    migration_params;
    };



    class SimConfig : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(SimConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const SimParams*   GetSimParams();

    protected:
        static       SimParams    sim_params;
    };

}
