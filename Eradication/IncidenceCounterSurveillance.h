/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "IncidenceEventCoordinator.h"
#include "BroadcasterObserver.h"
#include "SimulationEventContext.h"

namespace Kernel
{
    class IncidenceCounterSurveillance 
        : public IncidenceCounter, public ICoordinatorEventObserver, public INodeEventObserver
    {
    public:
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        IncidenceCounterSurveillance();
        virtual ~IncidenceCounterSurveillance();
        virtual bool Configure( const Configuration * inputJson ) override;
        virtual void ConfigureTriggers( const Configuration * inputJson ) override;
        virtual void CheckConfigurationTriggers() override;
        virtual void Update( float dt ) override;
        virtual void RegisterForEvents( ISimulationEventContext* context ) override;
        virtual void UnregisterForEvents( ISimulationEventContext * context ) override;
        virtual void RegisterForEvents( INodeEventContext* pNEC ) override;
        virtual void UnregisterForEvents( INodeEventContext* pNEC ) override;

        void SetPercentageEventsToCount( const std::vector<std::string>& rPercentageEvents );

    protected:
        virtual void StartCounting() override;
        virtual bool IsDoneCounting() const override;        

        // IIndividualEventObserver methods
        virtual bool notifyOnEvent( IEventCoordinatorEventContext *pEntity, const EventTrigger::Enum& trigger ) override;
        virtual uint32_t GetCountOfQualifyingPopulation( const std::vector<INodeEventContext*>& rNodes ) override;
        virtual bool notifyOnEvent( INodeEventContext *pEntity, const EventTrigger::Enum& trigger ) override;
        virtual bool notifyOnEvent( IIndividualHumanEventContext * pEntity, const EventTrigger::Enum & trigger ) override;

    private:
        float m_CounterPeriod;
        float m_CounterPeriod_current;
        EventType::Enum       m_CounterEventType;

        std::vector<EventTrigger::Enum>             m_TriggerConditionListIndividual;
        std::vector<EventTrigger::Enum>        m_TriggerConditionListNode;
        std::vector<EventTrigger::Enum> m_TriggerConditionListCoordinator;

        std::vector<EventTrigger::Enum>           m_PercentageEventsToCountIndividual;
        std::vector<EventTrigger::Enum>        m_PercentageEventsToCountNode;
        std::vector<EventTrigger::Enum> m_PercentageEventsToCountCoordinator;
        uint32_t m_PercentageEventsCounted;
    };
}
