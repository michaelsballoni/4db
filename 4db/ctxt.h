#pragma once

#include "db.h"
#include "items.h"
#include "names.h"
#include "sql.h"
#include "tables.h"
#include "types.h"
#include "values.h"

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
        ctxt(const std::string& dbFilePath, bool clearCaches = false)
		{
            {
                static std::mutex mutex;
                std::lock_guard<std::mutex> lock(mutex);
                if (!std::filesystem::exists(dbFilePath.c_str()))
                {
                    fourdb::db db(dbFilePath.c_str());

                    db.execSql(L"PRAGMA journal_mode = WAL");
                    db.execSql(L"PRAGMA synchronous = NORMAL");
                    
                    runSchemaSql(db, tables::createSql());
                    runSchemaSql(db, names::createSql());
                    runSchemaSql(db, values::createSql());
                    runSchemaSql(db, items::createSql());
                }
            }

            m_db = std::make_shared<fourdb::db>(dbFilePath.c_str());

            if (clearCaches)
            {
                names::clearCaches();
                tables::clearCaches();
            }
        }

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
        std::shared_ptr<dbreader> execQuery(const select& query)
        {
            std::wstring sql = sql::generateSql(*m_db, query);
            return m_db->execReader(sql, query.cmdParams);
        }

        /// <summary>
        /// Issue a generic scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>dbreader ready to process query results</returns>
        std::shared_ptr<dbreader> execScalar(const select& query)
        {
            auto reader = execQuery(query);
            if (reader->read())
                return reader;
            else
                return nullptr;
        }

        /// <summary>
        /// Issue a int64 scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>optional 64-bit integer</returns>
        std::optional<int64_t> execScalarInt64(const select& query)
        {
            auto reader = execScalar(query);
            if (reader != nullptr)
                return reader->getInt64(0);
            else
                return std::nullopt;
        }

        /// <summary>
        /// Issue a double scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>optional double</returns>
        std::optional<double> execScalarDouble(const select& query)
        {
            auto reader = execScalar(query);
            if (reader != nullptr)
                return reader->getDouble(0);
            else
                return std::nullopt;
        }

        /// <summary>
        /// Issue a string scalar query
        /// </summary>
        /// <param name="query">virtual query object from sql::parse</param>
        /// <returns>optional string</returns>
        std::optional<std::wstring> execScalarString(const select& query)
        {
            auto reader = execScalar(query);
            if (reader != nullptr)
                return reader->getString(0);
            else
                return std::nullopt;
        }

        /// <summary>
        /// Given a table and a primary key, get the items table's row ID
        /// Useful for having a column in one virtual table
        /// refer to ids of rows in another virtual table, foreign keys.
        /// </summary>
        /// <returns>Row ID, or -1 if not found</returns>
        int64_t getRowId(const std::wstring& tableName, const strnum& key)
        {
            validateTableName(tableName);
            auto select = sql::parse(L"SELECT id FROM " + tableName + L" WHERE value = @value");
            select.addParam(L"@value", key);
            return execScalarInt64(select).value_or(-1);
        }

        /// <summary>
        /// Given a table and a row ID, get the primary key as a number.
        /// </summary>
        std::optional<double> getRowNumberValue(const std::wstring& tableName, int64_t rowId)
        {
            validateTableName(tableName);
            auto select = sql::parse(L"SELECT value FROM " + tableName + L" WHERE id = @id");
            select.addParam(L"@id", static_cast<double>(rowId));
            return execScalarDouble(select);
        }

        /// <summary>
        /// Given a table and a row ID, get the primary key as a string.
        /// </summary>
        std::optional<std::wstring> getRowStringValue(const std::wstring& tableName, int64_t rowId)
        {
            validateTableName(tableName);
            auto select = sql::parse(L"SELECT value FROM " + tableName + L" WHERE id = @id");
            select.addParam(L"@id", static_cast<double>(rowId));
            return execScalarString(select);
        }

        /// <summary>
        /// UPSERT: Give it a table name, a primary key, and column data, and it does the rest
        /// </summary>
        /// <param name="table">Name of the table to UPSERT into; table created automatically</param>
        /// <param name="key">Primary key value</param>
        /// <param name="columnData">Column data</param>
        void define(const std::wstring& table, const strnum& key, const paramap& columnData = paramap())
        {
            bool isKeyNumeric = !key.isStr();
            int tableId = tables::getId(*m_db, table, isKeyNumeric);
            int64_t valueId = values::getId(*m_db, key);
            int64_t itemId = items::getId(*m_db, tableId, valueId);

            std::unordered_map<int, int64_t> nameValueIds(columnData.size());
            for (auto kvp : columnData)
            {
                bool isMetadataNumeric = !kvp.second.isStr();

                int nameId = names::getId(*m_db, tableId, kvp.first, isMetadataNumeric);
                bool isNameNumeric = names::getNameIsNumeric(*m_db, nameId);
                
                if (isMetadataNumeric != isNameNumeric)
                    throw fourdberr("Data numeric does not match name");

                nameValueIds[nameId] = values::getId(*m_db, kvp.second);
            }
            items::setItemData(*m_db, itemId, nameValueIds);
        }

        /// <summary>
        /// Okay fine, there are 5 things you can do.  UNDEFINE.
        /// I didn't want to add a notion of a null strnum, either in strnum, or in paramap.
        /// And it's a strange thing to do, some sort of schema change.
        /// So you get this strange little function.
        /// </summary>
        /// <param name="table">Name of the table to find the row to remove the column data from</param>
        /// <param name="key">Primary key of the row to remove the column data from</param>
        /// <param name="name">Name of the column data to return</param>
        void undefine(const std::wstring& table, const strnum& key, const std::wstring& name)
        {
            bool isKeyNumeric = !key.isStr();
            int tableId = tables::getId(*m_db, table, isKeyNumeric, true);
            int64_t valueId = values::getId(*m_db, key);
            int64_t itemId = items::getId(*m_db, tableId, valueId);
            int nameId = names::getId(*m_db, tableId, name, false, true);
            items::removeItemData(*m_db, itemId, nameId);
        }

        /// <summary>
        /// Given a virtual query, generate the SQLite SQL for executing the query.
        /// </summary>
        /// <param name="query"></param>
        /// <returns></returns>
        std::wstring generateSql(const select& query)
        {
            std::wstring sql = sql::generateSql(*m_db, query);
            return sql;
        }

        /// <summary>
        /// Remove a row from a table
        /// </summary>
        /// <param name="table">Table to remove the row from</param>
        /// <param name="key">Primary key of the row to remove</param>
        void deleteRow(const std::wstring& table, const strnum& key)
        {
            deleteRows(table, std::vector<strnum>{ key });
        }

        /// <summary>
        /// Delete multiple rows from a table
        /// </summary>
        /// <param name="table">Table to remove the rows from</param>
        /// <param name="keys">Primary keys or rows to remove</param>
        void deleteRows(const std::wstring& table, const std::vector<strnum>& keys)
        {
            int tableId = tables::getId(*m_db, table, true);
            for (auto val : keys)
            {
                int64_t valueId = values::getId(*m_db, val);
                std::wstring sql = L"DELETE FROM items WHERE valueid = " + std::to_wstring(valueId) + L" AND tableid = " + std::to_wstring(tableId);
                m_db->execSql(sql);
            }
        }

        /// <summary>
        /// Remove a table from the virtual schema
        /// </summary>
        /// <param name="table">Table to drop</param>
        bool drop(const std::wstring& table)
        {
            int tableId = tables::getId(*m_db, table, true, true, true);
            if (tableId < 0)
                return false;

            m_db->execSql(L"DELETE FROM itemnamevalues WHERE nameid IN (SELECT id FROM names WHERE tableid = " + std::to_wstring(tableId) + L")");
            m_db->execSql(L"DELETE FROM names WHERE tableid = " + std::to_wstring(tableId));
            m_db->execSql(L"DELETE FROM items WHERE tableid = " + std::to_wstring(tableId));
            m_db->execSql(L"DELETE FROM tables WHERE id = " + std::to_wstring(tableId));
            
            names::clearCaches();
            tables::clearCaches();

            return true;
        }

        /// <summary>
        /// Reset the database to its initial empty state
        /// </summary>
        void reset()
        {
            items::reset(*m_db);
            values::reset(*m_db);
            names::reset(*m_db);
            tables::reset(*m_db);
        }

        /// <summary>
        /// Get a snapshot of the virtual schema
        /// </summary>
        /// <param name="table">Optional name of single table to return schema of</param>
        /// <returns>Object with table and column names</returns>
        virtualschema getSchema(const std::wstring& table = L"")
        {
            std::wstring sql =
                L"SELECT t.name AS tablename, n.name AS colname "
                L"FROM tables t JOIN names n ON n.tableid = t.id";

            bool haveRequestedTableName = !table.empty();
            if (haveRequestedTableName)
                sql += L" WHERE t.name = @name";

            sql += L" ORDER BY tablename, colname";

            paramap cmdParams;
            if (haveRequestedTableName)
                cmdParams.insert({ L"@name", table });

            virtualschema response;
            auto reader = m_db->execReader(sql, cmdParams);
            while (reader->read())
            {
                std::wstring curTable = reader->getString(0);
                std::wstring colname = reader->getString(1);

                if (!response.contains(curTable))
                    response.insert(curTable, std::make_shared<std::vector<std::wstring>>());
                
                response.get(curTable)->push_back(colname);
            }
            return response;
        }

        /// <summary>
        /// Okay, so there are 6 things...but you really don't need this one! 
        /// </summary>
        /// <param name="name">Name of table to ensure exists in the virtual schema</param>
        /// <param name="isNumeric">Is the primary key of this table numeric?</param>
        /// <returns>Database row ID of the table</returns>
        int ensureTable(const std::wstring& name, bool isNumeric)
        {
            return tables::getId(*m_db, name, isNumeric);
        }

    private:
		static void runSchemaSql(fourdb::db& db, const wchar_t** queries)
		{
            for (size_t idx = 0; queries[idx] != nullptr; ++idx)
                db.execSql(queries[idx]);
		}

	private:
		std::shared_ptr<fourdb::db> m_db;
	};
}
