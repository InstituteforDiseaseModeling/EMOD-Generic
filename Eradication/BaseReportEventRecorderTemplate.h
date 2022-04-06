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
    std::string BaseReportEventRecorder<Broadcaster, Observer, Entity>::GetEnableParameterName()
    {
        return ENABLE_PARAMETER_NAME;
    }

    template<class Broadcaster, class Observer, class Entity>
    BaseReportEventRecorder<Broadcaster,Observer,Entity>::BaseReportEventRecorder( const std::string& rReportName )
        : BaseTextReportEventsTemplate<Broadcaster, Observer, Entity>( rReportName )
        , ignore_events_in_list( false )
        , m_EnableParameterName( ENABLE_PARAMETER_NAME )
        , m_EventsListName( EVENTS_LIST_NAME )
        , m_EventsListDesc( EVENTS_LIST_DESC )
        , m_IgnoreEventsListName( IGNORE_EVENTS_LIST_NAME )
        , m_IgnoreEventsListDesc( IGNORE_EVENTS_LIST_DESC )
    {
    }

    template<class Broadcaster, class Observer, class Entity>
    BaseReportEventRecorder<Broadcaster, Observer, Entity>::~BaseReportEventRecorder()
    {
    }

    template<class Broadcaster, class Observer, class Entity>
    bool BaseReportEventRecorder<Broadcaster, Observer, Entity>::Configure( const Configuration * inputJson )
    {
        std::vector<EventTrigger::Enum> tmp_event_trigger_list;

        JsonConfigurable::initVectorConfig( m_EventsListName.c_str(), tmp_event_trigger_list, inputJson, MetadataDescriptor::VectorOfEnum(m_EventsListName.c_str(), m_EventsListDesc.c_str(), MDD_ENUM_ARGS(EventTrigger)), m_EnableParameterName.c_str());
        JsonConfigurable::initConfigTypeMap( m_IgnoreEventsListName.c_str(), &ignore_events_in_list,  m_IgnoreEventsListDesc.c_str(), false, m_EnableParameterName.c_str() );

        
        ConfigureOther( inputJson );

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret && !JsonConfigurable::_dryrun )
        {
            if( !ignore_events_in_list && tmp_event_trigger_list.empty() )
            {
                std::stringstream ss;
                ss << "No data will be recorded.  The " << m_EventsListName << " list is empty and " << m_IgnoreEventsListName << " is false.\n";
                LOG_WARN( ss.str().c_str() );
            }
            else
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
                    bool in_event_list = std::find( tmp_event_trigger_list.begin(), tmp_event_trigger_list.end(), idx ) != tmp_event_trigger_list.end(); 
                    if( ignore_events_in_list != in_event_list )
                    {
                        // list of events to listen for
                        BaseTextReportEventsTemplate<Broadcaster, Observer, Entity>::eventTriggerList.push_back( idx );
                    }
                }
            }
        }

        return ret;
    }

    template<class Broadcaster, class Observer, class Entity>
    void BaseReportEventRecorder<Broadcaster, Observer, Entity>::ConfigureOther( const Configuration* inputJson )
    {
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
