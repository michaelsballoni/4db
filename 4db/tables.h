#pragma once

#include "db.h"

namespace fourdb
{
    /// <summary>
    /// implementation class for the tables in the virtual schema
    /// </summary>
    class tables
    {
    public:
        static const wchar_t** createSql();

        static void reset(db& db);

        static int getId(db& db, std::wstring name, bool isNumeric = false, bool noCreate = false, bool noException = false);

        struct table_obj
        {
            int id = -1;
            std::wstring name;
            bool isNumeric = false;
        };
        static std::optional<table_obj> getTable(db& db, int id);

        static void clearCaches();

    private:
        static std::mutex& getMutex();

        static std::unordered_map<std::wstring, int>& getCache();
        static std::unordered_map<int, table_obj>& getCacheBack();
    };
}
