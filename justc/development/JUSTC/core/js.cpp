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

#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "json.hpp"
#include "fetch.h"
#include "version.h"
#include <tuple>

template<typename... Args>
std::string outputString(std::string mode, Args... args) {
    if (mode == "xml") {
        return XmlSerializer::serialize(args...);
    } else if (mode == "yaml") {
        return YamlSerializer::serialize(args...);
    } else {
        return JsonSerializer::serialize(args...);
    }
}

extern "C" {

char* lexer(const char* input, const char* outputMode) {
    if (input == nullptr) return nullptr;
    std::string mode(outputMode == nullptr ? "json" : outputMode);

    try {
        auto parsed = Lexer::parse(input, true);
        std::string json = outputString(mode, parsed.second, parsed.first);
        return strdup(json.c_str());

    } catch (const std::exception& e) {
        std::string error = "{\"error\":\"" + JsonSerializer::escapeJsonString(std::string(e.what())) + "\",\"lexer\":true}";
        return strdup(error.c_str());
    }
}

char* parser(const char* tokensJson, const char* outputMode) {
    if (tokensJson == nullptr) return nullptr;
    std::string mode(outputMode == nullptr ? "json" : outputMode);

    try {
        std::vector<ParserToken> parserTokens;
        std::string input = "";

        if (JsonParser::parseJsonTokens(tokensJson, parserTokens, input)) {
            ParseResult result = Parser::parseTokens(parserTokens, false, false, input);
            std::string json = outputString(mode, result);
            return strdup(json.c_str());
        } else {
            std::string error = "{\"error\":\"Failed to parse tokens JSON\"}";
            return strdup(error.c_str());
        }

    } catch (const std::exception& e) {
        std::string error = "{\"error\":\"" + JsonSerializer::escapeJsonString(std::string(e.what())) + "\",\"parser\":true}";
        return strdup(error.c_str());
    }
}

char* parse(const char* input, const bool execute, const bool runAsync, const char* outputMode) {
    if (input == nullptr) return nullptr;
    std::string mode(outputMode == nullptr ? "json" : outputMode);

    try {
        auto lexerResult = Lexer::parse(input);
        ParseResult result = Parser::parseTokens(lexerResult.second, execute, runAsync, input);
        std::string json = outputString(mode, result);
        return strdup(json.c_str());

    } catch (const std::exception& e) {
        std::string error = "{\"error\":\"" + JsonSerializer::escapeJsonString(std::string(e.what())) + "\"}";
        return strdup(error.c_str());
    }
}

void free_string(char* str) {
    if (str != nullptr) {
        free(str);
    }
}

char* version() {
    return strdup(JUSTC_VERSION.c_str());
}

}
