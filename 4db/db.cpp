#include "pch.h"
#include "db.h"

namespace fourdb
{
    db::db(const std::string& filePath)
        : m_db(nullptr)
    {
        int rc = sqlite3_open(filePath.c_str(), &m_db);
        if (rc != SQLITE_OK)
            throw fourdberr(rc, m_db);
    }

    db::~db()
    {
        if (m_db != nullptr)
        {
            int rc = sqlite3_close(m_db);
            assert(rc == SQLITE_OK);
        }
    }

    std::shared_ptr<dbreader> db::execReader(const std::wstring& sql, const paramap& params)
    {
        std::wstring fullSql = applyParams(sql, params);
        auto reader = std::make_shared<dbreader>(m_db, fullSql);
        return reader;
    }

    int db::execSql(const std::wstring& sql, const paramap& params)
    {
        int rowCount = 0;
        {
            auto reader = execReader(sql, params);
            while (reader->read())
            {
                ++rowCount;
            }
        }
        return rowCount > 0 ? rowCount : execScalarInt32(L"SELECT changes()").value();
    }

    std::optional<int> db::execScalarInt32(const std::wstring& sql, const paramap& params)
    {
        auto reader = execReader(sql, params);
        if (reader->read())
            return reader->getInt32(0);
        else
            return std::nullopt;
    }

    std::optional<int64_t> db::execScalarInt64(const std::wstring& sql, const paramap& params)
    {
        auto reader = execReader(sql, params);
        if (reader->read())
            return reader->getInt64(0);
        else
            return std::nullopt;
    }

    std::optional<std::wstring> db::execScalarString(const std::wstring& sql, const paramap& params)
    {
        auto reader = execReader(sql, params);
        if (reader->read())
            return reader->getString(0);
        else
            return std::nullopt;
    }

    int64_t db::execInsert(const std::wstring& sql, const paramap& params)
    {
        execSql(sql, params);
        return execScalarInt64(L"select last_insert_rowid()").value();
    }

    std::wstring db::applyParams(const std::wstring& sql, const paramap& params)
    {
        std::wstring retVal = sql;
        for (const auto& it : params)
            replace(retVal, it.first, it.second.toSqlLiteral());
        return retVal;
    }
}
