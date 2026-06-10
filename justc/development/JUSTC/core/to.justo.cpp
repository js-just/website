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

#include "to.justo.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "parser.h"
#include <cmath>
#include "utility.h"
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "to.json.h"
#endif

std::string JUSTOSerializer::escapeJUSTOString(const std::string& str) {
    if (str.empty()) return "\"\"";
    std::stringstream ss;
    ss << "\"";
    for (char c : str) {
        switch (c) {
            case '\"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\'': ss << "\\\'"; break;
            default: ss << c; break;
        }
    }
    ss << "\"";
    return ss.str();
}

std::string JUSTOSerializer::valueToJUSTO(const Value& value) {
    switch (value.type) {
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
            return "n" + Utility::numberValue2string(value);
        case DataType::STRING:
        case DataType::LINK:
        case DataType::PATH:
        case DataType::VARIABLE:
            return escapeJUSTOString(value.string_value);
        case DataType::BOOLEAN:
            return value.boolean_value ? "1" : "0";
        case DataType::NULL_TYPE:
            return "";
        case DataType::NOT_A_NUMBER:
            return "'nan'";
        case DataType::INFINITE:
            return "'inf'";
        default:
            return "\"invalid\"";
    }
}

std::string JUSTOSerializer::tokensToJUSTO(const std::vector<ParserToken>& tokens) {
    std::stringstream justo;
    justo << "a[";

    bool first = true;
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& token = tokens[i];
        if (!first) justo << ",";
        first = false;
        justo << "o{type:" << escapeJUSTOString(token.type) << ";";
        justo << "value:" << escapeJUSTOString(token.value) << ";";
        justo << "start:n" << token.start << "}";
    }

    justo << "]";

    return justo.str();
}

std::string JUSTOSerializer::serialize(const ParseResult& result) {
    std::stringstream justo;

    bool first = true;
    bool doObj = false;

    if (result.array) {
        std::vector<int> indices;
        bool isArray = true;
        for (const auto& pair : result.returnValues) {
            try {
                int idx = std::stoi(pair.first);
                indices.push_back(idx);
            } catch (...) {
                isArray = false;
                break;
            }
        }

        if (isArray) {
            justo << "a[";
            std::sort(indices.begin(), indices.end());
            for (size_t i = 0; i < indices.size(); i++) {
                if (i > 0) justo << ",";
                std::string key = std::to_string(indices[i]);
                justo << valueToJUSTO(result.returnValues.at(key));
            }
            justo << "]";
        } else {
            doObj = true;
        }
    } else {
        doObj = true;
    }

    if (doObj) {
        justo << "o{";
        for (const auto& pair : result.returnValues) {
            if (!first) justo << ",";
            first = false;
            justo << "\"" << escapeJUSTOString(pair.first) << "\":" << valueToJUSTO(pair.second);
        }
        justo << "}";
    }

    #ifdef __EMSCRIPTEN__

    if (!result.error.empty()) {
        return "{\"error\":\"" + JsonSerializer::escapeJsonString(result.error) + "\"}";
    } else {
        std::stringstream json;
        json << "{";

        json << "\"type\":\"justo\",\"return\":\"" << JsonSerializer::escapeJsonString(justo.str()) << "\",";
        json << "\"logs\":" << JsonSerializer::serialize(result.logs) << ",";

        // logfile object
        json << "\"logfile\":{";
        json << "\"file\":\"" << JsonSerializer::escapeJsonString(result.logFilePath) << "\",";
        json << "\"logs\":\"" << JsonSerializer::escapeJsonString(result.logFileContent) << "\"";
        json << "},";

        // import logs array
        json << "\"imported\":";
        json << JsonSerializer::serialize(result.importLogs);

        json << "}";
        return json.str();
    }

    #else

    return justo.str();

    #endif
}

std::string JUSTOSerializer::serialize(const std::vector<ParserToken>& tokens, const std::string& input) {
    std::stringstream justo;
    justo << "o{version:" << escapeJUSTOString(JUSTC_VERSION) << ";";
    justo << "input:" << escapeJUSTOString(input) << ";";
    justo << "\"tokens\":" << tokensToJUSTO(tokens) << "}";
    return justo.str();
}
