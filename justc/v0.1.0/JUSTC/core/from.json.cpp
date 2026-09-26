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

#include "from.json.hpp"
#include <cstdlib>
#include <string>
#include <sstream>
#ifndef __EMSCRIPTEN__
#include <nlohmann/json.hpp>
#endif

namespace JsonParser {

bool parseJsonTokens(const char* tokensJson, std::vector<ParserToken>& parserTokens, std::string& input) {
    if (!tokensJson) return false;

    std::string jsonStr(tokensJson);

    size_t tokensStart = jsonStr.find("\"tokens\":[");
    if (tokensStart == std::string::npos) return false;
    size_t inputStart = jsonStr.find("\"input\":\"");
    if (inputStart == std::string::npos) return false;
    input += jsonStr.substr(inputStart, tokensStart - 2 - inputStart);

    size_t pos = tokensStart + 10;

    while (pos < jsonStr.length()) {
        if (jsonStr[pos] == '{') {
            ParserToken token;
            size_t tokenEnd = jsonStr.find('}', pos);
            if (tokenEnd == std::string::npos) break;

            std::string tokenStr = jsonStr.substr(pos, tokenEnd - pos + 1);

            size_t typeStart = tokenStr.find("\"type\":\"");
            if (typeStart != std::string::npos) {
                typeStart += 8;
                size_t typeEnd = tokenStr.find('"', typeStart);
                if (typeEnd != std::string::npos) {
                    token.type = tokenStr.substr(typeStart, typeEnd - typeStart);
                }
            }

            size_t valueStart = tokenStr.find("\"value\":\"");
            if (valueStart != std::string::npos) {
                valueStart += 9;
                size_t valueEnd = tokenStr.find('"', valueStart);
                if (valueEnd != std::string::npos) {
                    token.value = tokenStr.substr(valueStart, valueEnd - valueStart);
                }
            }

            size_t startStart = tokenStr.find("\"start\":");
            if (startStart != std::string::npos) {
                startStart += 8;
                size_t startEnd = tokenStr.find_first_of(",}", startStart);
                if (startEnd != std::string::npos) {
                    std::string startStr = tokenStr.substr(startStart, startEnd - startStart);
                    token.start = std::atoi(startStr.c_str());
                }
            }

            if (!token.type.empty()) {
                parserTokens.push_back(token);
            }

            pos = tokenEnd + 1;
        } else if (jsonStr[pos] == ']') {
            break;
        } else {
            pos++;
        }
    }

    return !parserTokens.empty();
}

#ifndef __EMSCRIPTEN__
std::string stringify(const std::string& input) {
    nlohmann::json j = nlohmann::json::parse(input);
    std::ostringstream output;

    bool first = true;
    for (auto& item : j.items()) {
        std::string name = item.key();
        auto& value = item.value();
        if (!first) {
            output << ",";
        }
        first = false;

        output << name << "=";
        if (value.is_string()) {
            output << "\"" << value.get<std::string>() << "\"";
        } else if (value.is_boolean()) {
            output << (value.get<bool>() ? "y" : "n");
        } else if (value.is_null()) {
            output << "";
        } else {
            output << value;
        }
    }
    output << ".";
    return output.str();
}
#endif

}
