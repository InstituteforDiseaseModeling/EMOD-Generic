/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"
#include <cstdlib>
#include "ClimateConstant.h"
#include "Common.h"
#include "Debug.h"
#include "ISimulationContext.h"
#include "INodeContext.h"

SETUP_LOGGING( "ClimateConstant" )

namespace Kernel {

    ClimateConstant * ClimateConstant::CreateClimate( ClimateUpdateResolution::Enum update_resolution,
                                                      INodeContext * _parent,
                                                      float start_time,
                                                      RANDOMBASE* pRNG )
    {
        ClimateConstant * new_climate = _new_ ClimateConstant(update_resolution, _parent);
        release_assert( new_climate );

        // initialize climate values
        new_climate->UpdateWeather( start_time, 1.0f, pRNG );

        return new_climate;
    }

    ClimateConstant::ClimateConstant()
    { }
    ClimateConstant::ClimateConstant(ClimateUpdateResolution::Enum update_resolution, INodeContext * _parent)
    : Climate(update_resolution, _parent)
    {
    }

    bool ClimateConstant::IsPlausible()
    {
        const ClimateParams* cp = ClimateConfig::GetClimateParams();

        if( cp->base_airtemperature + (2 * cp->airtemperature_variance) > max_airtemp ||
            cp->base_airtemperature - (2 * cp->airtemperature_variance) < min_airtemp ||
            cp->base_landtemperature + (2 * cp->landtemperature_variance) > max_landtemp ||
            cp->base_landtemperature - (2 * cp->landtemperature_variance) < min_landtemp ||
            cp->base_rainfall < 0.0 ||
            cp->base_humidity > 1.0 ||
            cp->base_humidity < 0.0 )
        {
            LOG_DEBUG( "IsPlausible returning false\n" );
            return false;
        }

        if((cp->rainfall_variance_enabled && (EXPCDF(-1 /cp->base_rainfall * max_rainfall) < 0.975)) ||
            (!cp->rainfall_variance_enabled && cp->base_rainfall > max_rainfall))
        {
            LOG_DEBUG( "IsPlausible returning false\n" );
            return false;
        }

        return true;
    }

    void ClimateConstant::UpdateWeather( float time, float dt, RANDOMBASE* pRNG )
    {
        const ClimateParams* cp = ClimateConfig::GetClimateParams();

        m_airtemperature       =      cp->base_airtemperature;
        m_landtemperature      =      cp->base_landtemperature;
        m_accumulated_rainfall = dt * cp->base_rainfall;
        m_humidity             =      cp->base_humidity;

        Climate::UpdateWeather( time, dt, pRNG ); // call base-class UpdateWeather() to add stochasticity and check values are within valid bounds
    }
}