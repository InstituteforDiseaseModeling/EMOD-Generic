/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "EnumSupport.h"

namespace Kernel
{
    // ENUM defs for Gender_Type
    #define IDM_ENUMSPEC_Gender                                                          \
        ENUM_VALUE_SPEC(MALE                                                , 0)         \
        ENUM_VALUE_SPEC(FEMALE                                              , 1)         \
        ENUM_VALUE_SPEC(COUNT                                               , 2)
    ENUM_DECLARE(Gender, IDM_ENUMSPEC_Gender)


    // ENUM defs for CLIMATE_STRUCTURE
    #define IDM_ENUMSPEC_ClimateStructure                                                \
        ENUM_VALUE_SPEC(CLIMATE_OFF                                         , 0)         \
        ENUM_VALUE_SPEC(CLIMATE_CONSTANT                                    , 1)         \
        ENUM_VALUE_SPEC(CLIMATE_KOPPEN                                      , 2)         \
        ENUM_VALUE_SPEC(CLIMATE_BY_DATA                                     , 3)
    ENUM_DECLARE(ClimateStructure, IDM_ENUMSPEC_ClimateStructure)


    // ENUM defs for CLIMATE_UPDATE_RESOLUTION
    #define IDM_ENUMSPEC_ClimateUpdateResolution                                         \
        ENUM_VALUE_SPEC(CLIMATE_UPDATE_YEAR                                 , 0)         \
        ENUM_VALUE_SPEC(CLIMATE_UPDATE_MONTH                                , 1)         \
        ENUM_VALUE_SPEC(CLIMATE_UPDATE_WEEK                                 , 2)         \
        ENUM_VALUE_SPEC(CLIMATE_UPDATE_DAY                                  , 3)         \
        ENUM_VALUE_SPEC(CLIMATE_UPDATE_HOUR                                 , 4)
    ENUM_DECLARE(ClimateUpdateResolution, IDM_ENUMSPEC_ClimateUpdateResolution)


    // ENUM defs for Distribution types (e.g. AgeInitializationDistributionType)
    #define IDM_ENUMSPEC_DistributionType                                                \
        ENUM_VALUE_SPEC(DISTRIBUTION_OFF                                    , 0)         \
        ENUM_VALUE_SPEC(DISTRIBUTION_SIMPLE                                 , 1)         \
        ENUM_VALUE_SPEC(DISTRIBUTION_COMPLEX                                , 2)
    ENUM_DECLARE(DistributionType, IDM_ENUMSPEC_DistributionType)


    // ENUM defs for IND_SAMPLING_TYPE
    #define IDM_ENUMSPEC_IndSamplingType                                                 \
        ENUM_VALUE_SPEC(TRACK_ALL                                           , 0)         \
        ENUM_VALUE_SPEC(FIXED_SAMPLING                                      , 1)         \
        ENUM_VALUE_SPEC(ADAPTED_SAMPLING_BY_POPULATION_SIZE                 , 2)         \
        ENUM_VALUE_SPEC(ADAPTED_SAMPLING_BY_AGE_GROUP                       , 3)         \
        ENUM_VALUE_SPEC(ADAPTED_SAMPLING_BY_AGE_GROUP_AND_POP_SIZE          , 4)         \
        ENUM_VALUE_SPEC(ADAPTED_SAMPLING_BY_IMMUNE_STATE                    , 5)
    ENUM_DECLARE(IndSamplingType, IDM_ENUMSPEC_IndSamplingType)


    // ENUM defs for MIGRATION_PATTERN
    #define IDM_ENUMSPEC_MigrationPattern                                                \
        ENUM_VALUE_SPEC(RANDOM_WALK_DIFFUSION                               , 0)         \
        ENUM_VALUE_SPEC(SINGLE_ROUND_TRIPS                                  , 1)         \
        ENUM_VALUE_SPEC(WAYPOINTS_HOME                                      , 2)
    ENUM_DECLARE(MigrationPattern, IDM_ENUMSPEC_MigrationPattern)


