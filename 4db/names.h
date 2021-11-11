#pragma once

#include "db.h"

namespace fourdb
{
    /// <summary>
    /// implementation class for the columns in the virtual schema
    /// </summary>
    class names
    {
    public:
        static const wchar_t** createSql();
        
        static void reset(db& db);

        static int getId(db& db, int tableId, std::wstring name, bool isNumeric = false, bool noCreate = false, bool noException = false);

        struct name_obj
        {
            int id = -1;
            int tableId = -1;
            std::wstring name;
            bool isNumeric = false;
        };
        static name_obj getName(db& db, int id);

        static bool getNameIsNumeric(db& db, int id);

        static void clearCaches();

    private:
        static std::mutex& getMutex();

        static std::unordered_map<std::wstring, int>& getCache();
        static std::unordered_map<int, name_obj>& getCacheBack();
        static std::unordered_map<int, bool>& getNumericCache();
    };
}
