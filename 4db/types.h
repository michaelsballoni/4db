#pragma once

#include "includes.h"

#include "db.h"
#include "strnum.h"
#include "vectormap.h"

namespace fourdb
{
    struct define
    {
        std::wstring table;
        strnum key;
        std::unordered_map<std::wstring, strnum> metadata;

        define(const std::wstring& _table, const strnum& _key)
            : table(_table)
            , key(_key)
        {}

        define& set(const std::wstring& name, const strnum& value)
        {
            metadata[name] = value;
            return *this;
        }
    };

    struct criteria // WHERE
    {
        std::wstring name;
        std::wstring op;
        std::wstring paramName;
    };

    enum class criteriaop { AND, OR };
    struct criteriaset
    {
        criteriaop combine = criteriaop::AND;
        std::vector<criteria> criterias;

        criteriaset() { }

        criteriaset(const criteria& _criteria)
        {
            addCriteria(_criteria);
        }

        static std::vector<criteriaset> genWhere(const criteria& criteria)
        {
            return { criteriaset(criteria) };
        }

        static std::vector<criteriaset> genWhere(const std::vector<criteria>& _criteria)
        {
            criteriaset set;
            for (const auto& c : _criteria)
                set.addCriteria(c);
            return { set };
        }

        void addCriteria(criteria _criteria)
        {
            criterias.push_back(_criteria);
        }

        const wchar_t* opName() const
        {
            switch (combine)
            {
            case criteriaop::AND: return L"AND";
            case criteriaop::OR: return L"OR";
            default: return L"unknown";
            }
        }
    };

    struct order // ORDER BY
    {
        std::wstring field;
        bool descending = false;
    };

    struct select
    {
        std::vector<std::wstring> selectCols;
        std::wstring from; // FROM
        std::vector<criteriaset> where;
        std::vector<order> orderBy;
        int limit;

        paramap cmdParams;

        select& addParam(const std::wstring& name, const strnum& value)
        {
            cmdParams.insert(name, value);
            return *this;
        }

        select& addOrder(std::wstring name, bool descending)
        {
            order _order;
            _order.field = name;
            _order.descending = descending;
            orderBy.push_back(_order);
            return *this;
        }
    };

    struct schemaresponse
    {
        // table name => column names, kept in order
        vectormap<std::wstring, std::vector<std::wstring>> tables;
    };
}
