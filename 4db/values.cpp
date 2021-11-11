#include "pch.h"
#include "values.h"

namespace fourdb
{
    const wchar_t** values::createSql()
    {
        static const wchar_t* sql[] =
        {
            L"CREATE TABLE bvalues\n(\n"
            L"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,\n"
            L"isNumeric BOOLEAN NOT NULL,\n"
            L"numberValue NUMBER NOT NULL,\n"
            L"stringValue TEXT\n"
            L")",

            L"CREATE UNIQUE INDEX idx_bvalues_unique ON bvalues (stringValue, numberValue, isNumeric)",

            L"CREATE INDEX idx_bvalues_prefix ON bvalues (stringValue, isNumeric, id)",
            L"CREATE INDEX idx_bvalues_number ON bvalues (numberValue, isNumeric, id)",

            L"CREATE VIRTUAL TABLE bvaluetext USING fts5 (valueid, stringSearchValue)",
            nullptr
        };
        return sql;
    }

    void values::reset(db& db)
    {
        db.execSql(L"DELETE FROM bvalues");
        db.execSql(L"DELETE FROM bvaluetext");
    }

    int64_t values::getId(db& db, const strnum& value)
    {
        int64_t id = getIdSelect(db, value);
        if (id >= 0)
            return id;

        id = getIdInsert(db, value);
        return id;
    }

    strnum values::getValue(db& db, int64_t id)
    {
        std::wstring sql =
            L"SELECT isNumeric, numberValue, stringValue FROM bvalues "
            L"WHERE id = " + std::to_wstring(id);

        auto reader = db.execReader(sql);
        if (!reader->read())
            throw fourdberr("values.getValue fails to find record with ID = " + std::to_string(id));
        bool isNumeric = reader->getBoolean(0);
        if (isNumeric)
            return reader->getDouble(1);
        else
            return reader->getString(2);
    }

    int64_t values::getIdSelect(db& db, const strnum& value)
    {
        if (value.isStr())
        {
            paramap params{ { L"@stringValue", value } };
            std::wstring selectSql =
                L"SELECT id FROM bvalues WHERE isNumeric = 0 AND stringValue = @stringValue";
            int64_t id = db.execScalarInt64(selectSql, params).value_or(-1);
            return id;
        }
        else
        {
            paramap params{ { L"@numberValue", value } };
            std::wstring selectSql =
                L"SELECT id FROM bvalues WHERE isNumeric = 1 AND numberValue = @numberValue";
            int64_t id = db.execScalarInt64(selectSql, params).value_or(-1);
            return id;
        }
    }

    int64_t values::getIdInsert(db& db, const strnum& value)
    {
        if (value.isStr())
        {
            paramap params{ { L"@stringValue", value } };
            std::wstring insertSql =
                L"INSERT INTO bvalues (isNumeric, numberValue, stringValue) VALUES (0, 0.0, @stringValue)";
            int64_t id = db.execInsert(insertSql, params);

            params.insert({ L"@id", static_cast<double>(id) });
            std::wstring textInsertSearchSql =
                L"INSERT INTO bvaluetext (valueid, stringSearchValue) VALUES (@id, @stringValue)";
            db.execInsert(textInsertSearchSql, params);

            return id;
        }
        else
        {
            paramap params{ { L"@numberValue", value } };
            std::wstring insertSql =
                L"INSERT INTO bvalues (isNumeric, numberValue, stringValue) VALUES (1, @numberValue, '')";
            int64_t id = db.execInsert(insertSql, params);
            return id;
        }
    }
}
