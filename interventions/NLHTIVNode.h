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
#include "NodeEventContext.h"
#include "Configure.h"
#include "PropertyRestrictions.h"
#include "EventTriggerNode.h"

namespace Kernel
{
    class NLHTIVNode : public INodeEventObserver, public BaseNodeIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, NLHTIVNode, INodeDistributableIntervention)

    public:        
        NLHTIVNode();
        NLHTIVNode( const NLHTIVNode& rMaster );
        virtual ~NLHTIVNode();
        virtual int AddRef() override;
        virtual int Release() override;
        virtual bool Configure( const Configuration* config ) override;

        // INodeDistributableIntervention
        virtual bool Distribute( INodeEventContext *pNodeEventContext, IEventCoordinator2 *pEC ) override;
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;
        virtual void Update(float dt) override;

        // INodeEventObserver
        virtual bool notifyOnEvent( INodeEventContext *context, const EventTriggerNode& trigger ) override;

    protected:
        void Unregister();

        std::vector<EventTrigger::Enum>   m_trigger_conditions;
        float max_duration;
        float duration;
        PropertyRestrictions<NPKey, NPKeyValue, NPKeyValueContainer> node_property_restrictions;
        float blackout_period ;
        float blackout_time_remaining ;
        EventTriggerNode blackout_event_trigger ;
        bool blackout_on_first_occurrence;
        bool notification_occurred ;
        std::vector<std::set<uint32_t>> event_occurred_list;
        INodeDistributableIntervention *_ndi;
        std::string m_ClassName;
    };
}
