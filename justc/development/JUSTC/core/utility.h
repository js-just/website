/*

MIT License

Copyright (c) 2025 JustStudio. <https://juststudio.is-a.dev/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef UTILITY_H
#define UTILITY_H

#include "parser.h"
#include <string>
#include <codecvt>
#include <locale>

class Utility {
public:
    static std::string numberValue2string(const Value& value);
    static std::string value2string(const Value& value);
    static std::string double2hexString(const double d);
    static std::string double2octString(const double d);
    static std::string double2binString(const double d);
    static bool checkNumbers(const Value& left, const Value& right);
    static std::pair<size_t, size_t> pos(const size_t& pos, const std::string& script);
    static std::string position(const size_t& pos_, const std::string& script);
    static DataType typeDeclaration2dataType(const std::string& typeDeclaration, const std::string& position);
    static Value convert(const Value value, const DataType type);
    static Value ParseResult2Value(const ParseResult parseresult);
    static bool isGitHubActions();
    static std::unordered_map<std::string, std::string> ParseHeaders(const std::string& headers);
    static std::string defaultHTTPAccept;
};
class UnicodeUtility {
public:
    static bool isValidUTF8(const std::string& str) {
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring wstr = converter.from_bytes(str);
            return true;
        } catch (...) {
            return false;
        }
    }

    static std::string toUTF8(const std::wstring& wstr) {
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.to_bytes(wstr);
        } catch (...) {
            return "";
        }
    }

    static std::wstring fromUTF8(const std::string& str) {
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.from_bytes(str);
        } catch (...) {
            return L"";
        }
    }

    static std::string toLowerUTF8(const std::string& str) {
        try {
            std::wstring wstr = fromUTF8(str);
            std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::towlower);
            return toUTF8(wstr);
        } catch (...) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }
    }

    static bool isUnicodeIdentifier(const std::string& str) {
        if (str.empty()) return false;

        try {
            std::wstring wstr = fromUTF8(str);

            wchar_t first = wstr[0];
            if (first != L'_' && !iswalpha(first)) {
                return false;
            }

            for (wchar_t c : wstr) {
                if (c != L'_' && !iswalnum(c)) {
                    return false;
                }
            }

            return true;
        } catch (...) {
            return false;
        }
    }
};

#endif