    // ENUM defs for MIGRATION_STRUCTURE
    #define IDM_ENUMSPEC_MigrationStructure                                              \
        ENUM_VALUE_SPEC(NO_MIGRATION                                        , 0)         \
        ENUM_VALUE_SPEC(FIXED_RATE_MIGRATION                                , 1)
    ENUM_DECLARE(MigrationStructure, IDM_ENUMSPEC_MigrationStructure)


    #define IDM_ENUMSPEC_MigrationType                                                   \
        ENUM_VALUE_SPEC(NO_MIGRATION                                        , 0)         \
        ENUM_VALUE_SPEC(LOCAL_MIGRATION                                     , 1)         \
        ENUM_VALUE_SPEC(AIR_MIGRATION                                       , 2)         \
        ENUM_VALUE_SPEC(REGIONAL_MIGRATION                                  , 3)         \
        ENUM_VALUE_SPEC(SEA_MIGRATION                                       , 4)         \
        ENUM_VALUE_SPEC(FAMILY_MIGRATION                                    , 5)         \
        ENUM_VALUE_SPEC(INTERVENTION_MIGRATION                              , 6)
    ENUM_DECLARE(MigrationType, IDM_ENUMSPEC_MigrationType)


    // Migration
    #define IDM_ENUMSPEC_GenderDataType                                                  \
        ENUM_VALUE_SPEC(SAME_FOR_BOTH_GENDERS                               , 0)         \
        ENUM_VALUE_SPEC(ONE_FOR_EACH_GENDER                                 , 1)
    ENUM_DECLARE(GenderDataType, IDM_ENUMSPEC_GenderDataType)


    // Migration
    #define IDM_ENUMSPEC_InterpolationType                                               \
        ENUM_VALUE_SPEC(LINEAR_INTERPOLATION                                , 0)         \
        ENUM_VALUE_SPEC(PIECEWISE_CONSTANT                                  , 1)
    ENUM_DECLARE(InterpolationType, IDM_ENUMSPEC_InterpolationType)


    // ENUM defs for Vector_Migration_Modifier_Equation
    #define IDM_ENUMSPEC_ModiferEquationType                                             \
        ENUM_VALUE_SPEC(NOT_DEFINED                                         , 0)         \
        ENUM_VALUE_SPEC(LINEAR                                              , 1)         \
        ENUM_VALUE_SPEC(EXPONENTIAL                                         , 2)
    ENUM_DECLARE(ModiferEquationType, IDM_ENUMSPEC_ModiferEquationType)


    // ENUM defs for MORTALITY_TIME_COURSE
    #define IDM_ENUMSPEC_MortalityTimeCourse                                             \
        ENUM_VALUE_SPEC(DAILY_MORTALITY                                     , 0)         \
        ENUM_VALUE_SPEC(MORTALITY_AFTER_INFECTIOUS                          , 1)
    ENUM_DECLARE(MortalityTimeCourse, IDM_ENUMSPEC_MortalityTimeCourse)


    // ENUM defs for PKPD_MODEL
    #define IDM_ENUMSPEC_PKPDModel                                                       \
        ENUM_VALUE_SPEC(FIXED_DURATION_CONSTANT_EFFECT                      , 0)         \
        ENUM_VALUE_SPEC(CONCENTRATION_VERSUS_TIME                           , 1)
    ENUM_DECLARE(PKPDModel, IDM_ENUMSPEC_PKPDModel)


    #define IDM_ENUMSPEC_RandomNumberGeneratorType                                       \
        ENUM_VALUE_SPEC(USE_PSEUDO_DES                                      , 0)         \
        ENUM_VALUE_SPEC(USE_LINEAR_CONGRUENTIAL                             , 1)         \
        ENUM_VALUE_SPEC(USE_AES_COUNTER                                     , 2)
    ENUM_DECLARE(RandomNumberGeneratorType, IDM_ENUMSPEC_RandomNumberGeneratorType)


