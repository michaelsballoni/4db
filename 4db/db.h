#pragma once

#include "core.h"
#include "strnum.h"
#include "vectormap.h"

#include "dbreader.h"

namespace fourdb
{
    /// <summary>
    /// Shorthand for a string-name-to-strnum-value query parameters map
    /// </summary>
    typedef std::unordered_map<std::wstring, strnum> paramap;

    /// <summary>
    /// SQLite wrapper class
    /// </summary>
    class db
	{
	public:
        db(const std::string& filePath)
            : m_db(nullptr)
        {
            int rc = sqlite3_open(filePath.c_str(), &m_db);
            if (rc != SQLITE_OK)
                throw fourdberr(rc, m_db);
        }

        ~db()
        {
            if (m_db != nullptr)
            {
                int rc = sqlite3_close(m_db);
                assert(rc == SQLITE_OK);
            }
        }

        /// <summary>
        /// Central query execution routine
        /// All querying goes through here
        /// </summary>
        /// <returns>dbreader ready to be processed</returns>
        std::shared_ptr<dbreader> execReader(const std::wstring& sql, const paramap& params = paramap())
        {
            std::wstring fullSql = applyParams(sql, params);
            auto reader = std::make_shared<dbreader>(m_db, fullSql);
            return reader;
        }

        /// <summary>
        /// Run a non-results query, returning the number of rows affected
        /// </summary>
        /// <returns>The number of rows affected</returns>
        int execSql(const std::wstring& sql, const paramap& params = paramap())
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

        std::optional<int> execScalarInt32(const std::wstring& sql, const paramap& params = paramap())
        {
            auto reader = execReader(sql, params);
            if (reader->read())
                return reader->getInt32(0);
            else
                return std::nullopt;
        }

        std::optional<int64_t> execScalarInt64(const std::wstring& sql, const paramap& params = paramap())
        {
            auto reader = execReader(sql, params);
            if (reader->read())
                return reader->getInt64(0);
            else
                return std::nullopt;
        }

        std::optional<std::wstring> execScalarString(const std::wstring& sql, const paramap& params = paramap())
        {
            auto reader = execReader(sql, params);
            if (reader->read())
                return reader->getString(0);
            else
                return std::nullopt;
        }

        int64_t execInsert(const std::wstring& sql, const paramap& params = paramap())
        {
            execSql(sql, params);
            return execScalarInt64(L"select last_insert_rowid()").value();
        }

    private:
        static std::wstring applyParams(const std::wstring& sql, const paramap& params)
        {
            std::wstring retVal = sql;
            for (const auto& it : params)
                replace<std::wstring>(retVal, it.first, it.second.toSqlLiteral());
            return retVal;
        }

    private:
        sqlite3* m_db;
    };
}
