#pragma once

#include "../sqlite/sqlite3.h"

#include "vectormap.h"

#include <assert.h>

#include <stdexcept>
#include <string>

namespace seadb
{
    class seadberr : public std::runtime_error
    {
    public:
        seadberr(int rc, sqlite3* db)
            : std::runtime_error(db == nullptr ? ("SQLite error: " + std::to_string(rc)) : ("SQLite error: " + std::string(sqlite3_errmsg(db)) + " (" + std::to_string(rc) + ")"))
        {}
    };

    class query
    {
    public:
        query(const std::u16string& sql, const vectormap<std::u16string, std::u16string>& params)
            : m_stmt(nullptr)
            , m_doneReading(false)
        {
            m_sql = sql;
            for (const auto& it : params.map())
            {
                m_sql.replace(m_sql.find(it.first), it.first.length(), it.second);
            }
        }

        ~query()
        {
            sqlite3_finalize(m_stmt);
        }

        void prepare(sqlite3* db)
        {
            m_db = db;
            int rc = sqlite3_prepare16_v3(m_db, m_sql.c_str(), m_sql.size() * 8, 0, &m_stmt, NULL);
            if (rc != SQLITE_OK)
                throw seadberr(rc, db);
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

        std::u16string getstring(unsigned idx);

        double getdouble(unsigned idx);
        int64_t getint64(unsigned idx);

    private:
        sqlite3* m_db;
        sqlite3_stmt* m_stmt;
        bool m_doneReading;
        std::u16string m_sql;
    };

    class db
	{
	public:
        db(const std::u16string& filePath)
            : m_db(nullptr)
        {
            int rc = sqlite3_open16(filePath.c_str(), &m_db);
            if (rc != SQLITE_OK)
                throw seadberr(rc, m_db);
        }

        ~db()
        {
            if (m_db != nullptr)
            {
                int rc = sqlite3_close(m_db);
                assert(rc == SQLITE_OK);
            }
        }

        /* FORNOW
        void begin();
        void commit();
        void rollback();

        int execsql(const query& q);
        long execinsert(const query& q, bool returnNewId = true);
        std::u16string execscalar(const query& q);
        DbDataReader execreader(const query& q);
        */

    private:
        sqlite3* m_db;
    };
}