    #define IDM_ENUMSPEC_RandomNumberGeneratorPolicy                                     \
        ENUM_VALUE_SPEC(ONE_PER_CORE                                        , 0)         \
        ENUM_VALUE_SPEC(ONE_PER_NODE                                        , 1)
    ENUM_DECLARE(RandomNumberGeneratorPolicy, IDM_ENUMSPEC_RandomNumberGeneratorPolicy)


     // ENUM defs for Maternal_Protection_Type (Susceptibility variation with agent age)
    #define IDM_ENUMSPEC_MaternalProtectionType                                          \
        ENUM_VALUE_SPEC(SIGMOID                                             , 0)         \
        ENUM_VALUE_SPEC(LINEAR                                              , 1)
    ENUM_DECLARE(MaternalProtectionType, IDM_ENUMSPEC_MaternalProtectionType)


     // ENUM defs for Susceptibility_Type
    #define IDM_ENUMSPEC_SusceptibilityType                                              \
        ENUM_VALUE_SPEC(FRACTIONAL                                          , 0)         \
        ENUM_VALUE_SPEC(BINARY                                              , 1)
    ENUM_DECLARE(SusceptibilityType, IDM_ENUMSPEC_SusceptibilityType)


    // ENUM defs for Sim_Type
    #define IDM_ENUMSPEC_SimType                                                         \
        ENUM_VALUE_SPEC(GENERIC_SIM                                         , 0)         \
        ENUM_VALUE_SPEC(VECTOR_SIM                                          , 1)         \
        ENUM_VALUE_SPEC(MALARIA_SIM                                         , 2)         \
        ENUM_VALUE_SPEC(ENVIRONMENTAL_SIM                                   , 3)         \
        ENUM_VALUE_SPEC(POLIO_SIM                                           , 4)         \
        ENUM_VALUE_SPEC(AIRBORNE_SIM                                        , 5)         \
        ENUM_VALUE_SPEC(TBHIV_SIM                                           , 6)         \
        ENUM_VALUE_SPEC(STI_SIM                                             , 7)         \
        ENUM_VALUE_SPEC(HIV_SIM                                             , 8)         \
        ENUM_VALUE_SPEC(PY_SIM                                              , 9)         \
        ENUM_VALUE_SPEC(TYPHOID_SIM                                         , 10)        \
        ENUM_VALUE_SPEC(DENGUE_SIM                                          , 11)
    ENUM_DECLARE(SimType, IDM_ENUMSPEC_SimType)


    // ENUM defs for transmission
    #define IDM_ENUMSPEC_TransmissionRoute                                               \
        ENUM_VALUE_SPEC(CONTACT                                             , 0)         \
        ENUM_VALUE_SPEC(ENVIRONMENTAL                                       , 1)         \
        ENUM_VALUE_SPEC(OUTBREAK                                            , 2)         \
        ENUM_VALUE_SPEC(VECTOR_TO_HUMAN                                     , 10)        \
        ENUM_VALUE_SPEC(VECTOR_TO_HUMAN_INDOOR                              , 11)        \
        ENUM_VALUE_SPEC(VECTOR_TO_HUMAN_OUTDOOR                             , 12)        \
        ENUM_VALUE_SPEC(HUMAN_TO_VECTOR                                     , 20)        \
        ENUM_VALUE_SPEC(HUMAN_TO_VECTOR_INDOOR                              , 21)        \
        ENUM_VALUE_SPEC(HUMAN_TO_VECTOR_OUTDOOR                             , 22)        \
        ENUM_VALUE_SPEC(INDOOR                                              , 31)        \
        ENUM_VALUE_SPEC(OUTDOOR                                             , 32)
    ENUM_DECLARE(TransmissionRoute, IDM_ENUMSPEC_TransmissionRoute)


