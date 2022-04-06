/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <math.h>
#include "Exceptions.h"
#include "EnumSupport.h"
#include "IdmApi.h"
#include "Types.h"

namespace Kernel {
    class RANDOMBASE;

    // ENUM defs for INCUBATION_DISTRIBUTION, INFECTIOUS_DISTRIBUTION, BASE_INFECTIVITY_DISTRIBUTION
    ENUM_DEFINE(DistributionFunction, 
        ENUM_VALUE_SPEC( NOT_INITIALIZED                                     ,-1 ) 
        ENUM_VALUE_SPEC( CONSTANT_DISTRIBUTION                               , 0 )
        ENUM_VALUE_SPEC( UNIFORM_DISTRIBUTION                                , 1 )
        ENUM_VALUE_SPEC( GAUSSIAN_DISTRIBUTION                               , 2 )
        ENUM_VALUE_SPEC( EXPONENTIAL_DISTRIBUTION                            , 3 )
        ENUM_VALUE_SPEC( POISSON_DISTRIBUTION                                , 4 )
        ENUM_VALUE_SPEC( LOG_NORMAL_DISTRIBUTION                             , 5 )
        ENUM_VALUE_SPEC( DUAL_CONSTANT_DISTRIBUTION                          , 6 )
//        ENUM_VALUE_SPEC( PIECEWISE_CONSTANT                                  , 7 )    // Disable these distributions, but leave index as is (see demographics)
//        ENUM_VALUE_SPEC( PIECEWISE_LINEAR                                    , 8 )
        ENUM_VALUE_SPEC( WEIBULL_DISTRIBUTION                                , 9 )
        ENUM_VALUE_SPEC( DUAL_EXPONENTIAL_DISTRIBUTION                       ,10 )
        ENUM_VALUE_SPEC( GAMMA_DISTRIBUTION                                  ,11 )
        )

    // calculate the great-circle distance between two points along the surface a spherical earth in kilometers
    double IDMAPI CalculateDistanceKm( double lon_1_deg, double lat_1_deg, double lon_2_deg, double lat_2_deg );

    float IDMAPI NTimeStepProbability( NonNegativeFloat PerTimeStepProbability, float dt);
}
