#pragma once

#include "db.h"

namespace fourdb
{
    /// <summary>
    /// implemetation for tracking the row data in virtual tables
    /// Values can either be strings or floating-point numbers
    /// This class takes the form of Names and Tables, where you take what you know, in this case a value,
    /// and you get back what you want, the ID of the value's row in the bvalues MySQL table,
    /// or you have a MySQL table row ID, and you want to the value back out
    /// </summary>
    class values
    {
    public:
        static const wchar_t** createSql()
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

        /// <summary>
        /// Return this to the factory original
        /// </summary>
        /// <param name="ctxt">Object for interacting with the database</param>
        static void reset(db& db)
        {
            db.execSql(L"DELETE FROM bvalues");
            db.execSql(L"DELETE FROM bvaluetext");
        }

        /// <summary>
        /// Given a value, get the row ID in the MySQL bvalues table.
        /// Note that there's caching 
        /// </summary>
        /// <param name="ctxt">Object for interacting with the database</param>
        /// <param name="value">
        /// Could be anything, but it's a either a string, or it better be convertible to a double
        /// Note that row IDs are cached, but only for strings or for numbers that are essentially integral
        /// </param>
        /// <returns>row ID in MySQL table</returns>
        static int64_t getId(db& db, const strnum& value)
        {
            int64_t id = getIdSelect(db, value);
            if (id >= 0)
                return id;

            id = getIdInsert(db, value);
            return id;
        }

        static int64_t getIdSelect(db& db, const strnum& value)
        {
            if (value.isStr())
            {
                paramap params;
                params.insert(L"@stringValue", value);
                std::wstring selectSql =
                    L"SELECT id FROM bvalues WHERE isNumeric = 0 AND stringValue = @stringValue";
                int64_t id = db.execScalarInt64(selectSql, params).value_or(-1);
                return id;
            }
            else
            {
                paramap params;
                params.insert(L"@numberValue", value);
                std::wstring selectSql =
                    L"SELECT id FROM bvalues WHERE isNumeric = 1 AND numberValue = @numberValue";
                int64_t id = db.execScalarInt64(selectSql, params).value_or(-1);
                return id;
            }
        }

        static int64_t getIdInsert(db& db, const strnum& value)
        {
            if (value.isStr())
            {
                paramap params;
                params.insert(L"@stringValue", value);
                std::wstring insertSql =
                    L"INSERT INTO bvalues (isNumeric, numberValue, stringValue) VALUES (0, 0.0, @stringValue)";
                int64_t id = db.execInsert(insertSql, params);
                
                params.insert(L"@id", static_cast<double>(id));
                std::wstring textInsertSearchSql =
                    L"INSERT INTO bvaluetext (valueid, stringSearchValue) VALUES (@id, @stringValue)";
                db.execInsert(textInsertSearchSql, params);
                
                return id;
            }
            else
            {
                paramap params;
                params.insert(L"@numberValue", value);
                std::wstring insertSql =
                    L"INSERT INTO bvalues (isNumeric, numberValue, stringValue) VALUES (1, @numberValue, '')";
                int64_t id = db.execInsert(insertSql, params);
                return id;
            }
        }

        /// <summary>
        /// Given a row ID in the MySQL bvalues table, get out the value
        /// </summary>
        /// <param name="ctxt">Object for interacting with the database</param>
        /// <param name="id">row ID in the MySQL bvalues table</param>
        /// <returns></returns>
        static strnum getValue(db& db, int64_t id)
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

        /* FORNOW - Needed?
        /// <summary>
        /// Pre-cache all values in given a set of MySQL table bvalues row IDs
        /// </summary>
        /// <param name="ctxt">Object for interacting with the database</param>
        /// <param name="ids">MySQL table bvalues row IDs</param>
        /// <returns></returns>
        public static async Task CacheValuesAsync(Context ctxt, IEnumerable<long> ids)
        {
            var totalTimer = ScopeTiming.StartTiming();
            try
            {
                var stillToGet = ids.Where(id => !sm_cacheBack.ContainsKey(id));
                if (!stillToGet.Any())
                    return;

                var valueIdInPart = string.Join(",", stillToGet.Select(i => i.ToString()));
                var sql = $"SELECT id, isNumeric, numberValue, stringValue FROM bvalues WHERE id IN ({valueIdInPart})";
                if (sql.Length > 32 * 1024)
                    throw new MetaStringsException("GetValues query exceeds SQL batch limit of 1M.  Use a smaller batch of items.");

                using (var reader = await ctxt.Db.ExecuteReaderAsync(sql).ConfigureAwait(false))
                {
                    while (await reader.ReadAsync().ConfigureAwait(false))
                    {
                        long id = reader.GetInt64(0);

                        bool isNumeric = reader.GetBoolean(1);

                        object obj;
                        if (isNumeric)
                            obj = reader.GetDouble(2);
                        else
                            obj = reader.GetString(3);

                        sm_cacheBack[id] = obj;
                    }
                }
            }
            finally
            {
                ScopeTiming.RecordScope("Values.CacheValues", totalTimer);
            }
        }
        */
    };
}
