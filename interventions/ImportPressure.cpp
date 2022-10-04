/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ImportPressure.h"

#include "Exceptions.h"
#include "InterventionFactory.h"
#include "NodeEventContext.h"  // for ISporozoiteChallengeConsumer methods
#include "RANDOM.h"

SETUP_LOGGING( "ImportPressure" );

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(ImportPressure)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(ImportPressure)

    IMPLEMENT_FACTORY_REGISTERED(ImportPressure)

    ImportPressure::ImportPressure() 
        : duration_counter(0.0f)
    {
        // Schema documentation
        initSimTypes( 12, "GENERIC_SIM", "VECTOR_SIM", "MALARIA_SIM", "AIRBORNE_SIM", "POLIO_SIM", "TBHIV_SIM", "STI_SIM", "HIV_SIM", "PY_SIM", "TYPHOID_SIM", "ENVIRONMENTAL_SIM", "DENGUE_SIM" );
    }

    ImportPressure::~ImportPressure() 
    {
        LOG_DEBUG_F( "dtor: expired = %d\n", expired );
    }

    bool ImportPressure::Configure( const Configuration * inputJson )
    {
        LOG_DEBUG_F( "%s\n", __FUNCTION__ );
        // TODO: specification for rate, seasonality, and age-biting function
        initConfigTypeMap( "Durations",              &durations,              IP_Durations_DESC_TEXT,               0.0f,  FLT_MAX );
        initConfigTypeMap( "Daily_Import_Pressures", &daily_import_pressures, IP_Daily_Import_Pressures_DESC_TEXT , 0.0f,  FLT_MAX );

        bool configured = Outbreak::Configure( inputJson );

        if( configured && !JsonConfigurable::_dryrun )
        {
            if( durations.size() != (daily_import_pressures.size()) )
            {
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                    "ImportPressure intervention requires Durations must be the same size as Daily_Import_Pressures" );
            }
            if( durations.size() == 0 )
            {
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                    "Empty Durations parameter in ImportPressure intervention." );
            }

            while (!durations.empty())
            {
                durations_and_pressures.push_back(std::make_pair(durations.back(), daily_import_pressures.back()));
                durations.pop_back();
                daily_import_pressures.pop_back();
            }
        }

        return configured;
    }

    bool ImportPressure::Distribute(INodeEventContext *context, IEventCoordinator2* pEC)
    {
        return BaseNodeIntervention::Distribute( context, pEC );
    }

    void ImportPressure::SetContextTo(INodeEventContext *context) 
    { 
        LOG_DEBUG_F( "%s\n", __FUNCTION__ );
        parent = context; 
    }

    // The durations are used in sequence as a series of relative deltas from the Start_Day of the campaign event.
    // t(now).......+t0....+t1...............+t2.+t3....+t4
    void ImportPressure::Update( float dt )
    {
        // Throw away any entries with durations less than current value of duration_counter, and reset duration_counter
        while( durations_and_pressures.size() > 0 && duration_counter >= durations_and_pressures.back().first )
        {
            LOG_DEBUG_F( "Discarding input entry with duration/pressure of %f/%f\n", durations_and_pressures.back().first, durations_and_pressures.back().second );
            durations_and_pressures.pop_back();
            duration_counter = 0.0f;
            LOG_DEBUG_F( "Remaining entries: %i\n", durations_and_pressures.size());
        }

        if ( durations_and_pressures.empty() )
        {
            expired = true;
            return;
        }

        duration_counter            += dt;
        float daily_import_pressure  = durations_and_pressures.back().second;   
        int   num_imports            = parent->GetRng()->Poisson(daily_import_pressure*dt);
        const StrainIdentity outbreak_strain(clade,genome);

        LOG_DEBUG_F("Duration counter = %f, Total duration = %f, import_pressure = %0.2f, import_cases = %d\n", duration_counter, durations_and_pressures.back().first, daily_import_pressure, num_imports);

        IOutbreakConsumer *ioc;
        if (s_OK == parent->QueryInterface(GET_IID(IOutbreakConsumer), (void**)&ioc))
        {
            ioc->AddImportCases(&outbreak_strain, import_age, num_imports, 1.0f, female_prob, mc_weight);
        }
    }
}

 
