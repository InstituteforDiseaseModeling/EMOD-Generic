/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Common.h"
#include "Configure.h"
#include "SimulationEnums.h"
#include "StrainIdentity.h"
#include "TBHIVDrugTypeParameters.h"
#include "VectorEnums.h"


namespace Kernel
{
    // ConfigParams is a convenience wrapper for initializing other configuration parameter classes.
    // Each of the other parameter classes has a static struct with themed-parameters; the ConfigParams
    // class itself has no static members / parameters and doesn't inherit from JsonConfigurable.
    class ConfigParams
    {
    public:
        bool Configure(Configuration* config);
    };



    struct AgentParams
    {
    public:
        AgentParams();

        bool  enable_genome_dependent_infectivity;
        bool  enable_genome_mutation;
        bool  enable_label_infector;
        bool  enable_label_mutator;
        bool  enable_nonuniform_shedding;
        bool  enable_strain_tracking;

        std::vector<bool>   genome_mutations_labeled;

        float correlation_acq_trans;
        float shedding_dist_alpha;
        float shedding_dist_beta;
        float symptomatic_infectious_offset;

        std::vector<float>  genome_infectivity_multipliers;
        std::vector<float>  genome_mutation_rates;
        std::vector<float>  shedding_beta_pdf_hash;
    };



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



    struct NodeParams
    {
    public:
        NodeParams();

        DistributionType::Enum       age_init_dist_type;
        IndSamplingType::Enum        ind_sampling_type;
        DistributionType::Enum       initial_sus_dist_type;
        VectorSamplingType::Enum     vector_sampling_type;
        VitalBirthDependence::Enum   vital_birth_dependence;
        VitalDeathDependence::Enum   vital_death_dependence;

        bool enable_acquisition_heterogeneity;
        bool enable_birth;
        bool enable_demographics_risk;
        bool enable_hint;
        bool enable_infectivity_overdispersion;
        bool enable_infectivity_reservoir;
        bool enable_infectivity_scaling;
        bool enable_initial_prevalence;
        bool enable_initial_sus_dist;
        bool enable_maternal_infect_trans;
        bool enable_natural_mortality;
        bool enable_percentage_children;
        bool enable_vital_dynamics;
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
        float min_sampling_cell_pop;
        float node_contagion_decay_fraction;
        float population_scaling_factor;
        float prob_maternal_infection_trans;
        float rel_sample_rate_immune;
        float sample_rate_0_18mo;
        float sample_rate_10_14;
        float sample_rate_15_19;
        float sample_rate_18mo_4yr;
        float sample_rate_20_plus;
        float sample_rate_5_9;
        float sample_rate_birth;
        float susceptibility_scaling_rate;
        float x_birth;
        float x_othermortality;

         int32_t mosquito_weight;
        uint32_t number_clades;
        uint64_t number_genomes;
    };



    struct PolioParams
    {
    public:
        PolioParams();

        EvolutionPolioClockType::Enum    evolution_polio_clock_type;   // evolution_polio_clock_type
        VDPVVirulenceModelType::Enum     VDPV_virulence_model_type;    // VDPV_virulence_model_type

