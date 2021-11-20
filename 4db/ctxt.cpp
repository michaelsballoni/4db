#include "pch.h"
#include "ctxt.h"

#include "items.h"
#include "names.h"
#include "sql.h"
#include "tables.h"
#include "values.h"

namespace fourdb
{
    ctxt::ctxt(const std::string& dbFilePath, bool clearCaches)
    {
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);
            if (!std::filesystem::exists(dbFilePath.c_str()))
            {
                fourdb::db db(dbFilePath.c_str());

                db.execSql(L"PRAGMA journal_mode = WAL");
                db.execSql(L"PRAGMA synchronous = NORMAL");

                runSchemaSql(db, tables::createSql());
                runSchemaSql(db, names::createSql());
                runSchemaSql(db, values::createSql());
                runSchemaSql(db, items::createSql());
            }
        }

        m_db = std::make_shared<fourdb::db>(dbFilePath.c_str());

        if (clearCaches)
        {
            names::clearCaches();
            tables::clearCaches();
        }
    }

    std::shared_ptr<dbreader> ctxt::execQuery(const select& query)
    {
        std::wstring sql = sql::generateSql(*m_db, query);
        return m_db->execReader(sql, query.cmdParams);
    }

    std::shared_ptr<dbreader> ctxt::execScalar(const select& query)
    {
        auto reader = execQuery(query);
        if (reader->read())
            return reader;
        else
            return nullptr;
    }

    std::optional<int64_t> ctxt::execScalarInt64(const select& query)
    {
        auto reader = execScalar(query);
        if (reader != nullptr)
            return reader->getInt64(0);
        else
            return std::nullopt;
    }

    std::optional<double> ctxt::execScalarDouble(const select& query)
    {
        auto reader = execScalar(query);
        if (reader != nullptr)
            return reader->getDouble(0);
        else
            return std::nullopt;
    }

    std::optional<std::wstring> ctxt::execScalarString(const select& query)
    {
        auto reader = execScalar(query);
        if (reader != nullptr)
            return reader->getString(0);
        else
            return std::nullopt;
    }

    int64_t ctxt::getRowId(const std::wstring& tableName, const strnum& key)
    {
        validateTableName(tableName);
        auto select = sql::parse(L"SELECT id FROM " + tableName + L" WHERE value = @value");
        select.addParam(L"@value", key);
        return execScalarInt64(select).value_or(-1);
    }

    std::optional<double> ctxt::getRowNumberValue(const std::wstring& tableName, int64_t rowId)
    {
        validateTableName(tableName);
        auto select = sql::parse(L"SELECT value FROM " + tableName + L" WHERE id = @id");
        select.addParam(L"@id", static_cast<double>(rowId));
        return execScalarDouble(select);
    }

    std::optional<std::wstring> ctxt::getRowStringValue(const std::wstring& tableName, int64_t rowId)
    {
        validateTableName(tableName);
        auto select = sql::parse(L"SELECT value FROM " + tableName + L" WHERE id = @id");
        select.addParam(L"@id", static_cast<double>(rowId));
        return execScalarString(select);
    }

    void ctxt::define(const std::wstring& table, const strnum& key, const paramap& columnData)
    {
        std::unordered_map<strnum, paramap> keysToColumnData{ { key, columnData} };
        define(table, keysToColumnData, [](const wchar_t* str) { (void)str; });
    }

    void ctxt::define(const std::wstring& table, const std::unordered_map<strnum, paramap>& keysToColumnData, const std::function<void(const wchar_t*)>& pacifier)
    {
        if (keysToColumnData.empty())
            return;

        pacifier(L"Setting up shop");
        std::vector<strnum> keys;
        for (const auto& it : keysToColumnData)
            keys.push_back(it.first);

        strnum firstKey = keys[0];
        bool firstIsString = firstKey.isStr();
        for (const auto& key : keys)
        {
            if (key.isStr() != firstIsString)
                throw fourdberr("Not all primary keys are of the same data type, string or number");
        }

        pacifier(L"Seeding database");
        bool isKeyNumeric = !firstIsString;
        int tableId = tables::getId(*m_db, table, isKeyNumeric);
        std::unordered_map<std::wstring, int64_t> valueIdCache;
        std::vector<std::wstring> allSqlStatements;
        for (const auto& keyToColumnData : keysToColumnData)
        {
            const strnum& key = keyToColumnData.first;
            const paramap& columnData = keyToColumnData.second;

            int64_t tableValueId = values::getId(*m_db, key); // no need to cache, all unique
            int64_t itemId = items::getId(*m_db, tableId, tableValueId);
            if (columnData.empty())
                continue;

            std::unordered_map<int, int64_t> nameValueIds;
            for (const auto& nameValue : columnData)
            {
                const std::wstring& name = nameValue.first;
                const strnum& value = nameValue.second;

                bool isMetadataNumeric = !value.isStr();

                int nameId = names::getId(*m_db, tableId, name, isMetadataNumeric);
                bool isNameNumeric = names::getNameIsNumeric(*m_db, nameId);

                if (isMetadataNumeric != isNameNumeric)
                    throw fourdberr("Data numeric does not match name");

                int64_t valueId = -1;
                {
                    std::wstring cacheKey =
                        value.isStr() ? (L"$" + value.str()) : (L"#" + num2str(value.num()));
                    const auto& cacheIt = valueIdCache.find(cacheKey);
                    if (cacheIt == valueIdCache.end())
                    {
                        valueId = values::getId(*m_db, value);
                        valueIdCache.insert({ cacheKey, valueId });
                    }
                    else
                        valueId = cacheIt->second;
                }
                nameValueIds[nameId] = valueId;
            }

            auto sqlStatements = items::setItemDataSql(itemId, nameValueIds);
            allSqlStatements.insert(allSqlStatements.end(), sqlStatements.begin(), sqlStatements.end());
        }

        pacifier(L"Populating database");
        try
        {
            m_db->execSql(L"BEGIN");
            for (const auto& sql : allSqlStatements)
                m_db->execSql(sql);
            m_db->execSql(L"COMMIT");
        }
        catch (...)
        {
            m_db->execSql(L"ROLLBACK");
            throw;
        }
    }

    void ctxt::undefine(const std::wstring& table, const strnum& key, const std::wstring& name)
    {
        bool isKeyNumeric = !key.isStr();
        int tableId = tables::getId(*m_db, table, isKeyNumeric, true);
        int64_t valueId = values::getId(*m_db, key);
        int64_t itemId = items::getId(*m_db, tableId, valueId);
        int nameId = names::getId(*m_db, tableId, name, false, true);
        items::removeItemData(*m_db, itemId, nameId);
    }

    std::wstring ctxt::generateSql(const select& query)
    {
        std::wstring sql = sql::generateSql(*m_db, query);
        return sql;
    }

    void ctxt::deleteRow(const std::wstring& table, const strnum& key)
    {
        deleteRows(table, std::vector<strnum>{ key });
    }

    void ctxt::deleteRows(const std::wstring& table, const std::vector<strnum>& keys)
    {
        int tableId = tables::getId(*m_db, table, true);
        for (auto val : keys)
        {
            int64_t valueId = values::getId(*m_db, val);
            std::wstring sql = L"DELETE FROM items WHERE valueid = " + std::to_wstring(valueId) + L" AND tableid = " + std::to_wstring(tableId);
            m_db->execSql(sql);
        }
    }

    bool ctxt::drop(const std::wstring& table)
    {
        int tableId = tables::getId(*m_db, table, true, true, true);
        if (tableId < 0)
            return false;

        m_db->execSql(L"DELETE FROM itemnamevalues WHERE nameid IN (SELECT id FROM names WHERE tableid = " + std::to_wstring(tableId) + L")");
        m_db->execSql(L"DELETE FROM names WHERE tableid = " + std::to_wstring(tableId));
        m_db->execSql(L"DELETE FROM items WHERE tableid = " + std::to_wstring(tableId));
        m_db->execSql(L"DELETE FROM tables WHERE id = " + std::to_wstring(tableId));

        names::clearCaches();
        tables::clearCaches();

        return true;
    }

    void ctxt::reset()
    {
        items::reset(*m_db);
        values::reset(*m_db);
        names::reset(*m_db);
        tables::reset(*m_db);
    }

    virtualschema ctxt::getSchema(const std::wstring& table)
    {
        std::wstring sql =
            L"SELECT t.name AS tablename, n.name AS colname "
            L"FROM tables t JOIN names n ON n.tableid = t.id";

        bool haveRequestedTableName = !table.empty();
        if (haveRequestedTableName)
            sql += L" WHERE t.name = @name";

        sql += L" ORDER BY tablename, colname";

        paramap cmdParams;
        if (haveRequestedTableName)
            cmdParams.insert({ L"@name", table });

        virtualschema response;
        auto reader = m_db->execReader(sql, cmdParams);
        while (reader->read())
        {
            std::wstring curTable = reader->getString(0);
            std::wstring colname = reader->getString(1);

            if (!response.contains(curTable))
                response.insert(curTable, std::make_shared<std::vector<std::wstring>>());

            response.get(curTable)->push_back(colname);
        }
        return response;
    }

    int ctxt::ensureTable(const std::wstring& name, bool isNumeric)
    {
        return tables::getId(*m_db, name, isNumeric);
    }

    void ctxt::runSchemaSql(fourdb::db& db, const wchar_t** queries)
    {
        for (size_t idx = 0; queries[idx] != nullptr; ++idx)
            db.execSql(queries[idx]);
    }
}