    #define IDM_ENUMSPEC_TransmissionGroupType                                           \
        ENUM_VALUE_SPEC(StrainAwareGroups                                   , 0)         \
        ENUM_VALUE_SPEC(RelationshipGroups                                  , 1)
    ENUM_DECLARE(TransmissionGroupType, IDM_ENUMSPEC_TransmissionGroupType)


    // ENUM defs for VITAL_BIRTH_DEPENDENCE
    #define IDM_ENUMSPEC_VitalBirthDependence                                            \
        ENUM_VALUE_SPEC(FIXED_BIRTH_RATE                                    , 0)         \
        ENUM_VALUE_SPEC(POPULATION_DEP_RATE                                 , 1)         \
        ENUM_VALUE_SPEC(DEMOGRAPHIC_DEP_RATE                                , 2)         \
        ENUM_VALUE_SPEC(INDIVIDUAL_PREGNANCIES                              , 3)         \
        ENUM_VALUE_SPEC(INDIVIDUAL_PREGNANCIES_BY_AGE_AND_YEAR              , 4)
    ENUM_DECLARE(VitalBirthDependence, IDM_ENUMSPEC_VitalBirthDependence)


    // ENUM defs for VITAL_DEATH_DEPENDENCE
    // TODO: FIXED_DEATH_RATE (Makeham), Gompertz-Makeham, Lifetable, Heligman-Pollard, Siler (5-Component Competing Hazard)
    #define IDM_ENUMSPEC_VitalDeathDependence                                            \
        ENUM_VALUE_SPEC(NONDISEASE_MORTALITY_BY_AGE_AND_GENDER              , 0)         \
        ENUM_VALUE_SPEC(NONDISEASE_MORTALITY_BY_YEAR_AND_AGE_FOR_EACH_GENDER, 1)
    ENUM_DECLARE(VitalDeathDependence, IDM_ENUMSPEC_VitalDeathDependence)


    // ENUM defs for INCUBATION_DISTRIBUTION, INFECTIOUS_DISTRIBUTION, BASE_INFECTIVITY_DISTRIBUTION
    #define IDM_ENUMSPEC_DistributionFunction                                            \
        ENUM_VALUE_SPEC(NOT_INITIALIZED                                     ,-1)         \
        ENUM_VALUE_SPEC(CONSTANT_DISTRIBUTION                               , 0)         \
        ENUM_VALUE_SPEC(UNIFORM_DISTRIBUTION                                , 1)         \
        ENUM_VALUE_SPEC(GAUSSIAN_DISTRIBUTION                               , 2)         \
        ENUM_VALUE_SPEC(EXPONENTIAL_DISTRIBUTION                            , 3)         \
        ENUM_VALUE_SPEC(POISSON_DISTRIBUTION                                , 4)         \
        ENUM_VALUE_SPEC(LOG_NORMAL_DISTRIBUTION                             , 5)         \
        ENUM_VALUE_SPEC(DUAL_CONSTANT_DISTRIBUTION                          , 6)         \
        ENUM_VALUE_SPEC(WEIBULL_DISTRIBUTION                                , 9)         \
        ENUM_VALUE_SPEC(DUAL_EXPONENTIAL_DISTRIBUTION                       ,10)         \
        ENUM_VALUE_SPEC(GAMMA_DISTRIBUTION                                  ,11)
//      ENUM_VALUE_SPEC(PIECEWISE_CONSTANT                                  , 7)    // Disable these distributions, but leave index as is (see demographics)
//      ENUM_VALUE_SPEC(PIECEWISE_LINEAR                                    , 8)
    ENUM_DECLARE(DistributionFunction, IDM_ENUMSPEC_DistributionFunction)


