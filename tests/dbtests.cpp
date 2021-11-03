#include "pch.h"
#include "CppUnitTest.h"

#include "db.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
	TEST_CLASS(DbTests)
	{
	public:
		TEST_METHOD(TestDb)
		{
			try
			{
				const char* testDbFilePath = "db_unit_tests.db";
				if (std::filesystem::exists(testDbFilePath))
					std::filesystem::remove(testDbFilePath);
				db my_db(testDbFilePath);

				{
					int affected = my_db.execSql(L"CREATE TABLE foo (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, bar INTEGER NOT NULL, blet TEXT NOT NULL)");
					Assert::AreEqual(0, affected);
				}

				int64_t rowId = -1;
				{
					paramap insertParams;
					insertParams.insert(L"@bar", 914);
					insertParams.insert(L"@blet", toWideStr("monkey"));
					rowId = my_db.execInsert(L"INSERT INTO foo (bar, blet) VALUES (@bar, @blet)", insertParams);
					Assert::IsTrue(rowId >= 0);
				}

				{
					paramap scalarParams;
					scalarParams.insert(L"@id", static_cast<double>(rowId));
					int bar = my_db.execScalarInt32(L"SELECT bar FROM foo WHERE id = @id", scalarParams).value();
					Assert::AreEqual(914, bar);
				}

				{
					paramap scalarParams;
					scalarParams.insert(L"@id", static_cast<double>(rowId));
					std::wstring blet = my_db.execScalarString(L"SELECT blet FROM foo WHERE id = @id", scalarParams).value();
					Assert::AreEqual(toWideStr("monkey"), blet);
				}

				int64_t rowId2 = -1;
				{
					paramap insertParams;
					insertParams.insert(L"@bar", 178);
					insertParams.insert(L"@blet", toWideStr("freddy"));
					rowId2 = my_db.execInsert(L"INSERT INTO foo (bar, blet) VALUES (@bar, @blet)", insertParams);
					Assert::IsTrue(rowId2 > rowId);
				}

				{
					auto reader = my_db.execReader(L"SELECT id, bar, blet FROM foo ORDER BY id");
					Assert::IsTrue(reader->read());
					Assert::AreEqual(rowId, reader->getInt64(0));
					Assert::AreEqual(914, reader->getInt32(1));
					Assert::AreEqual(toWideStr("monkey"), reader->getString(2));

					Assert::IsTrue(reader->read());
					Assert::AreEqual(rowId2, reader->getInt64(0));
					Assert::AreEqual(178, reader->getInt32(1));
					Assert::AreEqual(toWideStr("freddy"), reader->getString(2));

					Assert::IsTrue(!reader->read());
					Assert::IsTrue(!reader->read());
				}

				{
					paramap deleteParams;
					deleteParams.insert(L"@id", static_cast<double>(rowId));
					int affected = my_db.execSql(L"DELETE FROM foo WHERE id = @id", deleteParams);
					Assert::AreEqual(1, affected);
				}

				{
					paramap deleteParams;
					deleteParams.insert(L"@id", static_cast<double>(rowId));
					int affected = my_db.execSql(L"DELETE FROM foo WHERE id = @id", deleteParams);
					Assert::AreEqual(0, affected);
				}

				{
					paramap deleteParams;
					deleteParams.insert(L"@id", static_cast<double>(rowId2));
					int affected = my_db.execSql(L"DELETE FROM foo WHERE id = @id", deleteParams);
					Assert::AreEqual(1, affected);
				}
			}
			catch (const std::runtime_error& exp)
			{
				Logger::WriteMessage(("DB Tests EXCEPTION: " + std::string(exp.what())).c_str());
				throw;
			}
		}
	};
}
