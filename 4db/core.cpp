#include "pch.h"
#include "core.h"

namespace fourdb
{
    std::string fourdberr::getExceptionMsg(int rc, sqlite3* db)
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

    void replace(std::wstring& str, const std::wstring& from, const std::wstring& to)
    {
        size_t pos;
        size_t offset = 0;
        const size_t fromSize = from.size();
        const size_t increment = to.size();
        while ((pos = str.find(from, offset)) != std::wstring::npos)
        {
            str.replace(pos, fromSize, to);
            offset = pos + increment;
        }
    }

    bool isWord(const std::wstring& str)
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

    std::wstring toLower(const std::wstring& str)
    {
        std::wstring retVal;
        for (auto c : str)
            retVal += towlower(c);
        return retVal;
    }

    bool isNameReserved(const std::wstring& name)
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

    bool isValidOp(const std::wstring& op)
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

    std::wstring cleanseName(const std::wstring& name)
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

    std::wstring join(const std::vector<std::wstring>& strs, const wchar_t* seperator)
    {
        std::wstring retVal;
        for (const auto& str : strs)
        {
            if (!retVal.empty())
                retVal += seperator;
            retVal += str;
        }
        return retVal;
    }
}
