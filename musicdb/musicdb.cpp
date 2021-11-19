#include "ctxt.h"
#pragma comment(lib, "4db")

#include <iostream>

#include <sstream>
#include <fstream>
#include <codecvt>

std::unordered_map<std::wstring, std::wstring> fieldNames
{
    { L"Track ID", L"trackId" },
    { L"Size", L"sizeBytes" },
    { L"Total Time", L"timeMs" },
    { L"Disc Number", L"discNumber" },
    { L"Disc Count", L"discCount" },
    { L"Track Number", L"trackNumber" },
    { L"Track Count", L"trackCount" },
    { L"Year", L"year" },
    { L"Date Modified", L"dateModified" },
    { L"Date Added", L"dateAdded" },
    { L"Bit Rate", L"bitrateKbps" },
    { L"Sample Rate", L"sampleRateHz" },
    { L"Name", L"title" },
    { L"Artist", L"artist" },
    { L"Album Artist", L"albumArtist" },
    { L"Album", L"album" },
    { L"Genre", L"genre" },
    { L"Kind", L"format" }
};

void cleanXmlValue(std::wstring& str)
{
    fourdb::replace(str, L"&lt;", L"<");
    fourdb::replace(str, L"&gt;", L">");
    fourdb::replace(str, L"&quot;", L"\"");
    fourdb::replace(str, L"&apos;", L"\'");
    fourdb::replace(str, L"&amp;", L"&");
}

