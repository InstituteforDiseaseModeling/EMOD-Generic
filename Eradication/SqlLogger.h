/***************************************************************************************************

Copyright (c) 2020 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

/* Note that the following is formatted with markdown if you care to copy/paste it to a markdown viewer.

# SqlLogger

This utility was motivated by need for a more powerful "printf() debugging" tool.

## Usage

Usage is intended to be very straightforward:

1) Declare a `SqlLogger` instance in the code you want to log/debug:  
    `SqlLogger sqlLogger( `_filename_`, `_tablename_`, `_column-formats-string_`, `_column-names-vector_` );`  
    example  
    `SqlLogger sqlLogger( "migration.db", "MIGRATION", "IIIIF", { "STEP", "INDIVIDUAL", "ORIGIN", "DESTINATION", "TIMER" } );`
2) Use the `.log(`_data_`)` method, similar to `printf()` to log data to the database.  
    `sqlLogger.log( int32_t(currenttime), int32_t(suid.data), int32_t(parent->GetSuid().data), int32_t(migration_destination.data), float(migration_time_until_trip) );`

### Notes

1) The column formats string determines the column types when creating the table in the database. Logging via `.log(...)` checks to make sure that the data passed in agrees with the stated column datatype. See the table below to the format characters.
2) `SqlLogger` only knows about integers (32-bit and 64-bit), floating point (float and double), strings (`char*` or `std::string`), and NULL (`nullptr`). If you have a datatype that is compatible with that but the compile does not know how to convert, e.g. `suids::suid`, help the compiler out by casting to the appropriate data type.

## Column Data Types

|     type    | null allowed | null not allowed (NOT NULL) |         SQL        |
|:-----------:|:------------:|:---------------------------:|:------------------:|
|   int32_t   |       i      |              I              | INTEGER [NOT NULL] |
|   int64_t   |       q      |              Q              | INTEGER [NOT NULL] |
|    float    |       f      |              F              |   REAL [NOT NULL]  |
|    double   |       d      |              D              |   REAL [NOT NULL]  |
|    char*    |       s      |              S              |   TEXT [NOT NULL]  |
| std::string |       s      |              S              |   TEXT [NOT NULL]  |

## Implementation

### Prepared Statement

According to [this](https://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite) post, prepared statements are _much_ faster than plain `sqlite3_exec()` calls. This has been borne out in practice.

Using a prepared statement involves the [following](https://www.sqlite.org/c3ref/stmt.html):

1) Create the prepared statement with `sqlite3_prepare_v2()` (done in the constructor).
2) Bind values to the parameters of the prepared statement with `sqlite3_bind_*()` calls (done in the `log(...)` functions).
3) Run the SQL by calling `sqlite3_step()` (done in the `log(void)` function).
4) Reset the prepared statement using `sqlite3_reset()` (also done in the `log(void)` function).
5) Destroy the object using `sqlite3_finalize()` (done in the destructor).

### log(...) with variable argument lists

This is accomplished with a <strike>hack</strike> neat trick using C++ [variadic templates](https://eli.thegreenplace.net/2014/variadic-templates-in-c/).

The technique is to peel off the first argument of the list with a specific implementation, e.g. `log( int32_t value, Args... args)` with a different specific implementation for each type which is supported (`int32_t`, `int64_t`, `float`, `double`, `char*`, `std::string`) with each implentation "recursively" calling `log(...)` with the remaining arguments.

The compiler matches specific implementations against the type of the first argument in the list until no arguments remain at which point we end up invoking `log( void )`. This works perfectly for us since we can use the call to the empty argument function as a signal that it is time to execute and reset the prepared statement (all values have been bound).

For example:

Give the following specific implementations and a "final" method `log( void )`:

```cpp
    template<typename... Args> void log( int32_t value, Args... args )
    {
        assert(tolower(formats[index]) == 'i');
        sqlite3_bind_int( statement, ++index, value );
        log( args... );
    }

    template<typename... Args> void log( double value, Args... args )
    {
        assert(tolower(formats[index]) == 'f' || tolower(formats[index]) == 'd');
        sqlite3_bind_double(statement, ++index, value );
        log( args... );
    }
```

A call to `log(...)` in the code like this:

`logger.log( float(`_value_`), int32_t(`_other_`))`

Becomes a call to `log( double value, Args... args )` which binds the floating point value and calls `log( int32_t value, Args... args)` which binds the integer value and calls `log( void )` which executes the prepared statement, resets it, and ends the call chain.

*/

#include <cassert>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include <sqlite3.h>

// Prepared statement motivated by this: https://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite

class SqlLogger
{
public:
    SqlLogger( const std::string& filename, const std::string& tablename, const std::string& formats, const std::vector<std::string>& columns );
    ~SqlLogger();

    // https://eli.thegreenplace.net/2014/variadic-templates-in-c/
    // Bummer, templates require implementation in the header file.

    template<typename... Args> void log( double value, Args... args )
    {
        assert(tolower(formats[index]) == 'f' || tolower(formats[index]) == 'd');
        sqlite3_bind_double(statement, ++index, value );
        log( args... );
    }

    template<typename... Args> void log( int32_t value, Args... args )
    {
        assert(tolower(formats[index]) == 'i');
        sqlite3_bind_int( statement, ++index, value );
        log( args... );
    }

    template<typename... Args> void log( int64_t value, Args... args )
    {
        assert(tolower(formats[index]) == 'q');
        sqlite3_bind_int64( statement, ++index, value );
        log( args... );
    }

    template<typename... Args> void log( void* value, Args... args )
    {
        assert(islower(formats[index]));
        sqlite3_bind_null( statement, ++index );
        log( args... );
    }

    /* 1) compiler prefers const std::string& below for char* */
    /* 2) const char* and void* are ambiguous */
//      template<typename... Args> void log( const char* value, Args... args )
//      {
//          assert(tolower(formats[index]) == 's');
//          sqlite3_bind_text( statement, ++index, value , -1, SQLITE_TRANSIENT );
//          log( args... );
//      }

    template<typename... Args> void log( const std::string& value, Args... args )
    {
        assert(tolower(formats[index]) == 's');
        sqlite3_bind_text( statement, ++index, value.c_str(), value.size(), SQLITE_TRANSIENT );
        log( args... );
    }

protected:
    void log( void );

    void open_db( void );
    void create_table( void );
    void begin_transaction( void );
    void prepare_statement( void );
    void finalize_statement( void );
    void end_transaction( void );
    void close_db( void );

    std::string filename;
    std::string tablename;
    std::string formats;
    std::vector<std::string> columns;
    sqlite3* db;
    sqlite3_stmt* statement;
    int index;
};