        float boostLog2NAb_IPV1;                     // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_IPV2;                     // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_IPV3;                     // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_OPV1;                     // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_OPV2;                     // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_OPV3;                     // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_IPV1;              // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_IPV2;              // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_IPV3;              // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_OPV1;              // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_OPV2;              // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_OPV3;              // (dimensionless) multiplication of antibody titer in response to challenge infection
        float decayRatePassiveImmunity;              // (1/days) exponential decay rate
        float excrement_load;                        // (grams/day) feces
        float Incubation_Disease_Mu;                 // paralysis incubation period, lognormal parameter mu
        float Incubation_Disease_Sigma;              // paralysis incubation period, lognormal parameter sigma
        float maternalAbHalfLife;
        float maternal_log2NAb_mean;
        float maternal_log2NAb_std;
        float maxLog2NAb_IPV1;                       // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_IPV2;                       // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_IPV3;                       // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_OPV1;                       // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_OPV2;                       // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_OPV3;                       // (dimensionless) multiplication of antibody titer in response to challenge infection
        float MaxRandStdDev;                         // (dimensionless) limit to effect of a random normal
        float mucosalImmIPV;                         // (dimensionless, OPV mucosal / IPV mucosal) relative mucosal immunogenicity
        float mucosalImmIPVOPVExposed;               // (dimensionless, OPV mucosal / IPV mucosal) relative mucosal immunogenicity on OPV-exposed individuals
        float paralysis_base_rate_S1;                // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float paralysis_base_rate_S2;                // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float paralysis_base_rate_S3;                // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float paralysis_base_rate_W1;                // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float paralysis_base_rate_W2;                // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float paralysis_base_rate_W3;                // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float primeLog2NAb_OPV1;                     // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_OPV2;                     // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_OPV3;                     // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_IPV1;                     // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_IPV2;                     // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_IPV3;                     // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_stddev_IPV1;              // (log2 reciprocal titer) standard deviation antibody level at first infection
        float primeLog2NAb_stddev_IPV2;              // (log2 reciprocal titer) standard deviation antibody level at first infection
        float primeLog2NAb_stddev_IPV3;              // (log2 reciprocal titer) standard deviation antibody level at first infection
        float primeLog2NAb_stddev_OPV1;              // (log2 reciprocal titer) standard deviation antibody level at first infection
        float primeLog2NAb_stddev_OPV2;              // (log2 reciprocal titer) standard deviation antibody level at first infection
        float primeLog2NAb_stddev_OPV3;              // (log2 reciprocal titer) standard deviation antibody level at first infection
        float PVinf0_S1;                             // (dimensionless) probability of infection from single virion
        float PVinf0_S2;                             // (dimensionless) probability of infection from single virion
        float PVinf0_S3;                             // (dimensionless) probability of infection from single virion
        float PVinf0_W1;                             // (dimensionless) probability of infection from single virion
        float PVinf0_W2;                             // (dimensionless) probability of infection from single virion
        float PVinf0_W3;                             // (dimensionless) probability of infection from single virion
        float shedFecalDurationBlockLog2NAb;         // (log2 Nab)
        float shedFecalMaxLog10PeakTiter;            // (log10 TCID50)
        float shedFecalMaxLog10PeakTiter_Stddev;     // (log10 TCID50)
        float shedFecalMaxLnDuration;                // (Ln time)
        float shedFecalMaxLnDuration_Stddev;         // (Ln time)
        float shedFecalTiterBlockLog2NAb;            // (log2 NAb)
        float shedFecalTiterProfile_mu;              // (Ln time)
        float shedFecalTiterProfile_sigma;           // (Ln time)
        float shedOralDurationBlockLog2NAb;          // (log2 NAb)
        float shedOralMaxLog10PeakTiter;             // (log10 TCID50)
        float shedOralMaxLog10PeakTiter_Stddev;      // (log10 TCID50)
        float shedOralMaxLnDuration;                 // (Ln time)
        float shedOralMaxLnDuration_Stddev;          // (Ln time)
        float shedOralTiterBlockLog2NAb;             // (log2 NAb)
        float shedOralTiterProfile_mu;               // (Ln time)
        float shedOralTiterProfile_sigma;            // (Ln time)
        float TauNAb;                                // (days) neutralization time constant
        float vaccine_Dantigen_IPV1;                 // (D-antigen units) antigen content of each serotype
        float vaccine_Dantigen_IPV2;                 // (D-antigen units) antigen content of each serotype
        float vaccine_Dantigen_IPV3;                 // (D-antigen units) antigen content of each serotype
        float vaccine_take_multiplier_S1;            // counts for the host factors of vaccine take
        float vaccine_take_multiplier_S2;            // counts for the host factors of vaccine take
        float vaccine_take_multiplier_S3;            // counts for the host factors of vaccine take
        float vaccine_titer_bOPV1;                   // (TCID50) viral dose of serotype 1
        float vaccine_titer_bOPV3;                   // (TCID50) viral dose of serotype 3
        float vaccine_titer_mOPV1;                   // (TCID50) viral dose of each serotype
        float vaccine_titer_mOPV2;                   // (TCID50) viral dose of each serotype
        float vaccine_titer_mOPV3;                   // (TCID50) viral dose of each serotype
        float vaccine_titer_tOPV1;                   // (TCID50) viral dose of each serotype
        float vaccine_titer_tOPV2;                   // (TCID50) viral dose of each serotype
        float vaccine_titer_tOPV3;                   // (TCID50) viral dose of each serotype
        float viral_interference_S1;                 // (dimensionless) probability that infection will prevent infection of any heterologous serotype
        float viral_interference_S2;                 // (dimensionless) probability that infection will prevent infection of any heterologous serotype
        float viral_interference_S3;                 // (dimensionless) probability that infection will prevent infection of any heterologous serotype
        float viral_interference_W1;                 // (dimensionless) probability that infection will prevent infection of any heterologous serotype
        float viral_interference_W2;                 // (dimensionless) probability that infection will prevent infection of any heterologous serotype
        float viral_interference_W3;                 // (dimensionless) probability that infection will prevent infection of any heterologous serotype

