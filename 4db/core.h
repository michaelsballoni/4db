#pragma once

#include "includes.h"

namespace fourdb
{
    class seaerr : public std::runtime_error
    {
    public:
        seaerr(const std::string& msg)
            : std::runtime_error(msg)
        {}
    };

    class seadberr : public seaerr
    {
    public:
        seadberr(int rc, sqlite3* db)
            : seaerr(getExceptionMsg(rc, db))
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
        const wchar_t* from = str.c_str();
        std::size_t len = str.size();
        std::vector<char> buffer(len + 1);
        std::use_facet<std::ctype<wchar_t> >(std::locale("")).narrow(from, from + len, '_', buffer.data());
        return std::string(buffer.data(), &buffer[len]);
    }

    inline std::wstring toWideStr(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }

    inline std::wstring toWideStr(const std::u16string& str)
    {
        return std::wstring(str.begin(), str.end());
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
}
