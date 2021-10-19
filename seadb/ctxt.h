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

namespace seadb
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
                    {
                        /* FORNOW
                        RunSql(db, Tables.CreateSql);
                        RunSql(db, Names.CreateSql);
                        RunSql(db, Values.CreateSql);
                        RunSql(db, Items.CreateSql);
                        */
                    }
                    // FORNOW RunSql(db, { "PRAGMA journal_mode = WAL", "PRAGMA synchronous = NORMAL" });
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
            /* FORNOW
            try
            {
                *m_db << "begin";
                RunSql(*m_db, m_postItemOps);
                *m_db << "commit";
                m_postItemOps.clear();
            }
            catch (const std::exception&)
            {
                m_postItemOps.clear();
                *m_db << "rollback";
                throw;
            }
            */
        }

        void addPostOp(const std::string& sql)
        {
            m_postItemOps.push_back(sql);
        }

    private:
        std::vector<std::string> m_postItemOps;

	private:
		static void runSql(db& db, const std::vector<std::string>& queries)
		{
            /* FORNOW
			for (const auto& query : queries)
				db << query;
            */
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
