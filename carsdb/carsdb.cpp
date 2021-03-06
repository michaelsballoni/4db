/// <summary>
/// This program demonstrates creating a 4db database and using all four supported commands
/// to populate, access, and manipulate the data.
/// 1. UPSERT
/// 2. SELECT
/// 3. DELETE
/// 4. DROP
/// </summary>
#include "ctxt.h"
#pragma comment(lib, "4db")

#include <stdio.h>

// used by main()
void addCar(fourdb::ctxt& context, int year, const std::wstring& make, const std::wstring& model);

int main()
{
    // 4db is built on SQLite, so to create a 4db database
    // we simply need to specify the location of the database file.
    // If the file does not exist, an empty database is automatically created.
    // The ctxt class manages the database connection,
    // provides many useful functions for executing SELECT queries,
    // and implements the UPSERT, DELETE, and DROP functions.
    // There are many classes in 4db, but ctxt is the one you deal directly with;
    // you can auto the rest as seen here.
    printf("Opening database...\n");
    fourdb::ctxt context("cars.db");

    // Drop our database table to start things clean.
    printf("Starting up...\n");
    context.drop(L"cars");

    // Pass our context into addCar to add database records...so many cars...
    printf("Adding cars to database...\n");
    addCar(context, 1987, L"Nissan", L"Pathfinder");
    addCar(context, 1998, L"Toyota", L"Tacoma");
    addCar(context, 2001, L"Nissan", L"Xterra");
    //...

    // Select data out of the database using a basic dialect of SQL.
    // Here we gather the "value" pseudo-column which 
    // is the primary key added by the addCar function.
    // We create a query object with our SELECT query,
    // pass in the value for the @year parameter,
    // and use ctxt::execQuery function to execute the query,
    // handing back a reader object to process the results.
    printf("Getting cars...\n");
    std::vector<fourdb::strnum> oldCarKeys;
    auto select =
        fourdb::sql::parse
        (
            L"SELECT value, year, make, model "
            L"FROM cars "
            L"WHERE year < @year "
            L"ORDER BY year ASC"
        );
    select.addParam(L"@year", 2000);
    auto reader = context.execQuery(select);
    while (reader->read())
    {
        // Collect the primary key ("value") that addCar added
        oldCarKeys.push_back(reader->getString(0));

        // 4db values are either numbers (doubles) or strings
        printf("%d: %S - %S\n", 
               static_cast<int>(reader->getDouble(1)), 
               reader->getString(2).c_str(),
               reader->getString(3).c_str());
    }

    // We use the list of primary keys to delete some rows.
    printf("Deleting old cars... (%u)\n", static_cast<unsigned>(oldCarKeys.size()));
    context.deleteRows(L"cars", oldCarKeys);

    printf("All done.\n");
    return 0;
}

/// <summary>
/// UPSERT a car into our database using the define function.
/// You pass the table name, primary key value, and column data to this function.
/// No need to explicitly create the table, just refer to it by name 
/// and the define function takes care of it.
/// NOTE: The primary key value and column data values
///       can only be strings or numbers.
///       For numbers, they have to be convertible to doubles,
///       and are selected out of the database as doubles.
/// </summary>
/// <param name="context">ctxt for doing database work</param>
/// <param name="year">year of the car</param>
/// <param name="make">make of the car</param>
/// <param name="model">model of the car</param>
void addCar(fourdb::ctxt& context, int year, const std::wstring& make, const std::wstring& model)
{
    std::wstring tableName = L"cars";
    fourdb::strnum primaryKey = fourdb::num2str(year) + L"_" + make + L"_" + model;
    fourdb::paramap columnData
    {
        { L"year", year },
        { L"make", make },
        { L"model", model },
    };
    context.define(tableName, primaryKey, columnData);
}
