#include "pch.h"
#include "CppUnitTest.h"

#include "core.h"
#include "strnum.h"
#include "vectormap.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
	TEST_CLASS(CoreTests)
	{
	public:

		TEST_METHOD(TestCore)
		{
			std::string str = "foobar";
			replace<std::string>(str, "oo", "y");
			Assert::AreEqual(std::string("fybar"), str);

			str = "some'thing' else";
			replace<std::string>(str, "'", "''");
			Assert::AreEqual(std::string("some''thing'' else"), str);

			Assert::AreEqual(std::string("foobar"), toNarrowStr(L"foobar"));

			Assert::AreEqual(std::wstring(L"foobar"), toWideStr("foobar"));

			Assert::IsTrue(!isWord(L""));
			Assert::IsTrue(!isWord(L"9foobar"));
			Assert::IsTrue(!isWord(L"bletMonkey=="));
			Assert::IsTrue(isWord(L"foo_bar-914"));

			Assert::AreEqual(toWideStr(""), join<wchar_t>(std::vector<std::wstring>(), L"; "));
			Assert::AreEqual(toWideStr("1"), join<wchar_t>(std::vector<std::wstring>{ L"1" }, L"; "));
			Assert::AreEqual(toWideStr("1; 2"), join<wchar_t>(std::vector<std::wstring>{ L"1", L"2" }, L"; "));
			Assert::AreEqual(toWideStr("1; 2; 3"), join<wchar_t>(std::vector<std::wstring>{ L"1", L"2", L"3" }, L"; "));
		}

		TEST_METHOD(TestStrnum)
		{
			strnum numStr(9.14);
			Assert::IsTrue(!numStr.isStr());
			Assert::AreEqual(9.14, numStr.num());
			Assert::AreEqual(atof("9.14"), atof(toNarrowStr(numStr.toSqlLiteral()).c_str()));

			strnum numStr2(L"blet 'monkey'");
			Assert::IsTrue(numStr2.isStr());
			Assert::AreEqual(std::wstring(L"blet 'monkey'"), numStr2.str());
			Assert::AreEqual(std::wstring(L"'blet ''monkey'''"), numStr2.toSqlLiteral());
		}

		TEST_METHOD(TestVectorMap)
		{
			vectormap<int, std::string> map;
			map.insert(0, "foo");
			map.insert(1, "bar");

			Assert::AreEqual(size_t(2), map.vec().size());
			Assert::AreEqual(size_t(2), map.map().size());

			Assert::AreEqual(0, map.vec()[0].key);
			Assert::AreEqual(std::string("foo"), map.vec()[0].value);

			Assert::AreEqual(1, map.vec()[1].key);
			Assert::AreEqual(std::string("bar"), map.vec()[1].value);

			Assert::IsTrue(map.map().find(0) != map.map().end());
			Assert::AreEqual(std::string("foo"), map.map().find(0)->second);

			Assert::IsTrue(map.map().find(1) != map.map().end());
			Assert::AreEqual(std::string("bar"), map.map().find(1)->second);

			Assert::IsTrue(map.map().find(2) == map.map().end());

			Assert::AreEqual(std::string("bar"), map.get(1));

			Assert::AreEqual(size_t(2), map.size());

			Assert::IsTrue(map.contains(1));
			Assert::IsTrue(!map.contains(2));

			std::string val;
			Assert::IsTrue(map.tryGet(0, val));
			Assert::AreEqual(std::string("foo"), val);

			Assert::IsTrue(!map.tryGet(2, val));
		}
	};
}
