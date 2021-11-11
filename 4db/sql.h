#pragma once

#include "types.h"

namespace fourdb
{
    /// <summary>
    /// API for turning SQL strings to and from Select query objects
    /// </summary>
    class sql
    {
    public:
        /// <summary>
        /// Given a SQL query, return a a select object, ready for adding parameters and querying
        /// </summary>
        /// <param name="sql">SQL query</param>
        /// <returns>select object ready for adding parameters and executing</returns>
        static select parse(const std::wstring& sql);

        /// <summary>
        /// This is where the magic 4db query => SQL query conversion takes place
        /// </summary>
        /// <param name="db">Database connection</param>
        /// <param name="query">4db SQL query</param>
        /// <returns>Database SQL</returns>
        static std::wstring generateSql(db& db, select query);

    private:
        static std::vector<std::wstring> tokenize(const std::wstring& str);
    };
}
