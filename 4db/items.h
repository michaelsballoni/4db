#pragma once

#include "db.h"

namespace fourdb
{
    /// <summary>
    /// Items are the rows in the metastrings schema
    /// </summary>
    class items
    {
    public:
        static const wchar_t** createSql()
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

        /// <summary>
        /// Remove all rows from the items table
        /// </summary>
        static void reset(db& db)
        {
            db.execSql(L"DELETE FROM items");
        }

        /// <summary>
        /// Given a table and value, find the item
        /// </summary>
        /// <param name="ctxt">Database connection</param>
        /// <param name="tableId">The table ID</param>
        /// <param name="valueId">The value ID of the primary key</param>
        /// <param name="noCreate">Whether to return -1 on error</param>
        /// <returns></returns>
        static int64_t getId(db& db, int tableId, int64_t valueId, bool noCreate = false)
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

        /// <summary>
        /// Given an item ID, get the name=>value metadata for the item
        /// </summary>
        /// <param name="ctxt">Database connection</param>
        /// <param name="itemId">The item to get metadat for</param>
        /// <returns></returns>
        static std::unordered_map<int, int64_t> getItemData(db& db, int64_t itemId)
        {
            std::unordered_map<int, int64_t> retVal;
            std::wstring sql = L"SELECT nameid, valueid FROM itemnamevalues WHERE itemid = " + std::to_wstring(itemId);
            auto reader = db.execReader(sql);
            while (reader->read())
                retVal[reader->getInt32(0)] = reader->getInt64(1);
            return retVal;
        }

        /// <summary>
        /// Given an item put name=>value metadata into the database
        /// </summary>
        /// <param name="db">Database connection</param>
        /// <param name="itemId">The item to add metadata to</param>
        /// <param name="itemData">The name ID => value ID metadata</param>
        static void setItemData(db& db, int64_t itemId, const std::unordered_map<int, int64_t>& metadata)
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

        /// <summary>
        /// Given an item remove name metadata from the database
        /// </summary>
        /// <param name="db">Database connection</param>
        /// <param name="itemId">The item to add metadata to</param>
        /// <param name="nameId">The name ID of the metadata to remove from the item</param>
        static void removeItemData(db& db, int64_t itemId, int nameId)
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

        /// <summary>
        /// Delete an item
        /// </summary>
        /// <param name="db">Database connection</param>
        /// <param name="itemId">The item to delete</param>
        static void deleteItem(db& db, int64_t itemId)
        {
            db.execSql(L"DELETE FROM items WHERE id = " + std::to_wstring(itemId));
        }
    };
}
