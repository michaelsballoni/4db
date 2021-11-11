#pragma once

#include "db.h"
#include "sql.h"
#include "types.h"

namespace fourdb
{
	/// <summary>
	/// Interface to the class library
    /// This is the only class you need to interact with
    /// You can auto the rest
	/// </summary>
	class ctxt
	{
	public:
		/// <summary>
		/// Get going with the all-important path to the database file
        /// If the file does not exist, an empty database is created and initialized
		/// </summary>
		/// <param name="dbFilePath">file path for the database</param>
        ctxt(const std::string& dbFilePath, bool clearCaches = false);

        ~ctxt()
        {
            m_db.reset();
        }

        /// <summary>
        /// Access the SQLite wrapper object
        /// </summary>
        db& db()
        {
            return *m_db;
        }

        /// <summary>
        /// Prepare and execute a query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>dbreader ready to process query results</returns>
        std::shared_ptr<dbreader> execQuery(const select& query);

        /// <summary>
        /// Issue a generic scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>dbreader ready to process query results</returns>
        std::shared_ptr<dbreader> execScalar(const select& query);

        /// <summary>
        /// Issue a int64 scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>optional 64-bit integer</returns>
        std::optional<int64_t> execScalarInt64(const select& query);

        /// <summary>
        /// Issue a double scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>optional double</returns>
        std::optional<double> execScalarDouble(const select& query);

        /// <summary>
        /// Issue a string scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>optional string</returns>
        std::optional<std::wstring> execScalarString(const select& query);

        /// <summary>
        /// Given a table and a primary key, get the items table's row ID
        /// Useful for having a column in one virtual table
        /// refer to ids of rows in another virtual table, foreign keys.
        /// </summary>
        /// <returns>Row ID, or -1 if not found</returns>
        int64_t getRowId(const std::wstring& tableName, const strnum& key);

        /// <summary>
        /// Given a table and a row ID, get the primary key as a number.
        /// </summary>
        std::optional<double> getRowNumberValue(const std::wstring& tableName, int64_t rowId);

        /// <summary>
        /// Given a table and a row ID, get the primary key as a string.
        /// </summary>
        std::optional<std::wstring> getRowStringValue(const std::wstring& tableName, int64_t rowId);

        /// <summary>
        /// UPSERT: Give it a table name, a primary key, and column data, and it does the rest
        /// </summary>
        /// <param name="table">Name of the table to UPSERT into; table created automatically</param>
        /// <param name="key">Primary key value</param>
        /// <param name="columnData">Column data</param>
        void define(const std::wstring& table, const strnum& key, const paramap& columnData);

        /// <summary>
        /// Okay fine, there are 5 things you can do.  UNDEFINE.
        /// I didn't want to add a notion of a null strnum, either in strnum, or in paramap.
        /// And it's a strange thing to do, some sort of schema change.
        /// So you get this strange little function.
        /// </summary>
        /// <param name="table">Name of the table to find the row to remove the column data from</param>
        /// <param name="key">Primary key of the row to remove the column data from</param>
        /// <param name="name">Name of the column data to return</param>
        void undefine(const std::wstring& table, const strnum& key, const std::wstring& name);

        /// <summary>
        /// Given a virtual query, generate the SQLite SQL for executing the query.
        /// </summary>
        /// <param name="query"></param>
        /// <returns></returns>
        std::wstring generateSql(const select& query);

        /// <summary>
        /// Remove a row from a table
        /// </summary>
        /// <param name="table">Table to remove the row from</param>
        /// <param name="key">Primary key of the row to remove</param>
        void deleteRow(const std::wstring& table, const strnum& key);

        /// <summary>
        /// Delete multiple rows from a table
        /// </summary>
        /// <param name="table">Table to remove the rows from</param>
        /// <param name="keys">Primary keys or rows to remove</param>
        void deleteRows(const std::wstring& table, const std::vector<strnum>& keys);

        /// <summary>
        /// Remove a table from the virtual schema
        /// </summary>
        /// <param name="table">Table to drop</param>
        bool drop(const std::wstring& table);

        /// <summary>
        /// Reset the database to its initial empty state
        /// </summary>
        void reset();

        /// <summary>
        /// Get a snapshot of the virtual schema
        /// </summary>
        /// <param name="table">Optional name of single table to return schema of</param>
        /// <returns>Object with table and column names</returns>
        virtualschema getSchema(const std::wstring& table = L"");

        /// <summary>
        /// Okay, so there are 6 things...but you really don't need this one! 
        /// </summary>
        /// <param name="name">Name of table to ensure exists in the virtual schema</param>
        /// <param name="isNumeric">Is the primary key of this table numeric?</param>
        /// <returns>Database row ID of the table</returns>
        int ensureTable(const std::wstring& name, bool isNumeric);

    private:
        static void runSchemaSql(fourdb::db& db, const wchar_t** queries);

	private:
		std::shared_ptr<fourdb::db> m_db;
	};
}
