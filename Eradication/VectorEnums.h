/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <map>
#include "EnumSupport.h"

namespace Kernel
{
    typedef std::map<std::string, float> vector_float_map_t;
    typedef std::map<std::string, int32_t> vector_int_map_t;

    // ENUM defs for Egg_Hatch_Delay_Distribution
    #define IDM_ENUMSPEC_EggHatchDelayDist                                               \
        ENUM_VALUE_SPEC(NO_DELAY                                            ,   0)       \
        ENUM_VALUE_SPEC(EXPONENTIAL_DISTRIBUTION                            ,   1)
    ENUM_DECLARE(EggHatchDelayDist, IDM_ENUMSPEC_EggHatchDelayDist)


    // ENUM defs for Egg_Saturation_At_Oviposition
    #define IDM_ENUMSPEC_EggSaturation                                                   \
        ENUM_VALUE_SPEC(NO_SATURATION                                       ,   0)       \
        ENUM_VALUE_SPEC(SATURATION_AT_OVIPOSITION                           ,   1)       \
        ENUM_VALUE_SPEC(SIGMOIDAL_SATURATION                                ,   2)
    ENUM_DECLARE(EggSaturation, IDM_ENUMSPEC_EggSaturation)


    // ENUM defs for Larval_Density_Dependence
    #define IDM_ENUMSPEC_LarvalDensityDependence                                         \
        ENUM_VALUE_SPEC(UNIFORM_WHEN_OVERPOPULATION                         ,   0)       \
        ENUM_VALUE_SPEC(GRADUAL_INSTAR_SPECIFIC                             ,   1)       \
        ENUM_VALUE_SPEC(LARVAL_AGE_DENSITY_DEPENDENT_MORTALITY_ONLY         ,   2)       \
        ENUM_VALUE_SPEC(NO_DENSITY_DEPENDENCE                               ,   3)       \
        ENUM_VALUE_SPEC(DENSITY_DELAYED_GROWTH_NOT_MORTALITY                ,   4)
    ENUM_DECLARE(LarvalDensityDependence, IDM_ENUMSPEC_LarvalDensityDependence)


    // ENUM defs for Egg_Hatch_Density_Dependence
    #define IDM_ENUMSPEC_EggHatchDensityDependence                                       \
        ENUM_VALUE_SPEC(NO_DENSITY_DEPENDENCE                               ,   0)       \
        ENUM_VALUE_SPEC(DENSITY_DEPENDENCE                                  ,   1)
    ENUM_DECLARE(EggHatchDensityDependence, IDM_ENUMSPEC_EggHatchDensityDependence)


    // ENUM defs for Temperature_Dependent_Feeding_Cycle
    #define IDM_ENUMSPEC_TemperatureDependentFeedingCycle                                \
        ENUM_VALUE_SPEC(NO_TEMPERATURE_DEPENDENCE                           ,   0)       \
        ENUM_VALUE_SPEC(ARRHENIUS_DEPENDENCE                                ,   1)       \
        ENUM_VALUE_SPEC(BOUNDED_DEPENDENCE                                  ,   2)
    ENUM_DECLARE(TemperatureDependentFeedingCycle, IDM_ENUMSPEC_TemperatureDependentFeedingCycle)


    // ENUM defs for Vector_Larval_Rainfall_Mortality
    #define IDM_ENUMSPEC_VectorRainfallMortality                                         \
        ENUM_VALUE_SPEC(NONE                                                ,   0)       \
        ENUM_VALUE_SPEC(SIGMOID                                             ,   1)       \
        ENUM_VALUE_SPEC(SIGMOID_HABITAT_SHIFTING                            ,   2)
    ENUM_DECLARE(VectorRainfallMortality, IDM_ENUMSPEC_VectorRainfallMortality)


    // ENUM defs for Vector_Sampling_Type
    #define IDM_ENUMSPEC_VectorSamplingType                                              \
        ENUM_VALUE_SPEC(TRACK_ALL_VECTORS                                   ,   1)       \
        ENUM_VALUE_SPEC(SAMPLE_IND_VECTORS                                  ,   2)       \
        ENUM_VALUE_SPEC(VECTOR_COMPARTMENTS_NUMBER                          ,   3)       \
        ENUM_VALUE_SPEC(VECTOR_COMPARTMENTS_PERCENT                         ,   4)
    ENUM_DECLARE(VectorSamplingType, IDM_ENUMSPEC_VectorSamplingType)


