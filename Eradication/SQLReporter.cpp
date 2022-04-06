/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <string>
#include "SQLReporter.h"
#include "IndividualEventContext.h"
#include "IInfectable.h"
#include "Properties.h"
#include "NodeEventContext.h"
#include "SimulationEnums.h"
#include "IndividualEnvironmental.h" // hacky; put GetExposureRoute in interface
#include "MpiDataExchanger.h"
#include "IdmDateTime.h"

SETUP_LOGGING( "SQLReporter" )

namespace Kernel
{
    static const std::string _report_name = "SQLite Reporter";

    GET_SCHEMA_STATIC_WRAPPER_IMPL( SQLReporter, SQLReporter )

    IReport* SQLReporter::CreateReport()
    {
        return new SQLReporter();
    }

    // I'm not even sure wy this is here or where it came from anymore. :)
    static int sql_callback(void *NotUsed, int argc, char **argv, char **azColName) {
        int i;
        for(i = 0; i<argc; i++) {
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        printf("\n");
        return 0;
    }

    SQLReporter::SQLReporter()
    {
        if( EnvPtr->MPI.Rank != 0)
        {
            return;
        }
        remove( "simulation_events.db" );
        /* Open database */
        int rc = sqlite3_open("simulation_events.db", &db);

        if( rc ) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return;
        } else {
            fprintf(stdout, "Opened database successfully\n");
        }

        /* Create SQL statement */
        char * sql = "CREATE TABLE SIM_EVENTS ("  \
              /*"ID INT PRIMARY KEY     NOT NULL," \*/
              "SIM_TIME       INT    NOT NULL," \
              "EVENT          TEXT   NOT NULL," \
              "NODE           INT    NOT NULL," \
              "INDIVIDUAL     INT    NOT NULL," \
              "MCW            REAL   NOT NULL," \
              "MISC           INT    NOT NULL," \
              "NOTES          TEXT   DEFAULT NULL " \
              ");";

        /* Execute SQL statement */
        char * zErrMsg;
        rc = sqlite3_exec(db, sql, sql_callback, 0, &zErrMsg);

        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Table created successfully\n");
        }
        sqlite3_close(db);
    }

    SQLReporter::~SQLReporter()
    {
    } 
    
    bool SQLReporter::Configure( const Configuration * inputJson )
    {
        eventTriggerList.push_back( EventTrigger::NewInfection );
        eventTriggerList.push_back( EventTrigger::NewlySymptomatic );
        eventTriggerList.push_back( EventTrigger::NewSevereCase );
        eventTriggerList.push_back( EventTrigger::NewClinicalCase );
        eventTriggerList.push_back( EventTrigger::Recovered );
        eventTriggerList.push_back( EventTrigger::DiedOfDisease ); 
        return true;
    }

    void SQLReporter::WriteData( const std::string& rStringData )
    {
        sqlite3 *db = nullptr;
        int rc = sqlite3_open_v2("simulation_events.db", &db, SQLITE_OPEN_READWRITE, nullptr);
        if( rc == 14 ) {
            //fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return;
        }
        sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

        ostringstream sql;
        for( auto &rec : event_list )
        {
            if( rec.event == "NewInfection" ) // would be nice if this wasn't a 'magic string'
            {
                std::string other = rec.IPs.c_str();
                other += "|";
                other += rec.route_of_infection;
                sql << "INSERT INTO SIM_EVENTS (SIM_TIME,EVENT,NODE,INDIVIDUAL,MCW,MISC,NOTES) "  \
                    << "VALUES ("
                    << rec.time // GetParent()->GetTime().TimeAsSimpleTimestep() // should be in string already
                    << ", '" // TBD: Get sim time
                    << rec.event
                    << "', "
                    << rec.node_id
                    << ", "
                    << rec.individual_id
                    << ", "
                    << rec.mcw
                    << ", "
                    << rec.infector_id
                    << ", '"
                    << other.c_str()
                    << "');";
            }
            else
            {
                sql << "INSERT INTO SIM_EVENTS (SIM_TIME,EVENT,NODE,INDIVIDUAL,MCW,MISC,NOTES) "  \
                    << "VALUES ("
                    << rec.time // GetParent()->GetTime().TimeAsSimpleTimestep() // should be in string already
                    << ", '" // TBD: Get sim time
                    << rec.event
                    << "', "
                    << rec.node_id
                    << ", "
                    << rec.individual_id
                    << ", "
                    << rec.mcw
                    << ", "
                    << 0
                    << ", '"
                    << "');";
            }
        }
        char * zErrMsg = 0;
        rc = sqlite3_exec(db, sql.str().c_str(), sql_callback, 0, &zErrMsg);

        if( rc != SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
        sqlite3_close(db);

        event_list.clear();
        // process rStringData and call StoreTxData
    }

    std::string SQLReporter::GetOtherData( IIndividualHumanEventContext *context,
                                           const EventTrigger::Enum& trigger )
    {
        //std::string other_data = ReportEventRecorder::GetOtherData( context, trigger );
        IndividualForSQL record;

        int         id           = context->GetSuid().data;
        ExternalNodeId_t node_id = context->GetNodeEventContext()->GetExternalId();
        const char* event_name   = EventTrigger::pairs::lookup_key( trigger); 
        float       age          = context->GetAge();
        const char  gender       = (context->GetGender() == Gender::MALE) ? 'M' : 'F' ;
        record.time          = GetTime( context );
        record.node_id       = node_id;
        record.individual_id = id;
        record.event         = event_name;
        record.age           = age;
        record.gender        = gender;
        record.mcw           = context->GetMonteCarloWeight();

        if( sql_properties_to_report.size() == 0)
        {
            for( auto &prop : IPFactory::GetInstance()->GetKeysAsStringSet() )
            {
                sql_properties_to_report.insert( prop );
            }
        }

        const auto * pProp = context->GetProperties();

        std::stringstream ss;
        for (const auto& prop_name : sql_properties_to_report)
        {
            IPKey key( prop_name );
            if( !pProp->Contains( key ) )
            {
                throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "properties", prop_name.c_str() );
            }
            ss << prop_name << "=" << pProp->Get( key ).GetValueAsString() << "|";
        }
        record.IPs = ss.str();

        // Get infector id if trigger == NewInfection
        if( trigger == EventTrigger::NewInfection )
        {
            IInfectable* p_infectable = nullptr;
            if (context->QueryInterface(GET_IID(IInfectable), (void**)&p_infectable) != s_OK)
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "p_recipient", "IInfectable", "IIndividualHuman" );
            }

            if( p_infectable->GetInfections().size() > 0 )
            {
                auto inf = p_infectable->GetInfections().front();
                release_assert( inf );
                auto strain = inf->GetStrain();
                auto infector = strain->GetGeneticID();
                record.infector_id = infector;
                /*IIndividualHumanEnvironmental * pEnviroPerson = nullptr;
                if (context->QueryInterface(GET_IID(IIndividualHumanEnvironmental), (void**)&pEnviroPerson ) == s_OK)
                {
                    record.route_of_infection = TransmissionRoute::pairs::lookup_key( pEnviroPerson->GetExposureRoute() );
                }*/
            }
            else
            {
                LOG_VALID_F( "Individual %d has no infection even though we got a NewInfection event. This means they recovered same timestep. No way to record infector or route of infection.\n", context->GetSuid().data );
            }
        }

        std::string event;
        event_list.push_back( record );
        return "";
    }

    void SQLReporter::GetDataFromOtherCores()
    {
        WithSelfFunc to_self_func = [this](int myRank) 
        { 
            // no-op
        }; 

        SendToOthersFunc to_others_func = [this](IArchive* writer, int toRank)
        {
            // send our event_list to 0 (unless we are 0). data ends up in writer
            // I don't know where the serialization is actually being invoked
            if( EnvPtr->MPI.Rank == 0 || toRank != 0 )
            {
                return;
            }
            *writer & event_list;
        };

        ReceiveFromOthersFunc from_others_func = [this](IArchive* reader, int fromRank)
        {
            if( EnvPtr->MPI.Rank != 0 )
            {
                return;
            }
            std::list< IndividualForSQL > event_list_recv;
            *reader & event_list_recv;
            for( auto event : event_list_recv )
            {
                event_list.push_back( event );
            }
        };

        ClearDataFunc clear_data_func = [this](int rank)
        {
        };

        IdmDateTime fakeTime;
        MpiDataExchanger exchanger( "SQLReporter", to_self_func, to_others_func, from_others_func, clear_data_func );
        exchanger.ExchangeData( fakeTime /*GetParent()->currentTime*/ ); // pretty sure time isn't needed

        // OK, this is where we'd write the gathered/consolidated data to disk
        if( EnvPtr->MPI.Rank == 0 )
        {
            WriteData( "" );
        }
        else
        {
            event_list.clear();
        }
    }

    IndividualForSQL::IndividualForSQL()
    {
    }
    IndividualForSQL::~IndividualForSQL()
    {
    }
    void IndividualForSQL::serialize(IArchive& ar, IndividualForSQL& obj)
    {
        IndividualForSQL& record = obj;
        ar.labelElement("time") & record.time;
        ar.labelElement("node_id") & record.node_id;
        ar.labelElement("individual_id") & record.individual_id;
        ar.labelElement("event") & record.event;
        ar.labelElement("age") & record.age;
        //ar.labelElement("gender") & record.gender;
        ar.labelElement("mcw") & record.mcw;
        ar.labelElement("infector_id") & record.infector_id; 
        ar.labelElement("IPs") & record.IPs; 
    }
}
