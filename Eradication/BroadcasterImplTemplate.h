/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>
#include "BroadcasterImpl.h"
#include "Log.h"
#include "Exceptions.h"

namespace Kernel
{
    // This file contains the implementation for the BroadcasterImpl.  We want to reduce the number
    // of files that contain this implementation so that when we make changes we do not compile a
    // ton files.

    template<class Observer, class Entity>
    BroadcasterImpl<Observer,Entity>::BroadcasterImpl()
        : observers()
        , disposed_observers()
    {
        int num_triggers = EventTrigger::NUM_EVENT_TRIGGERS;
        observers.resize( num_triggers );
        disposed_observers.resize( num_triggers );
    }

    template<class Observer, class Entity>
    BroadcasterImpl<Observer, Entity>::~BroadcasterImpl()
    {
        DisposeOfUnregisteredObservers();

        for( auto &observer_list : observers )
        {
            LOG_DEBUG_F( "Deleting %d observers.\n", observer_list.size() );

            for( auto &observer : observer_list )
            {
                observer->Release();
            }
        }
    }

    template<class Observer, class Entity>
    void BroadcasterImpl<Observer, Entity>::RegisterObserver( Observer* pObserver, const EventTrigger::Enum& trigger )
    {
        std::vector<Observer*>& event_observer_list = observers[ trigger ];

        if( std::find( event_observer_list.begin(), event_observer_list.end(), pObserver ) != event_observer_list.end() )
        {
            std::stringstream ss;
            ss << "Trying to register an observer (" << typeid(*pObserver).name() << ") more than once to event " << EventTrigger::pairs::lookup_key( trigger );
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        LOG_DEBUG_F( "Observer is registering for event %s.\n", EventTrigger::pairs::lookup_key( trigger ) );
        event_observer_list.push_back( pObserver );
        pObserver->AddRef();
    }

    template<class Observer, class Entity>
    void BroadcasterImpl<Observer, Entity>::UnregisterObserver( Observer* pObserver, const EventTrigger::Enum& trigger )
    {
        LOG_DEBUG( "[UnregisterObserver] Putting observer into the disposed observers list .\n" );
        disposed_observers[ trigger ].push_back( pObserver );
    }

    template<class Observer, class Entity>
    void BroadcasterImpl<Observer, Entity>::TriggerObservers( Entity* pEntity, const EventTrigger::Enum& trigger )
    {
        if( trigger == EventTrigger::NoTrigger )
        {
            return;
        }

        std::vector<Observer*>& observer_list = observers[ trigger ];
        std::vector<Observer*>& disposed_list = disposed_observers[ trigger ];

        LOG_DEBUG_F( "We have %d observers of event %s.\n", observer_list.size(), EventTrigger::pairs::lookup_key( trigger ) );
        for( auto observer : observer_list )
        {
            // ---------------------------------------------------------------------
            // --- Make sure the observer has not been requested to be unregistered
            // --- from being notified of events.
            // ---------------------------------------------------------------------
            bool notify = true;
            if( disposed_list.size() > 0 )
            {
                // finding the observer will make notify FALSE which means we don't notify them of the event
                notify = std::find( disposed_list.begin(), disposed_list.end(), observer ) == disposed_list.end();
            }

            if( notify )
            {
                observer->notifyOnEvent( pEntity, trigger );
            }
        }
    }

    template<class Observer, class Entity>
    void BroadcasterImpl<Observer, Entity>::DisposeOfUnregisteredObservers()
    {
        if( disposed_observers.size() > 0 )
        {
            LOG_DEBUG_F( "We have %d disposed_observers to clean up.\n", disposed_observers.size() );
        }

        for( int event_index = 0; event_index < disposed_observers.size(); ++event_index )
        {
            std::vector<Observer*>& disposed_list = disposed_observers[ event_index ];
            std::vector<Observer*>& current_list = observers[ event_index ];

            for( auto observer : disposed_list )
            {
                for( int i = 0; i < current_list.size(); ++i )
                {
                    if( current_list[ i ] == observer )
                    {
                        current_list[ i ] = current_list.back();
                        current_list.pop_back();
                        const char * event_name = "<TBD: get event name from id>";
                        LOG_DEBUG_F( "[UnregisterObserver] Removed observer from list: now %d observers of event %s.\n",
                                    current_list.size(),
                                    event_name
                        );
                        break;
                    }
                }
            }
            disposed_list.clear();
        }
    }
}
