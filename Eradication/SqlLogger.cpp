/***************************************************************************************************

Copyright (c) 2020 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <map>
#include <sstream>

#include "SqlLogger.h"

static std::map<char, std::string> types = {
    { 'f', "REAL" },    { 'F', "REAL NOT NULL" },
    { 'd', "REAL" },    { 'D', "REAL NOT NULL" },
    { 'i', "INTEGER" }, { 'I', "INTEGER NOT NULL" },
    { 'q', "INTEGER" }, { 'Q', "INTEGER NOT NULL" },
    { 's', "TEXT" },    { 'S', "TEXT NOT NULL" },
};

template<class I, class O>
std::vector<O> map(O(fn(const I&)), const std::vector<I>& iterable)
{
    std::vector<O> output;
    for (const auto& entry : iterable)
    {
        output.push_back( fn(entry) );
    }
    return output;
}

std::vector<std::string> concat( const std::vector<std::string>& a, const std::vector<std::string>& b, const std::string& separator = " " )
{
    assert( a.size() == b.size() );
    std::vector<std::string> result;
    for (auto ia = a.begin(), ib = b.begin(); (ia != a.end()) && (ib != b.end()); ++ia, ++ib)
    {
        result.push_back( *ia + separator + *ib );
    }
    return result;
}

std::string join( const std::vector<std::string>& strings, char character = ',' )
{
    std::ostringstream result;
    if ( strings.size() > 0 )
    {
        result << strings[0];
        for ( size_t i = 1; i < strings.size(); ++i )
        {
            result << character << strings[i];
        }
    }
    return result.str();
}

using stringpair = std::pair<std::string, std::string>;

std::string first(const stringpair& pair) { return pair.first; }
std::string second(const stringpair& pair) { return pair.second; }
std::string both(const stringpair& pair) { return pair.first + " " + pair.second; }

SqlLogger::SqlLogger( const std::string& filename, const std::string& tablename, const std::string& formats, const std::vector<std::string>& columns )
    : filename( filename )
    , tablename( tablename )
    , formats( formats )
    , columns( columns )
    , db( nullptr )
    , statement( nullptr )
    , index( 0 )
{
    remove( filename.c_str() );
    open_db();
    create_table();
    prepare_statement();
    begin_transaction();
}

SqlLogger::~SqlLogger( void )
{
    end_transaction();
    finalize_statement();
    close_db();
}

void SqlLogger::log( void )
{
    sqlite3_step( statement );
    sqlite3_clear_bindings( statement );
    sqlite3_reset( statement );
    index = 0;
}

void SqlLogger::open_db( void )
{
    int rc = sqlite3_open( filename.c_str(), &db );
}

std::string typeof( const char& format ) { return types.at( format ); }

void SqlLogger::create_table( void )
{
    assert(formats.size() == columns.size());   // need a format for each column
    auto column_types = map<char, std::string>( typeof, std::vector<char>( formats.begin(), formats.end() ) );
    auto column_defs = concat( columns, column_types );
    auto table_def = join( column_defs );
    std::ostringstream statement;
    statement << "CREATE TABLE " << tablename << " ( " << table_def << " );";
    char* error;
    int rc = sqlite3_exec( db, statement.str().c_str(), nullptr, nullptr, &error );
    if ( rc != SQLITE_OK )
    {
        sqlite3_free( error );
    }
}

void SqlLogger::begin_transaction( void )
{
    char* error;
    int rc = sqlite3_exec( db, "BEGIN TRANSACTION", nullptr, nullptr, &error );
    if ( rc != SQLITE_OK )
    {
        sqlite3_free( error );
    }
}

std::string prefix(const std::string& string) { return std::string("@") + string; }

void SqlLogger::prepare_statement( void )
{
    std::ostringstream sql;
    sql << "INSERT INTO " << tablename << " VALUES (" << join( map<std::string, std::string>( prefix, columns ) ) << ")";
    std::string insert(sql.str());
    sqlite3_prepare_v2( db,  insert.c_str(), int(insert.size() + 1), &statement, nullptr );
}

void SqlLogger::finalize_statement( void )
{
    sqlite3_finalize( statement );
    statement = nullptr;
}

void SqlLogger::end_transaction( void )
{
    char* error;
    int rc = sqlite3_exec( db, "END TRANSACTION", nullptr, nullptr, &error );
    if ( rc != SQLITE_OK )
    {
        sqlite3_free( error );
    }
}

void SqlLogger::close_db( void )
{
    sqlite3_close( db );
    db = nullptr;
}
