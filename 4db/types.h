#pragma once

#include "db.h"
#include "strnum.h"
#include "vectormap.h"

namespace fourdb
{
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
        int limit = -1;

        paramap cmdParams;

        select& addParam(const std::wstring& name, const strnum& value)
        {
            cmdParams.insert({ name, value });
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

    // table name => column names, all kept in order
    typedef vectormap<std::wstring, std::shared_ptr<std::vector<std::wstring>>> virtualschema;
}
