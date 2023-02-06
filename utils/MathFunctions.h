/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <math.h>
#include "Exceptions.h"
#include "IdmApi.h"
#include "Types.h"

namespace Kernel {
    // calculate the great-circle distance between two points along the surface a spherical earth in kilometers
    double IDMAPI CalculateDistanceKm( double lon_1_deg, double lat_1_deg, double lon_2_deg, double lat_2_deg );

    float IDMAPI NTimeStepProbability( NonNegativeFloat PerTimeStepProbability, float dt);
}
