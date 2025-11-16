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

#include "lexer.h"
#include "keywords.h"
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <regex>
#include <string>
#include <cstring>
#include <sstream>
#include "utility.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include "parser.h"
#include "lexer.emscripten.h"
#endif

Lexer::Lexer(const std::string& input, const bool& warn) : input(input), warn(warn), position(0), dollarBefore(false) {
    if (input.empty()) {
        throw std::invalid_argument("Invalid Input.");
    }
    initializeKeywords();
    tokenize();
}

void Lexer::initializeKeywords() {
    keywords = ::keywords;
}

void Lexer::invalidInput() {
    throw std::invalid_argument("Invalid Input.");
}

void Lexer::invalidUsage() {
    throw std::invalid_argument("Invalid Usage.");
}

bool Lexer::isWhitespace(char ch) const {
    return std::isspace(static_cast<unsigned char>(ch));
}

bool Lexer::isLetter(char ch) const {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool Lexer::isDigit(char ch) const {
    return std::isdigit(static_cast<unsigned char>(ch));
}

bool Lexer::isHexDigit(char ch) const {
    return std::isxdigit(static_cast<unsigned char>(ch));
}

bool Lexer::isBase64Char(char ch) const {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '+' || ch == '/' || ch == '=';
}

char Lexer::peek(size_t offset) const {
    if (position + offset < input.length()) {
        return input[position + offset];
    }
    return '\0';
}

// single-line comment: -- comment
void Lexer::readComment() {
    while (position < input.length() && input[position] != '\n') {
        position++;
    }
    if (position < input.length() && input[position] == '\n') {
        position++;
    }
}
// multi-line comment: -{ comment }-
void Lexer::readMultiLineComment() {
    while (position < input.length() && input[position] != '}' && peek() != '-') {
        position++;
    }
    if (position < input.length() && input[position] == '}' && peek() == '-') {
        position += 2;
    }
}

ParserToken Lexer::readString() {
    size_t start = ++position;
    std::string value = "";
    while (position < input.length() &&
           (input[position] != '"' ||
           (input[position] == '"' && position > 0 && input[position - 1] == '\\'))) {
        value += input[position++];
    }
    position++;
    return ParserToken{"string", value, start};
}

ParserToken Lexer::readLink() {
    size_t start = ++position;
    std::string value = "";
    while (position < input.length() && input[position] != '>') {
        value += input[position++];
    }
    position++;
    return ParserToken{"link", value, start};
}

ParserToken Lexer::readNumber() {
    size_t start = position;
    bool point = false;
    while (position < input.length() &&
           (isDigit(input[position]) ||
            std::string(".#&bB").find(input[position]) != std::string::npos ||
            isHexDigit(input[position]) ||
            isBase64Char(input[position]))) {

        char ch = input[position];
        if ((std::isalnum(static_cast<unsigned char>(ch)) || ch == '.' ||
             ch == '#' || ch == '&' || ch == 'b' || ch == 'B') &&
            ((ch == '.' && position + 1 < input.length() &&
              isDigit(input[position + 1]) && !point) || ch != '.')) {

            position++;
            if (ch == '.') {
                point = true;
            }
        } else {
            break;
        }
    }

    std::string numStr = input.substr(start, position - start);
    std::string checkstr = numStr;
    std::transform(checkstr.begin(), checkstr.end(), checkstr.begin(), ::tolower);

    std::string type;
    if (checkstr[0] == '#') {
        type = "hex";
    } else if (checkstr[0] == '&') {
        type = "base64";
    } else if (checkstr[0] == 'b') {
        type = "binary";
    } else {
        type = "number";
    }

    return ParserToken{type, numStr, start};
}

ParserToken Lexer::readIdentifier() {
    size_t start = position;

    if (dollarBefore) {
        dollarBefore = false;
        start = position - 1;
    }

    while (position < input.length() &&
           (isLetter(input[position]) ||
            isDigit(input[position]) ||
            input[position] == '\'')) {
        position++;
    }

    std::string id = input.substr(start, position - start);

    std::string idWithoutDollar = id;
    if (!id.empty() && id[0] == '$') {
        idWithoutDollar = id.substr(1);
    }

    if (std::find(keywords.begin(), keywords.end(), idWithoutDollar) != keywords.end()) {
        return ParserToken{"keyword", id, start};
    }

    std::regex keyword_regex("^is$|^isn't$|^isif$|^then$|^elseif$|^else$|^isifn't$|^elseifn't$|^then't$|^elsen't$|^or$|^orn't$|^and$|^andn't$");
    std::regex boolean_regex("^true$|^True$|^TRUE$|^yes$|^Yes$|^YES$|^false$|^False$|^FALSE$|^no$|^No$|^NO$|^Y$|^y$|^N$|^n$");
    std::regex null_regex("^null$|^Null$|^NULL$|^nil$|^Nil$|^NIL$");
    std::regex undefined_regex("^undefined$");

    if (std::regex_match(idWithoutDollar, keyword_regex)) {
        return ParserToken{"keyword", id, start};
    } else if (std::regex_match(idWithoutDollar, boolean_regex)) {
        return ParserToken{"boolean", id, start};
    } else if (std::regex_match(idWithoutDollar, null_regex)) {
        return ParserToken{"null", id, start};
    } else if (std::regex_match(idWithoutDollar, undefined_regex)) {
        return ParserToken{"undefined", id, start};
    } else {
        return ParserToken{"identifier", id, start};
    }
}

void Lexer::addDollarBefore() {
    if (dollarBefore) {
        dollarBefore = false;
        position--;
    }
}
bool Lexer::isNonAscii(char c) {
    return static_cast<unsigned char>(c) > 127;
}

void Lexer::tokenize() {
    std::string warnPrefix = "";
    #ifndef __EMSCRIPTEN__
    if (Utility::isGitHubActions()) {
        warnPrefix = "::warning::";
    }
    #endif
    while (position < input.length()) {
        char ch = input[position];

        if (isWhitespace(ch)) {
            addDollarBefore();
            position++;
            continue;
        }

        if (ch == '-' && peek() == '-') {
            if ((isDigit(input[position - 1]) || isLetter(input[position - 1])) && (peek(2) == ',' || peek(2) == '.' || peek(2) == ')')) {
                addDollarBefore();
                position += 2;
                tokens.push_back(ParserToken{"--", "--", position});
                continue;
            } else {
                addDollarBefore();
                readComment();
                continue;
            }
        }
        if (ch == '-' && peek() == '{') {
            addDollarBefore();
            readMultiLineComment();
            continue;
        }

        if (ch == '"') {
            addDollarBefore();
            tokens.push_back(readString());
            continue;
        }

        if (ch == '<' && peek() != '<' && peek() != '=') {
            addDollarBefore();
            tokens.push_back(readLink());
            continue;
        }

        if (ch == '=' && peek() == '=') {
            addDollarBefore();
            tokens.push_back(ParserToken{"==", "==", position});
            position += 2;
            continue;
        }

        if (ch == '?' && peek() == '=') {
            addDollarBefore();
            tokens.push_back(ParserToken{"?=", "?=", position});
            position += 2;
            continue;
        }

        if (ch == '?' && peek() == '?') {
            addDollarBefore();
            tokens.push_back(ParserToken{"??", "??", position});
            position += 2;
            continue;
        }

        if (ch == '=' && peek() == '!') {
            addDollarBefore();
            tokens.push_back(ParserToken{"=!", "=!", position});
            position += 2;
            continue;
        }

        if (ch == '?' && peek() == '!') {
            addDollarBefore();
            tokens.push_back(ParserToken{"?!", "?!", position});
            position += 2;
            continue;
        }

        if (ch == '!' && peek() == '?') {
            if (peek(1) == '?') {
                addDollarBefore();
                tokens.push_back(ParserToken{"!??", "!??", position});
                position += 3;
            } else {
                addDollarBefore();
                tokens.push_back(ParserToken{"!?", "!?", position});
                position += 2;
            }
            continue;
        }

        if (ch == '.' && peek() == '.' && (position + 2) < input.length()) {
            addDollarBefore();
            tokens.push_back(ParserToken{"..", "..", position});
            position += 2;
            continue;
        }

        if (ch == '<' && peek() == '=') {
            addDollarBefore();
            tokens.push_back(ParserToken{"<=", "<=", position});
            position += 2;
            continue;
        }

        if (ch == '>' && peek() == '=') {
            addDollarBefore();
            tokens.push_back(ParserToken{">=", ">=", position});
            position += 2;
            continue;
        }

        if (ch == '!' && peek() == '=') {
            addDollarBefore();
            tokens.push_back(ParserToken{"!=", "!=", position});
            position += 2;
            continue;
        }

        if (ch == '|' && peek() == '|') {
            addDollarBefore();
            tokens.push_back(ParserToken{"||", "||", position});
            position += 2;
            continue;
        }

        if (ch == '!' && peek() == '|') {
            addDollarBefore();
            tokens.push_back(ParserToken{"!|", "!|", position});
            position += 2;
            continue;
        }

        if (ch == '&' && peek() == '&') {
            addDollarBefore();
            tokens.push_back(ParserToken{"&&", "&&", position});
            position += 2;
            continue;
        }

        if (ch == '!' && peek() == '&') {
            addDollarBefore();
            tokens.push_back(ParserToken{"!&", "!&", position});
            position += 2;
            continue;
        }

        if (isDigit(ch)) {
            addDollarBefore();
            tokens.push_back(readNumber());
            continue;
        }

        if (ch == '{' && peek() == '{') {
            addDollarBefore();
            std::stringstream JavaScript;
            size_t brackets = 2;
            size_t startPos = position;
            size_t str = 0;
            size_t comment = 0;
            position += 2;
            while(position < input.length() && brackets > 0) {
                JavaScript << input[position];
                if (input[position] == '{') {
                    brackets++;
                } else if (input[position] == '}' && str == 0 && comment == 0) {
                    brackets--;
                } else if (input[position] == '\'' && str == 0 && comment == 0) {
                    str = 1;
                } else if (input[position] == '\'' && str == 1 && input[position - 1] != '\\' && comment == 0) {
                    str = 0;
                } else if (input[position] == '"' && str == 0 && comment == 0) {
                    str = 2;
                } else if (input[position] == '"' && str == 2 && input[position - 1] != '\\' && comment == 0) {
                    str = 0;
                } else if (input[position] == '`' && str == 0 && comment == 0) {
                    str = 3;
                } else if (input[position] == '`' && str == 3 && input[position - 1] != '\\' && comment == 0) {
                    str = 0;
                } else if (input[position] == '/' && peek() == '/' && str == 0 && comment == 0) {
                    comment = 1;
                } else if (((input[position] == '\r' && peek() == '\n') || input[position] == '\n' || input[position] == '\r') && str == 0 && comment == 1) {
                    comment = 0;
                } else if (input[position] == '/' && peek() == '*' && str == 0 && comment == 0) {
                    comment = 2;
                } else if (input[position] == '*' && peek() == '/' && str == 0 && comment == 2) {
                    comment = 0;
                }
                position++;
            }
            if (brackets != 0 || str != 0 || comment != 0) throw new std::runtime_error("Unexpected EOF.");
            std::string JavaScript_str = JavaScript.str();
            std::string result = JavaScript_str.substr(0, JavaScript_str.size() - 2); // remove "}}" at the end
            if (warn) {
                #ifdef __EMSCRIPTEN__
                warn_lexer_js(Parser::getCurrentTimestamp().c_str(), Utility::position(position, input).c_str());
                #else
                std::cout << warnPrefix + "Warning: JavaScript may be corrupted in the lexer output." << std::endl;
                #endif
            }
            tokens.push_back(ParserToken{"JavaScript", result, startPos});
            continue;
        }
        if (ch == '<' && peek() == '<') {
            addDollarBefore();
            std::stringstream Luau;
            size_t brackets = 1;
            size_t startPos = position;
            size_t str = 0;
            size_t comment = 0;

            position += 2; // "<<"

            while (position < input.length() && brackets > 0) {
                char current = input[position];
                Luau << current;

                if (str == 0 && comment == 0) {
                    if (current == '<' && peek(1) == '<') {
                        brackets++;
                    } else if (current == '>' && peek(1) == '>') {
                        brackets--;
                    }
                }

                if (comment == 0) {
                    if (str == 0 && (current == '\'' || current == '"')) {
                        str = current;
                    } else if (str != 0 && current == str && input[position-1] != '\\') {
                        str = 0;
                    }
                }

                if (str == 0) {
                    if (comment == 0 && current == '-' && peek(1) == '-') {
                        comment = 1;
                    } else if (comment == 1 && (current == '\n' || current == '\r')) {
                        comment = 0;
                    }
                }

                position++;
            }

            if (brackets > 0 || str != 0) {
                throw std::runtime_error("Unexpected EOF.");
            }

            std::string Luau_str = Luau.str();
            if (Luau_str.length() >= 1) {
                Luau_str = Luau_str.substr(0, Luau_str.length() - 1); // ">"
            }

            tokens.push_back(ParserToken{"Luau", Luau_str, startPos});
            if (warn) {
                #ifdef __EMSCRIPTEN__
                warn_lexer_luau(Parser::getCurrentTimestamp().c_str(), Utility::position(position, input).c_str());
                #else
                std::cout << warnPrefix + "Warning: Luau may be corrupted in the lexer output." << std::endl;
                #endif
            }
            position++; // ">"
            continue;
        }

        if (ch == ',' || ch == '.' || ch == '[' || ch == ']' ||
            ch == '(' || ch == ')' || ch == '{' || ch == '}') {
            addDollarBefore();
            tokens.push_back(ParserToken{std::string(1, ch), std::string(1, ch), position});
            position++;
            continue;
        }

        if (isLetter(ch)) {
            tokens.push_back(readIdentifier());
            continue;
        }

        if (ch == '-') {
            addDollarBefore();
            tokens.push_back(ParserToken{"minus", "-", position});
            position++;
            continue;
        }

        if (ch == '$') {
            dollarBefore = true;
            position++;
            continue;
        }

        if (ch == '=' || ch == '?' || ch == '!' || ch == '<' ||
            ch == '>' || ch == '|' || ch == '&' || ch == '+' ||
            ch == '*' || ch == '/' || ch == '%' || ch == '^') {
            addDollarBefore();
            tokens.push_back(ParserToken{std::string(1, ch), std::string(1, ch), position});
            position++;
            continue;
        }

        if (isNonAscii(ch)) {
            tokens.push_back(readIdentifier());
            continue;
        }

        if (ch == ':' && peek() == ':') {
            addDollarBefore();
            tokens.push_back(ParserToken{"::", "::", position});
            position += 2;
            continue;
        }

        addDollarBefore();
        tokens.push_back(ParserToken{std::string(1, ch), std::string(1, ch), position});
        position++;
    }

    addDollarBefore();
}

std::vector<ParserToken> Lexer::getTokens() const {
    if (!tokens.empty()) {
        const auto& lastToken = tokens.back();
        if (lastToken.type != "." && lastToken.value != ".") {
            throw std::runtime_error("Expected \".\", got EOF at " + Utility::position(lastToken.start, input));
        }
    }
    return tokens;
}

std::pair<std::string, std::vector<ParserToken>> Lexer::parse(const std::string& input, const bool& warn) {
    Lexer lexer(input, warn);
    return std::make_pair(input, lexer.getTokens());
}
