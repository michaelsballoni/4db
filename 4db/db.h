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
        db(const std::string& filePath);
        ~db();

        /// <summary>
        /// Central query execution routine
        /// All querying goes through here
        /// </summary>
        /// <returns>dbreader ready to be processed</returns>
        std::shared_ptr<dbreader> execReader(const std::wstring& sql, const paramap& params = paramap());

        /// <summary>
        /// Run a non-results query, returning the number of rows affected
        /// </summary>
        /// <returns>The number of rows affected</returns>
        int execSql(const std::wstring& sql, const paramap& params = paramap());

        std::optional<int> execScalarInt32(const std::wstring& sql, const paramap& params = paramap());

        std::optional<int64_t> execScalarInt64(const std::wstring& sql, const paramap& params = paramap());

        std::optional<std::wstring> execScalarString(const std::wstring& sql, const paramap& params = paramap());

        int64_t execInsert(const std::wstring& sql, const paramap& params = paramap());

    private:
        static std::wstring applyParams(const std::wstring& sql, const paramap& params);

    private:
        sqlite3* m_db;
    };
}