void load(const std::string& xmlFilePath, fourdb::ctxt& context)
{
    std::wifstream xmlFileStream(xmlFilePath.c_str());
    if (!xmlFileStream)
        throw std::runtime_error("ERROR: Input file not found: " + xmlFilePath);

    xmlFileStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

    context.drop(L"tracks");

    bool inDict = false;
    fourdb::paramap dict;
    int addedCount = 0;
    printf("Loading data from input file...\n");
    while (xmlFileStream)
    {
        std::wstring line;
        std::getline(xmlFileStream, line);
        const wchar_t* tag = wcsstr(line.c_str(), L"<");
        if (tag == nullptr)
            continue;

        if (_wcsicmp(tag, L"<dict>") == 0)
        {
            inDict = true;
            dict.clear();
        }
        else if (_wcsicmp(tag, L"</dict>") == 0)
        {
            inDict = false;

            std::wstring key; // just needs to be unique
            for (const auto& kvp : dict)
            {
                const auto& snum = kvp.second;
                if (snum.isStr())
                    key += snum.str();
                else
                    key += fourdb::num2str(snum.num());
                key += '|';
            }

            context.define(L"tracks", key, dict);

            ++addedCount;
            if ((addedCount % 100) == 0)
                printf("%d ", addedCount);
        }
        else if (inDict && wcsncmp(tag, L"<key>", 5) == 0)
        {
            const wchar_t* closingKey = wcsstr(tag, L"</key>");
            if (closingKey == nullptr)
            {
                printf("Unclosed <key>: %S\n", line.c_str());
                continue;
            }

            std::wstring key(tag + 5, closingKey);
            cleanXmlValue(key);

            const auto& fieldNameIt = fieldNames.find(key);
            if (fieldNameIt == fieldNames.end())
                continue;
            const auto& fieldName = fieldNameIt->second;

            // It's either integer, string, or date
            const wchar_t* valueTag = nullptr;

            valueTag = wcsstr(closingKey, L"<integer>");
            if (valueTag != nullptr)
            {
                const wchar_t* closingValue = wcsstr(tag, L"</integer>");
                if (closingValue == nullptr)
                {
#ifdef _DEBUG
                    printf("Unclosed <integer>: %S\n", line.c_str());
#endif
                    continue;
                }

                std::wstring valueStr(valueTag + 9, closingValue);
                double valueNum = _wtof(valueStr.c_str());
                dict.insert({ fieldName, valueNum });
                continue;
            }

            valueTag = wcsstr(closingKey, L"<string>");
            if (valueTag != nullptr)
            {
                const wchar_t* closingValue = wcsstr(tag, L"</string>");
                if (closingValue == nullptr)
                {
#ifdef _DEBUG
                    printf("Unclosed <string>: %S\n", line.c_str());
#endif
                    continue;
                }

                std::wstring valueStr(valueTag + 8, closingValue);
                cleanXmlValue(valueStr);
                dict.insert({ fieldName, valueStr });
            }

            valueTag = wcsstr(closingKey, L"<date>");
            if (valueTag != nullptr)
            {
                const wchar_t* closingValue = wcsstr(tag, L"</date>");
                if (closingValue == nullptr)
                {
#ifdef _DEBUG
                    printf("Unclosed <date>: %S\n", line.c_str());
#endif
                    continue;
                }

                std::wstring valueStr(valueTag + 6, closingValue);
                cleanXmlValue(valueStr);
                dict.insert({ fieldName, valueStr });
            }
        }
    }

    printf("\nRecords added: %d\n", addedCount);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: musicdb <path to iTunes XML file>\n");
        return 0;
    }

    printf("Opening database file music.db...\n");
    fourdb::ctxt context("music.db");

    const char* xmlFilePath = argv[1];
    printf("Input file: %s\n", xmlFilePath);

    int64_t count = context.execScalarInt64(fourdb::sql::parse(L"SELECT count FROM tracks")).value_or(0);
    if (count <= 0)
        load(xmlFilePath, context);
    else
        printf("Records in DB: %d\n", static_cast<int>(count));

    printf("Database Fields:\n");
    std::vector<std::wstring> sortedFieldNames;
    for (const auto& fieldNameIt : fieldNames)
        sortedFieldNames.push_back(fieldNameIt.second);
    std::sort(sortedFieldNames.begin(), sortedFieldNames.end());
    for (const auto& fieldName : sortedFieldNames)
        printf("%S\n", fieldName.c_str());
    printf("\n");

    printf("Enter \"load\" or \"reload\" to rebuild the DB from the input file\n");
    printf("Otherwise, enter a SQL query to get track info from the DB\n");
    printf("The SQL has to look like \"SELECT field1, field2 FROM tracks WHERE field1 == @param1 ORDER BY field2 LIMIT 100\"\n");
    while (true)
    {
        try
        {
            printf("> ");
            std::wstring line;
            std::getline(std::wcin, line);

            if (line.empty())
                continue;

            if (line == L"exit" || line == L"quit")
                break;

            if (line == L"load" || line == L"reload")
            {
                load(xmlFilePath, context);
                continue;
            }

            auto select = fourdb::sql::parse(line);
            auto paramNames = fourdb::extractParamNames(line);
            if (!paramNames.empty())
            {
                printf("Enter values for the parameters in your query;"
                       " put # in front of numeric values:\n");
                for (const auto& paramName : paramNames)
                {
                    printf("%S: ", paramName.c_str());
                    std::getline(std::wcin, line);
                    if (!line.empty() && line[0] == '#')
                        select.addParam(paramName, _wtof(line.substr(1).c_str()));
                    else
                        select.addParam(paramName, line);
                }
            }

            auto reader = context.execQuery(select);
            auto colCount = reader->getColCount();
            
            std::vector<std::vector<std::wstring>> matrix;
            std::unordered_set<std::wstring> seenRowSummaries;

            while (reader->read())
            {
                std::vector<std::wstring> newRow;
                for (unsigned col = 0; col < colCount; ++col)
                    newRow.push_back(reader->getString(col));

                std::wstring newRowSummary = fourdb::join(newRow, L"\v");
                if (seenRowSummaries.find(newRowSummary) != seenRowSummaries.end())
                    continue;

                seenRowSummaries.insert(newRowSummary);
                matrix.push_back(newRow);
            }

            printf("Results: %u\n", static_cast<unsigned>(matrix.size()));
            if (matrix.empty())
                continue;

            std::vector<std::wstring> headerRow;
            for (unsigned col = 0; col < colCount; ++col)
                headerRow.push_back(reader->getColName(col));
            matrix.insert(matrix.begin(), headerRow);

            std::vector<unsigned> columnWidths;
            for (const auto& header : headerRow)
                columnWidths.push_back(static_cast<unsigned>(header.size()));
            for (const auto& row : matrix)
            {
                for (size_t cellidx = 0; cellidx < columnWidths.size(); ++cellidx)
                    columnWidths[cellidx] = std::max(columnWidths[cellidx], row[cellidx].size());
            }

            for (size_t cellidx = 0; cellidx < columnWidths.size(); ++cellidx)
            {
                const auto& header = headerRow[cellidx];
                auto headerWidth = columnWidths[cellidx];
                printf("%S", header.c_str());
                for (size_t s = header.size(); s <= headerWidth; ++s)
                    printf(" ");
            }
            printf("\n");

            for (size_t cellIdx = 0; cellIdx < columnWidths.size(); ++cellIdx)
            {
                auto headerWidth = columnWidths[cellIdx];
                for (size_t s = 0; s < headerWidth; ++s)
                    printf("-");
                printf(" ");
            }
            printf("\n");

            for (size_t rowIdx = 1; rowIdx < matrix.size(); ++rowIdx)
            {
                const auto& row = matrix[rowIdx];
                for (size_t cellIdx = 0; cellIdx < columnWidths.size(); ++cellIdx)
                {
                    const auto& value = row[cellIdx];
                    auto headerWidth = columnWidths[cellIdx];
                    printf("%S", value.c_str());
                    for (size_t s = value.size(); s <= headerWidth; ++s)
                        printf(" ");
                }
                printf("\n");
            }
        }
        catch (const std::exception& exp)
        {
            printf("ERROR: %s\n", exp.what());
        }
    }
    return 0;
}
