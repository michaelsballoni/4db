#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
	TEST_CLASS(ValuesTests)
	{
	public:
		TEST_METHOD(TestValues)
		{
			try
			{
				const char* testDbFilePath = "values_unit_tests.db";
				if (std::filesystem::exists(testDbFilePath))
					std::filesystem::remove(testDbFilePath);
				ctxt context(testDbFilePath);

				values::reset(context.db());

				for (int run = 1; run <= 3; ++run)
				{
					{
						int64_t stringId = values::getId(context.db(), toWideStr("string value"));
						Assert::IsTrue(stringId >= 0);

						strnum stringValue = values::getValue(context.db(), stringId);
						Assert::IsTrue(stringValue.isStr());
						Assert::AreEqual(toWideStr("string value"), stringValue.str());
					}

					{
						int64_t numberId = values::getId(context.db(), 99.14);
						Assert::IsTrue(numberId >= 0);

						strnum numberValue = values::getValue(context.db(), numberId);
						Assert::IsTrue(!numberValue.isStr());
						Assert::AreEqual(99.14, numberValue.num());	
					}
				}
			}
			catch (const std::runtime_error& exp)
			{
				Logger::WriteMessage(("Values Tests EXCEPTION: " + std::string(exp.what())).c_str());
				throw;
			}
		}
	};
}
