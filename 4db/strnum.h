#pragma once

#include "utils.h"

#include <assert.h>

#include <string>

namespace fourdb
{
    class strnum
    {
    public:
        strnum()
            : m_isStr(false)
            , m_str()
            , m_num(0.0)
        {}

        strnum(const std::u16string& str)
            : m_isStr(true)
            , m_str(str)
            , m_num(0.0)
        {}

        strnum(double num)
            : m_isStr(false)
            , m_str()
            , m_num(num)
        {}

        const std::u16string& str() const
        {
            assert(m_isStr);
            return m_str;
        }

        double num() const
        {
            assert(!m_isStr);
            return m_num;
        }

        bool isStr() const
        {
            return m_isStr;
        }

        std::u16string toSqlLiteral() const
        {
            if (m_isStr)
            {
                std::u16string retVal = m_str; // replace modifies param in place
                replaceFromTo<std::u16string>(retVal, u"\'", u"\'\'");
                retVal = u"'" + retVal + u"'";
                return retVal;
            }
            else
            {
                auto str = std::to_string(m_num);
                return std::u16string(str.begin(), str.end());
            }
        }

    private:
        bool m_isStr;
        std::u16string m_str;
        double m_num;
    };
}
