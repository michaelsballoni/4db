#pragma once

#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS

#include "db.h"
#include "types.h"

#include <assert.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace fourdb
{
	class ctxt
	{
	public:
		ctxt(const std::u16string& dbFilePath)
		{
            {
                std::lock_guard<std::mutex> lock(getDbBuildLock());
                if (!std::filesystem::exists(std::filesystem::path(dbFilePath.c_str())))
                {
                    db db(dbFilePath.c_str());
                    /* FORNOW
                    runSql(db, Tables.CreateSql);
                    runSql(db, Names.CreateSql);
                    runSql(db, Values.CreateSql);
                    runSql(db, Items.CreateSql);
                    */
                    db.execSql(u"PRAGMA journal_mode = WAL");
                    db.execSql(u"PRAGMA synchronous = NORMAL");
                }
            }

            m_db = std::make_shared<db>(dbFilePath.c_str());
        }

        ~ctxt()
        {
            m_db.reset();
            assert(m_postItemOps.empty());
        }

        void processPostOps()
        {
            if (m_postItemOps.empty())
                return;
            try
            {
                m_db->execSql(u"begin");
                runSql(*m_db, m_postItemOps);
                m_db->execSql(u"commit");
                m_postItemOps.clear();
            }
            catch (const std::exception&)
            {
                m_postItemOps.clear();
                m_db->execSql(u"rollback");
                throw;
            }
        }

        void addPostOp(const std::u16string& sql)
        {
            m_postItemOps.push_back(sql);
        }

    private:
        std::vector<std::u16string> m_postItemOps;

	private:
		static void runSql(db& db, const std::vector<std::u16string>& queries)
		{
            for (const auto& query : queries)
                db.execSql(query);
		}

		static std::mutex& getDbBuildLock()
		{
			static std::mutex mutex;
			return mutex;
		}

	private:
		std::shared_ptr<db> m_db;
	};
}
