#pragma once

#include "db.h"

namespace fourdb
{
    struct name_obj
    {
        int id = -1;
        int tableId = -1;
        std::wstring name;
        bool isNumeric = false;
    };

    /// <summary>
    /// implementation class for the columns in the virtual schema
    /// </summary>
    class names
    {
    public:
        static const wchar_t** createSql()
        {
            static const wchar_t* sql[] =
            {
                L"CREATE TABLE names\n(\n"
                L"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,\n"
                L"tableid INTEGER NOT NULL,\n"
                L"name TEXT NOT NULL,\n"
                L"isNumeric BOOLEAN NOT NULL,\n"
                L"FOREIGN KEY(tableid) REFERENCES tables(id)\n"
                L")",
                L"CREATE UNIQUE INDEX idx_names_name_tableid ON names (name, tableid)",
                nullptr
            };
            return sql;
        }
        
        static void reset(db& db)
        {
            db.execSql(L"DELETE FROM names");
            clearCaches();
        }

        static int getId(db& db, int tableId, std::wstring name, bool isNumeric = false, bool noCreate = false, bool noException = false)
        {
            std::lock_guard<std::mutex> lock(getMutex());

            std::wstring cacheKey = std::to_wstring(tableId) + L"_" + name;
            auto it = getCache().find(name);
            if (it != getCache().end())
                return it->second;

            if (!isWord(name))
                throw fourdberr("names.getId name is not valid: " + toNarrowStr(name));

            if (isNameReserved(name))
                throw fourdberr("names.getId name is reserved: " + toNarrowStr(name));

            paramap params
            {
                { L"@tableId", tableId },
                { L"@name", name }
            };
            std::wstring selectSql =
                L"SELECT id FROM names WHERE tableid = @tableId AND name = @name";
            int id = db.execScalarInt32(selectSql, params).value_or(-1);
            if (id >= 0)
            {
                getCache()[cacheKey] = id;
                return id;
            }

            if (noCreate)
            {
                if (noException)
                    return -1;
                else
                    throw fourdberr(toNarrowStr(L"names.getId cannot create new name: " + name));
            }

            params.insert({ L"@isNumeric", isNumeric });
	        std::wstring insertSql =
	            L"INSERT INTO names (tableid, name, isNumeric) VALUES (@tableId, @name, @isNumeric)";
            id = static_cast<int>(db.execInsert(insertSql, params));
            getCache()[cacheKey] = id;
            return id;
        }

        static name_obj getName(db& db, int id)
        {
            std::lock_guard<std::mutex> lock(getMutex());

            auto it = getCacheBack().find(id);
            if (it != getCacheBack().end())
                return it->second;

            std::wstring sql = L"SELECT tableid, name, isNumeric FROM names WHERE id = " + std::to_wstring(id);
            auto reader = db.execReader(sql);
            if (!reader->read())
                throw fourdberr("names.getName fails to find record: " + std::to_string(id));

            name_obj retVal;
            retVal.id = id;
            retVal.tableId = reader->getInt32(0);
            retVal.name = reader->getString(1);
            retVal.isNumeric = reader->getBoolean(2);

            getCacheBack()[id] = retVal;

            return retVal;
        }

        static bool getNameIsNumeric(db& db, int id)
        {
            std::lock_guard<std::mutex> lock(getMutex());

            if (id < 0)
                return false;

            auto it = getNumericCache().find(id);
            if (it != getNumericCache().end())
                return it->second;

            std::wstring sql = L"SELECT isNumeric FROM names WHERE id = " + std::to_wstring(id);
            int numericNum = db.execScalarInt32(sql).value_or(0);
            bool isNumeric = numericNum != 0;
            getNumericCache()[id] = isNumeric;
            return isNumeric;
        }

        static void clearCaches()
        {
            std::lock_guard<std::mutex> lock(getMutex());
            getCache().clear();
            getCacheBack().clear();
            getNumericCache().clear();
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

        static std::unordered_map<int, name_obj>& getCacheBack()
        {
            static std::unordered_map<int, name_obj> cacheBack;
            return cacheBack;
        }

        static std::unordered_map<int, bool>& getNumericCache()
        {
            static std::unordered_map<int, bool> numericCache;
            return numericCache;
        }
    };
}
