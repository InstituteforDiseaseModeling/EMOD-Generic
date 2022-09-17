/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <cmath>

#include "stdafx.h"
#include "ConfigParams.h"



namespace Kernel
{
    // Static param structures
    AgentParams      AgentConfig::agent_params;
    ClimateParams    ClimateConfig::climate_params;
    MigrationParams  MigrationConfig::migration_params;
    NodeParams       NodeConfig::node_params;
    PolioParams      PolioConfig::polio_params;
    SimParams        SimConfig::sim_params;
    TBHIVParams      TBHIVConfig::tbhiv_params;



    // ConfigParams Methods
    bool ConfigParams::Configure(Configuration* config)
    {
        // Configs with static param structures
        AgentConfig        agent_config_obj;
        ClimateConfig      climate_config_obj;
        MigrationConfig    migration_config_obj;
        NodeConfig         node_config_obj;
        PolioConfig        polio_config_obj;
        SimConfig          sim_config_obj;
        TBHIVConfig        tbhiv_config_obj;


        // Set fixed parameters; would love to not need this section (KF)
        if(sim_config_obj.MatchesDependency(config, "Simulation_Type", "DENGUE_SIM"))
        {
            config->Add("Enable_Immune_Decay",                          1);
            config->Add("Enable_Immunity",                              1);
            config->Add("Enable_Maternal_Infection_Transmission",       1);
        }
        if(sim_config_obj.MatchesDependency(config, "Simulation_Type", "HIV_SIM"))
        {
            config->Add("Enable_Disease_Mortality",                     1);
            config->Add("Enable_Immunity",                              1);
            config->Add("Enable_Maternal_Infection_Transmission",       1);
            config->Add("Enable_Vital_Dynamics",                        1);
        }
        if(sim_config_obj.MatchesDependency(config, "Simulation_Type", "MALARIA_SIM"))
        {
            config->Add("Enable_Immune_Decay",                          1);
            config->Add("Enable_Immunity",                              1);
        }
        if(sim_config_obj.MatchesDependency(config, "Simulation_Type", "POLIO_SIM"))
        {
            config->Add("Enable_Environmental_Route",                   1);
            config->Add("Enable_Disease_Mortality",                     1);
            config->Add("Enable_Immune_Decay",                          1);
            config->Add("Enable_Immunity",                              1);
            config->Add("Enable_Initial_Susceptibility_Distribution",   1);
            config->Add("Enable_Superinfection",                        1);
        }


        // Process configuration
        bool bRet = true;

        bRet &= agent_config_obj.Configure(config);
        bRet &= climate_config_obj.Configure(config);
        bRet &= migration_config_obj.Configure(config);
        bRet &= node_config_obj.Configure(config);
        bRet &= polio_config_obj.Configure(config);
        bRet &= sim_config_obj.Configure(config);
        bRet &= tbhiv_config_obj.Configure(config);

        return bRet;
    }



    AgentParams::AgentParams()
        : enable_genome_dependent_infectivity(false)
        , enable_genome_mutation(false)
        , enable_label_infector(false)
        , enable_label_mutator(false)
        , enable_nonuniform_shedding(false)
        , enable_strain_tracking(false)
        , genome_mutations_labeled()
        , correlation_acq_trans(0.0f)
        , shedding_dist_alpha(0.0f)
        , shedding_dist_beta(0.0f)
        , symptomatic_infectious_offset(FLT_MAX)
        , genome_infectivity_multipliers()
        , genome_mutation_rates()
        , shedding_beta_pdf_hash(SHEDDING_HASH_SIZE,1.0f)
    {}

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

    NodeParams::NodeParams()
        : age_init_dist_type(DistributionType::DISTRIBUTION_OFF)
        , ind_sampling_type(IndSamplingType::TRACK_ALL)
        , initial_sus_dist_type(DistributionType::DISTRIBUTION_OFF)
        , vector_sampling_type(VectorSamplingType::TRACK_ALL_VECTORS)
        , vital_birth_dependence(VitalBirthDependence::FIXED_BIRTH_RATE)
        , vital_death_dependence(VitalDeathDependence::NONDISEASE_MORTALITY_BY_AGE_AND_GENDER)
        , enable_acquisition_heterogeneity(false)
        , enable_birth(false)
        , enable_demographics_risk(false)
        , enable_environmental_route(false)
        , enable_hint(false)
        , enable_infectivity_overdispersion(false)
        , enable_infectivity_reservoir(false)
        , enable_infectivity_scaling(false)
        , enable_initial_prevalence(false)
        , enable_initial_sus_dist(false)
        , enable_maternal_infect_trans(false)
        , enable_natural_mortality(false)
        , enable_percentage_children(false)
        , enable_vital_dynamics(false)
        , susceptibility_scaling(false)
        , vector_mortality(false)
        , base_sample_rate(0.0f)
        , immune_downsample_min_age(0.0f)
        , immune_threshold_for_downsampling(0.0f)
        , max_sampling_cell_pop(0.0f)
        , min_sampling_cell_pop(0.0f)
        , node_contagion_decay_fraction(0.0f)
        , population_scaling_factor(0.0f)
        , prob_maternal_infection_trans(0.0f)
        , rel_sample_rate_immune(0.0f)
        , sample_rate_0_18mo(0.0f)
        , sample_rate_10_14(0.0f)
        , sample_rate_15_19(0.0f)
        , sample_rate_18mo_4yr(0.0f)
        , sample_rate_20_plus(0.0f)
        , sample_rate_5_9(0.0f)
        , sample_rate_birth(0.0f)
        , susceptibility_scaling_rate(0.0f)
        , x_birth(0.0f)
        , x_othermortality(0.0f)
        , mosquito_weight(0)
        , number_clades(1)
        , number_genomes(1)
    {}

    PolioParams::PolioParams()
        : evolution_polio_clock_type(EvolutionPolioClockType::POLIO_EVOCLOCK_NONE)
        , VDPV_virulence_model_type(VDPVVirulenceModelType::POLIO_VDPV_NONVIRULENT)
        , boostLog2NAb_IPV1(0.0f)
        , boostLog2NAb_IPV2(0.0f)
        , boostLog2NAb_IPV3(0.0f)
        , boostLog2NAb_OPV1(0.0f)
        , boostLog2NAb_OPV2(0.0f)
        , boostLog2NAb_OPV3(0.0f)
        , boostLog2NAb_stddev_IPV1(0.0f)
        , boostLog2NAb_stddev_IPV2(0.0f)
        , boostLog2NAb_stddev_IPV3(0.0f)
        , boostLog2NAb_stddev_OPV1(0.0f)
        , boostLog2NAb_stddev_OPV2(0.0f)
        , boostLog2NAb_stddev_OPV3(0.0f)
        , decayRatePassiveImmunity(0.0f)
        , excrement_load(0.0f)
        , Incubation_Disease_Mu(0.0f)
        , Incubation_Disease_Sigma(0.0f)
        , maternalAbHalfLife(0.0f)
        , maternal_log2NAb_mean(0.0f)
        , maternal_log2NAb_std(0.0f)
        , maxLog2NAb_IPV1(0.0f)
        , maxLog2NAb_IPV2(0.0f)
        , maxLog2NAb_IPV3(0.0f)
        , maxLog2NAb_OPV1(0.0f)
        , maxLog2NAb_OPV2(0.0f)
        , maxLog2NAb_OPV3(0.0f)
        , MaxRandStdDev(0.0f)
        , mucosalImmIPV(0.0f)
        , mucosalImmIPVOPVExposed(0.0f)
        , paralysis_base_rate_S1(0.0f)
        , paralysis_base_rate_S2(0.0f)
        , paralysis_base_rate_S3(0.0f)
        , paralysis_base_rate_W1(0.0f)
        , paralysis_base_rate_W2(0.0f)
        , paralysis_base_rate_W3(0.0f)
        , primeLog2NAb_OPV1(0.0f)
        , primeLog2NAb_OPV2(0.0f)
        , primeLog2NAb_OPV3(0.0f)
        , primeLog2NAb_IPV1(0.0f)
        , primeLog2NAb_IPV2(0.0f)
        , primeLog2NAb_IPV3(0.0f)
        , primeLog2NAb_stddev_IPV1(0.0f)
        , primeLog2NAb_stddev_IPV2(0.0f)
        , primeLog2NAb_stddev_IPV3(0.0f)
        , primeLog2NAb_stddev_OPV1(0.0f)
        , primeLog2NAb_stddev_OPV2(0.0f)
        , primeLog2NAb_stddev_OPV3(0.0f)
        , PVinf0_S1(0.0f)
        , PVinf0_S2(0.0f)
        , PVinf0_S3(0.0f)
        , PVinf0_W1(0.0f)
        , PVinf0_W2(0.0f)
        , PVinf0_W3(0.0f)
        , shedFecalDurationBlockLog2NAb(0.0f)
        , shedFecalMaxLog10PeakTiter(0.0f)
        , shedFecalMaxLog10PeakTiter_Stddev(0.0f)
        , shedFecalMaxLnDuration(0.0f)
        , shedFecalMaxLnDuration_Stddev(0.0f)
        , shedFecalTiterBlockLog2NAb(0.0f)
        , shedFecalTiterProfile_mu(0.0f)
        , shedFecalTiterProfile_sigma(0.0f)
        , shedOralDurationBlockLog2NAb(0.0f)
        , shedOralMaxLog10PeakTiter(0.0f)
        , shedOralMaxLog10PeakTiter_Stddev(0.0f)
        , shedOralMaxLnDuration(0.0f)
        , shedOralMaxLnDuration_Stddev(0.0f)
        , shedOralTiterBlockLog2NAb(0.0f)
        , shedOralTiterProfile_mu(0.0f)
        , shedOralTiterProfile_sigma(0.0f)
        , TauNAb(0.0f)
        , vaccine_Dantigen_IPV1(0.0f)
        , vaccine_Dantigen_IPV2(0.0f)
        , vaccine_Dantigen_IPV3(0.0f)
        , vaccine_take_multiplier_S1(0.0f)
        , vaccine_take_multiplier_S2(0.0f)
        , vaccine_take_multiplier_S3(0.0f)
        , vaccine_titer_bOPV1(0.0f)
        , vaccine_titer_bOPV3(0.0f)
        , vaccine_titer_mOPV1(0.0f)
        , vaccine_titer_mOPV2(0.0f)
        , vaccine_titer_mOPV3(0.0f)
        , vaccine_titer_tOPV1(0.0f)
        , vaccine_titer_tOPV2(0.0f)
        , vaccine_titer_tOPV3(0.0f)
        , viral_interference_S1(0.0f)
        , viral_interference_S2(0.0f)
        , viral_interference_S3(0.0f)
        , viral_interference_W1(0.0f)
        , viral_interference_W2(0.0f)
        , viral_interference_W3(0.0f)
        , genomeRelativeInfectivity1()
        , genomeRelativeInfectivity2()
        , genomeRelativeInfectivity3()
        , Sabin1_Site_Rates()
        , Sabin2_Site_Rates()
        , Sabin3_Site_Rates()
        , reversionSteps_cVDPV1(0)
        , reversionSteps_cVDPV2(0)
        , reversionSteps_cVDPV3(0)
        , vaccine_genome_OPV1(0)
        , vaccine_genome_OPV2(0)
        , vaccine_genome_OPV3(0)
        , vaccine_strain1()
        , vaccine_strain2()
        , vaccine_strain3()
    {}

