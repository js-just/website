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

#include "from.justo.hpp"
#include <cstdlib>
#include <string>
#include <sstream>

namespace JUSTO_Parser {
bool parseJUSTOTokens(const char* tokensJUSTO, std::vector<ParserToken>& parserTokens, std::string& input) {
    if (!tokensJUSTO) return false;

    std::string justoStr(tokensJUSTO);

    size_t tokensStart = justoStr.find("\"tokens\":a[");
    if (tokensStart == std::string::npos) return false;
    size_t inputStart = justoStr.find("input:\"");
    if (inputStart == std::string::npos) return false;
    input += justoStr.substr(inputStart, tokensStart - 2 - inputStart);

    size_t pos = tokensStart + 11;

    while (pos < justoStr.length()) {
        if (justoStr[pos] == 'o') {
            pos++;
            if (justoStr[pos] != '{') break;

            ParserToken token;
            size_t tokenEnd = justoStr.find('}', pos);
            if (tokenEnd == std::string::npos) break;

            std::string tokenStr = justoStr.substr(pos, tokenEnd - pos + 1);

            size_t typeStart = tokenStr.find("type:\"");
            if (typeStart != std::string::npos) {
                typeStart += 8;
                size_t typeEnd = tokenStr.find('"', typeStart);
                while (justoStr[typeEnd - 1] == '\\') {
                    typeEnd = tokenStr.find('"', typeEnd);
                }
                if (typeEnd != std::string::npos) {
                    token.type = tokenStr.substr(typeStart, typeEnd - typeStart);
                }
            }

            size_t valueStart = tokenStr.find("value:\"");
            if (valueStart != std::string::npos) {
                valueStart += 9;
                size_t valueEnd = tokenStr.find('"', valueStart);
                while (justoStr[valueEnd - 1] == '\\') {
                    valueEnd = tokenStr.find('"', valueEnd);
                }
                if (valueEnd != std::string::npos) {
                    token.value = tokenStr.substr(valueStart, valueEnd - valueStart);
                }
            }

            size_t startStart = tokenStr.find("start:n");
            if (startStart != std::string::npos) {
                startStart += 8;
                size_t startEnd = tokenStr.find_first_of(";}", startStart);
                if (startEnd != std::string::npos) {
                    std::string startStr = tokenStr.substr(startStart, startEnd - startStart);
                    token.start = std::atoi(startStr.c_str());
                    tokenEnd = startEnd;
                }
            }

            if (!token.type.empty()) {
                parserTokens.push_back(token);
            }

            pos = tokenEnd + 1;
        } else if (justoStr[pos] == ']') {
            break;
        } else {
            pos++;
        }
    }

    return !parserTokens.empty();
}
}
