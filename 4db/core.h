// Utility functions used throughout the class library and unit tests.
#pragma once

#include "includes.h"

#pragma warning(disable : 4996) // doesn't work when camped in includes.h...so here it is...

namespace fourdb
{
    class fourdberr : public std::runtime_error
    {
    public:
        fourdberr(const std::string& msg)
            : std::runtime_error(msg)
        {}

        fourdberr(int rc, sqlite3* db)
            : std::runtime_error(getExceptionMsg(rc, db))
        {}

    private:
        static std::string getExceptionMsg(int rc, sqlite3* db)
        {
            std::string dbErrMsg;
            if (db != nullptr)
                dbErrMsg = sqlite3_errmsg(db);

            std::string retVal;
            if (dbErrMsg.empty())
                retVal = "SQLite error: " + std::to_string(rc);
            else
                retVal = "SQLite error: " + dbErrMsg + " (" + std::to_string(rc) + ")";
            return retVal;
        }
    };

    // https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
    template <typename T>
    inline void replace(T& str, const T& from, const T& to)
    {
        size_t pos;
        size_t offset = 0;
        const size_t fromSize = from.size();
        const size_t increment = to.size();
        while ((pos = str.find(from, offset)) != T::npos)
        {
            str.replace(pos, fromSize, to);
            offset = pos + increment;
        }
    }

    inline std::string toNarrowStr(const std::wstring& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        return converter.to_bytes(str);
    }

    inline std::wstring toWideStr(const std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }

    inline std::wstring toWideStr(const void* bytes)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(reinterpret_cast<const char*>(bytes));
    }

    inline bool isWord(const std::wstring& str)
    {
        if (str.empty())
            return false;

        if (!std::isalpha(str[0]))
            return false;

        for (auto c : str)
        {
            if (!std::isalnum(c) && c != '_' && c != '-')
                return false;
        }
   
        return true;
    }

    inline std::wstring toLower(const std::wstring& str)
    {
        std::wstring retVal;
        for (auto c : str)
            retVal += towlower(c);
        return retVal;
    }

    inline bool isNameReserved(const std::wstring& name)
    {
        static std::unordered_set<std::wstring> names
        {
            L"select",
            L"from",
            L"where",
            L"order",
            L"limit",
            L"value",
            L"id",
            L"count",
            L"created",
            L"lastmodified",
            L"rank"
        };
        return names.find(toLower(name)) != names.end();
    }

    inline bool isValidOp(const std::wstring& op)
    {
        static std::unordered_set<std::wstring> ops
        {
            L"=",
            L"==",
            L"!=",
            L"<>",
            L">",
            L">=",
            L"!>",
            L"<",
            L"<=",
            L"!<",
            L"matches",
            L"like"
        };
        return ops.find(toLower(op)) != ops.end();
    }

    inline void validateTableName(const std::wstring& name)
    {
        if (!isWord(name))
            throw fourdberr("Invalid table name: " + toNarrowStr(name));
    }

    inline void validateColumnName(const std::wstring& name)
    {
        if (!isWord(name))
            throw fourdberr("Invalid column name: " + toNarrowStr(name));
    }

    inline void validateOperator(const std::wstring& op)
    {
        if (!isValidOp(op))
            throw fourdberr("Invalid query operator: " + toNarrowStr(op));
    }

    inline void validateParameterName(const std::wstring& name)
    {
        if (name.empty() || name[0] != L'@' || !isWord(name.substr(1)))
            throw fourdberr("Invalid parameter name: " + toNarrowStr(name));
    }

    inline std::wstring cleanseName(const std::wstring& name) // used for table and column aliases
    {
        std::wstring clean;
        for (auto c : name)
        {
            if (iswalnum(c))
                clean += c;
        }

        if (clean.empty())
            clean = L"a";

        return clean;
    }

    template <typename CharT>
    inline std::basic_string<CharT> join(const std::vector<std::basic_string<CharT>>& strs, const CharT* seperator)
    {
        std::basic_string<CharT> retVal;
        for (const auto& str : strs)
        {
            if (!retVal.empty())
                retVal += seperator;
            retVal += str;
        }
        return retVal;
    }
}
