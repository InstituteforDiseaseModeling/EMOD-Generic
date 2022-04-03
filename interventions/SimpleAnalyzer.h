/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "InterventionFactory.h"
#include "Interventions.h"
#include "InterventionEnums.h"
#include "Configuration.h"
#include "Configure.h"
#include "EventTrigger.h"

namespace Kernel
{
    class SimpleAnalyzer : public IIndividualEventObserver, 
                           public BaseNodeIntervention
    {
    protected:
        // ctor
        SimpleAnalyzer();

    public:
        // dtor
        virtual ~SimpleAnalyzer();

        // InterventionFactory
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleAnalyzer, INodeDistributableIntervention) 

        // JsonConfigurable
        DECLARE_CONFIGURED(SimpleAnalyzer)

        // INodeDistributableIntervention
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC=NULL); 
        virtual void SetContextTo(INodeEventContext *context);
        virtual void Update(float dt);

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        // IIndividualEventObserver
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger );
        virtual std::string GetTriggerCondition() const;

    protected:
        float  m_timer;
        float  m_reporting_interval;
        float  m_coverage;
        EventTrigger::Enum  m_trigger_condition;

        INodeEventContext * m_parent;
    };
}

#if 0
namespace Kernel {
    template<class Archive>
    void serialize(Archive &ar, SimpleAnalyzer& obj, const unsigned int v)
    {
        ar & obj.m_timer;
        ar & obj.m_reporting_interval;
        ar & obj.m_trigger_condition;
    }
}
#endif