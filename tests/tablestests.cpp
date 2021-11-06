#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
	TEST_CLASS(TablesTests)
	{
	public:
		TEST_METHOD(TestTables)
		{
			try
			{
				const char* testDbFilePath = "tables_unit_tests.db";
				if (std::filesystem::exists(testDbFilePath))
					std::filesystem::remove(testDbFilePath);
				ctxt context(testDbFilePath);
				namevalues::clearCaches();

				tables::reset(context.db());

				for (int run = 1; run <= 3; ++run)
				{
					bool isNumeric = true;
					int id = tables::getId(context.db(), L"test-table", isNumeric);
					Assert::IsTrue(id >= 0);

					auto table = tables::getTable(context.db(), id);
					Assert::IsTrue(table.has_value());
					Assert::AreEqual(id, table->id);
					Assert::AreEqual(isNumeric, table->isNumeric);
					Assert::AreEqual(toWideStr("test-table"), table->name);

					int id2 = tables::getId(context.db(), L"test-table");
					Assert::AreEqual(id, id2);
				}
			}
			catch (const std::runtime_error& exp)
			{
				Logger::WriteMessage(("Tables Tests EXCEPTION: " + std::string(exp.what())).c_str());
				throw;
			}
		}
	};
}
