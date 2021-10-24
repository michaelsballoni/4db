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

				tables::reset(context.getdb());

				int id = tables::getId(context.getdb(), L"test-table", true);
				Assert::IsTrue(id >= 0);

				auto table = tables::getTable(context.getdb(), id);
				Assert::IsTrue(table.has_value());
				Assert::AreEqual(id, table->id);
				Assert::AreEqual(true, table->isNumeric);
				Assert::AreEqual(toWideStr("test-table"), table->name);

				int id2 = tables::getId(context.getdb(), L"test-table");
				Assert::AreEqual(id, id2);
			}
			catch (const std::runtime_error& exp)
			{
				Logger::WriteMessage(("Tables Tests EXCEPTION: " + std::string(exp.what())).c_str());
				throw;
			}
		}
	};
}