    // ENUM defs for Box_Duration of WaningEffect; subset of DistributionFunction
    #define IDM_ENUMSPEC_WEBoxDist                                                       \
        ENUM_VALUE_SPEC(NOT_INITIALIZED                                     , 0)         \
        ENUM_VALUE_SPEC(GAUSSIAN_DISTRIBUTION                               , 1)         \
        ENUM_VALUE_SPEC(EXPONENTIAL_DISTRIBUTION                            , 2)         \
        ENUM_VALUE_SPEC(GAMMA_DISTRIBUTION                                  , 3)
    ENUM_DECLARE(WEBoxDist, IDM_ENUMSPEC_WEBoxDist)


    // IncidenceEventCoordinator
    #define IDM_ENUMSPEC_ThresholdType                                                   \
        ENUM_VALUE_SPEC(COUNT                                               , 1)         \
        ENUM_VALUE_SPEC(PERCENTAGE                                          , 2)         \
        ENUM_VALUE_SPEC(PERCENTAGE_EVENTS                                   , 3)
    ENUM_DECLARE(ThresholdType, IDM_ENUMSPEC_ThresholdType)


    #define IDM_ENUMSPEC_SerializationType                                               \
        ENUM_VALUE_SPEC(NONE                                                , 0)         \
        ENUM_VALUE_SPEC(TIME                                                , 1)         \
        ENUM_VALUE_SPEC(TIMESTEP                                            , 2)
    ENUM_DECLARE(SerializationType, IDM_ENUMSPEC_SerializationType)


    #define IDM_ENUMSPEC_SerializationTypeRead                                           \
        ENUM_VALUE_SPEC(NONE                                                , 0)         \
        ENUM_VALUE_SPEC(READ                                                , 1)
    ENUM_DECLARE(SerializationTypeRead, IDM_ENUMSPEC_SerializationTypeRead)


    #define IDM_ENUMSPEC_SerializationTypeWrite                                          \
        ENUM_VALUE_SPEC(NONE                                                , 0)         \
        ENUM_VALUE_SPEC(TIME                                                , 1)         \
        ENUM_VALUE_SPEC(TIMESTEP                                            , 2)
    ENUM_DECLARE(SerializationTypeWrite, IDM_ENUMSPEC_SerializationTypeWrite)


    #define IDM_ENUMSPEC_SerializationPrecision                                          \
        ENUM_VALUE_SPEC(REDUCED                                             , 0)         \
        ENUM_VALUE_SPEC(FULL                                                , 1)
    ENUM_DECLARE(SerializationPrecision, IDM_ENUMSPEC_SerializationPrecision)


    #define IDM_ENUMSPEC_PolioSerotypes                                                  \
        ENUM_VALUE_SPEC(PV1                                                 , 0)         \
        ENUM_VALUE_SPEC(PV2                                                 , 1)         \
        ENUM_VALUE_SPEC(PV3                                                 , 2)
    ENUM_DECLARE(PolioSerotypes, IDM_ENUMSPEC_PolioSerotypes)


    #define IDM_ENUMSPEC_PolioVirusTypes                                                 \
        ENUM_VALUE_SPEC(WPV1                                                , 0)         \
        ENUM_VALUE_SPEC(WPV2                                                , 1)         \
        ENUM_VALUE_SPEC(WPV3                                                , 2)         \
        ENUM_VALUE_SPEC(VRPV1                                               , 3)         \
        ENUM_VALUE_SPEC(VRPV2                                               , 4)         \
        ENUM_VALUE_SPEC(VRPV3                                               , 5)
    ENUM_DECLARE(PolioVirusTypes, IDM_ENUMSPEC_PolioVirusTypes)


