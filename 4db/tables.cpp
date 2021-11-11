#include "pch.h"
#include "tables.h"

namespace fourdb
{
    const wchar_t** tables::createSql()
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

    void tables::reset(db& db)
    {
        db.execSql(L"DELETE FROM tables");
        clearCaches();
    }

    int tables::getId(db& db, std::wstring name, bool isNumeric, bool noCreate, bool noException)
    {
        std::lock_guard<std::mutex> lock(getMutex());

        auto it = getCache().find(name);
        if (it != getCache().end())
            return it->second;

        paramap cmdParams{ { L"@name", name } };
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

        cmdParams.insert({ L"@isNumeric", isNumeric });
        std::wstring insertSql = L"INSERT INTO tables (name, isNumeric) VALUES (@name, @isNumeric)";
        id = static_cast<int>(db.execInsert(insertSql, cmdParams));
        getCache()[name] = id;
        return id;
    }

    std::optional<tables::table_obj> tables::getTable(db& db, int id)
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

    void tables::clearCaches()
    {
        std::lock_guard<std::mutex> lock(getMutex());
        getCache().clear();
        getCacheBack().clear();
    }

    std::mutex& tables::getMutex()
    {
        static std::mutex mtx;
        return mtx;
    }

    std::unordered_map<std::wstring, int>& tables::getCache()
    {
        static std::unordered_map<std::wstring, int> cache;
        return cache;
    }

    std::unordered_map<int, tables::table_obj>& tables::getCacheBack()
    {
        static std::unordered_map<int, table_obj> cacheBack;
        return cacheBack;
    }
}
