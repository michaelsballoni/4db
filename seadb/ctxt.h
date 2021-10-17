#pragma once

#include "../sqlite_modern_cpp/hdr/sqlite_modern_cpp.h"

namespace seadb
{
	class ctxt
	{
	public:
		ctxt(const char* dbFilePath)
			: m_db(dbFilePath)
		{}

	private:
		sqlite::database m_db;
	};
}
