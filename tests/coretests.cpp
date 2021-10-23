#include "pch.h"
#include "CppUnitTest.h"

#include "../4db/tables.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
	TEST_CLASS(CoreTests)
	{
	public:

		TEST_METHOD(TestCore)
		{
			std::string str = "foobar";
			replaceFromTo<std::string>(str, "oo", "yy");
			Assert::AreEqual(std::string("fyybar"), str);
		}
	};
}