    // ENUM defs for Vector_Sugar_Feeding_Frequency
    #define IDM_ENUMSPEC_VectorSugarFeeding                                              \
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_NONE                           ,   0)       \
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_ON_EMERGENCE_ONLY              ,   1)       \
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_EVERY_FEED                     ,   2)       \
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_EVERY_DAY                      ,   3)
    ENUM_DECLARE(VectorSugarFeeding, IDM_ENUMSPEC_VectorSugarFeeding)


    // ENUM defs for Habitat_Type in VectorSpeciesParameters
    #define IDM_ENUMSPEC_VectorHabitatType                                               \
        ENUM_VALUE_SPEC(NONE                                                ,   0)       \
        ENUM_VALUE_SPEC(TEMPORARY_RAINFALL                                  ,   1)       \
        ENUM_VALUE_SPEC(WATER_VEGETATION                                    ,   2)       \
        ENUM_VALUE_SPEC(HUMAN_POPULATION                                    ,   3)       \
        ENUM_VALUE_SPEC(CONSTANT                                            ,   4)       \
        ENUM_VALUE_SPEC(BRACKISH_SWAMP                                      ,   5)       \
        ENUM_VALUE_SPEC(MARSHY_STREAM                                       ,   6)       \
        ENUM_VALUE_SPEC(LINEAR_SPLINE                                       ,   7)       \
        ENUM_VALUE_SPEC(ALL_HABITATS                                        , 100)
    ENUM_DECLARE(VectorHabitatType, IDM_ENUMSPEC_VectorHabitatType)


    #define IDM_ENUMSPEC_HEGModel                                                        \
        ENUM_VALUE_SPEC(OFF                                                 ,   0)       \
        ENUM_VALUE_SPEC(GERMLINE_HOMING                                     ,   1)       \
        ENUM_VALUE_SPEC(EGG_HOMING                                          ,   2)       \
        ENUM_VALUE_SPEC(DUAL_GERMLINE_HOMING                                ,   3)       \
        ENUM_VALUE_SPEC(DRIVING_Y                                           ,   4)
    ENUM_DECLARE(HEGModel, IDM_ENUMSPEC_HEGModel)


    #define IDM_ENUMSPEC_AgeDependentBitingRisk                                          \
        ENUM_VALUE_SPEC(OFF                                                 ,   0)       \
        ENUM_VALUE_SPEC(LINEAR                                              ,   1)       \
        ENUM_VALUE_SPEC(SURFACE_AREA_DEPENDENT                              ,   2)
    ENUM_DECLARE(AgeDependentBitingRisk, IDM_ENUMSPEC_AgeDependentBitingRisk)


    #define IDM_ENUMSPEC_VectorPoolIdEnum                                                \
        ENUM_VALUE_SPEC(INDOOR_VECTOR_POOL                                  ,   0)       \
        ENUM_VALUE_SPEC(OUTDOOR_VECTOR_POOL                                 ,   1)       \
        ENUM_VALUE_SPEC(BOTH_VECTOR_POOLS                                   ,   2)
    ENUM_DECLARE(VectorPoolIdEnum, IDM_ENUMSPEC_VectorPoolIdEnum)


    // State counters for feeding cycle, reporting, etc.
    #define IDM_ENUMSPEC_VectorStateEnum                                                 \
        ENUM_VALUE_SPEC(STATE_ADULT                                         ,   1)       \
        ENUM_VALUE_SPEC(STATE_INFECTED                                      ,   2)       \
        ENUM_VALUE_SPEC(STATE_INFECTIOUS                                    ,   3)       \
        ENUM_VALUE_SPEC(STATE_MALE                                          ,   4)       \
        ENUM_VALUE_SPEC(STATE_EGG                                           ,   5)       \
        ENUM_VALUE_SPEC(STATE_LARVA                                         ,   6)       \
        ENUM_VALUE_SPEC(STATE_IMMATURE                                      ,   7)
    ENUM_DECLARE(VectorStateEnum, IDM_ENUMSPEC_VectorStateEnum)


    #define IDM_ENUMSPEC_VectorGender                                                    \
        ENUM_VALUE_SPEC(VECTOR_FEMALE                                       ,   0)       \
        ENUM_VALUE_SPEC(VECTOR_MALE                                         ,   1)       \
        ENUM_VALUE_SPEC(VECTOR_BOTH_GENDERS                                 ,   2)
    ENUM_DECLARE(VectorGender, IDM_ENUMSPEC_VectorGender)


    #define IDM_ENUMSPEC_VectorSterility                                                 \
        ENUM_VALUE_SPEC(VECTOR_FERTILE                                      ,   0)       \
        ENUM_VALUE_SPEC(VECTOR_STERILE                                      ,   1)
    ENUM_DECLARE(VectorSterility, IDM_ENUMSPEC_VectorSterility)


    #define IDM_ENUMSPEC_VectorWolbachia                                                 \
        ENUM_VALUE_SPEC(WOLBACHIA_FREE                                      ,   0)       \
        ENUM_VALUE_SPEC(VECTOR_WOLBACHIA_A                                  ,   1)       \
        ENUM_VALUE_SPEC(VECTOR_WOLBACHIA_B                                  ,   2)       \
        ENUM_VALUE_SPEC(VECTOR_WOLBACHIA_AB                                 ,   3)
    ENUM_DECLARE(VectorWolbachia, IDM_ENUMSPEC_VectorWolbachia)


    #define IDM_ENUMSPEC_VectorAllele                                                    \
        ENUM_VALUE_SPEC(WILD                                                ,   0)       \
        ENUM_VALUE_SPEC(HALF                                                ,   1)       \
        ENUM_VALUE_SPEC(FULL                                                ,   2)       \
        ENUM_VALUE_SPEC(NotMated                                            ,   3)
    ENUM_DECLARE(VectorAllele, IDM_ENUMSPEC_VectorAllele)
}
