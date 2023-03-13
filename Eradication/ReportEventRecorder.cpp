/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <string>
#include "ReportEventRecorder.h"
#include "Log.h"
#include "Exceptions.h"
#include "NodeEventContext.h"
#include "IndividualEventContext.h"
#include "FileSystem.h"
#include "SimulationEnums.h"
#include "ISimulation.h"
#include "EventTrigger.h"

SETUP_LOGGING( "ReportEventRecorder" )

// These need to be after SETUP_LOGGING so that the LOG messages in the
// templates don't make GCC complain.
#include "BaseTextReportEventsTemplate.h"
#include "BaseReportEventRecorderTemplate.h"

namespace Kernel
{
    template void BaseTextReportEventsTemplate< IIndividualEventBroadcaster,
                                                IIndividualEventObserver,
                                                IIndividualHumanEventContext
                                                >::Reduce();

    template void BaseTextReportEventsTemplate< IIndividualEventBroadcaster,
                                                IIndividualEventObserver,
                                                IIndividualHumanEventContext >::UnregisterAllBroadcasters();

    GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportEventRecorder,ReportEventRecorder)

    IReport* ReportEventRecorder::CreateReport()
    {
        return new ReportEventRecorder();
    }

    ReportEventRecorder::ReportEventRecorder()
        : BaseReportEventRecorder("ReportEventRecorder.csv")
        , properties_to_report()
    { }

    ReportEventRecorder::~ReportEventRecorder()
    { }

    bool ReportEventRecorder::Configure( const Configuration * inputJson )
    {
        properties_to_report.value_source = IPKey::GetConstrainedStringConstraintKey(); 

        initVectorConfig("Report_Event_Recorder_Events",  event_trigger_list,  inputJson,  MetadataDescriptor::VectorOfEnum("Report_Event_Recorder_Events", Report_Event_Recorder_Events_DESC_TEXT, MDD_ENUM_ARGS(EventTrigger)),  "Enable_Report_Event_Recorder");

        initConfigTypeMap("Report_Event_Recorder_Ignore_Events_In_List",  &ignore_events_in_list,  Report_Event_Recorder_Ignore_Events_In_List_DESC_TEXT, false,  "Enable_Report_Event_Recorder");
        initConfigTypeMap("Report_Event_Recorder_Individual_Properties",  &properties_to_report,   Property_Restriction_DESC_TEXT,                                "Enable_Report_Event_Recorder");

        return BaseReportEventRecorder::Configure(inputJson);
    }

    void ReportEventRecorder::Initialize( unsigned int nrmSize )
    {
        for( auto key_name : properties_to_report )
        {
            IndividualProperty* p_ip = IPFactory::GetInstance()->GetIP( key_name, "Report_Event_Recorder_Individual_Properties", false );
            if( p_ip == nullptr )
            {
                std::stringstream ss;
                ss << "The IP Key (" << key_name << ") specified in 'Report_Event_Recorder_Individual_Properties' is unknown.\n"
                    << "Valid values are: " << IPFactory::GetInstance()->GetKeysAsString();
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
        }
        BaseReportEventRecorder::Initialize( nrmSize );
    }

    void ReportEventRecorder::UpdateEventRegistration( float currentTime,
                                                       float dt,
                                                       std::vector<INodeEventContext*>& rNodeEventContextList,
                                                       ISimulationEventContext* pSimEventContext )
    {
        if( !is_registered )
        {
            for( auto pNEC : rNodeEventContextList )
            {
                release_assert( pNEC );
                IIndividualEventBroadcaster* broadcaster = pNEC->GetIndividualEventBroadcaster();
                UpdateRegistration( broadcaster, true );
                broadcaster_list.push_back( broadcaster );
            }
            is_registered = true;
        }
    }

    std::string ReportEventRecorder::GetHeader() const
    {
        std::stringstream header ;
        header << BaseReportEventRecorder::GetHeader()
               << "," << "Node_ID"
               << "," << "Event_Name"
               << "," << "Individual_ID"
               << "," << "Age"
               << "," << "Gender"
               << "," << "Infected"
               << "," << "Infectiousness" ;

        for (const auto& prop : properties_to_report)
        {
            header << "," << prop;
        }

        return header.str();
    }

    std::string ReportEventRecorder::GetOtherData( IIndividualHumanEventContext *context,
                                                   const EventTrigger::Enum& trigger )
    {
        int         id           = context->GetSuid().data;
        ExternalNodeId_t node_id = context->GetNodeEventContext()->GetExternalId();
        std::string event_name   = EventTrigger::pairs::lookup_key( trigger); 
        float       age          = context->GetAge();
        const char  gender       = (context->GetGender() == Gender::MALE) ? 'M' : 'F' ;
        bool        infected     = context->IsInfected();
        float       infectious   = context->GetInfectiousness() ;

        std::stringstream ss;
        ss << "," << node_id
           << "," << event_name
           << "," << id
           << "," << age
           << "," << gender
           << "," << infected
           << "," << infectious;

        // Report requested properties
        const auto * pProp = context->GetProperties();

        for (const auto& prop_name : properties_to_report)
        {
            IPKey key( prop_name );
            if( !pProp->Contains( key ) )
            {
                throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "properties", prop_name.c_str() );
            }
            ss << "," << pProp->Get( key ).GetValueAsString();
        }
        return ss.str();
    }

    float ReportEventRecorder::GetTime( IIndividualHumanEventContext* pEntity ) const
    {
        return pEntity->GetNodeEventContext()->GetTime().time;
    }
}
