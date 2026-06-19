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

#include "utility.h"
#include "parser.h"
#include <cmath>
#include <string>
#include <iomanip>
#include <bitset>
#include <cstring>
#include <sstream>
#include <cstddef>
#include <unordered_map>
#include <iostream>
#include <cstdint>
#ifdef __EMSCRIPTEN__
#include "utility.emscripten.h"
#endif

std::string Utility::numberValue2string(const Value& value) {
    if (value.number_value == std::floor(value.number_value)) {
        return std::to_string(static_cast<long long>(value.number_value));
    } else {
        return std::to_string(value.number_value);
    }
}

std::string Utility::value2string(const Value& value) {
    switch (value.type) {
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
            return numberValue2string(value);
        case DataType::JUSTC_OBJECT:
            if (value.name == "HTTP.Responce") {
                auto text = value.object_value.find("text");
                if (text != value.object_value.end()) return value2string(text->second);
                else return value.toString();
            } else return value.toString();
        default:
            return value.toString();
    }
}

std::string Utility::double2hexString(const double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(double));
    std::stringstream ss;
    ss << std::hex << bits;
    return ss.str();
}

std::string Utility::double2octString(const double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(double));
    std::stringstream ss;
    ss << std::oct << bits;
    return ss.str();
}

std::string Utility::double2binString(const double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(double));
    return std::bitset<64>(bits).to_string();
}

bool Utility::checkNumbers(const Value& left, const Value& right) {
    return ((
        left.type == DataType::NUMBER ||
        left.type == DataType::HEXADECIMAL ||
        left.type == DataType::BINARY ||
        left.type == DataType::OCTAL
    ) && (
        right.type == DataType::NUMBER ||
        right.type == DataType::HEXADECIMAL ||
        right.type == DataType::BINARY ||
        right.type == DataType::OCTAL
    ));
}

std::pair<size_t, size_t> Utility::pos(const size_t& pos, const std::string& script) {
    if (script.empty() || pos >= script.length()) {
        return {1, 1};
    }

    size_t line = 1;
    size_t column = 1;
    size_t current_pos = 0;

    while (current_pos < pos && current_pos < script.length()) {
        char current_char = script[current_pos];

        if (current_char == '\n') {                                                     //      \n
            line++;
            column = 1;
            current_pos++;
        } else if (current_char == '\r') {
            if (current_pos + 1 < script.length() && script[current_pos + 1] == '\n') { //      \r\n
                line++;
                column = 1;
                current_pos += 2;
            } else {                                                                    //      \r
                line++;
                column = 1;
                current_pos++;
            }
        } else {
            column++;
            current_pos++;
        }
    }

    return {line, column};
}

std::string Utility::position(const size_t& pos_, const std::string& script) {
    std::pair<size_t, size_t> position = pos(pos_, script);
    size_t line = position.first;
    size_t column = position.second;
    return "line " + std::to_string(line) + ", column " + std::to_string(column);
}

DataType Utility::typeDeclaration2dataType(const std::string& typeDeclaration, const std::string& position) {
    static const std::unordered_map<std::string, DataType> typeMap = {
        { "number",      DataType::NUMBER       },     { "num",  DataType::NUMBER       },
        { "string",      DataType::STRING       },     { "str",  DataType::STRING       },
        { "boolean",     DataType::BOOLEAN      },     { "bool", DataType::BOOLEAN      },
        { "null",        DataType::NULL_TYPE    },     { "nil",  DataType::NULL_TYPE    },
        { "link",        DataType::LINK         },
        { "path",        DataType::PATH         },
        { "binary",      DataType::BINARY       },     { "bin",  DataType::BINARY       },
        { "octal",       DataType::OCTAL        },     { "oct",  DataType::OCTAL        },
        { "hexadecimal", DataType::HEXADECIMAL  },     { "hex",  DataType::HEXADECIMAL  },
        { "object",      DataType::JUSTC_OBJECT },     { "obj",  DataType::JUSTC_OBJECT },
        { "json",        DataType::JSON_OBJECT  },
        { "array",       DataType::JSON_ARRAY   },
        { "nan",         DataType::NOT_A_NUMBER },
        { "infinity",    DataType::INFINITE     },     { "inf",  DataType::INFINITE     },
        { "data",        DataType::BINARY_DATA  },
        { "auto",        DataType::UNKNOWN      },
    };

    auto it = typeMap.find(typeDeclaration);
    if (it != typeMap.end()) {
        return it->second;
    }

    throw std::runtime_error("Invalid type declaration \"" + typeDeclaration + "\" at " + position + ".");
}

Value Utility::convert(const Value value, const DataType type) {
    Value result = value;
    result.type = type;
    switch (type) {
        case DataType::NUMBER:
            break;
        case DataType::BINARY:
            result.name = double2binString(value.number_value);
            break;
        case DataType::HEXADECIMAL:
            result.name = double2hexString(value.number_value);
            break;
        case DataType::OCTAL:
            result.name = double2octString(value.number_value);
            break;
        default: // warning: 15 enumeration values not handled in switch: 'UNKNOWN', 'JUSTC_OBJECT', 'STRING'... [-Wswitch]
            throw std::runtime_error("JUSTC/core/utility.cpp error: Incorrect usage.");
    }
    return result;
}

