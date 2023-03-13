/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportSurveillanceEventRecorder.h"
#include "Log.h"
#include "Exceptions.h"
#include "EventCoordinator.h"
#include "SimulationEventContext.h"
#include "ISurveillanceReporting.h"

SETUP_LOGGING( "ReportSurveillanceEventRecorder" )

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL( ReportSurveillanceEventRecorder, ReportSurveillanceEventRecorder )

    IReport* ReportSurveillanceEventRecorder::CreateReport()
    {
        return new ReportSurveillanceEventRecorder();
    }

    ReportSurveillanceEventRecorder::ReportSurveillanceEventRecorder()
        : ReportEventRecorderCoordinator( "ReportSurveillanceEventRecorder.csv" )
        , m_StatsByIpKeyNames()
        , m_ReportStatsByIP()
    { }

    ReportSurveillanceEventRecorder::~ReportSurveillanceEventRecorder()
    { }

    bool ReportSurveillanceEventRecorder::Configure( const Configuration* inputJson )
    {
        m_StatsByIpKeyNames.value_source = IPKey::GetConstrainedStringConstraintKey();

        initVectorConfig("Report_Surveillance_Event_Recorder_Events",  event_trigger_list,  inputJson,  MetadataDescriptor::VectorOfEnum("Report_Surveillance_Event_Recorder_Events", Report_Surveillance_Event_Recorder_Events_DESC_TEXT, MDD_ENUM_ARGS(EventTrigger)),  "Enable_Report_Surveillance_Event_Recorder");

        initConfigTypeMap("Report_Surveillance_Event_Recorder_Ignore_Events_In_List",  &ignore_events_in_list,  Report_Surveillance_Event_Recorder_Ignore_Events_In_List_DESC_TEXT,  false,  "Enable_Report_Surveillance_Event_Recorder");
        initConfigTypeMap("Report_Surveillance_Event_Recorder_Stats_By_IPs",           &m_StatsByIpKeyNames,    Report_Surveillance_Event_Recorder_Stats_By_IPs_DESC_TEXT,                   "Enable_Report_Surveillance_Event_Recorder");

        return BaseReportEventRecorder::Configure(inputJson);
    }

    void ReportSurveillanceEventRecorder::Initialize( unsigned int nrmSize )
    {
        m_ReportStatsByIP.SetIPKeyNames( "Report_Surveillance_Event_Recorder_Stats_By_IPs", m_StatsByIpKeyNames );

        ReportEventRecorderCoordinator::Initialize( nrmSize );
    }

    std::string ReportSurveillanceEventRecorder::GetHeader() const
    {
        std::stringstream header;
        header << ReportEventRecorderCoordinator::GetHeader()
               << "," << "NumCounted"
               << "," << "ThresholdOfAction"
               << "," << m_ReportStatsByIP.GetHeader();

        return header.str();
    }

    bool ReportSurveillanceEventRecorder::notifyOnEvent( IEventCoordinatorEventContext *pEntity, const EventTriggerCoordinator& trigger )
    {
        ISurveillanceReporting* p_isr = nullptr;
        if( pEntity->QueryInterface( GET_IID( ISurveillanceReporting ), (void**)&p_isr ) == s_OK )
        {
            return ReportEventRecorderCoordinator::notifyOnEvent( pEntity, trigger );
        }
        else
        {
            return false;
        }
    }

    std::string ReportSurveillanceEventRecorder::GetOtherData( IEventCoordinatorEventContext *pEntity,
                                                              const EventTriggerCoordinator& trigger )
    {
        ISurveillanceReporting* p_isr = nullptr;
        if( pEntity->QueryInterface( GET_IID( ISurveillanceReporting ), (void**)&p_isr ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                                           "pEntity", "ISurveillanceReporting", "IEventCoordinatorEventContext" );
        }

        m_ReportStatsByIP.ResetData();

        p_isr->CollectStats( m_ReportStatsByIP );

        std::stringstream ss;

        ss << ReportEventRecorderCoordinator::GetOtherData( pEntity, trigger )
           << "," << p_isr->GetNumCounted()
           << "," << p_isr->GetCurrentActionThreshold()
           << "," << m_ReportStatsByIP.GetReportData();

        return ss.str();
    }
}
