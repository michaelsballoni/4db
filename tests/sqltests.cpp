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
                    auto select = sql::parse(L"SELECT somethin\nFROM nothin");
                    auto reader = context.execQuery(select);
                    Assert::IsFalse(reader->read());
                }

                /* FORNOW
                // Add a row.
                {
                    var define = new Define("somethin", "foo");
                    define.Set("blet", "monkey");
                    ctxt.DefineAsync(define).Wait();
                }

                // Add another row.
                {
                    var define = new Define("somethin", "bar");
                    define.Set("flub", "snake");
                    ctxt.DefineAsync(define).Wait();
                }

                // Have a table now, but bogus SELECT column
                {
                    var select = Sql.Parse("SELECT nothin\nFROM somethin");
                    using (var reader = ctxt.ExecSelectAsync(select).Result)
                    {
                        Assert.IsTrue(reader.Read());
                        Assert.IsTrue(reader.IsDBNull(0));

                        Assert.IsTrue(reader.Read());
                        Assert.IsTrue(reader.IsDBNull(0));

                        Assert.IsFalse(reader.Read());
                    }
                }

                // Have a table now, but bogus WHERE column bdfadf
                {
                    var select = Sql.Parse("SELECT nothin\nFROM somethin\nWHERE value = @foo AND bdfadf = @bdfadf");
                    select.AddParam("@foo", "foo").AddParam("@bdfadf", 12.0);
                    using (var reader = ctxt.ExecSelectAsync(select).Result)
                        Assert.IsFalse(reader.Read());
                }

                // See that it all works
                {
                    var select = Sql.Parse("SELECT blet\nFROM somethin\nWHERE value = @foo");
                    select.AddParam("@foo", "foo");
                    using (var reader = ctxt.ExecSelectAsync(select).Result)
                    {
                        Assert.IsTrue(reader.Read());
                        Assert.AreEqual("monkey", reader.GetString(0));
                        Assert.IsFalse(reader.Read());
                    }
                }

                {
                    var select = Sql.Parse("SELECT flub\nFROM somethin\nWHERE value = @bar");
                    select.AddParam("@bar", "bar");
                    using (var reader = ctxt.ExecSelectAsync(select).Result)
                    {
                        Assert.IsTrue(reader.Read());
                        Assert.AreEqual("snake", reader.GetString(0));
                        Assert.IsFalse(reader.Read());
                    }
                }
                */
            }
            catch (const std::runtime_error& exp)
            {
                Logger::WriteMessage(("SQL Tests EXCEPTION: " + std::string(exp.what())).c_str());
                throw;
            }
        }
    };
}
