/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <sqlite3.h>
#include "ReportEventRecorder.h"

namespace Kernel
{
    class IndividualForSQL {
        public:
        IndividualForSQL ();
        ~IndividualForSQL ();
        unsigned int time;
        unsigned int node_id;
        unsigned int individual_id;
        std::string  event;
        float        age;
        char         gender;
        float        mcw;
        unsigned int infector_id;
        std::string  IPs;
        std::string route_of_infection;
        static void serialize(IArchive& ar, IndividualForSQL& obj);
    };

    class SQLReporter : public ReportEventRecorder
    {
        GET_SCHEMA_STATIC_WRAPPER( SQLReporter )
    public:
        static IReport* CreateReport();

    protected:
        SQLReporter();
        virtual ~SQLReporter();
        virtual bool Configure( const Configuration * inputJson ) override;

    protected:
        virtual void WriteData( const std::string& rStringData ) override; // This needs to be virtual so it gets called properly
        virtual std::string GetOtherData( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger ) override;
        virtual void GetDataFromOtherCores() override;
        sqlite3 *db; 
        std::string time_as_str;
        jsonConfigurable::tDynamicStringSet sql_properties_to_report;
        std::list< IndividualForSQL > event_list;
    };
}
