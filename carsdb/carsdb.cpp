#include "ctxt.h"

#include <stdio.h>
#ifdef WIN32
#include <objbase.h>
#endif

using namespace fourdb;

/// <summary>
/// This program demonstrates creating a 4db database and using all four supported commands
/// to populate, access, and manipulate the data.
/// 1. UPSERT
/// 2. SELECT
/// 3. DELETE
/// 4. DROP
/// </summary>

// Declarations.  Thanks C/C++!
void addCar(ctxt& context, int year, std::wstring make, std::wstring model);

int main()
{
    // 4db is built on SQLite, so to create a 4db database
    // we simply need to specify the location of the database file;
    // if the file does not exist, an empty database is automatically created.
    // The ctxt class manages the SQLite database connection,
    // provides many useful functions for executing SELECT queries,
    // and implements the UPSERT, DELETE, and DROP functions.
    // There are many classes in 4db, but ctxt is the one you deal directly with, auto the rest.
    printf("Opening database...\n");
    ctxt context("cars.db");

    // Drop our database table to start things clean.
    printf("Starting up...\n");
    context.drop(L"cars");

    // Pass our context into addCar to add database records...so many cars...
    printf("Adding cars to database...\n");
    addCar(context, 1987, L"Nissan", L"Pathfinder");
    addCar(context, 1998, L"Toyota", L"Tacoma");
    addCar(context, 2001, L"Nissan", L"Xterra");
    //...

    // Select data out of the database using a basic dialect of SQL
    // Restrictions:
    // 1. No JOINs
    // 2. WHERE criteria must use parameters
    // 3. ORDER BY colums must be in SELECT column list
    // Here we gather the "value" pseudo-column, the row ID created by the addCar function
    // We create a query object with our SELECT query,
    // pass in the value for the @year parameter,
    // and use context.execQuery function to execute the query,
    // handing back a DB reader.
    printf("Getting old cars...\n");
    std::vector<strnum> oldCarGuids;
    auto select =
        sql::parse
        (
            L"SELECT value, year, make, model "
            L"FROM cars "
            L"WHERE year < @year "
            L"ORDER BY year ASC"
        );
    select.addParam(L"@year", 1990);
    auto reader = context.execQuery(select);
    while (reader->read())
    {
        // Collect the row ID GUID that addCar added
        oldCarGuids.emplace_back(reader->getString(0));

        // 4db values are either numbers (doubles) or strings
        printf("%d: %S - %S\n", 
                static_cast<int>(reader->getDouble(0)), 
                reader->getString(1).c_str(),
                reader->getString(2).c_str());
    }

    // We use the list of row IDs to delete some rows.
    printf("Deleting old cars...\n");
    context.deleteRows(L"cars", oldCarGuids);

    printf("All done.\n");
    return 0;
}

/// <summary>
/// UPSERT a car into our database
/// </summary>
/// <param name="ctxt">The ctxt for doing database work</param>
/// <param name="year">The year of the car</param>
/// <param name="make">The make of the car</param>
/// <param name="model">The model of the car</param>
void addCar(ctxt& context, int year, std::wstring make, std::wstring model)
{
    // The define function is used to do UPSERTs.
    // You pass the table name, primary key value, and column data.
    // No need to create tables, just refer to them and the database takes care of it.
    // The second parameter to the Define function is the row ID.
    // There's no obvious primary key so we use a GUID.
    // We collect our column data in a simple map.
    // NOTE: The primary key value and column data values
    //       can only be strings or numbers.
    //       For numbers, they have to be convertible to doubles,
    //       and tolerated when selected out and returned as doubles.
    std::wstring tableName = L"cars";
    strnum primaryKey;
    {
        char guidBuf[1024];
#ifdef WIN32
        GUID guid;
        (void)CoCreateGuid(&guid);
        sprintf_s<sizeof(guidBuf)>
        (
            guidBuf,
            "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
        );
#else
        // Test program for reading and learning, not running.  Go fish.
#endif
        primaryKey = toWideStr(guidBuf);
    }
    paramap columnData
    {
        { L"year", year },
        { L"make", make },
        { L"model", model },
    };
    context.define(tableName, primaryKey, columnData);
}
