/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseReportEventRecorder.h"
#include "Log.h"

namespace Kernel
{
    template<class Broadcaster, class Observer, class Entity>
    BaseReportEventRecorder<Broadcaster,Observer,Entity>::BaseReportEventRecorder( const std::string& rReportName )
        : BaseTextReportEventsTemplate<Broadcaster, Observer, Entity>( rReportName )
        , ignore_events_in_list( false )
        , event_trigger_list()
    { }

    template<class Broadcaster, class Observer, class Entity>
    BaseReportEventRecorder<Broadcaster, Observer, Entity>::~BaseReportEventRecorder()
    { }

    template<class Broadcaster, class Observer, class Entity>
    bool BaseReportEventRecorder<Broadcaster, Observer, Entity>::Configure( const Configuration * inputJson )
    {
        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret && !JsonConfigurable::_dryrun )
        {
            // This is a pile of crap from the old way of doing it. Just iterate through an enum now. 
            std::vector<EventTrigger::Enum> all_trigger_list;
            for( unsigned int trigger_idx = EventTrigger::NoTrigger; trigger_idx < EventTrigger::NUM_EVENT_TRIGGERS; trigger_idx++ )
            {
                EventTrigger::Enum trigger = EventTrigger::Enum( trigger_idx );
                all_trigger_list.push_back( trigger );
            }

            for( auto idx : all_trigger_list )
            {
                bool in_event_list = std::find( event_trigger_list.begin(), event_trigger_list.end(), idx ) != event_trigger_list.end(); 
                if( ignore_events_in_list != in_event_list )
                {
                    // list of events to listen for
                    BaseTextReportEventsTemplate<Broadcaster, Observer, Entity>::eventTriggerList.push_back( idx );
                }
            }
        }

        return ret;
    }

    template<class Broadcaster, class Observer, class Entity>
    std::string BaseReportEventRecorder<Broadcaster, Observer, Entity>::GetHeader() const
    {
        return GetTimeHeader();
    }

    template<class Broadcaster, class Observer, class Entity>
    bool BaseReportEventRecorder<Broadcaster, Observer, Entity>::notifyOnEvent( Entity *pEntity, const EventTrigger::Enum& trigger )
    {
        GetOutputStream() << GetTime( pEntity );

        GetOutputStream() << GetOtherData( pEntity, trigger );

        GetOutputStream() << std::endl;

        return true;
    }

    template<class Broadcaster, class Observer, class Entity>
    std::string BaseReportEventRecorder<Broadcaster, Observer, Entity>::GetTimeHeader() const
    {
        return "Time";
    }

    template<class Broadcaster, class Observer, class Entity>
    std::string BaseReportEventRecorder<Broadcaster, Observer, Entity>::GetOtherData( Entity *pEntity, const EventTrigger::Enum& trigger )
    {
        return "";
    }
}
