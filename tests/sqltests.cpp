#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
    TEST_CLASS(SqlTests)
    {
    public:
        TEST_METHOD(TestSql)
        {
            try
            {
                const char* testDbFilePath = "sql_unit_tests.db";
                if (std::filesystem::exists(testDbFilePath))
                    std::filesystem::remove(testDbFilePath);
                ctxt context(testDbFilePath);

                // No tables, nothing should still work.
                {
                    auto select = sql::parse(L"SELECT somethin FROM nothin");
                    auto reader = context.execQuery(select);
                    Assert::IsFalse(reader->read());
                }

                // Add a row.
                {
                    paramap metadata;
                    metadata.insert(L"blet", toWideStr("monkey"));
                    context.define(L"somethin", toWideStr("foo"), metadata);
                }

                // Add another row.
                {
                    paramap metadata;
                    metadata.insert(L"flub", toWideStr("snake"));
                    context.define(L"somethin", toWideStr("bar"), metadata);
                }

                // Have a table now, but bogus SELECT column
                {
                    auto select = sql::parse(L"SELECT nothin FROM somethin");
                    auto reader = context.execQuery(select);
                    Assert::IsTrue(reader->read());
                    Assert::IsTrue(reader->isNull(0));
                }

                // Have a table now, but bogus WHERE column bdfadf
                {
                    auto select = sql::parse(L"SELECT nothin FROM somethin WHERE value = @foo AND bdfadf = @bdfadf");
                    select.addParam(L"@foo", toWideStr("foo")).addParam(L"@bdfadf", 12.0);
                    auto reader = context.execQuery(select);
                    Assert::IsFalse(reader->read());
                }

                // See that it all works
                {
                    auto select = sql::parse(L"SELECT blet FROM somethin WHERE value = @foo");
                    select.addParam(L"@foo", toWideStr("foo"));
                    auto reader = context.execQuery(select);
                    Assert::IsTrue(reader->read());
                    Assert::AreEqual(toWideStr("monkey"), reader->getString(0));
                    Assert::IsFalse(reader->read());
                }

                {
                    auto select = sql::parse(L"SELECT flub FROM somethin WHERE value = @bar");
                    select.addParam(L"@bar", toWideStr("bar"));
                    auto reader = context.execQuery(select);
                    Assert::IsTrue(reader->read());
                    Assert::AreEqual(toWideStr("snake"), reader->getString(0));
                    Assert::IsFalse(reader->read());
                }
            }
            catch (const std::runtime_error& exp)
            {
                Logger::WriteMessage(("SQL Tests EXCEPTION: " + std::string(exp.what())).c_str());
                throw;
            }
        }
    };
}
