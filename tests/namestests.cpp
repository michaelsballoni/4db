#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
	TEST_CLASS(NamesTests)
	{
	public:
		TEST_METHOD(TestNames)
		{
			try
			{
				const char* testDbFilePath = "names_unit_tests.db";
				if (std::filesystem::exists(testDbFilePath))
					std::filesystem::remove(testDbFilePath);
				ctxt context(testDbFilePath);

				names::reset(context.db());

				for (int run = 1; run <= 3; ++run)
				{
					int tableid = tables::getId(context.db(), L"test-table");
					Assert::IsTrue(tableid >= 0);

					bool isNumeric = true;
					int nameid = names::getId(context.db(), tableid, L"test-name", isNumeric);
					Assert::IsTrue(nameid >= 0);

					name_obj nameObj = names::getName(context.db(), nameid);
					Assert::AreEqual(nameid, nameObj.id);
					Assert::AreEqual(isNumeric, nameObj.isNumeric);
					Assert::AreEqual(toWideStr("test-name"), nameObj.name);
					Assert::AreEqual(tableid, nameObj.tableId);

					bool dbIsNumeric = names::getNameIsNumeric(context.db(), nameid);
					Assert::AreEqual(isNumeric, dbIsNumeric);
				}
			}
			catch (const std::runtime_error& exp)
			{
				Logger::WriteMessage(("Names Tests EXCEPTION: " + std::string(exp.what())).c_str());
				throw;
			}
		}
	};
}