    #define IDM_ENUMSPEC_PolioVaccines                                                   \
        ENUM_VALUE_SPEC(TOPV                                                , 0)         \
        ENUM_VALUE_SPEC(BOPV                                                , 1)         \
        ENUM_VALUE_SPEC(MOPV1                                               , 2)         \
        ENUM_VALUE_SPEC(MOPV2                                               , 3)         \
        ENUM_VALUE_SPEC(MOPV3                                               , 4)         \
        ENUM_VALUE_SPEC(IPV                                                 , 5)
    ENUM_DECLARE(PolioVaccines, IDM_ENUMSPEC_PolioVaccines)


    #define IDM_ENUMSPEC_PolioVaccineType                                                \
        ENUM_VALUE_SPEC(tOPV                                                , 0)         \
        ENUM_VALUE_SPEC(bOPV                                                , 1)         \
        ENUM_VALUE_SPEC(mOPV_1                                              , 2)         \
        ENUM_VALUE_SPEC(mOPV_2                                              , 3)         \
        ENUM_VALUE_SPEC(mOPV_3                                              , 4)         \
        ENUM_VALUE_SPEC(eIPV                                                , 5)
    ENUM_DECLARE(PolioVaccineType, IDM_ENUMSPEC_PolioVaccineType)


    #define IDM_ENUMSPEC_EvolutionPolioClockType                                         \
        ENUM_VALUE_SPEC(POLIO_EVOCLOCK_NONE                                 , 0)         \
        ENUM_VALUE_SPEC(POLIO_EVOCLOCK_LINEAR                               , 1)         \
        ENUM_VALUE_SPEC(POLIO_EVOCLOCK_IMMUNITY                             , 2)         \
        ENUM_VALUE_SPEC(POLIO_EVOCLOCK_REVERSION_AND_IMMUNITY               , 3)         \
        ENUM_VALUE_SPEC(POLIO_EVOCLOCK_REVERSION                            , 4)         \
        ENUM_VALUE_SPEC(POLIO_EVOCLOCK_POISSONSITES                         , 5)
    ENUM_DECLARE(EvolutionPolioClockType, IDM_ENUMSPEC_EvolutionPolioClockType)


    #define IDM_ENUMSPEC_VDPVVirulenceModelType                                          \
        ENUM_VALUE_SPEC(POLIO_VDPV_NONVIRULENT                              , 0)         \
        ENUM_VALUE_SPEC(POLIO_VDPV_PARALYSIS                                , 1)         \
        ENUM_VALUE_SPEC(POLIO_VDPV_PARALYSIS_AND_LOG_INFECTIVITY            , 2)         \
        ENUM_VALUE_SPEC(POLIO_VDPV_LOG_PARALYSIS_AND_LOG_INFECTIVITY        , 3)
    ENUM_DECLARE(VDPVVirulenceModelType, IDM_ENUMSPEC_VDPVVirulenceModelType)


    #define IDM_ENUMSPEC_RiskGroup                                                       \
        ENUM_VALUE_SPEC(LOW                                                 , 0)         \
        ENUM_VALUE_SPEC(HIGH                                                , 1)         \
        ENUM_VALUE_SPEC(COUNT                                               , 2)
    ENUM_DECLARE(RiskGroup, IDM_ENUMSPEC_RiskGroup)


    // The AssortivityGroup is used to specify the group of people that a person might have an affinity for or against.
    // NO_GROUP                    -   pair with the first person in the list
    // STI_INFECTION_STATUS        -   pair with someone who is/isn't STI positive
    // INDIVIDUAL_PROPERTY         -   pair with someone who has an individual property that you like
    // STI_COINFECTION_STATUS      -   pair with someone who is/isn't STI Co-infected (only applies to HIV model)
    // HIV_INFECTION_STATUS        -   pair with someone who is/isn't HIV positive (based on infectivity) (only applies to HIV model)
    // HIV_TESTED_POSITIVE_STATUS  -   pair with someone who is/isn't tested positive for HIV (only applies to HIV model)
    // HIV_RECEIVED_RESULTS_STATUS -   pair with someone who has received similar HIV test results (only applies to HIV model)
    #define IDM_ENUMSPEC_AssortivityGroup                                                \
        ENUM_VALUE_SPEC(NO_GROUP                                            , 0)         \
        ENUM_VALUE_SPEC(STI_INFECTION_STATUS                                , 1)         \
        ENUM_VALUE_SPEC(INDIVIDUAL_PROPERTY                                 , 2)         \
        ENUM_VALUE_SPEC(STI_COINFECTION_STATUS                              , 3)         \
        ENUM_VALUE_SPEC(HIV_INFECTION_STATUS                                , 4)         \
        ENUM_VALUE_SPEC(HIV_TESTED_POSITIVE_STATUS                          , 5)         \
        ENUM_VALUE_SPEC(HIV_RECEIVED_RESULTS_STATUS                         , 6)
    ENUM_DECLARE(AssortivityGroup, IDM_ENUMSPEC_AssortivityGroup)


