#pragma once

#include "core.h"

namespace fourdb
{
    /// <summary>
    /// A strnum is either a wstring or a double
    /// Poor man's variant, it maps directly onto the virtual schema
    /// </summary>
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

        bool operator==(const strnum& other) const
        {
            return m_isStr == other.m_isStr && m_str == other.m_str && m_num == other.m_num;
        }

        const std::wstring& str() const
        {
            if (!m_isStr)
                throw std::runtime_error("not a string");
            return m_str;
        }

        double num() const
        {
            if (m_isStr)
                throw std::runtime_error("not a number");
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
                replace(retVal, L"\'", L"\'\'");
                retVal = L"'" + retVal + L"'";
                return retVal;
            }
            else
            {
                return num2str(m_num);
            }
        }

    private:
        bool m_isStr;
        std::wstring m_str;
        double m_num;
    };
}

namespace std
{
    template <>
    struct hash<fourdb::strnum>
    {
        std::size_t operator()(const fourdb::strnum& sn) const
        {
            const std::hash<std::wstring> hasher;
            return hasher(sn.isStr() ? sn.str() : std::to_wstring(sn.num()));
        }
    };
}