    SimParams::SimParams()
        : sim_type(SimType::GENERIC_SIM)
        , campaign_filename()
        , loadbalance_filename()
        , enable_coordinator_event_report(false)
        , enable_default_report(false)
        , enable_demographic_tracking(false)
        , enable_event_db(false)
        , enable_event_report(false)
        , enable_interventions(false)
        , enable_net_infect(false)
        , enable_node_event_report(false)
        , enable_property_output(false)
        , enable_spatial_output(false)
        , enable_surveillance_event_report(false)
        , enable_termination_on_total_wall_time(false)
        , enable_termination_on_zero_total_infectivity(false)
        , net_infect_max_frac(0.0f)
        , net_infect_min_dist(0.0f)
        , sim_time_base_year(0.0f)
        , sim_time_delta(0.0f)
        , sim_time_end_min(0.0f)
        , sim_time_start(0.0f)
        , sim_time_total(0.0f)
        , wall_time_max_minutes(0.0f)
        , net_infect_grav_coeff()
        , net_infect_grav_dpow()
    {}

    TBHIVParams::TBHIVParams()
        : drugs_map()
    {}



// *****************************************************************************



    // AgentConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(AgentConfig,AgentConfig)
    BEGIN_QUERY_INTERFACE_BODY(AgentConfig)
    END_QUERY_INTERFACE_BODY(AgentConfig)

