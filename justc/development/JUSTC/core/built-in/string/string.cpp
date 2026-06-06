/*

MIT License

Copyright (c) 2025-2026 JustStudio. <https://juststudio.is-a.dev/>

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

#include "string.hpp"
#include <algorithm>

std::string String::Trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string String::Repeat(const std::string& str, size_t times) {
    std::string result;
    result.reserve(str.length() * times);
    for (size_t i = 0; i < times; ++i) {
        result += str;
    }
    return result;
}

std::vector<std::string_view> String::Split(std::string_view str, std::string_view delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    size_t end = str.find(delim);

    while (end != std::string_view::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    result.push_back(str.substr(start));
    return result;
}

bool String::StartsWith(std::string_view str, std::string_view prefix) {
    return str.substr(0, prefix.size()) == prefix;
}

bool String::EndsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}
