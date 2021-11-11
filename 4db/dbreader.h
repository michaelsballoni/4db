#pragma once

#include "core.h"

namespace fourdb
{
    /// <summary>
    /// dbreader implements processing database query results 
    /// not meant to be created outside of this project
    /// </summary>
    class dbreader
    {
    public:
        // Called by db, not meant to be called elsewhere
        dbreader(sqlite3* db, const std::wstring& sql);
        ~dbreader();

        bool read();

        unsigned getColCount();
        std::wstring getColName(unsigned idx);

        std::wstring getString(unsigned idx); // best-effort string conversion
        
        double getDouble(unsigned idx);
        
        int64_t getInt64(unsigned idx);
        int getInt32(unsigned idx);
        
        bool getBoolean(unsigned idx);
        
        bool isNull(unsigned idx);

    private:
        sqlite3* m_db;
        sqlite3_stmt* m_stmt;
        bool m_doneReading;
    };
}
