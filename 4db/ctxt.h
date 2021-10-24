#pragma once

#include "includes.h"

#include "db.h"
#include "types.h"

namespace fourdb
{
	class ctxt
	{
	public:
		ctxt(const std::string& dbFilePath)
		{
            {
                std::lock_guard<std::mutex> lock(getDbBuildLock());
                if (!std::filesystem::exists(dbFilePath.c_str()))
                {
                    db db(dbFilePath.c_str());
                    /* FORNOW
                    runSql(db, Tables.CreateSql);
                    runSql(db, Names.CreateSql);
                    runSql(db, Values.CreateSql);
                    runSql(db, Items.CreateSql);
                    */
                    db.execSql("PRAGMA journal_mode = WAL");
                    db.execSql("PRAGMA synchronous = NORMAL");
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
                m_db->execSql("BEGIN");
                runSql(*m_db, m_postItemOps);
                m_db->execSql("COMMIT");
                m_postItemOps.clear();
            }
            catch (const std::exception&)
            {
                m_postItemOps.clear();
                m_db->execSql("ROLLBACK");
                throw;
            }
        }

        void addPostOp(const std::wstring& sql)
        {
            m_postItemOps.push_back(sql);
        }

        db& getdb()
        {
            return *m_db;
        }

    private:
        std::vector<std::wstring> m_postItemOps;

	private:
		static void runSql(db& db, const std::vector<std::wstring>& queries)
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
