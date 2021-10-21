#pragma once

#include "utils.h"

#include <string>

namespace fourdb
{
    class dbreader
    {
    public:
        dbreader(sqlite3* db, const std::u16string& sql)
            : m_db(db)
            , m_stmt(nullptr)
            , m_doneReading(false)
        {
            int rc = sqlite3_prepare16_v3(m_db, sql.c_str(), sql.size() * 4, 0, &m_stmt, nullptr);
            if (rc != SQLITE_OK)
                throw seadberr(rc, db);
        }

        ~dbreader()
        {
            sqlite3_finalize(m_stmt);
        }

        bool read()
        {
            if (m_doneReading)
                return false;

            int rc = sqlite3_step(m_stmt);
            if (rc == SQLITE_ROW)
            {
                return true;
            }
            else if (rc == SQLITE_DONE)
            {
                m_doneReading = true;
                return false;
            }
            else
                throw seadberr(rc, m_db);
        }

        std::u16string getString(unsigned idx)
        {
            return (char16_t*)sqlite3_column_text16(m_stmt, idx);
        }

        double getDouble(unsigned idx)
        {
            return sqlite3_column_double(m_stmt, idx);
        }

        int64_t getInt64(unsigned idx)
        {
            return sqlite3_column_int64(m_stmt, idx);
        }

    private:
        sqlite3* m_db;
        sqlite3_stmt* m_stmt;
        bool m_doneReading;
    };
}
