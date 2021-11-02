#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
    TEST_CLASS(ItemsTests)
    {
    public:
        TEST_METHOD(TestEnsureItem)
        {
            try
            {
                const char* testDbFilePath = "items_unit_tests.db";
                if (std::filesystem::exists(testDbFilePath))
                    std::filesystem::remove(testDbFilePath);
                ctxt context(testDbFilePath);

                items::reset(context.db());

                int tableId = tables::getId(context.db(), L"foo");
                int64_t valueId = values::getId(context.db(), toWideStr("bar"));

                int64_t id0 = items::getId(context.db(), tableId, valueId);
                Assert::IsTrue(id0 >= 0);

                int64_t id1 = items::getId(context.db(), tableId, valueId);
                Assert::AreEqual(id0, id1);
            }
            catch (const std::runtime_error& exp)
            {
                Logger::WriteMessage(("Items Ensure Test EXCEPTION: " + std::string(exp.what())).c_str());
                throw;
            }
        }

        TEST_METHOD(TestItemData)
        {
            try
            {
                const char* testDbFilePath = "items_unit_tests.db";
                if (std::filesystem::exists(testDbFilePath))
                    std::filesystem::remove(testDbFilePath);
                ctxt context(testDbFilePath);

                items::reset(context.db());

                int tableId = tables::getId(context.db(), L"blet");
                int64_t itemId = items::getId(context.db(), tableId, values::getId(context.db(), toWideStr("monkey")));

                {
                    std::unordered_map<int, int64_t> itemData;
                    itemData[names::getId(context.db(), tableId, L"foo")] = values::getId(context.db(), toWideStr("bar"));
                    itemData[names::getId(context.db(), tableId, L"something")] = values::getId(context.db(), toWideStr("else"));
                    items::setItemData(context.db(), itemId, itemData);

                    auto metadata = items::getItemData(context.db(), itemId);
                    for (auto kvp : metadata)
                    {
                        std::wstring nameVal = names::getName(context.db(), kvp.first).name;
                        std::wstring valueVal = values::getValue(context.db(), kvp.second).str();
                        if (nameVal == L"foo")
                            Assert::AreEqual(toWideStr("bar"), valueVal);
                        else if (nameVal == L"something")
                            Assert::AreEqual(toWideStr("else"), valueVal);
                        else
                            Assert::Fail();
                    }
                }

                {
                    std::unordered_map<int, int64_t> itemData;
                    itemData[names::getId(context.db(), tableId, L"foo")] = values::getId(context.db(), toWideStr("blet"));
                    itemData[names::getId(context.db(), tableId, L"something")] = values::getId(context.db(), toWideStr("monkey"));
                    items::setItemData(context.db(), itemId, itemData);

                    auto metadata = items::getItemData(context.db(), itemId);
                    for (const auto& kvp : metadata)
                    {
                        std::wstring nameVal = names::getName(context.db(), kvp.first).name;
                        std::wstring valueVal = values::getValue(context.db(), kvp.second).str();
                        if (nameVal == L"foo")
                            Assert::AreEqual(toWideStr("blet"), valueVal);
                        else if (nameVal == L"something")
                            Assert::AreEqual(toWideStr("monkey"), valueVal);
                        else
                            Assert::Fail();
                    }
                }
            }
            catch (const std::runtime_error& exp)
            {
                std::string errMsg = exp.what();
                Logger::WriteMessage(("Items Data Test EXCEPTION: " + errMsg).c_str());
                throw;
            }
        }
    };
}
