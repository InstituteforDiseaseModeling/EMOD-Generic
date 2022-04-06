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
#include "FileSystem.h"
#include "IdmDateTime.h"

SETUP_LOGGING( "SQLReporter" )

#define SQL_FILE "simulation_events.db"

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(SQLReporter, SQLReporter)


    IndividualForSQL::IndividualForSQL()
        : time(0)
        , node_id(0)
        , individual_id(0)
        , event()
        , mcw(0.0f)
        , label(0)
        , IPs()
    { }


    IndividualForSQL::~IndividualForSQL()
    { }



    IReport* SQLReporter::CreateReport()
    {
        return new SQLReporter();
    }


    SQLReporter::SQLReporter()
        : db(nullptr)
        , event_list()
        , sql_properties_to_report()
    { }


    SQLReporter::~SQLReporter()
    { }


    void SQLReporter::Initialize( unsigned int nrmSize )
    {
        SetReportName(SQL_FILE);
        if(JsonConfigurable::_dryrun )
        {
            return;
        }

        if(EnvPtr->MPI.Rank == 0)
        {
            // Remove old file if present
            FileSystem::RemoveFile(SQL_FILE);

            // Open/create database; leave open for read/write
            int rc = 0;
            rc = sqlite3_open(SQL_FILE, &db);
            if( rc != SQLITE_OK )
            {
                std::stringstream ss;
                ss << "Received error '" << sqlite3_errmsg(db) << "' while creating database.";
                throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, SQL_FILE, ss.str().c_str() );
            }
            LOG_INFO("Database created\n");

            // Define create table SQL statement
            char* sql  = "CREATE TABLE SIM_EVENTS ("           \
                         "SIM_TIME       REAL   NOT NULL,"     \
                         "EVENT          TEXT   NOT NULL,"     \
                         "NODE           INT    NOT NULL,"     \
                         "INDIVIDUAL     INT    NOT NULL,"     \
                         "MCW            REAL   NOT NULL,"     \
                         "LABEL          INT    NOT NULL,"     \
                         "IPS            TEXT   DEFAULT NULL " \
                                                          ");" ;

            // Execute create table SQL statement
            char* zErrMsg;
            rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
            if( rc != SQLITE_OK )
            {
                std::stringstream ss;
                ss << "Received error '" << zErrMsg << "' while creating table.";
                throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, SQL_FILE, ss.str().c_str() );
            }
            LOG_INFO("Table created\n");
        }

        // Everyone wait until the Rank=0 process is done making the database
        EnvPtr->MPI.p_idm_mpi->Barrier();

        // Open database in other processes; leave open for read/write
        if(!db)
        {
            int rc = sqlite3_open_v2(SQL_FILE, &db, SQLITE_OPEN_READWRITE, nullptr);
            if( rc != SQLITE_OK )
            {
                std::stringstream ss;
                ss << "Received error '" << sqlite3_errmsg(db) << "' while creating database.";
                throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, SQL_FILE, ss.str().c_str() );
            }
            LOG_INFO("Database opened\n");
        }

        // User memory-only journal mode
        char* zErrMsg;
        int rc = sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, &zErrMsg);
        if( rc != SQLITE_OK )
        {
            std::stringstream ss;
            ss << "Received error '" << zErrMsg << "' while configuring database.";
            throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, SQL_FILE, ss.str().c_str() );
        }

        // Set 3 second busy timeout
        sqlite3_busy_timeout(db, 3000);

        return;
    }


    bool SQLReporter::Configure( const Configuration* inputJson )
    {
        bool ret_val;

        initVectorConfig("SQL_Events", eventTriggerList, inputJson, MetadataDescriptor::VectorOfEnum("SQL_Events", SQL_Events_DESC_TEXT, MDD_ENUM_ARGS(EventTrigger)), "Enable_Event_DB");

        ret_val = JsonConfigurable::Configure(inputJson);

        return ret_val;
    }


    bool SQLReporter::notifyOnEvent(IIndividualHumanEventContext* context, const EventTrigger::Enum& trigger)
    {
        IndividualForSQL record;

        int               id           = context->GetSuid().data;
        ExternalNodeId_t  node_id      = context->GetNodeEventContext()->GetExternalId();
        const char*       event_name   = EventTrigger::pairs::lookup_key(trigger);

        record.time          = GetTime( context );
        record.node_id       = node_id;
        record.individual_id = id;
        record.event         = event_name;
        record.mcw           = context->GetMonteCarloWeight();

        if(sql_properties_to_report.size() == 0)
        {
            for( auto &prop : IPFactory::GetInstance()->GetKeysAsStringSet() )
            {
                sql_properties_to_report.insert( prop );
            }
        }

        const auto * pProp = context->GetProperties();

        std::stringstream ss;
        std::string       sep_val("");
        for (const auto& prop_name : sql_properties_to_report)
        {
            IPKey key( prop_name );
            if( !pProp->Contains( key ) )
            {
                throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "properties", prop_name.c_str() );
            }
            ss << sep_val << prop_name << "=" << pProp->Get( key ).GetValueAsString();
            sep_val = "|";
        }
        record.IPs = ss.str();

        IInfectable* p_infectable = nullptr;
        if (context->QueryInterface(GET_IID(IInfectable), (void**)&p_infectable) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "p_recipient", "IInfectable", "IIndividualHuman" );
        }

        if( p_infectable->GetInfections().size() > 0 )
        {
            auto inf           = p_infectable->GetInfections().front();
            auto genome_data   = inf->GetStrain()->GetGeneticID();
            record.label       = (genome_data >> SHIFT_BIT);
        }

        event_list.push_back(record);

        return true;
    }


    void SQLReporter::EndTimestep(float currentTime, float dt)
    {
        // Build insert into table SQL statements
        stringstream sql;

        sql << "BEGIN TRANSACTION;";
        for( auto &rec : event_list )
        {
            sql << "INSERT INTO SIM_EVENTS (SIM_TIME,EVENT,NODE,INDIVIDUAL,MCW,LABEL,IPS) "
                << "VALUES ("
                <<        rec.time                   << ", "
                << "'" << rec.event          << "'"  << ", "
                <<        rec.node_id                << ", "
                <<        rec.individual_id          << ", "
                <<        rec.mcw                    << ", "
                <<        rec.label                  << ", "
                << "'" << rec.IPs            << "'"  << ");";
        }
        sql << "END TRANSACTION;";
        event_list.clear();

        // Execute insert into table SQL statements
        char* zErrMsg;
        int rc = sqlite3_exec(db, sql.str().c_str(), nullptr, nullptr, &zErrMsg);
        if(rc != SQLITE_OK)
        {
            std::stringstream ss;
            ss << "Received error '" << zErrMsg << "' while writing to database.";
            throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, SQL_FILE, ss.str().c_str() );
        }

        return;
    }


    void SQLReporter::Finalize()
    {
        // Close database
        int rc = sqlite3_close_v2(db);
        if( rc != SQLITE_OK )
        {
            std::stringstream ss;
            ss << "Received error '" << sqlite3_errmsg(db) << "' while closing database.";
            throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, SQL_FILE, ss.str().c_str() );
        }
        LOG_INFO("Database closed\n");

        return;
    }
}
