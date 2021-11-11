#pragma once

#include "db.h"

namespace fourdb
{
    /// <summary>
    /// implemetation for tracking the row data in virtual tables
    /// </summary>
    class values
    {
    public:
        static const wchar_t** createSql();

        static void reset(db& db);

        static int64_t getId(db& db, const strnum& value);

        static strnum getValue(db& db, int64_t id);

    private:
        static int64_t getIdSelect(db& db, const strnum& value);
        static int64_t getIdInsert(db& db, const strnum& value);
    };
}
