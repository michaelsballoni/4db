#pragma once

#include "db.h"

#include "items.h"
#include "names.h"
#include "tables.h"
#include "values.h"

namespace fourdb
{
    /// <summary>
    /// metastrings implementation class for the name-value metadata in the virtual schema
    /// </summary>
    class namevalues
    {
    public:
        /// <summary>
        /// Remove everying from the database
        /// </summary>
        /// <param name="ctxt"></param>
        static void reset(db& db)
        {
            items::reset(db);

            values::reset(db);
            names::reset(db);
            tables::reset(db);
        }

        /// <summary>
        /// Clear the object->id and id-object caches in the name-value classes
        /// </summary>
        static void clearCaches()
        {
            names::clearCaches();
            tables::clearCaches();
        }

        /// <summary>
        /// Given name-value IDs, return name-value string->object values
        /// </summary>
        /// <param name="db">Database connection</param>
        /// <param name="ids">name=>value IDs</param>
        /// <returns>string-to-value metadata</returns>
        static std::unordered_map<std::wstring, strnum> getMetadataValues(db& db, const std::unordered_map<int, int64_t>& ids)
        {
            std::unordered_map<std::wstring, strnum> retVal;
            for (auto kvp : ids)
            {
                name_obj name = names::getName(db, kvp.first);
                strnum value = values::getValue(db, kvp.second);
                retVal.insert({ name.name, value });
            }
            return retVal;
        }
    };
}