    bool AgentConfig::Configure(const Configuration* config)
    {
        // Agent parameter dependencies
        const std::map<std::string, std::string> dset_agent00  {{"Simulation_Type","GENERIC_SIM"}};
        const std::map<std::string, std::string> dset_agent01  {{"Simulation_Type","GENERIC_SIM"},{"Enable_Strain_Tracking","1"}};
        const std::map<std::string, std::string> dset_agent02  {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","GENERIC_SIM"},{"Enable_Acquisition_Heterogeneity","1"}};
        const std::map<std::string, std::string> dset_agent03  {{"Simulation_Type","GENERIC_SIM"},{"Enable_Strain_Tracking","1"},{"Enable_Genome_Dependent_Infectivity","1"}};
        const std::map<std::string, std::string> dset_agent04  {{"Simulation_Type","GENERIC_SIM"},{"Enable_Strain_Tracking","1"},{"Enable_Genome_Mutation","1"}};
        const std::map<std::string, std::string> dset_agent05  {{"Simulation_Type","GENERIC_SIM"},{"Enable_Strain_Tracking","1"},{"Enable_Genome_Mutation","1"},{"Enable_Label_By_Mutator","1"}};
        const std::map<std::string, std::string> dset_agent06  {{"Simulation_Type","GENERIC_SIM"},{"Enable_Nonuniform_Shedding","1"}};


        // Agent parameters
        initConfigTypeMap("Enable_Genome_Dependent_Infectivity",   &agent_params.enable_genome_dependent_infectivity,  Enable_Genome_Dependent_Infectivity_DESC_TEXT,     false,                    nullptr, nullptr, &dset_agent01);
        initConfigTypeMap("Enable_Genome_Mutation",                &agent_params.enable_genome_mutation,               Enable_Genome_Mutation_DESC_TEXT ,                 false,                    nullptr, nullptr, &dset_agent01);
        initConfigTypeMap("Enable_Label_By_Infector",              &agent_params.enable_label_infector,                Enable_Label_By_Infector_DESC_TEXT,                false,                    nullptr, nullptr, &dset_agent01);
        initConfigTypeMap("Enable_Label_By_Mutator",               &agent_params.enable_label_mutator,                 Enable_Label_By_Mutator_DESC_TEXT,                 false,                    nullptr, nullptr, &dset_agent04);
        initConfigTypeMap("Enable_Nonuniform_Shedding",            &agent_params.enable_nonuniform_shedding,           Enable_Nonuniform_Shedding_DESC_TEXT,              false,                    nullptr, nullptr, &dset_agent00);
        initConfigTypeMap("Enable_Strain_Tracking",                &agent_params.enable_strain_tracking,               Enable_Strain_Tracking_DESC_TEXT,                  false);

        initConfigTypeMap("Genome_Mutations_Labeled",              &agent_params.genome_mutations_labeled,             Genome_Mutations_Labeled_DESC_TEXT,                                          nullptr, nullptr, &dset_agent05);

        initConfigTypeMap("Acquisition_Transmission_Correlation",  &agent_params.correlation_acq_trans,                Acquisition_Transmission_Correlation_DESC_TEXT,     0.0f,     1.0f,   0.0f,  nullptr, nullptr, &dset_agent02);
        initConfigTypeMap("Shedding_Distribution_Alpha",           &agent_params.shedding_dist_alpha,                  Shedding_Distribution_Alpha_DESC_TEXT,              1.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_agent06);
        initConfigTypeMap("Shedding_Distribution_Beta",            &agent_params.shedding_dist_beta,                   Shedding_Distribution_Beta_DESC_TEXT,               1.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_agent06);
        initConfigTypeMap("Symptomatic_Infectious_Offset",         &agent_params.symptomatic_infectious_offset,        Symptomatic_Infectious_Offset_DESC_TEXT,        -FLT_MAX,  FLT_MAX,   0.0f,  nullptr, nullptr, &dset_agent00);

        initConfigTypeMap("Genome_Infectivity_Multipliers",        &agent_params.genome_infectivity_multipliers,       Genome_Infectivity_Multipliers_DESC_TEXT,           0.0f,  FLT_MAX,  false,  nullptr, nullptr, &dset_agent03);
        initConfigTypeMap("Genome_Mutation_Rates",                 &agent_params.genome_mutation_rates,                Genome_Mutation_Rates_DESC_TEXT,                    0.0f,  FLT_MAX,  false,  nullptr, nullptr, &dset_agent04);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        // Post-process values
        if(agent_params.enable_nonuniform_shedding)
        {
            float aval  = agent_params.shedding_dist_alpha;
            float bval  = agent_params.shedding_dist_beta;
            float denom = static_cast<float>(tgamma(aval)*tgamma(bval)/tgamma(aval+bval));

            for(int k1 = 0; k1 < SHEDDING_HASH_SIZE; k1++)
            {
                float xval = static_cast<float>(k1)/static_cast<float>(SHEDDING_HASH_SIZE-1);

                if(aval > 1.0f && bval > 1.0f)
                {
                    agent_params.shedding_beta_pdf_hash[k1] = pow(xval,aval-1.0f)*pow(1.0f-xval,bval-1.0f)/denom;
                }
                else if(aval > 1.0f)
                {
                    agent_params.shedding_beta_pdf_hash[k1] = pow(xval,aval-1.0f)/denom;
                }
                else if(bval > 1.0f)
                {
                    agent_params.shedding_beta_pdf_hash[k1] = pow(1.0f-xval,bval-1.0f)/denom;
                }
                else
                {
                    agent_params.shedding_beta_pdf_hash[k1] = 1.0f;
                }
            }
        }


        return bRet;
    }

    const AgentParams* AgentConfig::GetAgentParams()
    {
        return &agent_params;
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

        initConfigTypeMap("Air_Temperature_Filename",    &climate_params.climate_airtemperature_filename,    Air_Temperature_Filename_DESC_TEXT,     "",  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Koppen_Filename",             &climate_params.climate_koppen_filename,            Koppen_Filename_DESC_TEXT,              "",  nullptr, nullptr, &dset_clim05);
        initConfigTypeMap("Land_Temperature_Filename",   &climate_params.climate_landtemperature_filename,   Land_Temperature_Filename_DESC_TEXT,    "",  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Rainfall_Filename",           &climate_params.climate_rainfall_filename,          Rainfall_Filename_DESC_TEXT,            "",  nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Relative_Humidity_Filename",  &climate_params.climate_relativehumidity_filename,  Relative_Humidity_Filename_DESC_TEXT,   "",  nullptr, nullptr, &dset_clim06);

        initConfigTypeMap("Enable_Climate_Stochasticity",   &climate_params.enable_climate_stochasticity,  Enable_Climate_Stochasticity_DESC_TEXT,   false, nullptr, nullptr, &dset_clim03);
        initConfigTypeMap("Enable_Rainfall_Stochasticity",  &climate_params.rainfall_variance_enabled,     Enable_Rainfall_Stochasticity_DESC_TEXT,  true,  nullptr, nullptr, &dset_clim04);

        initConfigTypeMap("Air_Temperature_Offset",         &climate_params.airtemperature_offset,     Air_Temperature_Offset_DESC_TEXT,          -20.0f,   20.0f,    0.0f,   nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Air_Temperature_Variance",       &climate_params.airtemperature_variance,   Air_Temperature_Variance_DESC_TEXT,          0.0f,    5.0f,    2.0f,   nullptr, nullptr, &dset_clim04);
        initConfigTypeMap("Base_Air_Temperature",           &climate_params.base_airtemperature,       Base_Air_Temperature_DESC_TEXT,            -55.0f,   45.0f,   22.0f,   nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Base_Land_Temperature",          &climate_params.base_landtemperature,      Base_Land_Temperature_DESC_TEXT,           -55.0f,   60.0f,   26.0f,   nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Base_Rainfall",                  &climate_params.base_rainfall,             Base_Rainfall_DESC_TEXT,                     0.0f,  150.0f,   10.0f,   nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Base_Relative_Humidity",         &climate_params.base_humidity,             Base_Relative_Humidity_DESC_TEXT,            0.0f,    1.0f,    0.75f,  nullptr, nullptr, &dset_clim07);
        initConfigTypeMap("Land_Temperature_Offset",        &climate_params.landtemperature_offset,    Land_Temperature_Offset_DESC_TEXT,         -20.0f,   20.0f,    0.0f,   nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Land_Temperature_Variance",      &climate_params.landtemperature_variance,  Land_Temperature_Variance_DESC_TEXT,         0.0f,    7.0f,    2.0f,   nullptr, nullptr, &dset_clim04);
        initConfigTypeMap("Rainfall_Scale_Factor",          &climate_params.rainfall_scale_factor,     Rainfall_Scale_Factor_DESC_TEXT,             0.1f,   10.0f,    1.0f,   nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Relative_Humidity_Scale_Factor", &climate_params.humidity_scale_factor,     Relative_Humidity_Scale_Factor_DESC_TEXT,    0.1f,   10.0f,    1.0f,   nullptr, nullptr, &dset_clim06);
        initConfigTypeMap("Relative_Humidity_Variance",     &climate_params.humidity_variance,         Relative_Humidity_Variance_DESC_TEXT,        0.0f,    0.12f,   0.05f,  nullptr, nullptr, &dset_clim04);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        // Set fixed values; config param in mm, need m for calculations
        climate_params.base_rainfall /= MILLIMETERS_PER_METER;


        return bRet;
    }

    const ClimateParams* ClimateConfig::GetClimateParams()
    {
        return &climate_params;
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
        initConfig("Migration_Model",                     migration_params.migration_structure,  config,  MetadataDescriptor::Enum("Migration_Model",                    Migration_Model_DESC_TEXT,                    MDD_ENUM_ARGS(MigrationStructure)));
        initConfig("Migration_Pattern",                   migration_params.migration_pattern,    config,  MetadataDescriptor::Enum("Migration_Pattern",                  Migration_Pattern_DESC_TEXT,                  MDD_ENUM_ARGS(MigrationPattern)),    nullptr, nullptr, &dset_mig00);
        initConfig("Vector_Migration_Modifier_Equation",  migration_params.vec_mod_equ,          config,  MetadataDescriptor::Enum("Vector_Migration_Modifier_Equation", Vector_Migration_Modifier_Equation_DESC_TEXT, MDD_ENUM_ARGS(ModiferEquationType)), nullptr, nullptr, &dset_mig_vec02b); 

        initConfigTypeMap("Air_Migration_Filename",             &migration_params.mig_file_air,          Air_Migration_Filename_DESC_TEXT,             "", nullptr, nullptr, &dset_mig03);
        initConfigTypeMap("Family_Migration_Filename",          &migration_params.mig_file_family,       Family_Migration_Filename_DESC_TEXT,          "", nullptr, nullptr, &dset_mig06);
        initConfigTypeMap("Local_Migration_Filename",           &migration_params.mig_file_local,        Local_Migration_Filename_DESC_TEXT,           "", nullptr, nullptr, &dset_mig02b);
        initConfigTypeMap("Regional_Migration_Filename",        &migration_params.mig_file_regional,     Regional_Migration_Filename_DESC_TEXT,        "", nullptr, nullptr, &dset_mig04);
        initConfigTypeMap("Sea_Migration_Filename",             &migration_params.mig_file_sea,          Sea_Migration_Filename_DESC_TEXT,             "", nullptr, nullptr, &dset_mig05);
        initConfigTypeMap("Vector_Migration_Filename_Local",    &migration_params.mig_file_vec_local,    Vector_Migration_Filename_Local_DESC_TEXT,    "", nullptr, nullptr, &dset_mig_vec03b);
        initConfigTypeMap("Vector_Migration_Filename_Regional", &migration_params.mig_file_vec_regional, Vector_Migration_Filename_Regional_DESC_TEXT, "", nullptr, nullptr, &dset_mig_vec04);

        initConfigTypeMap("Enable_Air_Migration",               &migration_params.enable_mig_air,          Enable_Air_Migration_DESC_TEXT,             false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Family_Migration",            &migration_params.enable_mig_family,       Enable_Family_Migration_DESC_TEXT,          false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Local_Migration",             &migration_params.enable_mig_local,        Enable_Local_Migration_DESC_TEXT,           false, nullptr, nullptr, &dset_mig01a);
        initConfigTypeMap("Enable_Regional_Migration",          &migration_params.enable_mig_regional,     Enable_Regional_Migration_DESC_TEXT,        false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Sea_Migration",               &migration_params.enable_mig_sea,          Enable_Sea_Migration_DESC_TEXT,             false, nullptr, nullptr, &dset_mig01b);
        initConfigTypeMap("Enable_Vector_Migration",            &migration_params.enable_mig_vec,          Enable_Vector_Migration_DESC_TEXT,          false, nullptr, nullptr, &dset_mig_vec01);
        initConfigTypeMap("Enable_Vector_Migration_Local",      &migration_params.enable_mig_vec_local,    Enable_Vector_Migration_Local_DESC_TEXT,    false, nullptr, nullptr, &dset_mig_vec02a);
        initConfigTypeMap("Enable_Vector_Migration_Regional",   &migration_params.enable_mig_vec_regional, Enable_Vector_Migration_Regional_DESC_TEXT, false, nullptr, nullptr, &dset_mig_vec02b); 

        initConfigTypeMap("Air_Migration_Roundtrip_Duration",          &migration_params.air_roundtrip_duration,     Air_Migration_Roundtrip_Duration_DESC_TEXT,          0.0f, 10000.0f,   1.0f,  nullptr, nullptr, &dset_mig08);
        initConfigTypeMap("Air_Migration_Roundtrip_Probability",       &migration_params.air_roundtrip_prob,         Air_Migration_Roundtrip_Probability_DESC_TEXT,       0.0f,     1.0f,   0.80f, nullptr, nullptr, &dset_mig08);
        initConfigTypeMap("Family_Migration_Roundtrip_Duration",       &migration_params.family_roundtrip_duration,  Family_Migration_Roundtrip_Duration_DESC_TEXT,       0.0f, 10000.0f,   1.0f,  nullptr, nullptr, &dset_mig11);
        initConfigTypeMap("Local_Migration_Roundtrip_Probability",     &migration_params.local_roundtrip_prob,       Local_Migration_Roundtrip_Probability_DESC_TEXT,     0.0f,     1.0f,   0.95f, nullptr, nullptr, &dset_mig07);
        initConfigTypeMap("Local_Migration_Roundtrip_Duration",        &migration_params.local_roundtrip_duration,   Local_Migration_Roundtrip_Duration_DESC_TEXT,        0.0f, 10000.0f,   1.0f,  nullptr, nullptr, &dset_mig07);
        initConfigTypeMap("Regional_Migration_Roundtrip_Duration",     &migration_params.region_roundtrip_duration,  Regional_Migration_Roundtrip_Duration_DESC_TEXT,     0.0f, 10000.0f,   1.0f,  nullptr, nullptr, &dset_mig09);
        initConfigTypeMap("Regional_Migration_Roundtrip_Probability",  &migration_params.region_roundtrip_prob,      Regional_Migration_Roundtrip_Probability_DESC_TEXT,  0.0f,     1.0f,   0.10f, nullptr, nullptr, &dset_mig09);
        initConfigTypeMap("Sea_Migration_Roundtrip_Duration",          &migration_params.sea_roundtrip_duration,     Sea_Migration_Roundtrip_Duration_DESC_TEXT,          0.0f, 10000.0f,   1.0f,  nullptr, nullptr, &dset_mig10);
        initConfigTypeMap("Sea_Migration_Roundtrip_Probability",       &migration_params.sea_roundtrip_prob,         Sea_Migration_Roundtrip_Probability_DESC_TEXT,       0.0f,     1.0f,   0.25f, nullptr, nullptr, &dset_mig10);
        initConfigTypeMap("Vector_Migration_Food_Modifier",            &migration_params.vec_mod_food,               Vector_Migration_Food_Modifier_DESC_TEXT,            0.0f,  FLT_MAX,   0.0f,  nullptr, nullptr, &dset_mig_vec02b);
        initConfigTypeMap("Vector_Migration_Habitat_Modifier",         &migration_params.vec_mod_habitat,            Vector_Migration_Habitat_Modifier_DESC_TEXT,         0.0f,  FLT_MAX,   0.0f,  nullptr, nullptr, &dset_mig_vec02b);
        initConfigTypeMap("Vector_Migration_Stay_Put_Modifier",        &migration_params.vec_mod_stayput,            Vector_Migration_Stay_Put_Modifier_DESC_TEXT,        0.0f,  FLT_MAX,   0.0f,  nullptr, nullptr, &dset_mig_vec02b);
        initConfigTypeMap("x_Air_Migration",                           &migration_params.mig_mult_air,               x_Air_Migration_DESC_TEXT,                           0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig03);
        initConfigTypeMap("x_Family_Migration",                        &migration_params.mig_mult_family,            x_Family_Migration_DESC_TEXT,                        0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig06);
        initConfigTypeMap("x_Local_Migration",                         &migration_params.mig_mult_local,             x_Local_Migration_DESC_TEXT,                         0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig02a);
        initConfigTypeMap("x_Regional_Migration",                      &migration_params.mig_mult_regional,          x_Regional_Migration_DESC_TEXT,                      0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig04);
        initConfigTypeMap("x_Sea_Migration",                           &migration_params.mig_mult_sea,               x_Sea_Migration_DESC_TEXT,                           0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig05);
        initConfigTypeMap("x_Vector_Migration_Local",                  &migration_params.mig_mult_vec_local,         x_Vector_Migration_Local_DESC_TEXT,                  0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig_vec03a);
        initConfigTypeMap("x_Vector_Migration_Regional",               &migration_params.mig_mult_vec_regional,      x_Vector_Migration_Regional_DESC_TEXT,               0.0f,  FLT_MAX,   1.0f,  nullptr, nullptr, &dset_mig_vec04);

        initConfigTypeMap("Roundtrip_Waypoints", &migration_params.roundtrip_waypoints, Roundtrip_Waypoints_DESC_TEXT, 0, 1000, 10, nullptr, nullptr, &dset_mig12);


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



    // NodeConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(NodeConfig,NodeConfig)
    BEGIN_QUERY_INTERFACE_BODY(NodeConfig)
    END_QUERY_INTERFACE_BODY(NodeConfig)

    bool NodeConfig::Configure(const Configuration* config)
    {
        // Local variables
        uint32_t log2genomes = 0;


        // Node parameter dependencies
        const std::map<std::string, std::string> dset_env01    {{"Simulation_Type","GENERIC_SIM,ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM"}};
        const std::map<std::string, std::string> dset_env02    {{"Simulation_Type","GENERIC_SIM,ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM"},{"Enable_Environmental_Route","1"}};

        const std::map<std::string, std::string> dset_hint     {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","GENERIC_SIM,STI_SIM,HIV_SIM,AIRBORNE_SIM,TBHIV_SIM,PY_SIM,ENVIRONMENTAL_SIM,TYPHOID_SIM,POLIO_SIM"}};

        const std::map<std::string, std::string> dset_inf01    {{"Simulation_Type","GENERIC_SIM,ENVIRONMENTAL_SIM"}};

        const std::map<std::string, std::string> dset_iprev    {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","GENERIC_SIM,VECTOR_SIM,STI_SIM,ENVIRONMENTAL_SIM,MALARIA_SIM"}};

        const std::map<std::string, std::string> dset_isus01   {{"Simulation_Type","GENERIC_SIM,ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM,STI_SIM,AIRBORNE_SIM,TBHIV_SIM,VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,PY_SIM"},{"Enable_Immunity","1"}};
        const std::map<std::string, std::string> dset_isus02   {{"Simulation_Type","GENERIC_SIM,ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM,STI_SIM,AIRBORNE_SIM,TBHIV_SIM,VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,PY_SIM"},{"Enable_Immunity","1"},{"Enable_Initial_Susceptibility_Distribution","1"}};

        const std::map<std::string, std::string> dset_risk01   {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","GENERIC_SIM"}};
        const std::map<std::string, std::string> dset_risk02   {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,TBHIV_SIM,DENGUE_SIM"}};

        const std::map<std::string, std::string> dset_polio01  {{"Simulation_Type","POLIO_SIM"}};
        const std::map<std::string, std::string> dset_polio02  {{"Simulation_Type","POLIO_SIM"},{"Enable_Susceptibility_Scaling","1"}};

        const std::map<std::string, std::string> dset_samp01   {{"Individual_Sampling_Type","FIXED_SAMPLING,ADAPTED_SAMPLING_BY_IMMUNE_STATE"}};
        const std::map<std::string, std::string> dset_samp02   {{"Individual_Sampling_Type","ADAPTED_SAMPLING_BY_IMMUNE_STATE"}};
        const std::map<std::string, std::string> dset_samp03   {{"Individual_Sampling_Type","ADAPTED_SAMPLING_BY_AGE_GROUP,ADAPTED_SAMPLING_BY_AGE_GROUP_AND_POP_SIZE"}};
        const std::map<std::string, std::string> dset_samp04   {{"Individual_Sampling_Type","ADAPTED_SAMPLING_BY_POPULATION_SIZE,ADAPTED_SAMPLING_BY_AGE_GROUP_AND_POP_SIZE"}};

        const std::map<std::string, std::string> dset_strain01 {{"Simulation_Type","GENERIC_SIM,VECTOR_SIM,STI_SIM,HIV_SIM,AIRBORNE_SIM,PY_SIM,TBHIV_SIM,MALARIA_SIM,ENVIRONMENTAL_SIM,TYPHOID_SIM"},{"Enable_Strain_Tracking","1"}};
        const std::map<std::string, std::string> dset_strain02 {{"Simulation_Type","GENERIC_SIM,VECTOR_SIM,STI_SIM,HIV_SIM,AIRBORNE_SIM,PY_SIM,DENGUE_SIM"},{"Enable_Strain_Tracking","1"}};

        const std::map<std::string, std::string> dset_vec01    {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"}};
        const std::map<std::string, std::string> dset_vec02    {{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"},{"Vector_Sampling_Type", "SAMPLE_IND_VECTORS"}};
        const std::map<std::string, std::string> dset_vec03    {{"Enable_Demographics_Builtin","0"},{"Simulation_Type","VECTOR_SIM,MALARIA_SIM,DENGUE_SIM"}};

        const std::map<std::string, std::string> dset_birth01  {{"Enable_Vital_Dynamics","1"}};
        const std::map<std::string, std::string> dset_birth02  {{"Enable_Vital_Dynamics","1"},{"Enable_Birth","1"}};
        const std::map<std::string, std::string> dset_birth03  {{"Simulation_Type","STI_SIM,HIV_SIM,AIRBORNE_SIM,TBHIV_SIM,VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,PY_SIM"},{"Enable_Vital_Dynamics","1"},{"Enable_Birth","1"}};
        const std::map<std::string, std::string> dset_birth04  {{"Simulation_Type","STI_SIM,HIV_SIM,AIRBORNE_SIM,TBHIV_SIM,VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,PY_SIM"},{"Enable_Vital_Dynamics","1"},{"Enable_Birth","1"},{"Enable_Maternal_Infection_Transmission","1"}};
        const std::map<std::string, std::string> dset_death01  {{"Enable_Demographics_Builtin","0"},{"Enable_Vital_Dynamics","1"}};
        const std::map<std::string, std::string> dset_death02  {{"Enable_Demographics_Builtin","0"},{"Enable_Vital_Dynamics","1"},{"Enable_Natural_Mortality","1"}};


        // Node parameters
        initConfig("Age_Initialization_Distribution_Type",             node_params.age_init_dist_type,     config,  MetadataDescriptor::Enum("Age_Initialization_Distribution_Type",            Age_Initialization_Distribution_Type_DESC_TEXT,            MDD_ENUM_ARGS(DistributionType)));
        initConfig("Birth_Rate_Dependence",                            node_params.vital_birth_dependence, config,  MetadataDescriptor::Enum("Birth_Rate_Dependence",                           Birth_Rate_Dependence_DESC_TEXT,                           MDD_ENUM_ARGS(VitalBirthDependence)),  nullptr, nullptr,   &dset_birth02 );
        initConfig("Death_Rate_Dependence",                            node_params.vital_death_dependence, config,  MetadataDescriptor::Enum("Death_Rate_Dependence",                           Death_Rate_Dependence_DESC_TEXT,                           MDD_ENUM_ARGS(VitalDeathDependence)),  nullptr,  nullptr,  &dset_death02);
        initConfig("Individual_Sampling_Type",                         node_params.ind_sampling_type,      config,  MetadataDescriptor::Enum("Individual_Sampling_Type",                        Individual_Sampling_Type_DESC_TEXT,                        MDD_ENUM_ARGS(IndSamplingType)));
        initConfig("Susceptibility_Initialization_Distribution_Type",  node_params.initial_sus_dist_type,  config,  MetadataDescriptor::Enum("Susceptibility_Initialization_Distribution_Type", Susceptibility_Initialization_Distribution_Type_DESC_TEXT, MDD_ENUM_ARGS(DistributionType)),      nullptr,  nullptr,  &dset_isus02);
        initConfig("Vector_Sampling_Type",                             node_params.vector_sampling_type,   config,  MetadataDescriptor::Enum("Vector_Sampling_Type",                            Vector_Sampling_Type_DESC_TEXT,                            MDD_ENUM_ARGS(VectorSamplingType)),    nullptr,  nullptr,  &dset_vec01);

        initConfigTypeMap("Enable_Acquisition_Heterogeneity",             &node_params.enable_acquisition_heterogeneity,  Enable_Acquisition_Heterogeneity_DESC_TEXT,             false,  nullptr,  nullptr,  &dset_risk01);
        initConfigTypeMap("Enable_Birth",                                 &node_params.enable_birth,                      Enable_Birth_DESC_TEXT,                                 true,   nullptr,  nullptr,  &dset_birth01);
        initConfigTypeMap("Enable_Demographics_Risk",                     &node_params.enable_demographics_risk,          Enable_Demographics_Risk_DESC_TEXT,                     false,  nullptr,  nullptr,  &dset_risk02);
        initConfigTypeMap("Enable_Environmental_Route",                   &node_params.enable_environmental_route,        Enable_Environmental_Route_DESC_TEXT,                   false,  nullptr,  nullptr,  &dset_env01);
        initConfigTypeMap("Enable_Heterogeneous_Intranode_Transmission",  &node_params.enable_hint,                       Enable_Heterogeneous_Intranode_Transmission_DESC_TEXT,  false,  nullptr,  nullptr,  &dset_hint);
        initConfigTypeMap("Enable_Infection_Rate_Overdispersion",         &node_params.enable_infectivity_overdispersion, Enable_Infection_Rate_Overdispersion_DESC_TEXT,         false,  nullptr,  nullptr,  &dset_inf01);
        initConfigTypeMap("Enable_Infectivity_Reservoir",                 &node_params.enable_infectivity_reservoir,      Enable_Infectivity_Reservoir_DESC_TEXT,                 false); 
        initConfigTypeMap("Enable_Infectivity_Scaling",                   &node_params.enable_infectivity_scaling,        Enable_Infectivity_Scaling_DESC_TEXT,                   false,  nullptr,  nullptr,  &dset_inf01);
        initConfigTypeMap("Enable_Initial_Prevalence",                    &node_params.enable_initial_prevalence,         Enable_Initial_Prevalence_DESC_TEXT,                    false,  nullptr,  nullptr,  &dset_iprev);
        initConfigTypeMap("Enable_Initial_Susceptibility_Distribution",   &node_params.enable_initial_sus_dist,           Enable_Initial_Susceptibility_Distribution_DESC_TEXT,   false,  nullptr,  nullptr,  &dset_isus01);
        initConfigTypeMap("Enable_Maternal_Infection_Transmission",       &node_params.enable_maternal_infect_trans,      Enable_Maternal_Infection_Transmission_DESC_TEXT,       false,  nullptr,  nullptr,  &dset_birth03);
        initConfigTypeMap("Enable_Natural_Mortality",                     &node_params.enable_natural_mortality,          Enable_Natural_Mortality_DESC_TEXT,                     false,  nullptr,  nullptr,  &dset_death01);
        initConfigTypeMap("Enable_Percentage_Children",                   &node_params.enable_percentage_children,        Enable_Percentage_Children_DESC_TEXT,                   false,  nullptr,  nullptr,  &dset_vec03);
        initConfigTypeMap("Enable_Susceptibility_Scaling",                &node_params.susceptibility_scaling,            Enable_Susceptibility_Scaling_DESC_TEXT,                false,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Enable_Vector_Mortality",                      &node_params.vector_mortality,                  Enable_Vector_Mortality_DESC_TEXT,                       true,  nullptr,  nullptr,  &dset_vec01);
        initConfigTypeMap("Enable_Vital_Dynamics",                        &node_params.enable_vital_dynamics,             Enable_Vital_Dynamics_DESC_TEXT,                         true);

        initConfigTypeMap("Base_Individual_Sample_Rate",                  &node_params.base_sample_rate,                  Base_Individual_Sample_Rate_DESC_TEXT,                     1.0e-5f,         1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp01);
        initConfigTypeMap("Immune_Downsample_Min_Age",                    &node_params.immune_downsample_min_age,         Immune_Downsample_Min_Age_DESC_TEXT,                       0.0f,         FLT_MAX,     0.0f,  nullptr,  nullptr,  &dset_samp02);
        initConfigTypeMap("Immune_Threshold_For_Downsampling",            &node_params.immune_threshold_for_downsampling, Immune_Threshold_For_Downsampling_DESC_TEXT,               0.0f,            1.0f,     0.0f,  nullptr,  nullptr,  &dset_samp02);
        initConfigTypeMap("Maternal_Infection_Transmission_Probability",  &node_params.prob_maternal_infection_trans,     Maternal_Infection_Transmission_Probability_DESC_TEXT,     0.0f,            1.0f,     0.0f,  nullptr,  nullptr,  &dset_birth04);
        initConfigTypeMap("Max_Node_Population_Samples",                  &node_params.max_sampling_cell_pop,             Max_Node_Population_Samples_DESC_TEXT,                     1.0f,         FLT_MAX,    30.0f,  nullptr,  nullptr,  &dset_samp04);
        initConfigTypeMap("Min_Node_Population_Samples",                  &node_params.min_sampling_cell_pop,             Min_Node_Population_Samples_DESC_TEXT,                     0.0f,         FLT_MAX,     0.0f,  nullptr,  nullptr,  &dset_samp01);
        initConfigTypeMap("Node_Contagion_Decay_Rate",                    &node_params.node_contagion_decay_fraction,     Node_Contagion_Decay_Rate_DESC_TEXT,                       0.0f,            1.0f,     1.0f,  nullptr,  nullptr,  &dset_env02);
        initConfigTypeMap("Relative_Sample_Rate_Immune",                  &node_params.rel_sample_rate_immune,            Relative_Sample_Rate_Immune_DESC_TEXT,                     0.001f,          1.0f,     0.1f,  nullptr,  nullptr,  &dset_samp02);
        initConfigTypeMap("Sample_Rate_Birth",                            &node_params.sample_rate_birth,                 Sample_Rate_Birth_DESC_TEXT,                               0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Sample_Rate_0_18mo",                           &node_params.sample_rate_0_18mo,                Sample_Rate_0_18mo_DESC_TEXT,                              0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Sample_Rate_18mo_4yr",                         &node_params.sample_rate_18mo_4yr,              Sample_Rate_18mo_4yr_DESC_TEXT,                            0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Sample_Rate_5_9",                              &node_params.sample_rate_5_9,                   Sample_Rate_5_9_DESC_TEXT,                                 0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Sample_Rate_10_14",                            &node_params.sample_rate_10_14,                 Sample_Rate_10_14_DESC_TEXT,                               0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Sample_Rate_15_19",                            &node_params.sample_rate_15_19,                 Sample_Rate_15_19_DESC_TEXT,                               0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Sample_Rate_20_Plus",                          &node_params.sample_rate_20_plus,               Sample_Rate_20_plus_DESC_TEXT,                             0.001f,          1.0f,     1.0f,  nullptr,  nullptr,  &dset_samp03);
        initConfigTypeMap("Susceptibility_Scaling_Rate",                  &node_params.susceptibility_scaling_rate,       Susceptibility_Scaling_Rate_DESC_TEXT,                     0.0f,         FLT_MAX,     0.0f,  nullptr,  nullptr,  &dset_polio02);
        initConfigTypeMap("x_Base_Population",                            &node_params.population_scaling_factor,         x_Base_Population_DESC_TEXT,                               0.0f,         FLT_MAX,     1.0f);
        initConfigTypeMap("x_Birth",                                      &node_params.x_birth,                           x_Birth_DESC_TEXT,                                         0.0f,         FLT_MAX,     1.0f,  nullptr,  nullptr,  &dset_birth02 );
        initConfigTypeMap("x_Other_Mortality",                            &node_params.x_othermortality,                  x_Other_Mortality_DESC_TEXT,                               0.0f,         FLT_MAX,     1.0f,  nullptr,  nullptr,  &dset_death02);

        initConfigTypeMap("Log2_Number_of_Genomes_per_Clade",  &log2genomes,                  Log2_Number_of_Genomes_per_Clade_DESC_TEXT,   0, SHIFT_BIT,     0,  nullptr,  nullptr,  &dset_strain02);
        initConfigTypeMap("Mosquito_Weight",                   &node_params.mosquito_weight,  Mosquito_Weight_DESC_TEXT,                    1,     10000,     1,  nullptr,  nullptr,  &dset_vec02);
        initConfigTypeMap("Number_of_Clades",                  &node_params.number_clades,    Number_of_Clades_DESC_TEXT,                   1,        10,     1,  nullptr,  nullptr,  &dset_strain01);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        // Post-process values
        node_params.number_genomes = static_cast<uint64_t>(1) << log2genomes;


        return bRet;
    }

    const NodeParams* NodeConfig::GetNodeParams()
    {
        return &node_params;
    }



    // PolioConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(PolioConfig,PolioConfig)
    BEGIN_QUERY_INTERFACE_BODY(PolioConfig)
    END_QUERY_INTERFACE_BODY(PolioConfig)

    bool PolioConfig::Configure(const Configuration* config)
    {
        // Polio parameter dependencies
        const std::map<std::string, std::string> dset_polio01  {{"Simulation_Type","POLIO_SIM"}};

        // Polio parameters
        initConfig("Evolution_Polio_Clock_Type", polio_params.evolution_polio_clock_type, config, MetadataDescriptor::Enum("Evolution_Polio_Clock_Type", Evolution_Polio_Clock_Type_DESC_TEXT, MDD_ENUM_ARGS(EvolutionPolioClockType)), nullptr, nullptr, &dset_polio01);
        initConfig("VDPV_Virulence_Model_Type",  polio_params.VDPV_virulence_model_type,  config, MetadataDescriptor::Enum("VDPV_Virulence_Model_Type",  VDPV_Virulence_Model_Type_DESC_TEXT,  MDD_ENUM_ARGS(VDPVVirulenceModelType)),  nullptr, nullptr, &dset_polio01);

        initConfigTypeMap("Boost_Log2_NAb_IPV1",                          &polio_params.boostLog2NAb_IPV1,                   Boost_Log2_NAb_IPV1_DESC_TEXT,                          0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_IPV2",                          &polio_params.boostLog2NAb_IPV2,                   Boost_Log2_NAb_IPV2_DESC_TEXT,                          0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_IPV3",                          &polio_params.boostLog2NAb_IPV3,                   Boost_Log2_NAb_IPV3_DESC_TEXT,                          0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_OPV1",                          &polio_params.boostLog2NAb_OPV1,                   Boost_Log2_NAb_OPV1_DESC_TEXT,                          0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_OPV2",                          &polio_params.boostLog2NAb_OPV2,                   Boost_Log2_NAb_OPV2_DESC_TEXT,                          0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_OPV3",                          &polio_params.boostLog2NAb_OPV3,                   Boost_Log2_NAb_OPV3_DESC_TEXT,                          0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_Stddev_IPV1",                   &polio_params.boostLog2NAb_stddev_IPV1,            Boost_Log2_NAb_Stddev_IPV1_DESC_TEXT,                   0.0f,     1.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_Stddev_IPV2",                   &polio_params.boostLog2NAb_stddev_IPV2,            Boost_Log2_NAb_Stddev_IPV2_DESC_TEXT,                   0.0f,     1.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_Stddev_IPV3",                   &polio_params.boostLog2NAb_stddev_IPV3,            Boost_Log2_NAb_Stddev_IPV3_DESC_TEXT,                   0.0f,     1.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_Stddev_OPV1",                   &polio_params.boostLog2NAb_stddev_OPV1,            Boost_Log2_NAb_Stddev_OPV1_DESC_TEXT,                   0.0f,    10.0f,    1.5f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_Stddev_OPV2",                   &polio_params.boostLog2NAb_stddev_OPV2,            Boost_Log2_NAb_Stddev_OPV2_DESC_TEXT,                   0.0f,    10.0f,    1.5f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Boost_Log2_NAb_Stddev_OPV3",                   &polio_params.boostLog2NAb_stddev_OPV3,            Boost_Log2_NAb_Stddev_OPV3_DESC_TEXT,                   0.0f,    10.0f,    1.5f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Excrement_Load",                               &polio_params.excrement_load,                      Excrement_Load_DESC_TEXT,                               0.0f,  1000.0f,  300.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Incubation_Disease_Mu",                        &polio_params.Incubation_Disease_Mu,               Incubation_Disease_Mu_DESC_TEXT,                        0.0f,   100.0f,    2.3893f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Incubation_Disease_Sigma",                     &polio_params.Incubation_Disease_Sigma,            Incubation_Disease_Sigma_DESC_TEXT,                     0.0f,   100.0f,    0.4558f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Maternal_Ab_Halflife",                         &polio_params.maternalAbHalfLife,                  Maternal_Ab_Halflife_DESC_TEXT,                         0.0f,  1000.0f,   22.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Maternal_log2NAb_mean",                        &polio_params.maternal_log2NAb_mean,               Maternal_log2NAb_mean_DESC_TEXT,                        0.0f,    18.0f,    6.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Maternal_log2NAb_std",                         &polio_params.maternal_log2NAb_std,                Maternal_log2NAb_std_DESC_TEXT,                         0.0f,    18.0f,    3.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Log2_NAb_IPV1",                            &polio_params.maxLog2NAb_IPV1,                     Max_Log2_NAb_IPV1_DESC_TEXT,                            0.0f,  FLT_MAX,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Log2_NAb_IPV2",                            &polio_params.maxLog2NAb_IPV2,                     Max_Log2_NAb_IPV2_DESC_TEXT,                            0.0f,  FLT_MAX,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Log2_NAb_IPV3",                            &polio_params.maxLog2NAb_IPV3,                     Max_Log2_NAb_IPV3_DESC_TEXT,                            0.0f,  FLT_MAX,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Log2_NAb_OPV1",                            &polio_params.maxLog2NAb_OPV1,                     Max_Log2_NAb_OPV1_DESC_TEXT,                            0.0f,  FLT_MAX,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Log2_NAb_OPV2",                            &polio_params.maxLog2NAb_OPV2,                     Max_Log2_NAb_OPV2_DESC_TEXT,                            0.0f,  FLT_MAX,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Log2_NAb_OPV3",                            &polio_params.maxLog2NAb_OPV3,                     Max_Log2_NAb_OPV3_DESC_TEXT,                            0.0f,  FLT_MAX,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Max_Rand_Standard_Deviations",                 &polio_params.MaxRandStdDev,                       Max_Rand_Standard_Deviations_DESC_TEXT,                 0.0f,    10.0f,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Mucosal_Immunogenicity_IPV",                   &polio_params.mucosalImmIPV,                       Mucosal_Immunogenicity_IPV_DESC_TEXT,                   0.0f,   100.0f,    3.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Mucosal_Immunogenicity_IPV_OPVExposed",        &polio_params.mucosalImmIPVOPVExposed,             Mucosal_Immunogenicity_IPV_OPVEXPOSED_DESC_TEXT,        0.0f,   100.0f,    3.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Neutralization_Time_Tau",                      &polio_params.TauNAb,                              Neutralization_Time_Tau_DESC_TEXT,                      0.0f,     1.0f,    0.04f,    nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Paralysis_Base_Rate_Sabin1",                   &polio_params.paralysis_base_rate_S1,              Paralysis_Base_Rate_Sabin1_DESC_TEXT,                   0.0f,     1.0f,    5.0e-3f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Paralysis_Base_Rate_Sabin2",                   &polio_params.paralysis_base_rate_S2,              Paralysis_Base_Rate_Sabin2_DESC_TEXT,                   0.0f,     1.0f,    3.3e-4f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Paralysis_Base_Rate_Sabin3",                   &polio_params.paralysis_base_rate_S3,              Paralysis_Base_Rate_Sabin3_DESC_TEXT,                   0.0f,     1.0f,    1.0e-3f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Paralysis_Base_Rate_WPV1",                     &polio_params.paralysis_base_rate_W1,              Paralysis_Base_Rate_WPV1_DESC_TEXT,                     0.0f,     1.0f,    0.05f,    nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Paralysis_Base_Rate_WPV2",                     &polio_params.paralysis_base_rate_W2,              Paralysis_Base_Rate_WPV2_DESC_TEXT,                     0.0f,     1.0f,    3.3e-4f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Paralysis_Base_Rate_WPV3",                     &polio_params.paralysis_base_rate_W3,              Paralysis_Base_Rate_WPV3_DESC_TEXT,                     0.0f,     1.0f,    1.0e-3f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_IPV1",                          &polio_params.primeLog2NAb_IPV1,                   Prime_Log2_NAb_IPV1_DESC_TEXT,                          0.0f,    10.0f,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_IPV2",                          &polio_params.primeLog2NAb_IPV2,                   Prime_Log2_NAb_IPV2_DESC_TEXT,                          0.0f,    10.0f,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_IPV3",                          &polio_params.primeLog2NAb_IPV3,                   Prime_Log2_NAb_IPV3_DESC_TEXT,                          0.0f,    10.0f,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_OPV1",                          &polio_params.primeLog2NAb_OPV1,                   Prime_Log2_NAb_OPV1_DESC_TEXT,                          0.0f,    10.0f,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_OPV2",                          &polio_params.primeLog2NAb_OPV2,                   Prime_Log2_NAb_OPV2_DESC_TEXT,                          0.0f,    10.0f,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_OPV3",                          &polio_params.primeLog2NAb_OPV3,                   Prime_Log2_NAb_OPV3_DESC_TEXT,                          0.0f,    10.0f,    2.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_Stddev_OPV1",                   &polio_params.primeLog2NAb_stddev_OPV1,            Prime_Log2_NAb_Stddev_OPV1_DESC_TEXT,                   0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_Stddev_OPV2",                   &polio_params.primeLog2NAb_stddev_OPV2,            Prime_Log2_NAb_Stddev_OPV2_DESC_TEXT,                   0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_Stddev_OPV3",                   &polio_params.primeLog2NAb_stddev_OPV3,            Prime_Log2_NAb_Stddev_OPV3_DESC_TEXT,                   0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_Stddev_IPV1",                   &polio_params.primeLog2NAb_stddev_IPV1,            Prime_Log2_NAb_Stddev_IPV1_DESC_TEXT,                   0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_Stddev_IPV2",                   &polio_params.primeLog2NAb_stddev_IPV2,            Prime_Log2_NAb_Stddev_IPV2_DESC_TEXT,                   0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Prime_Log2_NAb_Stddev_IPV3",                   &polio_params.primeLog2NAb_stddev_IPV3,            Prime_Log2_NAb_Stddev_IPV3_DESC_TEXT,                   0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_Duration_Block_Log2NAb",            &polio_params.shedFecalDurationBlockLog2NAb,       Shed_Fecal_Duration_Block_Log2NAb_DESC_TEXT,            1.0f,   100.0f,   15.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_MaxLog10_Peak_Titer",               &polio_params.shedFecalMaxLog10PeakTiter,          Shed_Fecal_MaxLog10_Peak_Titer_DESC_TEXT,               0.0f,    10.0f,    5.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_MaxLog10_Peak_Titer_Stddev",        &polio_params.shedFecalMaxLog10PeakTiter_Stddev,   Shed_Fecal_MaxLog10_Peak_Titer_Stddev_DESC_TEXT,        0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_MaxLn_Duration",                    &polio_params.shedFecalMaxLnDuration,              Shed_Fecal_MaxLn_Duration_DESC_TEXT,                    0.0f,   100.0f,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_MaxLn_Duration_Stddev",             &polio_params.shedFecalMaxLnDuration_Stddev,       Shed_Fecal_MaxLn_Duration_Stddev_DESC_TEXT,             0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_Titer_Block_Log2NAb",               &polio_params.shedFecalTiterBlockLog2NAb,          Shed_Fecal_Titer_Block_Log2NAb_DESC_TEXT,               1.0f,   100.0f,   15.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_Titer_Profile_Mu",                  &polio_params.shedFecalTiterProfile_mu,            Shed_Fecal_Titer_Profile_Mu_DESC_TEXT,                  0.0f,    10.0f,    3.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Fecal_Titer_Profile_Sigma",               &polio_params.shedFecalTiterProfile_sigma,         Shed_Fecal_Titer_Profile_Sigma_DESC_TEXT,               0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_Duration_Block_Log2NAb",             &polio_params.shedOralDurationBlockLog2NAb,        Shed_Oral_Duration_Block_Log2NAb_DESC_TEXT,             1.0f,   100.0f,   15.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_MaxLog10_Peak_Titer",                &polio_params.shedOralMaxLog10PeakTiter,           Shed_Oral_MaxLog10_Peak_Titer_DESC_TEXT,                0.0f,    10.0f,    5.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_MaxLog10_Peak_Titer_Stddev",         &polio_params.shedOralMaxLog10PeakTiter_Stddev,    Shed_Oral_MaxLog10_Peak_Titer_Stddev_DESC_TEXT,         0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_MaxLn_Duration",                     &polio_params.shedOralMaxLnDuration,               Shed_Oral_MaxLn_Duration_DESC_TEXT,                     0.0f,    10.0f,    4.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_MaxLn_Duration_Stddev",              &polio_params.shedOralMaxLnDuration_Stddev,        Shed_Oral_MaxLn_Duration_Stddev_DESC_TEXT,              0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_Titer_Block_Log2NAb",                &polio_params.shedOralTiterBlockLog2NAb,           Shed_Oral_Titer_Block_Log2NAb_DESC_TEXT,                1.0f,   100.0f,   15.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_Titer_Profile_Mu",                   &polio_params.shedOralTiterProfile_mu,             Shed_Oral_Titer_Profile_Mu_DESC_TEXT,                   0.0f,    10.0f,    3.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Shed_Oral_Titer_Profile_Sigma",                &polio_params.shedOralTiterProfile_sigma,          Shed_Oral_Titer_Profile_Sigma_DESC_TEXT,                0.0f,    10.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Specific_Infectivity_Sabin1",                  &polio_params.PVinf0_S1,                           Specific_Infectivity_Sabin1_DESC_TEXT,                  0.0f,  1000.0f,    3.02e-5f, nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Specific_Infectivity_Sabin2",                  &polio_params.PVinf0_S2,                           Specific_Infectivity_Sabin2_DESC_TEXT,                  0.0f,  1000.0f,    1.35e-3f, nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Specific_Infectivity_Sabin3",                  &polio_params.PVinf0_S3,                           Specific_Infectivity_Sabin3_DESC_TEXT,                  0.0f,  1000.0f,    1.15e-4f, nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Specific_Infectivity_WPV1",                    &polio_params.PVinf0_W1,                           Specific_Infectivity_WPV1_DESC_TEXT,                    0.0f,  1000.0f,    4.0e-3f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Specific_Infectivity_WPV2",                    &polio_params.PVinf0_W2,                           Specific_Infectivity_WPV2_DESC_TEXT,                    0.0f,  1000.0f,    4.0e-3f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Specific_Infectivity_WPV3",                    &polio_params.PVinf0_W3,                           Specific_Infectivity_WPV3_DESC_TEXT,                    0.0f,  1000.0f,    4.0e-3f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Dantigen_IPV1",                        &polio_params.vaccine_Dantigen_IPV1,               Vaccine_Dantigen_IPV1_DESC_TEXT,                        0.0f,  1000.0f,   40.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Dantigen_IPV2",                        &polio_params.vaccine_Dantigen_IPV2,               Vaccine_Dantigen_IPV2_DESC_TEXT,                        0.0f,  1000.0f,    8.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Dantigen_IPV3",                        &polio_params.vaccine_Dantigen_IPV3,               Vaccine_Dantigen_IPV3_DESC_TEXT,                        0.0f,  1000.0f,   32.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Take_Scaling_Sabin1",                  &polio_params.vaccine_take_multiplier_S1,          Vaccine_Take_Multiplier_Sabin1_DESC_TEXT,               0.0f,     1.0f,    1.0f,     nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Take_Scaling_Sabin2",                  &polio_params.vaccine_take_multiplier_S2,          Vaccine_Take_Multiplier_Sabin2_DESC_TEXT,               0.0f,     1.0f,    0.278f,   nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Take_Scaling_Sabin3",                  &polio_params.vaccine_take_multiplier_S3,          Vaccine_Take_Multiplier_Sabin3_DESC_TEXT,               0.0f,     1.0f,    0.263f,   nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_bOPV1",                          &polio_params.vaccine_titer_bOPV1,                 Vaccine_Titer_bOPV1_DESC_TEXT,                          0.0f,  FLT_MAX,    1.0e+6f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_bOPV3",                          &polio_params.vaccine_titer_bOPV3,                 Vaccine_Titer_bOPV3_DESC_TEXT,                          0.0f,  FLT_MAX,    6.3e+5f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_mOPV1",                          &polio_params.vaccine_titer_mOPV1,                 Vaccine_Titer_mOPV1_DESC_TEXT,                          0.0f,  FLT_MAX,    1.0e+6f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_mOPV2",                          &polio_params.vaccine_titer_mOPV2,                 Vaccine_Titer_mOPV2_DESC_TEXT,                          0.0f,  FLT_MAX,    1.0e+5f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_mOPV3",                          &polio_params.vaccine_titer_mOPV3,                 Vaccine_Titer_mOPV3_DESC_TEXT,                          0.0f,  FLT_MAX,    1.0e+6f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_tOPV1",                          &polio_params.vaccine_titer_tOPV1,                 Vaccine_Titer_tOPV1_DESC_TEXT,                          0.0f,  FLT_MAX,    1.0e+6f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_tOPV2",                          &polio_params.vaccine_titer_tOPV2,                 Vaccine_Titer_tOPV2_DESC_TEXT,                          0.0f,  FLT_MAX,    1.0e+5f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Titer_tOPV3",                          &polio_params.vaccine_titer_tOPV3,                 Vaccine_Titer_tOPV3_DESC_TEXT,                          0.0f,  FLT_MAX,    6.3e+5f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Viral_Interference_Sabin1",                    &polio_params.viral_interference_S1,               Viral_Interference_Sabin1_DESC_TEXT,                    0.0f,     1.0f,    0.0653f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Viral_Interference_Sabin2",                    &polio_params.viral_interference_S2,               Viral_Interference_Sabin2_DESC_TEXT,                    0.0f,     1.0f,    0.278f,   nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Viral_Interference_Sabin3",                    &polio_params.viral_interference_S3,               Viral_Interference_Sabin3_DESC_TEXT,                    0.0f,     1.0f,    0.263f,   nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Viral_Interference_WPV1",                      &polio_params.viral_interference_W1,               Viral_Interference_WPV1_DESC_TEXT,                      0.0f,     1.0f,    0.0653f,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Viral_Interference_WPV2",                      &polio_params.viral_interference_W2,               Viral_Interference_WPV2_DESC_TEXT,                      0.0f,     1.0f,    0.278f,   nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Viral_Interference_WPV3",                      &polio_params.viral_interference_W3,               Viral_Interference_WPV3_DESC_TEXT,                      0.0f,     1.0f,    0.263f,   nullptr,  nullptr,  &dset_polio01);

        initConfigTypeMap("Substrain_Relative_Infectivity_String_VDPV1",  &polio_params.genomeRelativeInfectivity1,    Substrain_Relative_Infectivity_String_VDPV1_DESC_TEXT,  0.0f,  FLT_MAX,  false,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Substrain_Relative_Infectivity_String_VDPV2",  &polio_params.genomeRelativeInfectivity2,    Substrain_Relative_Infectivity_String_VDPV2_DESC_TEXT,  0.0f,  FLT_MAX,  false,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Substrain_Relative_Infectivity_String_VDPV3",  &polio_params.genomeRelativeInfectivity3,    Substrain_Relative_Infectivity_String_VDPV3_DESC_TEXT,  0.0f,  FLT_MAX,  false,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Sabin1_Site_Reversion_Rates",                  &polio_params.Sabin1_Site_Rates,             Sabin1_Site_Reversion_Rates_DESC_TEXT,                  0.0f,  FLT_MAX,  false,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Sabin2_Site_Reversion_Rates",                  &polio_params.Sabin2_Site_Rates,             Sabin2_Site_Reversion_Rates_DESC_TEXT,                  0.0f,  FLT_MAX,  false,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Sabin3_Site_Reversion_Rates",                  &polio_params.Sabin3_Site_Rates,             Sabin3_Site_Reversion_Rates_DESC_TEXT,                  0.0f,  FLT_MAX,  false,  nullptr,  nullptr,  &dset_polio01);

        initConfigTypeMap("Reversion_Steps_Sabin1",  &polio_params.reversionSteps_cVDPV1,  Reversion_Steps_Sabin1_DESC_TEXT,   0,    8,  6,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Reversion_Steps_Sabin2",  &polio_params.reversionSteps_cVDPV2,  Reversion_Steps_Sabin2_DESC_TEXT,   0,    8,  2,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Reversion_Steps_Sabin3",  &polio_params.reversionSteps_cVDPV3,  Reversion_Steps_Sabin3_DESC_TEXT,   0,    8,  3,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Genome_OPV1",     &polio_params.vaccine_genome_OPV1,    Vaccine_Genome_OPV1_DESC_TEXT,      0, 1023,  1,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Genome_OPV2",     &polio_params.vaccine_genome_OPV2,    Vaccine_Genome_OPV2_DESC_TEXT,      0, 1023,  1,  nullptr,  nullptr,  &dset_polio01);
        initConfigTypeMap("Vaccine_Genome_OPV3",     &polio_params.vaccine_genome_OPV3,    Vaccine_Genome_OPV3_DESC_TEXT,      0, 1023,  1,  nullptr,  nullptr,  &dset_polio01);
 

        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        // Set fixed values; Ogra 1968, Warren 1964, Dexiang1956, Plotkin 1959
        polio_params.decayRatePassiveImmunity = log(2.0f)/(polio_params.maternalAbHalfLife+FLT_MIN);

        polio_params.vaccine_strain1.SetCladeID(PolioVirusTypes::VRPV1);
        polio_params.vaccine_strain2.SetCladeID(PolioVirusTypes::VRPV2);
        polio_params.vaccine_strain3.SetCladeID(PolioVirusTypes::VRPV3);
        polio_params.vaccine_strain1.SetGeneticID(polio_params.vaccine_genome_OPV1);
        polio_params.vaccine_strain2.SetGeneticID(polio_params.vaccine_genome_OPV2);
        polio_params.vaccine_strain3.SetGeneticID(polio_params.vaccine_genome_OPV3);


        return bRet;
    }

    const PolioParams* PolioConfig::GetPolioParams()
    {
        return &polio_params;
    }



    // SimConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(SimConfig,SimConfig)
    BEGIN_QUERY_INTERFACE_BODY(SimConfig)
    END_QUERY_INTERFACE_BODY(SimConfig)

    bool SimConfig::Configure(const Configuration* config)
    {
        // Sim parameter dependencies
        const std::map<std::string, std::string> dset_abort01   {{"Enable_Termination_On_Zero_Total_Infectivity","1"}};
        const std::map<std::string, std::string> dset_abort02   {{"Enable_Termination_On_Total_Wall_Time","1"}};
        const std::map<std::string, std::string> dset_iv01      {{"Enable_Interventions","1"}};
        const std::map<std::string, std::string> dset_netinf01  {{"Simulation_Type","GENERIC_SIM"}};
        const std::map<std::string, std::string> dset_netinf02  {{"Simulation_Type","GENERIC_SIM"},{"Enable_Network_Infectivity","1"}};
        const std::map<std::string, std::string> dset_time01    {{"Simulation_Type","STI_SIM,HIV_SIM,TYPHOID_SIM"}};


        // Sim parameters
        initConfig("Simulation_Type",   sim_params.sim_type, config, MetadataDescriptor::Enum("Simulation_Type", Simulation_Type_DESC_TEXT, MDD_ENUM_ARGS(SimType)));

        initConfigTypeMap("Campaign_Filename",                &sim_params.campaign_filename,      Campaign_Filename_DESC_TEXT,      "",  nullptr,  nullptr,  &dset_iv01);
        initConfigTypeMap("Load_Balance_Filename",            &sim_params.loadbalance_filename,   Load_Balance_Filename_DESC_TEXT,  "");

        initConfigTypeMap("Enable_Default_Reporting",                         &sim_params.enable_default_report,                            Enable_Default_Reporting_DESC_TEXT,                        true);
        initConfigTypeMap("Enable_Demographics_Reporting",                    &sim_params.enable_demographic_tracking,                      Enable_Demographics_Reporting_DESC_TEXT,                   false);
        initConfigTypeMap("Enable_Event_DB",                                  &sim_params.enable_event_db,                                  Enable_Event_DB_DESC_TEXT,                                 false);
        initConfigTypeMap("Enable_Interventions",                             &sim_params.enable_interventions,                             Enable_Interventions_DESC_TEXT,                            false);
        initConfigTypeMap("Enable_Network_Infectivity",                       &sim_params.enable_net_infect,                                Enable_Network_Infectivity_DESC_TEXT,                      false,  nullptr,  nullptr,  &dset_netinf01);
        initConfigTypeMap("Enable_Property_Output",                           &sim_params.enable_property_output,                           Enable_Property_Output_DESC_TEXT,                          false);
        initConfigTypeMap("Enable_Report_Coordinator_Event_Recorder",         &sim_params.enable_coordinator_event_report,                  Report_Coordinator_Event_Recorder_DESC_TEXT,               false);
        initConfigTypeMap("Enable_Report_Event_Recorder",                     &sim_params.enable_event_report,                              Report_Event_Recorder_DESC_TEXT,                           false);
        initConfigTypeMap("Enable_Report_Node_Event_Recorder",                &sim_params.enable_node_event_report,                         Report_Node_Event_Recorder_DESC_TEXT,                      false);
        initConfigTypeMap("Enable_Report_Surveillance_Event_Recorder",        &sim_params.enable_surveillance_event_report,                 Report_Coordinator_Event_Recorder_DESC_TEXT,               false);
        initConfigTypeMap("Enable_Spatial_Output",                            &sim_params.enable_spatial_output,                            Enable_Spatial_Output_DESC_TEXT,                           false);
        initConfigTypeMap("Enable_Termination_On_Total_Wall_Time",            &sim_params.enable_termination_on_total_wall_time,            Enable_Termination_On_Total_Wall_Time_DESC_TEXT,           false);
        initConfigTypeMap("Enable_Termination_On_Zero_Total_Infectivity",     &sim_params.enable_termination_on_zero_total_infectivity,     Enable_Termination_On_Zero_Total_Infectivity_DESC_TEXT,    false);

        initConfigTypeMap("Base_Year",                                &sim_params.sim_time_base_year,      Base_Year_DESC_TEXT,                           MIN_YEAR,  MAX_YEAR,   2015.0f,  nullptr,  nullptr,  &dset_time01);
        initConfigTypeMap("Minimum_End_Time",                         &sim_params.sim_time_end_min,        Minimum_End_Time_DESC_TEXT,                        0.0f,   FLT_MAX,      0.0f,  nullptr,  nullptr,  &dset_abort01);
        initConfigTypeMap("Network_Infectivity_Max_Export_Frac",      &sim_params.net_infect_max_frac,     Network_Infectivity_Max_Export_Frac_DESC_TEXT,     0.0f,      1.0f,      0.5f,  nullptr,  nullptr,  &dset_netinf02);
        initConfigTypeMap("Network_Infectivity_Min_Distance",         &sim_params.net_infect_min_dist,     Network_Infectivity_Min_Distance_DESC_TEXT,        0.01f,  FLT_MAX,      1.0f,  nullptr,  nullptr,  &dset_netinf02);
        initConfigTypeMap("Simulation_Duration",                      &sim_params.sim_time_total,          Simulation_Duration_DESC_TEXT,                     0.0f,   FLT_MAX,      1.0f);
        initConfigTypeMap("Simulation_Timestep",                      &sim_params.sim_time_delta,          Simulation_Timestep_DESC_TEXT,                     0.0f,   FLT_MAX,      1.0f);
        initConfigTypeMap("Start_Time",                               &sim_params.sim_time_start,          Start_Time_DESC_TEXT,                              0.0f,   FLT_MAX,      1.0f);
        initConfigTypeMap("Wall_Time_Maximum_In_Minutes",             &sim_params.wall_time_max_minutes,   Wall_Time_Maximum_In_Minutes_DESC_TEXT,            0.0f,   FLT_MAX,      1.0f,  nullptr,  nullptr,  &dset_abort02);

        initConfigTypeMap("Network_Infectivity_Coefficient",       &sim_params.net_infect_grav_coeff,    Network_Infectivity_Coefficient_DESC_TEXT,           0.0f,   FLT_MAX,     false,  nullptr,  nullptr,  &dset_netinf02);
        initConfigTypeMap("Network_Infectivity_Exponent",          &sim_params.net_infect_grav_dpow,     Network_Infectivity_Exponent_DESC_TEXT,              0.0f,   FLT_MAX,     false,  nullptr,  nullptr,  &dset_netinf02);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        return bRet;
    }

    const SimParams* SimConfig::GetSimParams()
    {
        return &sim_params;
    }



    // TBHIVConfig Methods
    GET_SCHEMA_STATIC_WRAPPER_IMPL(TBHIVConfig,TBHIVConfig)
    BEGIN_QUERY_INTERFACE_BODY(TBHIVConfig)
    END_QUERY_INTERFACE_BODY(TBHIVConfig)

    bool TBHIVConfig::Configure(const Configuration* config)
    {
        // TBHIV parameter dependencies
        const std::map<std::string, std::string> dset_tbhiv01    {{"Simulation_Type","TBHIV_SIM"}};

        // TBHIV parameters
        initConfigComplexType("TBHIV_Drug_Params",     &tbhiv_params.drugs_map,      TBHIV_Drug_Params_DESC_TEXT,  nullptr,  nullptr,  &dset_tbhiv01);


        // Process configuration
        bool bRet = JsonConfigurable::Configure(config);


        return bRet;
    }

    const TBHIVParams* TBHIVConfig::GetTBHIVParams()
    {
        return &tbhiv_params;
    }
}

// *****************************************************************************