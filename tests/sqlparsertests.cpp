#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
    TEST_CLASS(SqlParserTests)
    {
    public:
        TEST_METHOD(TestSqlParser)
        {
            try
            {
                const char* testDbFilePath = "sqlparser_unit_tests.db";
                if (std::filesystem::exists(testDbFilePath))
                    std::filesystem::remove(testDbFilePath);
                ctxt context(testDbFilePath);
                namevalues::clearCaches();
                {
                    auto select = sql::parse(L"SELECT foo, bar\nFROM bletmonkey");
                    Assert::AreEqual(toWideStr("foo, bar"), join(select.selectCols, L", "));
                    Assert::AreEqual(toWideStr("bletmonkey"), select.from);
                }

                {
                    auto select = sql::parse(L"SELECT foo, bar\nFROM bletmonkey\nWHERE some <> @all");
                    Assert::AreEqual(toWideStr("foo, bar"), join(select.selectCols, L", "));
                    Assert::AreEqual(toWideStr("bletmonkey"), select.from);
                    Assert::AreEqual(1U, select.where[0].criterias.size());
                    Assert::AreEqual(toWideStr("some"), select.where[0].criterias[0].name);
                    Assert::AreEqual(toWideStr("<>"), select.where[0].criterias[0].op);
                    Assert::AreEqual(toWideStr("@all"), select.where[0].criterias[0].paramName);
                }

                {
                    auto select = sql::parse(L"SELECT foo, bar\nFROM bletmonkey\nWHERE some <> @all AND all > @okay");
                    Assert::AreEqual(toWideStr("foo, bar"), join(select.selectCols, L", "));
                    Assert::AreEqual(toWideStr("bletmonkey"), select.from);
                    Assert::AreEqual(2U, select.where[0].criterias.size());
                    Assert::AreEqual(toWideStr("some"), select.where[0].criterias[0].name);
                    Assert::AreEqual(toWideStr("<>"), select.where[0].criterias[0].op);
                    Assert::AreEqual(toWideStr("@all"), select.where[0].criterias[0].paramName);
                    Assert::AreEqual(toWideStr("all"), select.where[0].criterias[1].name);
                    Assert::AreEqual(toWideStr(">"), select.where[0].criterias[1].op);
                    Assert::AreEqual(toWideStr("@okay"), select.where[0].criterias[1].paramName);
                }

                {
                    auto select = sql::parse(L"SELECT foo, bar, good\nFROM bletmonkey\nORDER BY good DESC");
                    Assert::AreEqual(toWideStr("foo, bar, good"), join(select.selectCols, L", "));
                    Assert::AreEqual(toWideStr("bletmonkey"), select.from);
                    Assert::AreEqual(1U, select.orderBy.size());
                    Assert::AreEqual(toWideStr("good"), select.orderBy[0].field);
                    Assert::AreEqual(true, select.orderBy[0].descending);
                }

                {
                    auto select = sql::parse(L"SELECT foo, bar, good, bad\nFROM bletmonkey\nORDER BY good DESC, bad ASC");
                    Assert::AreEqual(toWideStr("foo, bar, good, bad"), join(select.selectCols, L", "));
                    Assert::AreEqual(toWideStr("bletmonkey"), select.from);
                    Assert::AreEqual(2U, select.orderBy.size());
                    Assert::AreEqual(toWideStr("good"), select.orderBy[0].field);
                    Assert::AreEqual(true, select.orderBy[0].descending);
                    Assert::AreEqual(toWideStr("bad"), select.orderBy[1].field);
                    Assert::AreEqual(false, select.orderBy[1].descending);
                }

                {
                    auto select = sql::parse(L"SELECT foo, bar\nFROM bletmonkey\nLIMIT 1492");
                    Assert::AreEqual(toWideStr("foo, bar"), join(select.selectCols, L", "));
                    Assert::AreEqual(toWideStr("bletmonkey"), select.from);
                    Assert::AreEqual(1492, select.limit);
                }
            }
            catch (const std::runtime_error& exp)
            {
                Logger::WriteMessage(("SQL Parser Tests EXCEPTION: " + std::string(exp.what())).c_str());
                throw;
            }
        }
    };
}
