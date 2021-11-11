#include "ctxt.h"
#pragma comment(lib, "4db")

#include <iostream>

#include <sstream>
#include <fstream>
#include <codecvt>

void cleanXmlValue(std::wstring& str)
{
    fourdb::replace(str, L"&lt;", L"<");
    fourdb::replace(str, L"&gt;", L">");
    fourdb::replace(str, L"&quot;", L"\"");
    fourdb::replace(str, L"&apos;", L"\'");
    fourdb::replace(str, L"&amp;", L"&");
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: musicdb <path to iTunes XML file>\n");
        return 0;
    }

    const char* xmlFilePath = argv[1];
    printf("iTunes XML File: %s\n", xmlFilePath);

    fourdb::ctxt context("music.db");

    std::wifstream xmlFileStream(xmlFilePath);
    xmlFileStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

    bool inDict = false;
    fourdb::paramap dict;
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
            /* FORNOW
            std::wstring key; // just needs to be unique
            for (const auto& kvp : dict)
                key += kvp.second.toSqlLiteral();

            context.define(L"tracks", key, dict);
            */
        }
        else if (inDict && wcsncmp(tag, L"<key>", 5) == 0)
        {
            const wchar_t* closingKey = wcsstr(tag, L"</key>");
            if (closingKey == nullptr)
            {
#ifdef _DEBUG
                printf("Unclosed <key>: %S\n", line.c_str());
#endif
                continue;
            }

            std::wstring key(tag + 5, closingKey);
            cleanXmlValue(key);

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
                int valueNum = _wtoi(valueStr.c_str());
                (void)valueNum; // FORNOW
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
                (void)valueStr;
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
                (void)valueStr;
            }
        }
    }
}
