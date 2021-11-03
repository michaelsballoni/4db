using System;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace fourdb
{
    [TestClass]
    public class SqlParserTestsEx
    {
        [TestMethod]
        public void TestSqlParserEx()
        {
            string sql;
            Select select;

            sql = "";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp) 
            {
                Assert.AreEqual("No tokens", exp.Message);
            }

            sql = "FRED";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No SELECT", exp.Message);
            }

            sql = "SELECT";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No SELECT columns", exp.Message);
            }

            sql = "SELECT foo";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No FROM", exp.Message);
            }

            sql = "SELECT foo, fred";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No FROM", exp.Message);
            }

            sql = "SELECT foo, fred, blet";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No FROM", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No FROM table", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);

            sql = "SELECT foo, fred, blet FROM something WHERE";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No WHERE criteria", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo";
            try
            {
                select = Sql.Parse(sql); Assert.Fail();
            }
            catch (SqlException) { }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = ";
            try
            {
                select = Sql.Parse(sql); Assert.Fail();
            }
            catch (SqlException) { }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            Criteria criteria1 = select.where[0].criteria[0];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo AND";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid final statement", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo AND blet";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid final statement", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo AND blet > ";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid final statement", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo AND blet > @monkey";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            criteria1 = select.where[0].criteria[0];
            Criteria criteria2 = select.where[0].criteria[1];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);
            Assert.AreEqual("blet", criteria2.name);
            Assert.AreEqual(">", criteria2.op);
            Assert.AreEqual("@monkey", criteria2.paramName);

            sql = "SELECT foo, fred, blet FROM something " +
                    "WHERE foo = @foo AND blet > @monkey AND amazing LIKE @greatness";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            criteria1 = select.where[0].criteria[0];
            criteria2 = select.where[0].criteria[1];
            Criteria criteria3 = select.where[0].criteria[2];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);
            Assert.AreEqual("blet", criteria2.name);
            Assert.AreEqual(">", criteria2.op);
            Assert.AreEqual("@monkey", criteria2.paramName);
            Assert.AreEqual("amazing", criteria3.name);
            Assert.AreEqual("LIKE", criteria3.op);
            Assert.AreEqual("@greatness", criteria3.paramName);

            sql = "SELECT foo FROM something WHERE !foo = @foo";
            try
            {
                select = Sql.Parse(sql);
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid column name: !foo", exp.Message);
            }

            sql = "SELECT foo FROM something WHERE foo !!! @foo";
            try
            {
                select = Sql.Parse(sql);
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid query operator: !!!", exp.Message);
            }

            sql = "SELECT foo FROM something WHERE foo = @@@foo";
            try
            {
                select = Sql.Parse(sql);
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid parameter name: @@@foo", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid final statement", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            criteria1 = select.where[0].criteria[0];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);
            Assert.AreEqual(1, select.orderBy.Count);
            Assert.AreEqual("fred", select.orderBy[0].field);
            Assert.AreEqual(false, select.orderBy[0].descending);

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo " +
                    "ORDER BY fred, barney DESC";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            criteria1 = select.where[0].criteria[0];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);
            Assert.AreEqual(2, select.orderBy.Count);
            Assert.AreEqual("fred", select.orderBy[0].field);
            Assert.AreEqual(false, select.orderBy[0].descending);
            Assert.AreEqual("barney", select.orderBy[1].field);
            Assert.AreEqual(true, select.orderBy[1].descending);

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo " +
                    "ORDER BY fred ASC, barney";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            criteria1 = select.where[0].criteria[0];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);
            Assert.AreEqual(2, select.orderBy.Count);
            Assert.AreEqual("fred", select.orderBy[0].field);
            Assert.AreEqual(false, select.orderBy[0].descending);
            Assert.AreEqual("barney", select.orderBy[1].field);
            Assert.AreEqual(false, select.orderBy[1].descending);

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo LIMIT";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No LIMIT value", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred LIMIT";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("No LIMIT value", exp.Message);
            }

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred LIMIT useful";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Invalid LIMIT value", exp.Message);
            }

            sql = "SELECT foo, fred, blet " +
                    "FROM something " +
                    "WHERE foo = @foo " +
                    "ORDER BY fred, barney DESC " +
                    "LIMIT 100";
            select = Sql.Parse(sql);
            Assert.AreEqual("foo|fred|blet", string.Join('|', select.select));
            Assert.AreEqual("something", select.from);
            criteria1 = select.where[0].criteria[0];
            Assert.AreEqual("foo", criteria1.name);
            Assert.AreEqual("=", criteria1.op);
            Assert.AreEqual("@foo", criteria1.paramName);
            Assert.AreEqual(2, select.orderBy.Count);
            Assert.AreEqual("fred", select.orderBy[0].field);
            Assert.AreEqual(false, select.orderBy[0].descending);
            Assert.AreEqual("barney", select.orderBy[1].field);
            Assert.AreEqual(true, select.orderBy[1].descending);
            Assert.AreEqual(100, select.limit);

            sql = "SELECT foo, fred, blet FROM something WHERE foo = @foo ORDER BY fred LIMIT 100 AFTER PARTY";
            try
            {
                select = Sql.Parse(sql); 
                Assert.Fail();
            }
            catch (SqlException exp)
            {
                Assert.AreEqual("Not all parsed", exp.Message);
            }
        }
    }
}
