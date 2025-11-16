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

#include "import.hpp"
#include "parser.h"
#include "lexer.h"
#ifndef __EMSCRIPTEN__
#include <fstream>
#endif
#include "fetch.h"
#include <string>
#include <cstring>
#include <sstream>
#include "utility.h"
#include <utility>

std::string Import::ReadFile(const std::string path, const std::string position) {
    #ifndef __EMSCRIPTEN__
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Import error: Unable to read the file \"" + path + "\" at " + position + ".");
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    #else
        return Utility::value2string(Fetch::request(path));
    #endif
}

std::pair<ParseResult, std::string> Import::JUSTC(const std::string path, const std::string position, const bool doExecute, const bool asynchronously, const bool allowJavaScript, const bool imports) {
    std::string File = ReadFile(path, position);
    auto lexerResult = Lexer::parse(File);
    return {Parser::parseTokens(lexerResult.second, doExecute, asynchronously, lexerResult.first, allowJavaScript, false, path, imports ? "module" : "script"), File};
}
