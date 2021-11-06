#include "pch.h"
#include "CppUnitTest.h"

#include "ctxt.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fourdb
{
    TEST_CLASS(NameValuesTests)
    {
    public:
        TEST_METHOD(TestNameValues)
        {
            try
            {
                const char* testDbFilePath = "namevalues_unit_tests.db";
                if (std::filesystem::exists(testDbFilePath))
                    std::filesystem::remove(testDbFilePath);
                ctxt context(testDbFilePath);
                namevalues::clearCaches();

                db& db = context.db();

                int tableId = tables::getId(db, L"ape");
                int64_t monkeyValueId = values::getId(db, toWideStr("monkey"));

                int64_t itemId0 = items::getId(db, tableId, monkeyValueId);
                int64_t itemId1 = items::getId(db, tableId, monkeyValueId);
                Assert::AreEqual(itemId0, itemId1);

                items::deleteItem(db, itemId1);
                int64_t itemId2 = items::getId(db, tableId, monkeyValueId);
                Assert::AreNotEqual(itemId0, itemId2);

                std::unordered_map<int, int64_t> metaDict;
                int fredNameId = names::getId(db, tableId, L"fred");
                int64_t earnyValueId = values::getId(db, toWideStr("earny"));
                metaDict.insert({ fredNameId, earnyValueId });
                items::setItemData(db, itemId2, metaDict);

                auto outMetaDict = items::getItemData(db, itemId2);
                Assert::AreEqual(1, static_cast<int>(outMetaDict.size()));
                Assert::IsTrue(outMetaDict.find(fredNameId) != outMetaDict.end());
                Assert::AreEqual(earnyValueId, outMetaDict[fredNameId]);
            }
            catch (const std::runtime_error& exp)
            {
                std::string errMsg = exp.what();
                Logger::WriteMessage(("NameValues Test EXCEPTION: " + errMsg).c_str());
                throw;
            }
        }
    };
}