    #define IDM_ENUMSPEC_FormationRateType                                               \
        ENUM_VALUE_SPEC(CONSTANT                                            , 0)         \
        ENUM_VALUE_SPEC(SIGMOID_VARIABLE_WIDTH_HEIGHT                       , 1)         \
        ENUM_VALUE_SPEC(INTERPOLATED_VALUES                                 , 2)
    ENUM_DECLARE(FormationRateType, IDM_ENUMSPEC_FormationRateType)


    #define IDM_ENUMSPEC_RelationshipType                                                \
        ENUM_VALUE_SPEC(TRANSITORY                                          , 0)         \
        ENUM_VALUE_SPEC(INFORMAL                                            , 1)         \
        ENUM_VALUE_SPEC(MARITAL                                             , 2)         \
        ENUM_VALUE_SPEC(COMMERCIAL                                          , 3)         \
        ENUM_VALUE_SPEC(COUNT                                               , 4)
    ENUM_DECLARE(RelationshipType, IDM_ENUMSPEC_RelationshipType)


    #define IDM_ENUMSPEC_RelationshipState                                               \
        ENUM_VALUE_SPEC(NORMAL                                              , 0)         \
        ENUM_VALUE_SPEC(PAUSED                                              , 1)         \
        ENUM_VALUE_SPEC(MIGRATING                                           , 2)         \
        ENUM_VALUE_SPEC(TERMINATED                                          , 3)
    ENUM_DECLARE(RelationshipState, IDM_ENUMSPEC_RelationshipState)


    #define IDM_ENUMSPEC_RelationshipMigrationAction                                     \
        ENUM_VALUE_SPEC(PAUSE                                               , 0)         \
        ENUM_VALUE_SPEC(MIGRATE                                             , 1)         \
        ENUM_VALUE_SPEC(TERMINATE                                           , 2)
    ENUM_DECLARE(RelationshipMigrationAction, IDM_ENUMSPEC_RelationshipMigrationAction)


    #define IDM_ENUMSPEC_RelationshipTerminationReason                                   \
        ENUM_VALUE_SPEC(NOT_TERMINATING                                     , 0)         \
        ENUM_VALUE_SPEC(BROKEUP                                             , 1)         \
        ENUM_VALUE_SPEC(SELF_DIED                                           , 2)         \
        ENUM_VALUE_SPEC(SELF_MIGRATING                                      , 3)         \
        ENUM_VALUE_SPEC(PARTNER_TERMINATED                                  , 4)         \
        ENUM_VALUE_SPEC(PARTNER_MIGRATING                                   , 5)
    ENUM_DECLARE(RelationshipTerminationReason, IDM_ENUMSPEC_RelationshipTerminationReason)


    #define IDM_ENUMSPEC_ExtraRelationalFlagType                                         \
        ENUM_VALUE_SPEC(Independent                                         , 0)         \
        ENUM_VALUE_SPEC(Correlated                                          , 1)         \
        ENUM_VALUE_SPEC(COUNT                                               , 2)
    ENUM_DECLARE(ExtraRelationalFlagType, IDM_ENUMSPEC_ExtraRelationalFlagType)


