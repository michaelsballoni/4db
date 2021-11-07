#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
    TEST_CLASS(SqlParserExTests)
    {
    public:
        TEST_METHOD(TestSqlParserEx)
        {
            try
            {
                const char* testDbFilePath = "sqlparserex_unit_tests.db";
                if (std::filesystem::exists(testDbFilePath))
                    std::filesystem::remove(testDbFilePath);
                ctxt context(testDbFilePath);
                namevalues::clearCaches();

                std::wstring sql;
                select select;

                sql = L"";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No tokens"), std::string(exp.what()));
                }

                sql = L"FRED";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No SELECT"), std::string(exp.what()));
                }

                sql = L"SELECT";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No SELECT columns"), std::string(exp.what()));
                }

                sql = L"SELECT foo";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No FROM"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No FROM"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No FROM"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No FROM table"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);

                sql = L"SELECT foo, fred, blet FROM something WHERE";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No WHERE criteria"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo";
                try
                {
                    select = sql::parse(sql); Assert::Fail();
                }
                catch (const std::runtime_error&) {}

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = ";
                try
                {
                    select = sql::parse(sql); Assert::Fail();
                }
                catch (const std::runtime_error&) {}

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria criteria1 = select.where[0].criterias[0];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo AND";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid final statement"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo AND blet";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid final statement"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo AND blet > ";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid final statement"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo AND blet > @monkey";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria1 = select.where[0].criterias[0];
                criteria criteria2 = select.where[0].criterias[1];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);
                Assert::AreEqual(toWideStr("blet"), criteria2.name);
                Assert::AreEqual(toWideStr(">"), criteria2.op);
                Assert::AreEqual(toWideStr("@monkey"), criteria2.paramName);

                sql = L"SELECT foo, fred, blet FROM something "
                      L"WHERE foo = @foo AND blet > @monkey AND amazing LIKE @greatness";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria1 = select.where[0].criterias[0];
                criteria2 = select.where[0].criterias[1];
                criteria criteria3 = select.where[0].criterias[2];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);
                Assert::AreEqual(toWideStr("blet"), criteria2.name);
                Assert::AreEqual(toWideStr(">"), criteria2.op);
                Assert::AreEqual(toWideStr("@monkey"), criteria2.paramName);
                Assert::AreEqual(toWideStr("amazing"), criteria3.name);
                Assert::AreEqual(toWideStr("LIKE"), criteria3.op);
                Assert::AreEqual(toWideStr("@greatness"), criteria3.paramName);

                sql = L"SELECT foo FROM something WHERE !foo = @foo";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid column name: !foo"), std::string(exp.what()));
                }

                sql = L"SELECT foo FROM something WHERE foo !!! @foo";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid query operator: !!!"), std::string(exp.what()));
                }

                sql = L"SELECT foo FROM something WHERE foo = @@@foo";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid parameter name: @@@foo"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid final statement"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria1 = select.where[0].criterias[0];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);
                Assert::AreEqual(1U, select.orderBy.size());
                Assert::AreEqual(toWideStr("fred"), select.orderBy[0].field);
                Assert::AreEqual(false, select.orderBy[0].descending);

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo "
                      L"ORDER BY fred, barney DESC";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria1 = select.where[0].criterias[0];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);
                Assert::AreEqual(2U, select.orderBy.size());
                Assert::AreEqual(toWideStr("fred"), select.orderBy[0].field);
                Assert::AreEqual(false, select.orderBy[0].descending);
                Assert::AreEqual(toWideStr("barney"), select.orderBy[1].field);
                Assert::AreEqual(true, select.orderBy[1].descending);

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo "
                      L"ORDER BY fred ASC, barney";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria1 = select.where[0].criterias[0];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);
                Assert::AreEqual(2U, select.orderBy.size());
                Assert::AreEqual(toWideStr("fred"), select.orderBy[0].field);
                Assert::AreEqual(false, select.orderBy[0].descending);
                Assert::AreEqual(toWideStr("barney"), select.orderBy[1].field);
                Assert::AreEqual(false, select.orderBy[1].descending);

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo LIMIT";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No LIMIT value"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred LIMIT";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("No LIMIT value"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred LIMIT useful";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Invalid LIMIT value"), std::string(exp.what()));
                }

                sql = L"SELECT foo, fred, blet "
                      L"FROM something "
                      L"WHERE foo = @foo "
                      L"ORDER BY fred, barney DESC "
                      L"LIMIT 100";
                select = sql::parse(sql);
                Assert::AreEqual(toWideStr("foo|fred|blet"), join<wchar_t>(select.selectCols, L"|"));
                Assert::AreEqual(toWideStr("something"), select.from);
                criteria1 = select.where[0].criterias[0];
                Assert::AreEqual(toWideStr("foo"), criteria1.name);
                Assert::AreEqual(toWideStr("="), criteria1.op);
                Assert::AreEqual(toWideStr("@foo"), criteria1.paramName);
                Assert::AreEqual(2U, select.orderBy.size());
                Assert::AreEqual(toWideStr("fred"), select.orderBy[0].field);
                Assert::AreEqual(false, select.orderBy[0].descending);
                Assert::AreEqual(toWideStr("barney"), select.orderBy[1].field);
                Assert::AreEqual(true, select.orderBy[1].descending);
                Assert::AreEqual(100, select.limit);

                sql = L"SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred LIMIT 100 AFTER PARTY";
                try
                {
                    select = sql::parse(sql);
                    Assert::Fail();
                }
                catch (const std::runtime_error& exp)
                {
                    Assert::AreEqual(std::string("Not all parsed"), std::string(exp.what()));
                }
            }
            catch (const std::runtime_error& exp)
            {
                Logger::WriteMessage(("SQL Parser Extended Tests EXCEPTION: " + std::string(exp.what())).c_str());
                throw;
            }
        }
    };
}
