/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IdmApi.h"
#include "Common.h" // for constants
#include "Types.h" // for NaturalNumber

namespace Kernel {

    struct IDMAPI IdmDateTime
    {

    public:
        IdmDateTime()
          : time(0.0f)
          , timestep(0)
          , _base_year(0.0f)
        { }

        explicit IdmDateTime(float time_in)
          : time(time_in)
          , timestep(0)
          , _base_year(0.0f)
        { }

        NonNegativeFloat Year() const
        {
            return _base_year + time/DAYSPERYEAR;
        }
    
        void Update(float time_delta)
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

        NonNegativeFloat time;

    private:
        NonNegativeFloat  _base_year;

        NaturalNumber     timestep;
    };
}
