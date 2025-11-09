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

Lexer::Lexer(const std::string& input) : input(input), position(0), dollarBefore(false) {
    if (input.empty()) {
        throw std::invalid_argument("Invalid Input.");
    }
    initializeKeywords();
    tokenize();
}

void Lexer::initializeKeywords() {
    keywords = ::keywords;
    smallkeywords = ::smallKeywords;
    bigkeywords = ::bigKeywords;

    for (const auto& pair : smallkeywords) {
        skw.push_back(pair.first);
    }
    for (const auto& pair : bigkeywords) {
        bkw.push_back(pair.first);
    }
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

void Lexer::readComment() {
    while (position < input.length() && input[position] != '\n') {
        position++;
    }
    if (position < input.length() && input[position] == '\n') {
        position++;
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

ParserToken Lexer::readAngleString() {
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

    auto smallIt = smallkeywords.find(idWithoutDollar);
    if (smallIt != smallkeywords.end()) {
        std::string fullKeyword = id[0] == '$' ? "$" + smallIt->second : smallIt->second;
        return ParserToken{"keyword", fullKeyword, start};
    }
    auto bigIt = bigkeywords.find(idWithoutDollar);
    if (bigIt != bigkeywords.end()) {
        std::string fullKeyword = id[0] == '$' ? "$" + bigIt->second : bigIt->second;
        return ParserToken{"keyword", fullKeyword, start};
    }

    if (std::find(keywords.begin(), keywords.end(), idWithoutDollar) != keywords.end()) {
        return ParserToken{"keyword", id, start};
    } else if (smallkeywords.find(idWithoutDollar) != smallkeywords.end()) {
        return ParserToken{"keyword", id, start};
    } else if (bigkeywords.find(idWithoutDollar) != bigkeywords.end()) {
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

void Lexer::tokenize() {
    while (position < input.length()) {
        char ch = input[position];

        if (isWhitespace(ch)) {
            addDollarBefore();
            position++;
            continue;
        }

        if (ch == '-' && peek() == '-') {
            addDollarBefore();
            readComment();
            continue;
        }

        if (ch == '"') {
            addDollarBefore();
            tokens.push_back(readString());
            continue;
        }

        if (ch == '<') {
            addDollarBefore();
            tokens.push_back(readAngleString());
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
            position += 2;
            while(position < input.length() && brackets > 0) {
                JavaScript << input[position];
                if (input[position] == '{') {
                    brackets++;
                } else if (input[position] == '}') {
                    brackets--;
                }
                position++;
            }
            std::string JavaScript_str = JavaScript.str();
            std::string result = JavaScript_str.substr(0, JavaScript_str.size() - 2); // remove "}}" at the end
            tokens.push_back(ParserToken{"JavaScript", result, startPos});
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

std::pair<std::string, std::vector<ParserToken>> Lexer::parse(const std::string& input) {
    Lexer lexer(input);
    return std::make_pair(input, lexer.getTokens());
}
