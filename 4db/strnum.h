#pragma once

#include "core.h"

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

        strnum(const std::wstring& str)
            : m_isStr(true)
            , m_str(str)
            , m_num(0.0)
        {}

        strnum(double num)
            : m_isStr(false)
            , m_str()
            , m_num(num)
        {}

        const std::wstring& str() const
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

        std::wstring toSqlLiteral() const
        {
            if (m_isStr)
            {
                std::wstring retVal = m_str; // replace modifies param in place
                replace<std::wstring>(retVal, L"\'", L"\'\'");
                retVal = L"'" + retVal + L"'";
                return retVal;
            }
            else
            {
                return std::to_wstring(m_num);
            }
        }

    private:
        bool m_isStr;
        std::wstring m_str;
        double m_num;
    };
}
