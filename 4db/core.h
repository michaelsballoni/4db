// Utility functions used throughout the class library and unit tests.
#pragma once

#pragma warning(disable : 4996) // doesn't work when camped in includes.h...so here it is...
#pragma warning(disable : 4005) // thanks SQLite!

#include "includes.h"

namespace fourdb
{
    class fourdberr : public std::runtime_error
    {
    public:
        fourdberr(const std::string& msg) : std::runtime_error(msg) {}
        fourdberr(int rc, sqlite3* db) : std::runtime_error(getExceptionMsg(rc, db)) {}
    private:
        static std::string getExceptionMsg(int rc, sqlite3* db);
    };

    void replace(std::wstring& str, const std::wstring& from, const std::wstring& to);

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

    std::wstring num2str(double num);

    bool isWord(const std::wstring& str);

    std::wstring toLower(const std::wstring& str);

    bool isNameReserved(const std::wstring& name);
    bool isValidOp(const std::wstring& op);

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
        if (name.empty() || name[0] != '@' || !isWord(name.substr(1)))
            throw fourdberr("Invalid parameter name: " + toNarrowStr(name));
    }

    std::wstring cleanseName(const std::wstring& name); // used for table and column aliases

    std::wstring join(const std::vector<std::wstring>& strs, const wchar_t* seperator);

    std::vector<std::wstring> extractParamNames(const std::wstring& sql);
}
