#pragma once

#include "db.h"

namespace fourdb
{
    /// <summary>
    /// implementation class for the rows in the virtual schema
    /// </summary>
    class items
    {
    public:
        static const wchar_t** createSql();

        static void reset(db& db);

        static int64_t getId(db& db, int tableId, int64_t valueId, bool noCreate = false);
        static std::unordered_map<int, int64_t> getItemData(db& db, int64_t itemId);

        static void setItemData(db& db, int64_t itemId, const std::unordered_map<int, int64_t>& metadata);
        static std::vector<std::wstring> setItemDataSql(int64_t itemId, const std::unordered_map<int, int64_t>& metadata);
        
        static void removeItemData(db& db, int64_t itemId, int nameId);

        static void deleteItem(db& db, int64_t itemId);
    };
}
