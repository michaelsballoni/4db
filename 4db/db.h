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

        std::shared_ptr<dbreader> execReader(const std::u16string& sql, const vectormap<std::u16string, strnum>& params = vectormap<std::u16string, strnum>())
        {
            std::u16string fullSql = applyParams(sql, params);
            auto reader = std::make_shared<dbreader>(m_db, fullSql);
            return reader;
        }

        void execSql(const std::u16string& sql, const vectormap<std::u16string, strnum>& params = vectormap<std::u16string, strnum>())
        {
            auto reader = execReader(sql, params);
            while (reader->read())
            {
            }
        }

        std::optional<int64_t> execScalarInt64(const std::u16string& sql, const vectormap<std::u16string, strnum>& params = vectormap<std::u16string, strnum>())
        {
            auto reader = execReader(sql, params);
            if (reader->read())
                return reader->getInt64(0);
            else
                return std::nullopt;
        }

        std::optional<std::u16string> execScalarString(const std::u16string& sql, const vectormap<std::u16string, strnum>& params = vectormap<std::u16string, strnum>())
        {
            auto reader = execReader(sql, params);
            if (reader->read())
                return reader->getString(0);
            else
                return std::nullopt;
        }

        int64_t execInsert(const std::u16string& sql, const vectormap<std::u16string, strnum>& params = vectormap<std::u16string, strnum>())
        {
            return execScalarInt64(sql, params).value();
        }

        int64_t execWithCount(const std::u16string& sql, const vectormap<std::u16string, strnum>& params = vectormap<std::u16string, strnum>())
        {
            execSql(sql, params);
            return execScalarInt64(u"SELECT changes()").value();
        }

        static std::u16string applyParams(const std::u16string& sql, const vectormap<std::u16string, strnum>& params)
        {
            std::u16string retVal = sql;
            for (const auto& it : params.map())
            {
                retVal.replace(retVal.find(it.first), it.first.length(), it.second.toSqlLiteral());
            }
            return retVal;
        }

    private:
        sqlite3* m_db;
    };
}
