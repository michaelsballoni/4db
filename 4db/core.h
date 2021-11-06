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
