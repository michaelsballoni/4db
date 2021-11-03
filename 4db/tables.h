#pragma once

#include "db.h"

namespace fourdb
{
    struct table_obj
    {
        int id = -1;
        std::wstring name;
        bool isNumeric = false;
    };

    /// <summary>
    /// implementation class for the tables in the virtual schema
    /// </summary>
    class tables
    {
    public:
        static const wchar_t** createSql()
        {
            static const wchar_t* sql[] =
            {
                L"CREATE TABLE tables\n(\n"
                L"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,\n"
                L"name TEXT NOT NULL UNIQUE,\n"
                L"isNumeric BOOLEAN NOT NULL\n"
                L")",
                nullptr
            };
            return sql;
        }

        /// <summary>
        /// Remove all tables from the database
        /// </summary>
        /// <param name="db">Database connection</param>
        static void reset(db& db)
        {
            clearCaches();
            db.execSql(L"DELETE FROM tables");
        }

        /// <summary>
        /// Given a table name, get the row ID for the table
        /// </summary>
        /// <param name="ctxt">Database connection</param>
        /// <param name="name">Table name</param>
        /// <param name="isNumeric">Is the table's primary key numeric or string</param>
        /// <param name="noCreate">Should an exception be thrown if no table found</param>
        /// <param name="noException">Should -1 be returned instead of throwing an exception if the table is not found</param>
        /// <returns>Database row ID for the table</returns>
        static int getId(db& db, std::wstring name, bool isNumeric = false, bool noCreate = false, bool noException = false)
        {
            std::lock_guard<std::mutex> lock(getMutex());

            auto it = getCache().find(name);
            if (it != getCache().end())
                return it->second;

            paramap cmdParams;
            cmdParams.insert(L"@name", name);
            std::wstring selectSql = L"SELECT id FROM tables WHERE name = @name";
            std::optional<int> idObj = db.execScalarInt32(selectSql, cmdParams);
            int id = idObj.value_or(-1);
            if (id >= 0)
            {
                getCache()[name] = id;
                return id;
            }

            if (noCreate)
            {
                if (noException)
                    return -1;

                throw fourdberr("tables.getid cannot create new table: " + toNarrowStr(name));
            }

            cmdParams.insert(L"@isNumeric", isNumeric);
            std::wstring insertSql = L"INSERT INTO tables (name, isNumeric) VALUES (@name, @isNumeric)";
            id = static_cast<int>(db.execInsert(insertSql, cmdParams));
            getCache()[name] = id;
            return id;
        }

        /// <summary>
        /// Get info about the table found by looking up the row ID
        /// </summary>
        /// <param name="ctxt">Database connection</param>
        /// <param name="id">Table database row ID</param>
        /// <returns></returns>
        static std::optional<table_obj> getTable(db& db, int id)
        {
            std::lock_guard<std::mutex> lock(getMutex());

            if (id < 0)
                return std::nullopt;

            auto it = getCacheBack().find(id);
            if (it != getCacheBack().end())
                return it->second;

            std::wstring sql = L"SELECT name, isNumeric FROM tables WHERE id = " + std::to_wstring(id);
            auto reader = db.execReader(sql);
            if (!reader->read())
                throw fourdberr("tables.getTable fails to find record: " + std::to_string(id));

            table_obj obj;
            obj.id = id;
            obj.name = reader->getString(0);
            obj.isNumeric = reader->getBoolean(1);
            getCacheBack()[id] = obj;
            return obj;
        }

        static void clearCaches()
        {
            std::lock_guard<std::mutex> lock(getMutex());
            getCache().clear();
            getCacheBack().clear();
        }

    private:
        static std::mutex& getMutex()
        {
            static std::mutex mtx;
            return mtx;
        }

        static std::unordered_map<std::wstring, int>& getCache()
        {
            static std::unordered_map<std::wstring, int> cache;
            return cache;
        }

        static std::unordered_map<int, table_obj>& getCacheBack()
        {
            static std::unordered_map<int, table_obj> cacheBack;
            return cacheBack;
        }
    };
}
