/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ISupports.h"
#include "EventTrigger.h"

namespace Kernel
{
    // This file defines a set of interfaces used to interested objects when things happen to
    // other objects.  These set of interfaces support a modified version of the Gang of Four's
    // Observer pattern.  In this patten, we have the following objects:
    // - IEventObserver - This is an object that wants to hear/be notified about things that happen to other objects.
    // - IEventBroadcaster - This is the object that notifies the observers when stuff happens to the entities.
    // - IEntity - This is the object that has had something happen to it i.e. an event
    // - Trigger - This is object defines the event that happened to the entity.
    // Notice that is not quite the Observer pattern described by the Gang of Four.  In our pattern,
    // The "broadcaster" has the job of notifying "observers" when a "trigger" happens to an "entity".

    struct IEventCoordinatorEventContext;
    struct INodeEventContext;
    struct IIndividualHumanEventContext;
    typedef EventTrigger::Enum EventTriggerNode;
    typedef EventTrigger::Enum EventTriggerCoordinator;

    // Objects that are interested when events are "triggered" on a particular entity.
    // Objects that implement this interface and register with the broadcaster will
    // have this interface called when the registered event occurs.
    template<class IEntity, class Trigger>
    struct IEventObserver : ISupports
    {
        virtual bool notifyOnEvent( IEntity *pEntity, const Trigger& trigger ) = 0;
    };

    // The "broadcaster" has the job of notifying "observers" when a "trigger" happens to an "entity".
    template<class IObserver, class IEntity>
    struct IEventBroadcaster : ISupports
    {
        virtual void RegisterObserver(   IObserver* pObserver, const EventTrigger::Enum& trigger ) = 0;
        virtual void UnregisterObserver( IObserver* pObserver, const EventTrigger::Enum& trigger ) = 0;
        virtual void TriggerObservers(   IEntity*   pEntity,   const EventTrigger::Enum& trigger ) = 0;
    };

    // There are three sets of observer, broadcaster, entity combinations.  One for:
    // - Event Coordinators
    // - Nodes
    // - Individuals

    struct ICoordinatorEventObserver : IEventObserver<IEventCoordinatorEventContext, EventTriggerCoordinator>
    {
    };
    struct ICoordinatorEventBroadcaster : IEventBroadcaster<ICoordinatorEventObserver, IEventCoordinatorEventContext>
    {
    };

    struct INodeEventObserver : IEventObserver<INodeEventContext, EventTriggerNode>
    {
    };
    struct INodeEventBroadcaster : IEventBroadcaster<INodeEventObserver, INodeEventContext>
    {
    };

    struct IIndividualEventObserver : IEventObserver<IIndividualHumanEventContext, EventTrigger::Enum>
    {
    };
    struct IIndividualEventBroadcaster : IEventBroadcaster<IIndividualEventObserver, IIndividualHumanEventContext>
    {
    };
}