    #define IDM_ENUMSPEC_TBDrugType                                                      \
        ENUM_VALUE_SPEC(DOTS                                                , 1)         \
        ENUM_VALUE_SPEC(DOTSImproved                                        , 2)         \
        ENUM_VALUE_SPEC(EmpiricTreatment                                    , 3)         \
        ENUM_VALUE_SPEC(FirstLineCombo                                      , 4)         \
        ENUM_VALUE_SPEC(SecondLineCombo                                     , 5)         \
        ENUM_VALUE_SPEC(ThirdLineCombo                                      , 6)         \
        ENUM_VALUE_SPEC(LatentTreatment                                     , 7)
    ENUM_DECLARE(TBDrugType, IDM_ENUMSPEC_TBDrugType)


    #define IDM_ENUMSPEC_TBFastProgressorType                                            \
        ENUM_VALUE_SPEC(AGE                                                 , 0)         \
        ENUM_VALUE_SPEC(POVERTY                                             , 1)         \
        ENUM_VALUE_SPEC(POVERTY_SUSCEPTIBILITY_TO_INFECTION                 , 2)         \
        ENUM_VALUE_SPEC(POVERTY_AND_SUSCEPTIBILITY                          , 3)
    ENUM_DECLARE(TBFastProgressorType, IDM_ENUMSPEC_TBFastProgressorType)


    #define IDM_ENUMSPEC_TBInfectionState                                                \
        ENUM_VALUE_SPEC(None                                                , 0)         \
        ENUM_VALUE_SPEC(Latent                                              , 1)         \
        ENUM_VALUE_SPEC(ActivePreSymptomatic                                , 2)         \
        ENUM_VALUE_SPEC(ActiveSymptomaticSmearPositive                      , 3)         \
        ENUM_VALUE_SPEC(ActiveSymptomaticSmearNegative                      , 4)         \
        ENUM_VALUE_SPEC(ActiveSymptomaticExtraPulmonary                     , 5)
    ENUM_DECLARE(TBInfectionState, IDM_ENUMSPEC_TBInfectionState)


    // NChooserEventCoordinatorHIV
    #define IDM_ENUMSPEC_TargetedDiseaseState                                            \
        ENUM_VALUE_SPEC(HIV_Positive                                        , 1)         \
        ENUM_VALUE_SPEC(HIV_Negative                                        , 2)         \
        ENUM_VALUE_SPEC(Tested_Positive                                     , 3)         \
        ENUM_VALUE_SPEC(Tested_Negative                                     , 4)         \
        ENUM_VALUE_SPEC(Male_Circumcision_Positive                          , 5)         \
        ENUM_VALUE_SPEC(Male_Circumcision_Negative                          , 6)         \
        ENUM_VALUE_SPEC(Has_Intervention                                    , 7)         \
        ENUM_VALUE_SPEC(Not_Have_Intervention                               , 8)
    ENUM_DECLARE(TargetedDiseaseState, IDM_ENUMSPEC_TargetedDiseaseState)


    // ReferenceTrackingEventCoordinatorHIV
    #define IDM_ENUMSPEC_TargetDiseaseStateType                                          \
        ENUM_VALUE_SPEC(Everyone                                            , 1)         \
        ENUM_VALUE_SPEC(HIV_Positive                                        , 2)         \
        ENUM_VALUE_SPEC(HIV_Negative                                        , 3)         \
        ENUM_VALUE_SPEC(Tested_Positive                                     , 4)         \
        ENUM_VALUE_SPEC(Tested_Negative                                     , 5)         \
        ENUM_VALUE_SPEC(Not_Tested_Or_Tested_Negative                       , 6)
    ENUM_DECLARE(TargetDiseaseStateType, IDM_ENUMSPEC_TargetDiseaseStateType)
}
