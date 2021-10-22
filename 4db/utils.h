#pragma once

#include "../../sqlite/sqlite3.h"

#include <stdexcept>
#include <string>

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
    inline void replaceFromTo(T& str, const T& from, const T& to)
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

    std::wstring toWideStr(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }

    std::wstring toWideStr(const std::u16string& str)
    {
        return std::wstring(str.begin(), str.end());
    }
}
