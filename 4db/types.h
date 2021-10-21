#pragma once

#include "db.h"
#include "strnum.h"
#include "vectormap.h"

#include <exception>
#include <string>
#include <unordered_map>

namespace fourdb
{
    struct define
    {
        std::u16string table;
        strnum key;
        std::unordered_map<std::u16string, strnum> metadata;

        define(const std::u16string& _table, const strnum& _key)
            : table(_table)
            , key(_key)
        {}

        define& set(const std::u16string& name, const strnum& value)
        {
            metadata[name] = value;
            return *this;
        }
    };

    struct criteria // WHERE
    {
        std::u16string name;
        std::u16string op;
        std::u16string paramName;
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
    };

    struct order // ORDER BY
    {
        std::u16string field;
        bool descending = false;
    };

    struct select
    {
        std::vector<std::u16string> selectCols;
        std::u16string from; // FROM
        std::vector<criteriaset> where;
        std::vector<order> orderby;
        int limit;

        std::unordered_map<std::u16string, strnum> cmdparams;

        select& addParam(const std::u16string& name, const strnum& value)
        {
            cmdparams[name] = value;
            return *this;
        }

        select& addOrder(std::u16string name, bool descending)
        {
            order _order;
            _order.field = name;
            _order.descending = descending;
            orderby.push_back(_order);
            return *this;
        }
    };

    struct schemaresponse
    {
        // table name => column names, kept in order
        vectormap<std::u16string, std::vector<std::u16string>> tables;
    };
}