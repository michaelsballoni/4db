#include "ctxt.h"

#include <stdio.h>

#include <string>
#include <iostream>

using namespace fourdb;

FILE* outputFile = nullptr;
void writeLineToFile(const std::wstring& line)
{
    fprintf(outputFile, "%s\n", toNarrowStr(line).c_str());
    fflush(outputFile);
}

void writeLine(const std::wstring& line)
{
    writeLineToFile(line);
    printf("%S\n", line.c_str());
}

std::vector<std::wstring> extractParamNames(const std::wstring& sql)
{
    std::vector<std::wstring> paramNames;
    std::wstring sb;
    int lookFrom = 0;
    while (true)
    {
        if (static_cast<unsigned>(lookFrom) >= sql.length())
            break;

        int at = sql.find(L'@', lookFrom);
        if (at == std::wstring::npos)
            break;

        sb.clear();
        unsigned idx = static_cast<unsigned>(at + 1);
        while (idx < sql.length())
        {
            auto c = sql[idx++];
            if (iswalnum(c) || c == L'_')
            {
                sb += c;
            }
            else
            {
                break;
            }
        }

        if (!sb.empty())
        {
            paramNames.push_back(L"@" + sb);
            sb.clear();
        }
        lookFrom = idx;
    }

    if (!sb.empty())
    {
        paramNames.push_back(L"@" + sb);
        sb.clear();
    }
    return paramNames;
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc <= 1)
        {
            printf("Usage: 4dbq <database file path>\n");
            return 0;
        }

        printf("Opening output file...");
        outputFile = fopen("4dbq.log", "w");
        printf("done!\n");
        if (outputFile == nullptr)
        {
            printf("ERROR: Could not create output log file 4dbq.log\n");
            return 1;
        }

        const char* dbFilePath = argv[1];
        printf("Opening database %s...", dbFilePath);
        ctxt context(dbFilePath);
        printf("done!\n");

        printf("\n");

        printf("Enter your SQL on one or more lines, end with a blank line,\n");
        printf("then supply param values, and away we go!\n");

        while (true)
        {
            try
            {
                printf("\n> ");
                std::wstring query;
                std::getline(std::wcin, query);
                if (query.empty())
                    continue;
                else if (query == L"quit" || query == L"exit")
                    break;

                while (true)
                {
                    std::wstring nextLine;
                    std::getline(std::wcin, nextLine);
                    if (nextLine.empty())
                        break;
                    query += L"\n" + nextLine;
                }

                auto select = sql::parse(query);
                auto paramNames = extractParamNames(query);
                if (!paramNames.empty())
                {
                    for (auto paramName : paramNames)
                    {
                        printf("%S: ", paramName.c_str());
                        std::wstring paramValue;
                        std::getline(std::wcin, paramValue);
                        try
                        {
                            select.addParam(paramName, std::stod(paramValue));
                        }
                        catch (const std::exception&)
                        {
                            select.addParam(paramName, paramValue);
                        }
                    }
                    printf("\n");
                }

                std::wstring sql = context.generateSql(select);
                writeLineToFile(query);
                writeLineToFile(L"");
                writeLineToFile(L"===");
                writeLineToFile(L"");
                writeLineToFile(sql);
                writeLineToFile(L"");
                writeLineToFile(L"===");
                writeLineToFile(L"");

                unsigned resultCount = 0;
                auto reader = context.execQuery(select);
                while (reader->read())
                {
                    for (unsigned idx = 0; idx < reader->getColCount(); ++idx)
                        writeLine(reader->getColName(idx) + L": " + reader->getString(idx));
                    writeLine(L"===");
                    ++resultCount;
                }

                writeLine(L"Results: " + std::to_wstring(resultCount));
            }
            catch (const fourdberr& exp)
            {
                printf("ERROR: %s\n", exp.what());
            }
        } // loop forever
    }
    catch (const std::exception& exp)
    {
        printf("ERROR: %s\n", exp.what());
        return 1;
    }
    return 0;
}