        std::vector<float> genomeRelativeInfectivity1;
        std::vector<float> genomeRelativeInfectivity2;
        std::vector<float> genomeRelativeInfectivity3;
        std::vector<float> Sabin1_Site_Rates;
        std::vector<float> Sabin2_Site_Rates;
        std::vector<float> Sabin3_Site_Rates;

        int reversionSteps_cVDPV1;                   // (bits) number of mutation steps to revert from Sabin to cVDPV, must be <= number_genomes
        int reversionSteps_cVDPV2;                   // (bits) number of mutation steps to revert from Sabin to cVDPV, must be <= number_genomes
        int reversionSteps_cVDPV3;                   // (bits) number of mutation steps to revert from Sabin to cVDPV, must be <= number_genomes
        int vaccine_genome_OPV1;
        int vaccine_genome_OPV2;
        int vaccine_genome_OPV3;

        StrainIdentity  vaccine_strain1;
        StrainIdentity  vaccine_strain2;
        StrainIdentity  vaccine_strain3;
    };



    struct SimParams
    {
    public:
        SimParams();

        SimType::Enum sim_type;

        std::string campaign_filename;
        std::string loadbalance_filename;

        bool enable_coordinator_event_report;
        bool enable_default_report;
        bool enable_demographic_tracking;
        bool enable_event_db;
        bool enable_event_report;
        bool enable_interventions;
        bool enable_net_infect;
        bool enable_node_event_report;
        bool enable_property_output;
        bool enable_spatial_output;
        bool enable_surveillance_event_report;
        bool enable_termination_on_total_wall_time;
        bool enable_termination_on_zero_total_infectivity;

        float net_infect_max_frac;
        float net_infect_min_dist;
        float sim_time_base_year;
        float sim_time_delta;
        float sim_time_end_min;
        float sim_time_start;
        float sim_time_total;
        float wall_time_max_minutes;

        std::vector<float> net_infect_grav_coeff;
        std::vector<float> net_infect_grav_dpow;
    };



    struct TBHIVParams
    {
    public:
        TBHIVParams();

        TBHIVDrugCollection   drugs_map;
    };



// *****************************************************************************



    class AgentConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(AgentConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const AgentParams*     GetAgentParams();

    protected:
        static       AgentParams      agent_params;
    };



    class ClimateConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(ClimateConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const ClimateParams*     GetClimateParams();

    protected:
        static       ClimateParams      climate_params;
    };



    class MigrationConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(MigrationConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const MigrationParams*   GetMigrationParams();

    protected:
        static       MigrationParams    migration_params;
    };


    class NodeConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(NodeConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const NodeParams*        GetNodeParams();

    protected:
        static       NodeParams         node_params;
    };



    class PolioConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(PolioConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const PolioParams*       GetPolioParams();

    protected:
        static       PolioParams         polio_params;
    };



    class SimConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(SimConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const SimParams*   GetSimParams();

    protected:
        static       SimParams    sim_params;
    };



    class TBHIVConfig : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(TBHIVConfig)

    public:
        virtual bool Configure(const Configuration* config) override;

        static const TBHIVParams*   GetTBHIVParams();

    protected:
        static       TBHIVParams    tbhiv_params;
    };
}

// *****************************************************************************