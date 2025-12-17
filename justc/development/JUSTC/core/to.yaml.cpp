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

#include "to.yaml.h"
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

std::string YamlSerializer::escapeYamlString(const std::string& str) {
    if (str.empty()) return "\"\"";
    bool needsQuoting = false;
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c == '\"' || c == '\\' || c == '\0' || c == '\n' || c == '\r' || c == '\t' ||
            c ==  '[' || c ==  ']' || c ==  '{' || c ==  '}' || c ==  ',' || c ==  ':' ||
            c ==  '#' || c ==  '&' || c ==  '*' || c ==  '!' || c ==  '|' || c ==  '>' ||
            c == '\'' || c ==  '%' || c ==  '@' || c ==  '`'
        ) {
            needsQuoting = true;
            break;
        }

        if (i > 100000) {
            #ifdef __EMSCRIPTEN__
            EM_ASM({
                console.warn("[JUSTC] (" + $0 + ") string is too long. It will be truncated in the YAML output.");
            }, Parser::getCurrentTimestamp().c_str());
            #else
            std::cout << "JUSTC: Warning: string is too long. It will be truncated in the YAML output." << std::endl;
            #endif
            break;
        }
    }

    if (!needsQuoting) {
        if (str == "true" || str == "false" || str == "null" ||
            str == "yes" || str == "no" || str == "on" || str == "off" ||
            (str.length() > 0 && isdigit(str[0]))) {
            needsQuoting = true;
        }
    }

    if (!needsQuoting) {
        return str;
    }

    std::stringstream ss;
    ss << "\"";
    for (char c : str) {
        switch (c) {
            case '\"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            case '\0': ss << "\\0"; break;
            default: ss << c; break;
        }
    }
    ss << "\"";
    return ss.str();
}

std::string YamlSerializer::valueToYaml(const Value& value) {
    switch (value.type) {
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
            return Utility::numberValue2string(value);
        case DataType::STRING:
        case DataType::LINK:
        case DataType::PATH:
        case DataType::VARIABLE:
            return escapeYamlString(value.string_value);
        case DataType::BOOLEAN:
            return value.boolean_value ? "true" : "false";
        case DataType::NULL_TYPE:
            return "null";
        case DataType::NOT_A_NUMBER:
            return ".NaN";
        case DataType::INFINITE:
            return ".inf";
        default:
            return "invalid";
    }
}

std::string YamlSerializer::tokensToYaml(const std::vector<ParserToken>& tokens) {
    std::stringstream yaml;
    yaml << "tokens:\n";

    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& token = tokens[i];
        yaml << "  - type: " << escapeYamlString(token.type) << "\n";
        yaml << "    value: " << escapeYamlString(token.value) << "\n";
        yaml << "    start: " << token.start << "\n";
    }

    return yaml.str();
}

std::string YamlSerializer::serialize(const ParseResult& result) {
    std::stringstream yaml;

    #ifdef __EMSCRIPTEN__

    if (!result.error.empty()) {
        return "{\"error\":\"" + JsonSerializer::escapeJsonString(result.error) + "\"}";
    } else {
        std::stringstream valuesYaml;
        if (result.array) {
            for (const auto& pair : result.returnValues) {
                valuesYaml << "- " << valueToYaml(pair.second) << "\n";
            }
        } else {
            valuesYaml << "---\n";
            for (const auto& pair : result.returnValues) {
                valuesYaml << escapeYamlString(pair.first) << ": " << valueToYaml(pair.second) << "\n";
            }
        }

        std::stringstream json;
        json << "{";

        json << "\"type\":\"yaml\",\"return\":\"" << JsonSerializer::escapeJsonString(valuesYaml.str()) << "\",";
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

    if (result.array) {
        for (const auto& pair : result.returnValues) {
            yaml << "- " << valueToYaml(pair.second) << "\n";
        }
    } else {
        yaml << "---\n";
        for (const auto& pair : result.returnValues) {
            yaml << escapeYamlString(pair.first) << ": " << valueToYaml(pair.second) << "\n";
        }
    }

    return yaml.str();

    #endif
}

std::string YamlSerializer::serialize(const std::vector<ParserToken>& tokens, const std::string& input) {
    std::stringstream yaml;
    yaml << "---\n";
    yaml << "version: " << escapeYamlString(JUSTC_VERSION) << "\n";
    yaml << "input: " << escapeYamlString(input) << "\n";
    yaml << tokensToYaml(tokens);
    return yaml.str();
}
