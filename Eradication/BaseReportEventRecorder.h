/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>

#include "BaseTextReportEvents.h"
#include "BroadcasterObserver.h"

namespace Kernel
{
    struct IdmDateTime;

    template<class Broadcaster, class Observer, class Entity>
    class BaseReportEventRecorder : public BaseTextReportEventsTemplate<Broadcaster, Observer, Entity>
    {
    public:

    protected:
        BaseReportEventRecorder( const std::string& rReportName );
        virtual ~BaseReportEventRecorder();

        // -----------------------------
        // --- BaseTextReportEvents
        // -----------------------------
        virtual bool Configure( const Configuration* inputJson ) override;
        virtual std::string GetHeader() const;

        // IObserver
        bool notifyOnEvent( Entity *pEntity, const EventTrigger::Enum& trigger );

    protected:
        virtual std::string GetOtherData( Entity *pEntity, const EventTrigger::Enum& trigger );
        virtual std::string GetTimeHeader() const;

        virtual float GetTime( Entity* pEntity ) const = 0;

        bool ignore_events_in_list;
        std::vector<EventTrigger::Enum> event_trigger_list;
    };
}
