#pragma once

#include "includes.h"

#include "db.h"
#include "items.h"
#include "names.h"
#include "sql.h"
#include "tables.h"
#include "types.h"
#include "values.h"

namespace fourdb
{
	class ctxt
	{
	public:
		ctxt(const std::string& dbFilePath)
		{
            {
                static std::mutex mutex;
                std::lock_guard<std::mutex> lock(mutex);
                if (!std::filesystem::exists(dbFilePath.c_str()))
                {
                    fourdb::db db(dbFilePath.c_str());

                    db.execSql(L"PRAGMA journal_mode = WAL");
                    db.execSql(L"PRAGMA synchronous = NORMAL");
                    
                    runSql(db, tables::createSql());
                    runSql(db, names::createSql());
                    runSql(db, values::createSql());
                    runSql(db, items::createSql());
                }
            }

            m_db = std::make_shared<fourdb::db>(dbFilePath.c_str());
        }

        ~ctxt()
        {
            m_db.reset();
        }

        db& db()
        {
            return *m_db;
        }

        /// <summary>
        /// Get a reader for a query
        /// </summary>
        /// <param name="select">query to execute</param>
        /// <returns>Reader to get results from</returns>
        std::shared_ptr<dbreader> execQuery(select query)
        {
            std::wstring sql = sql::generateSql(*m_db, query);
            return m_db->execReader(sql, query.cmdParams);
        }

	private:
		static void runSql(fourdb::db& db, const wchar_t** queries)
		{
            for (size_t idx = 0; queries[idx] != nullptr; ++idx)
                db.execSql(queries[idx]);
		}

	private:
		std::shared_ptr<fourdb::db> m_db;
	};
}
