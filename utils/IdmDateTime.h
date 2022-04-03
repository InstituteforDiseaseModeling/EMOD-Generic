/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IdmApi.h"
#include "Common.h" // for constants
#include "Types.h" // for NaturalNumber

// fwd declare outside namespace
class BaseChannelReport;

namespace Kernel {

    struct IDMAPI IdmDateTime
    {
        friend class Simulation;
        friend class SimulationHIV;
        friend class ReportHIVByAgeAndGender;
        friend class CampaignEventByYear;
        friend class SimulationTyphoid;
        friend class ReportTyphoidByAgeAndGender;

    public:
        IdmDateTime()
        : time(0.0f)
        , timestep(0)
        , _base_year(0.0f)
        , time_delta(0.0f)
        { }

        explicit IdmDateTime( NonNegativeFloat start_time )
        {
            time = start_time;
        }

        bool operator<( const float& compThis ) const
        {
            if( time < compThis )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool operator<=( const float& compThis ) const
        {
            if( time <= compThis )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool operator==( const float& compThis ) const
        {
            if( time == compThis )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        NonNegativeFloat Year() const
        {
            return _base_year + time/DAYSPERYEAR;
        }
    
        void Update()
        {
            timestep ++;
            time += time_delta;
        }

        float TimeAsSimpleTimestep() const
        {
            return timestep;
        }

        void setBaseYear( NonNegativeFloat base_year_in )
        {
            _base_year = base_year_in;
        }

        NonNegativeFloat getBaseYear() const
        {
            return _base_year;
        }

        float GetTimeDelta() const
        {
            return time_delta;
        }

        void SetTimeDelta(float time_delta_in)
        {
            time_delta = time_delta_in;
        }

        NaturalNumber timestep;
        NonNegativeFloat time;

    private:
        NonNegativeFloat _base_year;
        float time_delta;
    };
}
