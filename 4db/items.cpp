#include "pch.h"
#include "items.h"

namespace fourdb
{
    const wchar_t** items::createSql()
    {
        static const wchar_t* sql[] =
        {
            L"CREATE TABLE items\n("
            L"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,"
            L"tableid INTEGER NOT NULL,"
            L"valueid INTEGER NOT NULL,"
            L"created TIMESTAMP NOT NULL,"
            L"lastmodified TIMESTAMP NOT NULL,"
            L"FOREIGN KEY(tableid) REFERENCES tables(id),"
            L"FOREIGN KEY(valueid) REFERENCES bvalues(id)"
            L")",

            L"CREATE UNIQUE INDEX idx_items_valueid_tableid ON items (valueid, tableid)",
            L"CREATE INDEX idx_items_created ON items (created)",
            L"CREATE INDEX idx_items_lastmodified ON items (lastmodified)",

            L"CREATE TABLE itemnamevalues\n("
            L"itemid INTEGER NOT NULL,"
            L"nameid INTEGER NOT NULL,"
            L"valueid INTEGER NOT NULL,"
            L"PRIMARY KEY (itemid, nameid),"
            L"FOREIGN KEY(itemid) REFERENCES items(id),"
            L"FOREIGN KEY(nameid) REFERENCES names(id),"
            L"FOREIGN KEY(valueid) REFERENCES bvalues(id)"
            L")",

            L"CREATE VIEW itemvalues AS "
            L"SELECT "
            L"inv.itemid AS itemid,"
            L"inv.nameid AS nameid,"
            L"v.id AS valueid,"
            L"v.isNumeric AS isNumeric,"
            L"v.numberValue AS numberValue,"
            L"v.stringValue AS stringValue "
            L"FROM itemnamevalues AS inv "
            L"JOIN bvalues AS v ON v.id = inv.valueid",

            nullptr
        };
        return sql;
    }

    void items::reset(db& db)
    {
        db.execSql(L"DELETE FROM items");
    }

    int64_t items::getId(db& db, int tableId, int64_t valueId, bool noCreate)
    {
        paramap params
        {
            { L"@tableId", static_cast<double>(tableId) },
            { L"@valueId", static_cast<double>(valueId) }
        };

        {
            std::wstring selectSql =
                L"SELECT id FROM items WHERE tableId = @tableId AND valueId = @valueId";
            int64_t id = db.execScalarInt64(selectSql, params).value_or(-1);
            if (id >= 0)
                return id;
        }

        if (noCreate)
            return -1;

        {
            std::wstring insertSql =
                L"INSERT INTO items (tableid, valueid, created, lastmodified) "
                L"VALUES (@tableId, @valueId, DATETIME('now'), DATETIME('now'))";
            int64_t id = db.execInsert(insertSql, params);
            return id;
        }
    }

    std::unordered_map<int, int64_t> items::getItemData(db& db, int64_t itemId)
    {
        std::unordered_map<int, int64_t> retVal;
        std::wstring sql = L"SELECT nameid, valueid FROM itemnamevalues WHERE itemid = " + std::to_wstring(itemId);
        auto reader = db.execReader(sql);
        while (reader->read())
            retVal[reader->getInt32(0)] = reader->getInt64(1);
        return retVal;
    }

    void items::setItemData(db& db, int64_t itemId, const std::unordered_map<int, int64_t>& metadata)
    {
        std::wstring updateSql =
            L"UPDATE items SET lastmodified = DATETIME('now') WHERE id = " + std::to_wstring(itemId);
        db.execSql(updateSql);

        for (auto it : metadata)
        {
            int nameId = it.first;
            int64_t valueId = it.second;

            paramap params
            {
                { L"@itemId", static_cast<double>(itemId) },
                { L"@nameId", static_cast<double>(nameId) },
                { L"@valueId", static_cast<double>(valueId) }
            };
            std::wstring sql =
                L"INSERT INTO itemnamevalues (itemid, nameid, valueid) "
                L"VALUES (@itemId, @nameId, @valueId) "
                L"ON CONFLICT(itemid, nameid) "
                L"DO UPDATE SET valueid = @valueId";
            db.execSql(sql, params);
        }
    }

    void items::removeItemData(db& db, int64_t itemId, int nameId)
    {
        std::wstring updateSql =
            L"UPDATE items SET lastmodified = DATETIME('now') WHERE id = " + std::to_wstring(itemId);
        db.execSql(updateSql);

        paramap params
        {
            { L"@itemId", static_cast<double>(itemId) },
            { L"@nameId", static_cast<double>(nameId) }
        };
        std::wstring sql = L"DELETE FROM itemnamevalues WHERE itemid = @itemId AND nameid = @nameId";
        db.execSql(sql, params);
    }

    void items::deleteItem(db& db, int64_t itemId)
    {
        db.execSql(L"DELETE FROM items WHERE id = " + std::to_wstring(itemId));
    }
}
