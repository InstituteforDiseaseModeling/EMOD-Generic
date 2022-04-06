/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"
#include "CampaignEventByYear.h"
#ifndef DISABLE_STI
#include "SimulationSTI.h"
#endif
#ifdef ENABLE_TYPHOID
#include "SimulationTyphoid.h"
#endif

SETUP_LOGGING("CampaignEventByYear")

namespace Kernel
{
#if !defined(DISABLE_STI) || defined(ENABLE_TYPHOID)
    IMPL_QUERY_INTERFACE1(CampaignEventByYear, IConfigurable)
    IMPLEMENT_FACTORY_REGISTERED(CampaignEventByYear)

    CampaignEventByYear::CampaignEventByYear()
    : start_year(0.0f)
    { }

    CampaignEventByYear::~CampaignEventByYear()
    { }

    bool CampaignEventByYear::Configure(const Configuration* inputJson)
    {
        initConfigTypeMap( "Start_Year", &start_year, Start_Year_DESC_TEXT, MIN_YEAR, MAX_YEAR, MIN_YEAR );
        initConfigComplexType( "Nodeset_Config", &nodeset_config, Nodeset_Config_DESC_TEXT );
        initConfigComplexType( "Event_Coordinator_Config", &event_coordinator_config, Event_Coordinator_Config_DESC_TEXT );

        // Bypasss CampaignEvent base class so that we don't break without Start_Day!
        bool ret = JsonConfigurable::Configure( inputJson );

        return ret;
    }

    bool CampaignEventByYear::Validate( const ISimulationContext* parent_sim )
    {
        if( parent_sim->GetParams()->sim_type != SimType::STI_SIM    &&
            parent_sim->GetParams()->sim_type != SimType::HIV_SIM    &&
            parent_sim->GetParams()->sim_type != SimType::TYPHOID_SIM  )
        {
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "CampainEventByYear can only be used in TYPHOID, STI, and HIV simulations." );
        }

        start_day = (start_year - parent_sim->GetParams()->sim_time_base_year) * DAYSPERYEAR;

        return true;
    }

#endif
}
