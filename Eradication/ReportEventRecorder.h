/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>

#include "BaseReportEventRecorder.h"

namespace Kernel
{
    class ReportEventRecorder : public BaseReportEventRecorder< IIndividualEventBroadcaster,
                                                                IIndividualEventObserver,
                                                                IIndividualHumanEventContext >
    {
    public:
        GET_SCHEMA_STATIC_WRAPPER( ReportEventRecorder )

    public:
        static IReport* CreateReport();

    public:
        ReportEventRecorder();
        virtual ~ReportEventRecorder();

        // ------------
        // --- IReport
        // ------------
        virtual bool Configure( const Configuration* inputJson ) override;
        virtual void Initialize( unsigned int nrmSize ) override;
        virtual void UpdateEventRegistration( float currentTime,
                                              float dt,
                                              std::vector<INodeEventContext*>& rNodeEventContextList,
                                              ISimulationEventContext* pSimEventContext ) override;

        virtual std::string GetHeader() const override;

    protected:
        virtual std::string GetOtherData( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger ) override;
        virtual float GetTime( IIndividualHumanEventContext* pEntity ) const override;

        jsonConfigurable::tDynamicStringSet properties_to_report;
    };
}