Value Utility::ParseResult2Value(const ParseResult parseresult) {
    Value result;
    result.type = DataType::JUSTC_OBJECT;
    result.object_value = parseresult.returnValues;
    result.name = "(Object)";
    return result;
}

bool Utility::isGitHubActions() {
    const char* githubActions = std::getenv("GITHUB_ACTIONS");
    return (githubActions && std::string(githubActions) == "true");
}

std::unordered_map<std::string, std::string> Utility::ParseHeaders(const std::string& headers) {
    std::unordered_map<std::string, std::string> output;
    std::istringstream lines_stream(headers);
    std::string line;
    while (std::getline(lines_stream, line, '\n')) {
        std::istringstream pair_stream(line);
        std::string key;
        std::string value;
        if (std::getline(pair_stream, key, ':')) {
            if (std::getline(pair_stream, value)) {
                output[key] = value;
            }
        }
    }
    return output;
}

std::string Utility::defaultHTTPAccept = "text/*, application/x-justc, application/json, application/lua, application/hocon, application/xml, application/yaml, */*";

void Utility::Warn(const std::string& warning) {
    #ifdef __EMSCRIPTEN__
    console_warn(Parser::getCurrentTimestamp().c_str(), warning.c_str());
    #else
    if (isGitHubActions()) {
        std::cout << "::warning::" + warning << std::endl;
    } else {
        std::cout << "JUSTC: Warning: " + warning << std::endl;
    }
    #endif
}

std::string Utility::escapeJUSTCString(const std::string& str) {
    return str; // TODO: 1. escape sequences in JUSTC; 2. this function
}

std::string Utility::_stringifyValue(const Value& value, int indentLevel) {
    std::string indent(indentLevel * 2, ' ');
    std::string nextIndent((indentLevel + 1) * 2, ' ');

    switch (value.type) {
        case DataType::NUMBER:
            return numberValue2string(value);

        case DataType::HEXADECIMAL:
            return "0x" + double2hexString(value.number_value);

        case DataType::BINARY:
            return "0b" + double2binString(value.number_value);

        case DataType::OCTAL:
            return "0o" + double2octString(value.number_value);

        case DataType::STRING:
            return "\"" + escapeJUSTCString(value.string_value) + "\"";

        case DataType::LINK:
            return "<" + escapeJUSTCString(value.string_value) + ">";

        case DataType::PATH:
            return value.string_value;

        case DataType::BOOLEAN:
            return value.boolean_value ? "y" : "n";

        case DataType::NULL_TYPE:
            return "";

        case DataType::NOT_A_NUMBER:
            return "NaN";

        case DataType::INFINITE:
            return "Infinity";

        case DataType::JUSTC_OBJECT:
        case DataType::JSON_OBJECT: {
            std::string result = "{" + nextIndent;
            bool first = true;

            const auto& props = value.properties;
            for (const auto& [key, val] : props) {
                if (!first) result += "," + nextIndent;
                first = false;
                result += "\"" + escapeJUSTCString(key) + "\":" + _stringifyValue(val, indentLevel + 1);
            }

            if (value.object_context && !value.object_context->variables.empty()) {
                for (const auto& [key, val] : value.object_context->variables) {
                    if (props.find(key) != props.end()) continue;

                    if (!first) result += "," + nextIndent;
                    first = false;
                    result += "\"" + escapeJUSTCString(key) + "\":" + _stringifyValue(val, indentLevel + 1);
                }
            }

            result += indent + "}";
            return result;
        }

        case DataType::JSON_ARRAY: {
            std::string result = "[" + nextIndent;
            for (size_t i = 0; i < value.array_elements.size(); i++) {
                if (i > 0) result += "," + nextIndent;
                result += _stringifyValue(value.array_elements[i], indentLevel + 1);
            }
            result += indent + "]";
            return result;
        }

        case DataType::VARIABLE:
            return value.string_value;

        case DataType::FUNCTION: {
            if (value.native) {
                return value.name;
            }

            std::string result;
            if (value.function_info.isIsolated) {
                result = "isolated ";
            }
            result += "function " + value.name + "(";
            for (size_t i = 0; i < value.function_info.paramNames.size(); i++) {
                if (i > 0) result += ", ";
                result += value.function_info.paramNames[i];
            }
            result += "){" + nextIndent;
            result += value.string_value;
            result += indent + "}";
            return result;
        }

        case DataType::BINARY_DATA: {
            std::string binData;
            for (size_t i = 0; i < value.binary_data.size(); i++) {
                binData += std::to_string(static_cast<int>(value.binary_data[i]));
            }
            return "Binary::FromText(\"" + escapeJUSTCString(binData) + "\")";
        }

        default:
            return "nil";
    }
}

std::string Utility::stringifyValue(const Value& value) {
    return "return " + _stringifyValue(value, 0) + " .";
}
