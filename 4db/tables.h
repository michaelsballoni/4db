#pragma once

#include "ctxt.h"

namespace fourdb
{
    struct table_obj
    {
        int id = -1;
        std::wstring name;
        bool isNumeric = false;
    };

    /// <summary>
    /// metastrings implementation class for the tables in the virtual schema
    /// </summary>
    class tables
    {
    public:
        static const char** createSql()
        {
            static const char* sql[] =
            {
                "CREATE TABLE tables\n(\n"
                "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,\n"
                "name TEXT NOT NULL UNIQUE,\n"
                "isNumeric BOOLEAN NOT NULL\n"
                ")",
                nullptr
            };
            return sql;
        }

        /// <summary>
        /// Remove all tables from the database
        /// </summary>
        /// <param name="ctxt">Database connection</param>
        static void reset(ctxt& context)
        {
            clearCaches();
            context.getdb().execSql("DELETE FROM tables");
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
        static int getId(ctxt& context, std::wstring name, bool isNumeric = false, bool noCreate = false, bool noException = false)
        {
            std::lock_guard<std::mutex> lock(getmutx());

            auto it = getcache().find(name);
            if (it != getcache().end())
                return it->second;

            paramap cmdParams;
            cmdParams.insert("@name", name);
            std::wstring selectSql = L"SELECT id FROM tables WHERE name = @name";
            std::optional<int> idObj = context.getdb().execScalarInt32(selectSql, cmdParams);
            int id = idObj.value_or(-1);
            if (id >= 0)
            {
                getcache()[name] = id;
                return id;
            }

            if (noCreate)
            {
                if (noException)
                    return -1;

                throw seaerr("tables.getid cannot create new table: " + toNarrowStr(name));
            }

            cmdParams.insert("@isNumeric", isNumeric);
            std::wstring insertSql = L"INSERT INTO tables (name, isNumeric) VALUES (@name, @isNumeric)";
            id = static_cast<int>(context.getdb().execInsert(insertSql, cmdParams));
            getcache()[name] = id;
            return id;
        }

        /// <summary>
        /// Get info about the table found by looking up the row ID
        /// </summary>
        /// <param name="ctxt">Database connection</param>
        /// <param name="id">Table database row ID</param>
        /// <returns></returns>
        static std::optional<table_obj> getTable(ctxt context, int id)
        {
            std::lock_guard<std::mutex> lock(getmutx());

            if (id < 0)
                return std::nullopt;

            auto it = getcacheback().find(id);
            if (it != getcacheback().end())
                return it->second;

            std::wstring sql = L"SELECT name, isNumeric FROM tables WHERE id = " + std::to_wstring(id);
            auto reader = context.getdb().execReader(sql);
            if (!reader->read())
                throw seaerr("tables.getTable fails to find record: " + std::to_string(id));

            table_obj obj;
            obj.id = id;
            obj.name = reader->getString(0);
            obj.isNumeric = reader->getBoolean(1);
            getcacheback()[id] = obj;
            return obj;
        }

        static void clearCaches()
        {
            std::lock_guard<std::mutex> lock(getmutx());
            getcache().clear();
            getcacheback().clear();
        }

    private:
        static std::mutex& getmutx()
        {
            static std::mutex mtx;
            return mtx;
        }

        static std::unordered_map<std::wstring, int>& getcache()
        {
            static std::unordered_map<std::wstring, int> cache;
            return cache;
        }

        static std::unordered_map<int, table_obj>& getcacheback()
        {
            static std::unordered_map<int, table_obj> cacheBack;
            return cacheBack;
        }
    };
}
