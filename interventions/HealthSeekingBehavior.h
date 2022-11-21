/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "Configure.h"
#include "NodeEventContext.h"    // for INodeEventContext (ICampaignCostObserver)
#include "EventTrigger.h"

namespace Kernel
{

    class SimpleHealthSeekingBehavior : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleHealthSeekingBehavior, IDistributableIntervention)

    public:
        SimpleHealthSeekingBehavior();
        SimpleHealthSeekingBehavior( const SimpleHealthSeekingBehavior& rMaster );
        virtual ~SimpleHealthSeekingBehavior();

        // We inherit AddRef/Release abstractly through IHealthSeekBehavior,
        // even though BaseIntervention has a non-abstract version.
        virtual int32_t AddRef() override { return BaseIntervention::AddRef(); }
        virtual int32_t Release() override { return BaseIntervention::Release(); }

        virtual bool Configure( const Configuration* config ) override;

        // IDistributingDistributableIntervention
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;
        virtual void Update(float dt) override;

    protected:

        float probability_of_seeking;
        EventOrConfig::Enum use_event_or_config;
        EventTrigger::Enum actual_intervention_event;
        IDistributableIntervention* m_di;
        bool single_use;

        DECLARE_SERIALIZABLE(SimpleHealthSeekingBehavior);
    };
}
