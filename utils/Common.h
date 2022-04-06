/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#define DAYSPERYEAR         (365)
#define IDEALDAYSPERMONTH   (30)
#define DAYSPERWEEK         (7)
#define HOURSPERDAY         (24)
#define MONTHSPERYEAR       (12)
#define MIN_YEAR            (1900)
#define MAX_YEAR            (2200)

#define ARCMINUTES_PER_DEGREE   (60)
#define KM_PER_ARCMINUTE        (1.86)
#define EARTH_RADIUS_KM         (6371.2213)

#define LOG_2                   (0.6931472f)

#define SHIFT_BIT                    (24)
#define MAX_24BIT               (1677215)
#define SHEDDING_HASH_SIZE         (1000)

#define EXPCDF(x)   (1 - exp(x))

#define NO_LESS_THAN( x, y ) { if ( x < y ) { x = y; } }
#define NO_MORE_THAN( x, y ) { if ( x > y ) { x = y; } }
#define BOUND_RANGE( x, y, z ) { NO_LESS_THAN(x, y); NO_MORE_THAN(x, z); }

struct AffectedPopulation {
    enum _enum {
        Minimum                 = 1,
        Everyone                = 1,
        ChildrenUnderFive       = 2,
        PossibleMothers         = 3,
        Infants                 = 4,    // <18 months
        ArrivingAirTravellers   = 5,
        DepartingAirTravellers  = 6,
        ArrivingRoadTravellers  = 7,
        DepartingRoadTravellers = 8,
        AllArrivingTravellers   = 9,
        AllDepartingTravellers  = 10,
        Maximum                 = 10
    };
};

struct InfectionStateChange {
    enum _enum {
        None            = 0,
        Cleared         = 1,
        Fatal           = 2,
        New             = 3,

        // polio only
        PolioParalysis_WPV1  = 4,
        PolioParalysis_WPV2  = 5,
        PolioParalysis_WPV3  = 6,
        PolioParalysis_VDPV1 = 7,
        PolioParalysis_VDPV2 = 8,
        PolioParalysis_VDPV3 = 9,

        // tb only
        TBLatent        = 10,
        TBActivation    = 11,
        TBInactivation  = 12,
        TBActivationSmearPos = 13,
        TBActivationSmearNeg = 14,
        TBActivationExtrapulm = 15,
        ClearedPendingRelapse = 16,
        TBActivationPresymptomatic = 17,

        DengueIncubated = 20,
        DengueReportable = 21
    };
};

enum struct HumanStateChange : unsigned int {
    None                  = 0,
    DiedFromNaturalCauses = 1,
    KilledByInfection     = 2,
    KilledByCoinfection   = 3,
    KilledByMCSampling    = 4,
    KilledByOpportunisticInfection = 5,
    Migrating             = 10
};

struct NewInfectionState {
    enum _enum {
        Invalid        = 0,
        NewInfection   = 1,
        NewlyDetected  = 2,
        NewAndDetected = 3,

        // tb only
        NewlyActive    = 7,
        NewlyInactive  = 8,
        NewlyCleared   = 9
    };
};

#define MAXIMUM_TRAVEL_WAYPOINTS    (10)

#define INFINITE_TIME   (365*1000)  // a full millennium
#define MAX_HUMAN_LIFETIME   45000  // ~123 years
#define MAX_HUMAN_AGE       (125.0f)

#define MAX_INTERVENTION_TYPE   (20)

#define DEFAULT_POVERTY_THRESHOLD   (0.5f)

#define DEFAULT_VACCINE_COST    (10)

struct SpatialCoverageType {
    enum _enum {
        None          = 0,
        All           = 1,
        CommunityList = 2,
        Polygon       = 4
    };
};

#define WEEKS_FOR_GESTATION   (40)

#define CHECK_INDEX(_i, _min, _count) assert((_i >= _min) && (_i < _count))

#define MILLIMETERS_PER_METER   (1000.0f)

////////////////////////////////////////

#ifndef ZERO_ITEM
#define ZERO_ITEM(_item)    memset(&(_item), 0, sizeof(_item))
#endif

#ifndef ZERO_ARRAY
#define ZERO_ARRAY(_array)  memset(_array, 0, sizeof(_array))
#endif

#ifndef INIT_ITEM
#define INIT_ITEM(_item) assert(sizeof(_item) == sizeof(init_##_item)); memcpy(&_item, init_##_item, sizeof(_item))
#endif

#ifndef INIT_ARRAY
#define INIT_ARRAY(_array) assert(sizeof(_array) == sizeof(init_##_array)); memcpy(_array, init_##_array, sizeof(_array))
#endif
