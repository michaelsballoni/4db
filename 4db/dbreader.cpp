#include "pch.h"
#include "dbreader.h"

namespace fourdb
{
    dbreader::dbreader(sqlite3* db, const std::wstring& sql)
        : m_db(db)
        , m_stmt(nullptr)
        , m_doneReading(false)
    {
        int rc = sqlite3_prepare_v3(m_db, toNarrowStr(sql).c_str(), -1, 0, &m_stmt, nullptr);
        if (rc != SQLITE_OK)
            throw fourdberr(rc, db);
    }

    dbreader::~dbreader()
    {
        sqlite3_finalize(m_stmt);
    }

    bool dbreader::read()
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
            throw fourdberr(rc, m_db);
    }

    unsigned dbreader::getColCount()
    {
        return static_cast<unsigned>(sqlite3_column_count(m_stmt));
    }

    std::wstring dbreader::getColName(unsigned idx)
    {
        return toWideStr(sqlite3_column_name(m_stmt, idx));
    }

    std::wstring dbreader::getString(unsigned idx)
    {
        auto str = sqlite3_column_text(m_stmt, idx);
        if (str != nullptr)
            return toWideStr(str);

        int columnType = sqlite3_column_type(m_stmt, idx);
        switch (columnType)
        {
        case SQLITE_INTEGER:
            return num2str(static_cast<double>(sqlite3_column_int64(m_stmt, idx)));
        case SQLITE_FLOAT:
            return num2str(sqlite3_column_double(m_stmt, idx));
        case SQLITE_NULL:
            return L"null";
        case SQLITE_BLOB:
            return L"blob";
        default:
            throw fourdberr("Unknown column type: " + std::to_string(columnType));
        }
    }

    double dbreader::getDouble(unsigned idx)
    {
        return sqlite3_column_double(m_stmt, idx);
    }

    int64_t dbreader::getInt64(unsigned idx)
    {
        return sqlite3_column_int64(m_stmt, idx);
    }

    int dbreader::getInt32(unsigned idx)
    {
        return sqlite3_column_int(m_stmt, idx);
    }

    bool dbreader::getBoolean(unsigned idx)
    {
        return getInt32(idx) != 0;
    }

    bool dbreader::isNull(unsigned idx)
    {
        return sqlite3_column_type(m_stmt, idx) == SQLITE_NULL;
    }
}
