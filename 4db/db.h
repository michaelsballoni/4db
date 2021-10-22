#pragma once

#include "utils.h"
#include "strnum.h"
#include "vectormap.h"

#include "dbreader.h"

#include <assert.h>

#include <memory>
#include <optional>
#include <string>

namespace fourdb
{
    typedef vectormap<std::string, strnum> paramap;

    class db
	{
	public:
        db(const std::string& filePath)
            : m_db(nullptr)
        {
            int rc = sqlite3_open(filePath.c_str(), &m_db);
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

        std::shared_ptr<dbreader> execReader(const std::wstring& sql, const paramap& params = paramap())
        {
            std::wstring fullSql = applyParams(sql, params);
            auto reader = std::make_shared<dbreader>(m_db, fullSql);
            return reader;
        }

        void execSql(const std::string& sql, const paramap& params = paramap())
        {
            execSql(toWideStr(sql), params);
        }

        void execSql(const std::wstring& sql, const paramap& params = paramap())
        {
            auto reader = execReader(sql, params);
            while (reader->read())
            {
            }
        }

        std::optional<int64_t> execScalarInt32(const std::wstring& sql, const paramap& params = paramap())
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
            return execScalarInt64(sql, params).value();
        }

        int64_t execWithCount(const std::wstring& sql, const paramap& params = paramap())
        {
            execSql(sql, params);
            return execScalarInt64(L"SELECT changes()").value();
        }

        int64_t execWithCount(const std::string& sql, const paramap& params = paramap())
        {
            return execWithCount(toWideStr(sql), params);
        }

        static std::wstring applyParams(const std::wstring& sql, const paramap& params)
        {
            std::wstring retVal = sql;
            for (const auto& it : params.map())
            {
                auto paramWstr = toWideStr(it.first);
                retVal.replace(retVal.find(paramWstr), paramWstr.length(), it.second.toSqlLiteral());
            }
            return retVal;
        }

    private:
        sqlite3* m_db;
    };
}
