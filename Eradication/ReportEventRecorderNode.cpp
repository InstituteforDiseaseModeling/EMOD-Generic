/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportEventRecorderNode.h"
#include "Log.h"
#include "Exceptions.h"
#include "NodeEventContext.h"
#include "SimulationEventContext.h"
#include "EventTriggerNode.h"
#include "NodeProperties.h"
#include "Properties.h"
#include "IdmDateTime.h"

SETUP_LOGGING( "ReportEventRecorderNode" )

// These need to be after SETUP_LOGGING so that the LOG messages in the
// templates don't make GCC complain.
#include "BaseTextReportEventsTemplate.h"
#include "BaseReportEventRecorderTemplate.h"

namespace Kernel
{
    template void BaseTextReportEventsTemplate< INodeEventBroadcaster,
                                                INodeEventObserver,
                                                INodeEventContext>::Reduce();

    GET_SCHEMA_STATIC_WRAPPER_IMPL( ReportEventRecorderNode, ReportEventRecorderNode )

    IReport* ReportEventRecorderNode::CreateReport()
    {
        return new ReportEventRecorderNode();
    }

    ReportEventRecorderNode::ReportEventRecorderNode()
        : BaseReportEventRecorder( "ReportNodeEventRecorder.csv" )
        , m_NodePropertiesToReport()
        , m_StatsByIpKeyNames()
        , m_ReportStatsByIP()
    { }

    ReportEventRecorderNode::~ReportEventRecorderNode()
    { }

    bool ReportEventRecorderNode::Configure( const Configuration * inputJson )
    {
        m_StatsByIpKeyNames.value_source       = IPKey::GetConstrainedStringConstraintKey();
        m_NodePropertiesToReport.value_source  = NPKey::GetConstrainedStringConstraintKey();

        initVectorConfig("Report_Node_Event_Recorder_Events",  event_trigger_list,  inputJson,  MetadataDescriptor::VectorOfEnum("Report_Node_Event_Recorder_Events", "TBD", MDD_ENUM_ARGS(EventTrigger)),  "Enable_Report_Node_Event_Recorder");

        initConfigTypeMap("Report_Node_Event_Recorder_Ignore_Events_In_List",  &ignore_events_in_list,     "TBD",                                                 false,  "Enable_Report_Node_Event_Recorder");
        initConfigTypeMap("Report_Node_Event_Recorder_Node_Properties",        &m_NodePropertiesToReport,  Report_Node_Event_Recorder_Node_Properties_DESC_TEXT,          "Enable_Report_Node_Event_Recorder");
        initConfigTypeMap("Report_Node_Event_Recorder_Stats_By_IPs",           &m_StatsByIpKeyNames,       Report_Node_Event_Recorder_Stats_By_IPs_DESC_TEXT,             "Enable_Report_Node_Event_Recorder");

        return BaseReportEventRecorder::Configure(inputJson);
    }

    void ReportEventRecorderNode::Initialize( unsigned int nrmSize )
    {
        for( auto key_name : m_NodePropertiesToReport )
        {
            NodeProperty* p_np = NPFactory::GetInstance()->GetNP( key_name, "Report_Node_Event_Recorder_Node_Properties", false );
            if( p_np == nullptr )
            {
                std::stringstream ss;
                ss << "The NP Key (" << key_name << ") specified in 'Report_Node_Event_Recorder_Node_Properties' is unknown.\n"
                   << "Valid values are: " << NPFactory::GetInstance()->GetKeysAsString();
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
        }
        m_ReportStatsByIP.SetIPKeyNames( "Report_Node_Event_Recorder_Stats_By_IPs", m_StatsByIpKeyNames );
        BaseReportEventRecorder::Initialize( nrmSize );
    }

    void ReportEventRecorderNode::UpdateEventRegistration( float currentTime,
                                                           float dt,
                                                           std::vector<INodeEventContext*>& rNodeEventContextList,
                                                           ISimulationEventContext* pSimEventContext )
    {
        if( !is_registered )
        {
            for( auto trigger : eventTriggerList )
            {
                pSimEventContext->GetNodeEventBroadcaster()->RegisterObserver( this, trigger );
            }
            is_registered = true;
        }
    }

    std::string ReportEventRecorderNode::GetHeader() const
    {
        std::stringstream header;
        header << BaseReportEventRecorder::GetHeader()
               << "," << "NodeID"
               << "," << "NodeEventName"
               << "," << m_ReportStatsByIP.GetHeader();

        for( const auto& prop : m_NodePropertiesToReport )
        {
            header << "," << prop;
        }

        return header.str();
    }

    std::string ReportEventRecorderNode::GetOtherData( INodeEventContext *pEntity,
                                                       const EventTrigger::Enum& trigger )
    {
        m_ReportStatsByIP.ResetData();

        auto qual_func = [] ( IIndividualHumanEventContext* ) { return true; };
        m_ReportStatsByIP.CollectDataFromNode( pEntity, qual_func );

        std::stringstream ss;

        ss << "," << pEntity->GetExternalId()
           << "," << EventTrigger::pairs::lookup_key( trigger )
           << "," << m_ReportStatsByIP.GetReportData();

        // Report requested properties
        const NPKeyValueContainer& r_props = pEntity->GetNodeContext()->GetNodeProperties();

        for( const auto& prop_name : m_NodePropertiesToReport )
        {
            NPKey key( prop_name );
            if( !r_props.Contains( key ) )
            {
                throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "properties", prop_name.c_str() );
            }
            ss << "," << r_props.Get( key ).GetValueAsString();
        }
        return ss.str();
    }

    float ReportEventRecorderNode::GetTime( INodeEventContext* pEntity ) const
    {
        return pEntity->GetTime().time;
    }
}
