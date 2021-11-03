using System;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace fourdb
{
    [TestClass]
    public class SqlParserTests
    {
        [TestMethod]
        public void TestSqlParser()
        {
            {
                var select = Sql.Parse("SELECT foo, bar\nFROM bletmonkey");
                Assert.AreEqual("foo, bar", string.Join(", ", select.select));
                Assert.AreEqual("bletmonkey", select.from);
            }

            {
                var select = Sql.Parse("SELECT foo, bar\nFROM bletmonkey\nWHERE some <> @all");
                Assert.AreEqual("foo, bar", string.Join(", ", select.select));
                Assert.AreEqual("bletmonkey", select.from);
                Assert.AreEqual(1, select.where[0].criteria.Count);
                Assert.AreEqual("some", select.where[0].criteria[0].name);
                Assert.AreEqual("<>", select.where[0].criteria[0].op);
                Assert.AreEqual("@all", select.where[0].criteria[0].paramName);
            }

            {
                var select = Sql.Parse("SELECT foo, bar\nFROM bletmonkey\nWHERE some <> @all AND all > @okay");
                Assert.AreEqual("foo, bar", string.Join(", ", select.select));
                Assert.AreEqual("bletmonkey", select.from);
                Assert.AreEqual(2, select.where[0].criteria.Count);
                Assert.AreEqual("some", select.where[0].criteria[0].name);
                Assert.AreEqual("<>", select.where[0].criteria[0].op);
                Assert.AreEqual("@all", select.where[0].criteria[0].paramName);
                Assert.AreEqual("all", select.where[0].criteria[1].name);
                Assert.AreEqual(">", select.where[0].criteria[1].op);
                Assert.AreEqual("@okay", select.where[0].criteria[1].paramName);
            }

            {
                var select = Sql.Parse("SELECT foo, bar\nFROM bletmonkey\nORDER BY good DESC");
                Assert.AreEqual("foo, bar", string.Join(", ", select.select));
                Assert.AreEqual("bletmonkey", select.from);
                Assert.AreEqual(1, select.orderBy.Count);
                Assert.AreEqual("good", select.orderBy[0].field);
                Assert.AreEqual(true, select.orderBy[0].descending);
            }

            {
                var select = Sql.Parse("SELECT foo, bar\nFROM bletmonkey\nORDER BY good DESC, bad ASC");
                Assert.AreEqual("foo, bar", string.Join(", ", select.select));
                Assert.AreEqual("bletmonkey", select.from);
                Assert.AreEqual(2, select.orderBy.Count);
                Assert.AreEqual("good", select.orderBy[0].field);
                Assert.AreEqual(true, select.orderBy[0].descending);
                Assert.AreEqual("bad", select.orderBy[1].field);
                Assert.AreEqual(false, select.orderBy[1].descending);
            }

            {
                var select = Sql.Parse("SELECT foo, bar\nFROM bletmonkey\nLIMIT 1492");
                Assert.AreEqual("foo, bar", string.Join(", ", select.select));
                Assert.AreEqual("bletmonkey", select.from);
                Assert.AreEqual(1492, select.limit);
            }
        }
    }
}
