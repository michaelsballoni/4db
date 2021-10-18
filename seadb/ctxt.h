#pragma once

#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS

#include "../sqlite_modern_cpp/hdr/sqlite_modern_cpp.h"

#include "types.h"

#include <assert.h>

#include <exception>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace seadb
{
	class ctxt
	{
	public:
		ctxt(const std::string& dbFilePath)
		{
            {
                std::lock_guard<std::mutex> lock(GetDbBuildLock());
                if (!std::filesystem::exists(std::filesystem::path(dbFilePath.c_str())))
                {
                    sqlite::database db(dbFilePath.c_str());
                    {
                        /* FORNOW
                        RunSql(db, Tables.CreateSql);
                        RunSql(db, Names.CreateSql);
                        RunSql(db, Values.CreateSql);
                        RunSql(db, Items.CreateSql);
                        */
                    }
                    RunSql(db, { "PRAGMA journal_mode = WAL", "PRAGMA synchronous = NORMAL" });
                }
            }

            m_db = std::make_shared<sqlite::database>(dbFilePath.c_str());
        }

        ~ctxt()
        {
            m_db.reset();
            assert(m_postItemOps.empty());
        }

        void ProcessPostOps()
        {
            if (m_postItemOps.empty())
                return;
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
        }

        void AddPostOp(const std::string& sql)
        {
            m_postItemOps.push_back(sql);
        }

    private:
        std::vector<std::string> m_postItemOps;

	private:
		static void RunSql(sqlite::database& db, const std::vector<std::string>& queries)
		{
			for (const auto& query : queries)
				db << query;
		}

		static std::mutex& GetDbBuildLock()
		{
			static std::mutex mutex;
			return mutex;
		}

	private:
		std::shared_ptr<sqlite::database> m_db;
	};
}
