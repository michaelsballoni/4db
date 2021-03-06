#include "pch.h"
#include "sql.h"

#include "db.h"
#include "names.h"
#include "tables.h"

namespace fourdb
{
    enum class state
    {
        SELECT,
        FROM,
        WHERE,
        ORDER,
        LIMIT
    };

    /// <summary>
    /// Given a SQL query, return a a select object, ready for adding parameters and querying
    /// </summary>
    /// <param name="sql">SQL query</param>
    /// <returns>select object ready for adding parameters and executing</returns>
    select sql::parse(const std::wstring& sql)
    {
        std::vector<std::wstring> tokens = tokenize(sql);
        if (tokens.empty())
            throw fourdberr("No tokens");

        select retVal;
        state curState = state::SELECT;
        size_t idx = 0;
        while (idx < tokens.size())
        {
            std::wstring currentToken = tokens[idx];
            if (curState == state::SELECT)
            {
                // Should start with SELECT
                if (_wcsicmp(currentToken.c_str(), L"SELECT") != 0)
                    throw fourdberr("No SELECT");

                // Slurp up the SELECT columns
                while (true)
                {
                    ++idx;
                    if (idx >= tokens.size())
                        throw fourdberr("No SELECT columns");

                    currentToken = tokens[idx];

                    bool lastColumn = currentToken.back() != ',';
                    if (!lastColumn)
                        currentToken = currentToken.substr(0, currentToken.length() - 1);

                    validateColumnName(currentToken);
                    retVal.selectCols.push_back(currentToken);

                    if (lastColumn)
                        break;
                }

                ++idx;
                curState = state::FROM;
                continue;
            }

            if (curState == state::FROM)
            {
                if (_wcsicmp(currentToken.c_str(), L"FROM") != 0)
                    throw fourdberr("No FROM");

                ++idx;
                if (idx >= tokens.size())
                    throw fourdberr("No FROM table");

                currentToken = tokens[idx];
                validateTableName(currentToken);
                retVal.from = currentToken;
                ++idx;
                curState = state::WHERE;
                continue;
            }

            if (curState == state::WHERE)
            {
                if (_wcsicmp(currentToken.c_str(), L"WHERE") != 0)
                {
                    curState = state::ORDER;
                    continue;
                }

                // Gobble up WHERE criteria
                ++idx;
                retVal.where.emplace_back(); // gotta start somewhere
                criteriaset& crits = retVal.where.back();
                while ((idx + 3) <= tokens.size())
                {
                    criteria crit;
                    crit.name = tokens[idx++];
                    crit.op = tokens[idx++];
                    crit.paramName = tokens[idx++];
                    validateColumnName(crit.name);
                    validateOperator(crit.op);
                    validateParameterName(crit.paramName);
                    crits.addCriteria(crit);

                    if
                        (
                            (idx + 3) <= tokens.size()
                            &&
                            _wcsicmp(tokens[idx].c_str(), L"AND") == 0
                            )
                    {
                        ++idx;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                if (crits.criterias.empty())
                    throw fourdberr("No WHERE criteria");

                curState = state::ORDER;
                continue;
            }

            if (curState == state::ORDER)
            {
                std::wstring nextToken =
                    (idx + 1) < tokens.size() ? tokens[idx + 1] : std::wstring();
                if
                    (
                        (idx + 3) > tokens.size()
                        ||
                        _wcsicmp(currentToken.c_str(), L"ORDER") != 0
                        ||
                        _wcsicmp(nextToken.c_str(), L"BY") != 0
                        )
                {
                    curState = state::LIMIT;
                    continue;
                }

                idx += 2;

                while (idx < tokens.size())
                {
                    currentToken = tokens[idx];

                    bool currentEnds = idx == tokens.size() - 1 || currentToken.back() == ',';

                    nextToken = L"ASC";
                    if (!currentEnds)
                    {
                        if ((idx + 1) < tokens.size())
                            nextToken = tokens[++idx];
                    }

                    bool nextEnds = nextToken.back() == ',';

                    bool isLimit = _wcsicmp(nextToken.c_str(), L"LIMIT") == 0;

                    bool lastColumn = isLimit || !(currentEnds || nextEnds);

                    if (!currentToken.empty() && currentToken.back() == ',')
                        currentToken = currentToken.substr(0, currentToken.length() - 1);

                    if (!nextToken.empty() && nextToken.back() == ',')
                        nextToken = nextToken.substr(0, nextToken.length() - 1);

                    bool isDescending;
                    {
                        if (_wcsicmp(nextToken.c_str(), L"ASC") == 0)
                            isDescending = false;
                        else if (_wcsicmp(nextToken.c_str(), L"DESC") == 0)
                            isDescending = true;
                        else if (isLimit)
                            isDescending = false;
                        else
                            throw fourdberr("Invalid ORDER BY");
                    }

                    validateColumnName(currentToken);

                    order orderObj;
                    orderObj.field = currentToken;
                    orderObj.descending = isDescending;
                    retVal.orderBy.push_back(orderObj);

                    if (!isLimit)
                        ++idx;

                    if (lastColumn)
                        break;
                }

                curState = state::LIMIT;
                continue;
            }

            if (curState == state::LIMIT)
            {
                if (_wcsicmp(currentToken.c_str(), L"LIMIT") == 0)
                {
                    ++idx;
                    if (idx >= tokens.size())
                        throw fourdberr("No LIMIT value");
                    currentToken = tokens[idx];

                    int limitVal = _wtoi(currentToken.c_str());
                    if (limitVal <= 0)
                        throw fourdberr("Invalid LIMIT value");
                    retVal.limit = limitVal;

                    ++idx;
                    break;
                }
                else
                {
                    throw fourdberr("Invalid final statement");
                }
            }

            throw fourdberr("Invalid SQL state");
        }

        if (idx < tokens.size() - 1)
            throw fourdberr("Not all parsed");

        if (retVal.selectCols.empty())
            throw fourdberr("No SELECT columns");

        if (retVal.from.empty())
            throw fourdberr("No FROM");

        return retVal;
    }

    /// <summary>
    /// This is where the magic 4db query => SQL query conversion takes place
    /// </summary>
    /// <param name="db">Database connection</param>
    /// <param name="query">4db SQL query</param>
    /// <returns>Database SQL</returns>
    std::wstring sql::generateSql(db& db, select query)
    {
        //
        // "COMPILE"
        //
        if (query.from.empty())
            throw fourdberr("Invalid query, FROM is missing");

        if (query.selectCols.empty())
            throw fourdberr("Invalid query, SELECT is empty");

        for (const auto& order : query.orderBy)
        {
            const std::wstring& orderField = order.field;
            if (std::find(query.selectCols.begin(), query.selectCols.end(), orderField) == query.selectCols.end())
            {
                throw
                    fourdberr
                    (
                        "Invalid query, ORDER BY columns must be present in SELECT column list: "
                        + toNarrowStr(orderField)
                    );
            }
        }

        for (const auto& crits : query.where)
        {
            for (const auto& crit : crits.criterias)
            {
                validateColumnName(crit.name);
                validateOperator(crit.op);
                validateParameterName(crit.paramName);
            }
        }


        //
        // SETUP
        //
        int tableId = tables::getId(db, query.from, false, true, true);
        auto tableObj = tables::getTable(db, tableId);

        // Gather columns
        std::vector<std::wstring> names(query.selectCols);

        for (const auto& orderBy : query.orderBy)
            names.push_back(orderBy.field);

        for (const auto& crits : query.where)
        {
            for (const auto& crit : crits.criterias)
                names.push_back(crit.name);
        }

        // Cut them down
        std::vector<std::wstring> distinctNames;
        for (const auto& name : names)
        {
            if (!name.empty() && std::find(distinctNames.begin(), distinctNames.end(), name) == distinctNames.end())
                distinctNames.push_back(name);
        }
        names = distinctNames;

        // Get name objects
        std::unordered_map<std::wstring, std::optional<names::name_obj>> nameObjs;
        for (const auto& name : names)
        {
            if (isNameReserved(name))
            {
                nameObjs.insert({ name, std::nullopt });
            }
            else
            {
                std::optional<names::name_obj> nameObj;
                {
                    int nameId = names::getId(db, tableId, name, false, true, true);
                    if (nameId < 0)
                        nameObj = std::nullopt;
                    else
                        nameObj = names::getName(db, nameId);
                }
                nameObjs.insert({ name, nameObj });
            }
        }


        //
        // SELECT
        //
        std::wstring selectPart;
        for (const auto& name : query.selectCols)
        {
            auto cleanName = cleanseName(name);

            if (!selectPart.empty())
                selectPart += L",\n";

            if (name == L"value")
            {
                if (!tableObj.has_value())
                    selectPart += L"NULL";
                else if (tableObj->isNumeric)
                    selectPart += L"bv.numberValue";
                else
                    selectPart += L"bv.stringValue";
            }
            else if (name == L"id")
                selectPart += L"i.id";
            else if (name == L"created")
                selectPart += L"i.created";
            else if (name == L"lastmodified")
                selectPart += L"i.lastmodified";
            else if (name == L"count")
                selectPart += L"COUNT(*)";
            else if (name == L"rank")
                selectPart += L"rank";
            else if (!nameObjs[name].has_value())
                selectPart += L"NULL";
            else if (nameObjs[name]->isNumeric)
                selectPart += L"iv" + cleanName + L".numberValue";
            else
                selectPart += L"iv" + cleanName + L".stringValue";

            selectPart += L" AS " + cleanName;
        }
        selectPart = L"SELECT\n" + selectPart;


        //
        // FROM
        //
        std::wstring fromPart = L"FROM\nitems AS i";
        if (nameObjs.find(L"value") != nameObjs.end())
            fromPart += L"\nJOIN bvalues bv ON bv.id = i.valueid";

        for (const auto& name : names)
        {
            if (!isNameReserved(name) && nameObjs[name].has_value())
            {
                auto cleanName = cleanseName(name);
                fromPart +=
                    L"\nLEFT OUTER JOIN itemvalues AS iv" + cleanName + L" ON iv" + cleanName + L".itemid = i.id"
                    L" AND iv" + cleanName + L".nameid = " + std::to_wstring(nameObjs[name]->id);
            }
        }


        //
        // WHERE
        //
        std::wstring wherePart = L"i.tableid = " + std::to_wstring(tableId);
        for (const auto& crits : query.where)
        {
            wherePart += L"\nAND\n";

            wherePart += L"(";
            bool addedOneYet = false;
            for (const auto& where : crits.criterias)
            {
                std::wstring name = where.name;
                if (!addedOneYet)
                    addedOneYet = true;
                else
                    wherePart += L" " + std::wstring(crits.opName()) + L" ";

                auto nameObj = nameObjs[name];
                auto cleanName = cleanseName(name);

                if (_wcsicmp(where.op.c_str(), L"MATCHES") == 0)
                {
                    std::wstring matchTableLabel = cleanName == L"value" ? L"bvtValue" : L"bvt" + cleanName;
                    std::wstring matchColumnLabel = cleanName == L"value" ? L"i.valueid" : L"iv" + cleanName + L".valueid";

                    fromPart += L"\nJOIN bvaluetext " + matchTableLabel + L" ON " + matchColumnLabel + L" = " + matchTableLabel + L".valueid";

                    wherePart += matchTableLabel + L".stringSearchValue MATCH " + where.paramName;

                    order orderBy;
                    orderBy.field = L"rank";
                    orderBy.descending = false;
                    query.orderBy.push_back(orderBy);
                }
                else if (cleanName == L"id")
                {
                    wherePart += L"i.id " + where.op + L" " + where.paramName;
                }
                else if (cleanName == L"value")
                {
                    if (!tableObj.has_value())
                        wherePart += L"1 = 0"; // no table, no match
                    else if (tableObj->isNumeric)
                        wherePart += L"bv.numberValue " + where.op + L" " + where.paramName;
                    else
                        wherePart += L"bv.stringValue " + where.op + L" " + where.paramName;
                }
                else if (cleanName == L"created" || cleanName == L"lastmodified")
                {
                    wherePart += cleanName + L" " + where.op + L" " + where.paramName;
                }
                else if (!nameObj.has_value())
                {
                    wherePart += L"1 = 0"; // name doesn't exist, no match!
                }
                else if (nameObj->isNumeric)
                {
                    wherePart += L"iv" + cleanName + L".numberValue " + where.op + L" " + where.paramName;
                }
                else
                {
                    wherePart += L"iv" + cleanName + L".stringValue " + where.op + L" " + where.paramName;
                }
            }
            wherePart += L")";
        }
        if (!wherePart.empty())
            wherePart = L"WHERE\n" + wherePart;


        //
        // ORDER BY
        //
        std::wstring orderBy;
        for (const auto& order : query.orderBy)
        {
            if (!orderBy.empty())
                orderBy += L",\n";

            std::wstring orderColumn = order.field;
            if (!isNameReserved(orderColumn))
                orderColumn = cleanseName(orderColumn);

            orderBy += orderColumn + (order.descending ? L" DESC" : L" ASC");
        }

        if (!orderBy.empty())
            orderBy = L"ORDER BY\n" + orderBy;


        //
        // LIMIT
        //
        std::wstring limitPart;
        if (query.limit > 0)
            limitPart = L"LIMIT\n" + std::to_wstring(query.limit);


        //
        // SQL
        //
        std::wstring sql;

        sql += selectPart;

        sql += L"\n\n" + fromPart;

        if (!wherePart.empty())
            sql += L"\n\n" + wherePart;

        if (!orderBy.empty())
            sql += L"\n\n" + orderBy;

        if (!limitPart.empty())
            sql += L"\n\n" + limitPart;

        return sql;
    }

    std::vector<std::wstring> sql::tokenize(const std::wstring& str)
    {
        std::vector<std::wstring> retVal;
        std::wstring cur;

        for (auto c : str)
        {
            if (iswspace(c))
            {
                if (!cur.empty())
                {
                    retVal.push_back(cur);
                    cur.clear();
                }
            }
            else
                cur += c;
        }

        if (!cur.empty())
            retVal.push_back(cur);

        return retVal;
    }
}
