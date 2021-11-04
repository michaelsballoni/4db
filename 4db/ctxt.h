#pragma once

#include "includes.h"

#include "db.h"
#include "items.h"
#include "names.h"
#include "namevalues.h"
#include "sql.h"
#include "tables.h"
#include "types.h"
#include "values.h"

namespace fourdb
{
	class ctxt
	{
	public:
		ctxt(const std::string& dbFilePath)
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
        }

        ~ctxt()
        {
            m_db.reset();
        }

        db& db()
        {
            return *m_db;
        }

        std::shared_ptr<dbreader> execQuery(const select& query)
        {
            std::wstring sql = sql::generateSql(*m_db, query);
            return m_db->execReader(sql, query.cmdParams);
        }

        std::shared_ptr<dbreader> execScalar(const select& query)
        {
            auto reader = execQuery(query);
            if (reader->read())
                return reader;
            else
                return nullptr;
        }

        int64_t execScalarInt64(const select& query)
        {
            auto reader = execScalar(query);
            if (reader != nullptr)
                return reader->getInt64(0);
            else
                return -1;
        }

        double execScalarDouble(const select& query)
        {
            auto reader = execScalar(query);
            if (reader != nullptr)
                return reader->getDouble(0);
            else
                return -1;
        }

        std::optional<std::wstring> execScalarString(const select& query)
        {
            auto reader = execScalar(query);
            if (reader != nullptr)
                return reader->getString(0);
            else
                return std::nullopt;
        }

        int64_t getRowId(const std::wstring& tableName, const strnum& key)
        {
            // FORNOW
            //Utils.ValidateTableName(tableName, "GetRowId");
            auto select = sql::parse(L"SELECT id FROM " + tableName + L" WHERE value = @value");
            select.addParam(L"@value", key);
            return execScalarInt64(select);
        }

        double getRowNumberValue(const std::wstring& tableName, int64_t rowId)
        {
            // FORNOW
            //Utils.ValidateTableName(tableName, "GetRowId");
            auto select = sql::parse(L"SELECT value FROM " + tableName + L" WHERE id = @id");
            select.addParam(L"@id", static_cast<double>(rowId));
            return execScalarDouble(select);
        }

        std::wstring getRowStringValue(const std::wstring& tableName, int64_t rowId)
        {
            // FORNOW
            //Utils.ValidateTableName(tableName, "GetRowId");
            auto select = sql::parse(L"SELECT value FROM " + tableName + L" WHERE id = @id");
            select.addParam(L"@id", static_cast<double>(rowId));
            return execScalarString(select).value_or(L"");
        }

        void define(const std::wstring& table, const strnum& key, const paramap& columnData = paramap())
        {
            bool isKeyNumeric = !key.isStr();
            int tableId = tables::getId(*m_db, table, isKeyNumeric);
            int64_t valueId = values::getId(*m_db, key);
            int64_t itemId = items::getId(*m_db, tableId, valueId);

            // name => nameid
            std::unordered_map<int, int64_t> nameValueIds;
            for (auto kvp : columnData.map())
            {
                bool isMetadataNumeric = !kvp.second.isStr();
                int nameId = names::getId(*m_db, tableId, kvp.first, isMetadataNumeric);
                /* FORNOW
                if (kvp.Value == null) // erase value
                {
                    nameValueIds[nameId] = -1;
                    continue;
                }
                */
                bool isNameNumeric = names::getNameIsNumeric(*m_db, nameId);
                bool isValueNumeric = !kvp.second.isStr();
                if (isValueNumeric != isNameNumeric)
                    throw fourdberr("Data numeric does not match name");
                nameValueIds[nameId] = values::getId(*m_db, kvp.second);
            }
            items::setItemData(*m_db, itemId, nameValueIds);
        }

        std::wstring generateSql(select query)
        {
            std::wstring sql = sql::generateSql(*m_db, query);
            return sql;
        }

        void deleteRow(const std::wstring& table, const strnum& value)
        {
            deleteRows(table, std::vector<strnum>{ value });
        }

        void deleteRows(const std::wstring& table, const std::vector<strnum>& values)
        {
            int tableId = tables::getId(*m_db, table, true);
            for (auto val : values)
            {
                int64_t valueId = values::getId(*m_db, val);
                std::wstring sql = L"DELETE FROM items WHERE valueid = " + std::to_wstring(valueId) + L" AND tableid = " + std::to_wstring(tableId);
                m_db->execSql(sql);
            }
        }

        void drop(const std::wstring& table)
        {
            int tableId = tables::getId(*m_db, table, true);
            m_db->execSql(L"DELETE FROM itemnamevalues WHERE nameid IN (SELECT id FROM names WHERE tableid = " + std::to_wstring(tableId) + L")");
            m_db->execSql(L"DELETE FROM names WHERE tableid = " + std::to_wstring(tableId));
            m_db->execSql(L"DELETE FROM items WHERE tableid = " + std::to_wstring(tableId));
            m_db->execSql(L"DELETE FROM tables WHERE id = " + std::to_wstring(tableId));
            namevalues::clearCaches();
        }

        void reset()
        {
            namevalues::reset(*m_db);
        }

        schemaresponse getSchema(const std::wstring& table = L"")
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
                cmdParams.insert(L"@name", table);

            schemaresponse response;
            auto reader = m_db->execReader(sql, cmdParams);
            while (reader->read())
            {
                std::wstring curTable = reader->getString(0);
                std::wstring colname = reader->getString(1);

                if (!response.contains(curTable))
                    response.insert(curTable, std::vector<std::wstring>());
                
                response.get(curTable).push_back(colname);
            }
            return response;
        }

        int ensureTable(const std::wstring& name, bool isNumeric)
        {
            return tables::getId(*m_db, name, isNumeric);
        }

    private:
		static void runSchemaSql(fourdb::db& db, const wchar_t** queries)
		{
            for (size_t idx = 0; queries[idx] != nullptr; ++idx)
                db.execSql(queries[idx]);
		}

	private:
		std::shared_ptr<fourdb::db> m_db;
	};
}
