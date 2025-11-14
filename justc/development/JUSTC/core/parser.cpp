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

#include "parser.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstring>
#include "fetch.h"
#include "version.h"
#include "utility.h"
#include <vector>
#include "import.hpp"

#ifdef __EMSCRIPTEN__
    #include "parser.emscripten.h"
    #include <emscripten.h>

    #include <emscripten/val.h>
    #include <emscripten/bind.h>
    Value runJavaScript(const std::string& script, const std::string position, const bool warning) {
        Value output;
        output.name = "{{" + script + "}}";
        try {
            emscripten::val window = emscripten::val::global("window");
            emscripten::val result = window.call<emscripten::val>("eval", script);

            std::string result_type = result.typeOf().as<std::string>();
            if (result.isNull() || result.isUndefined()) {
                output.type = DataType::NULL_TYPE;
                output.string_value = "null";
            } else if (result_type == "string") {
                output.type = DataType::STRING;
                output.string_value = result.as<std::string>();
            } else if (result_type == "number") {
                output.type = DataType::NUMBER;
                output.number_value = result.as<double>();
            } else if (result_type == "boolean") {
                output.type = DataType::BOOLEAN;
                output.boolean_value = result.as<bool>();
            } else if (result_type == "object") {
                emscripten::val JSON = emscripten::val::global("JSON");
                emscripten::val json_string_val = JSON.call<emscripten::val>("stringify", result);
                if (result.isArray()) {
                    output.type = DataType::JSON_ARRAY;
                } else {
                    output.type = DataType::JSON_OBJECT;
                }
                output.string_value = json_string_val.as<std::string>();
            } else {
                emscripten::val String_global = emscripten::val::global("String");
                emscripten::val coerced_string_val = String_global.call<emscripten::val>("call", emscripten::val::undefined(), result);
                output.type = DataType::STRING;
                output.string_value = coerced_string_val.as<std::string>();
                if (warning) {
                    warn_unsupported_js_type(Parser::getCurrentTimestamp().c_str(), output.string_value.c_str(), position.c_str());
                }
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("JavaScript error at " + position + ":\n" + e.what());
        }
        return output;
    }
#else
    #include "run.js.hpp"
#endif

std::string Value::toString() const {
    switch (type) {
        case DataType::STRING:
        case DataType::LINK:
        case DataType::PATH:
        case DataType::VARIABLE:
            return string_value;
        case DataType::NUMBER:
            return std::to_string(number_value);
        case DataType::HEXADECIMAL:
            return "x" + std::to_string(static_cast<int>(number_value));
        case DataType::BINARY: {
            int num = static_cast<int>(number_value);
            std::string binary;
            if (num == 0) return "b0";
            while (num > 0) {
                binary = (num % 2 == 0 ? "0" : "1") + binary;
                num /= 2;
            }
            return "b" + binary;
        }
        case DataType::OCTAL: {
            std::stringstream ss;
            ss << "o" << std::oct << static_cast<int>(number_value);
            return ss.str();
        }
        case DataType::BOOLEAN:
            return boolean_value ? "true" : "false";
        case DataType::NULL_TYPE:
            return "null";
        case DataType::NOT_A_NUMBER:
            return "NaN";
        case DataType::INFINITE:
            return "Infinity";
        default:
            return "unknown";
    }
}

double Value::toNumber() const {
    switch (type) {
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
            return number_value;
        case DataType::STRING:
            try {
                return std::stod(string_value);
            } catch (...) {
                return 0.0;
            }
        case DataType::BOOLEAN:
            return boolean_value ? 1.0 : 0.0;
        case DataType::NULL_TYPE:
            return 0.0;
        case DataType::NOT_A_NUMBER:
            return std::numeric_limits<double>::quiet_NaN();
        case DataType::INFINITE:
            return std::numeric_limits<double>::infinity();
        default:
            return 0.0;
    }
}

bool Value::toBoolean() const {
    switch (type) {
        case DataType::BOOLEAN:
            return boolean_value;
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
            return number_value != 0.0;
        case DataType::STRING: {
            if (string_value.empty()) return false;
            auto toLower = [](const std::string& str) {
                std::string result = str;
                std::transform(result.begin(), result.end(), result.begin(),
                              [](unsigned char c) { return std::tolower(c); });
                return result;
            };
            std::string lower = toLower(string_value);
            if (lower == "true" || lower == "yes" || lower == "y" ||
                   lower == "+" ||  lower == "1"  || lower == "!0"
            ) {
                return true;
            }
            return false;
        }
        case DataType::LINK:
        case DataType::PATH:
        case DataType::VARIABLE:
        case DataType::INFINITE:
            return true;
        default:
            return false;
    }
}

Value Value::createNumber(double num) {
    Value result;
    result.type = DataType::NUMBER;
    result.number_value = num;
    result.name = std::to_string(num);
    return result;
}

Value Value::createString(const std::string& str) {
    Value result;
    result.type = DataType::STRING;
    result.string_value = str;
    result.name = "\"" + str + "\"";
    return result;
}

Value Value::createBoolean(bool b) {
    Value result;
    result.type = DataType::BOOLEAN;
    result.boolean_value = b;
    result.name = b;
    return result;
}

Value Value::createNull() {
    Value result;
    result.type = DataType::NULL_TYPE;
    result.name = "nil";
    return result;
}

Value Value::createLink(const std::string& link) {
    Value result;
    result.type = DataType::LINK;
    result.string_value = link;
    result.name = "<" + link + ">";
    return result;
}

Value Value::createPath(const std::string& path) {
    Value result;
    result.type = DataType::PATH;
    result.string_value = path;
    result.name = path;
    return result;
}

Value Value::createVariable(const std::string& varName) {
    Value result;
    result.type = DataType::VARIABLE;
    result.string_value = varName;
    result.name = varName;
    return result;
}

Value Value::createHexadecimal(double num) {
    Value result;
    result.type = DataType::HEXADECIMAL;
    result.number_value = num;
    result.name = "x" + Utility::double2hexString(num);
    return result;
}

Value Value::createBinary(double num) {
    Value result;
    result.type = DataType::BINARY;
    result.number_value = num;
    result.name = "b" + Utility::double2binString(num);
    return result;
}

Value Value::createOctal(double num) {
    Value result;
    result.type = DataType::OCTAL;
    result.number_value = num;
    result.name = "o" + Utility::double2octString(num);
    return result;
}

namespace {

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isHexDigit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool isBinaryDigit(char c) {
    return c == '0' || c == '1';
}

bool isOctalDigit(char c) {
    return c >= '0' && c <= '7';
}

double parseNumber(const std::string& str) {
    try {
        return std::stod(str);
    } catch (...) {
        return 0.0;
    }
}

bool isValidLink(const std::string& str) {
    return str.find("://") != std::string::npos ||
           str.find("www.") != std::string::npos ||
           (str.find('.') != std::string::npos && str.find('/') != std::string::npos);
}

long getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

}

Parser::Parser(const std::vector<ParserToken>& tokens, bool doExecute, bool runAsync, const std::string& input, const bool allowJavaScript, const bool canAllowJS, const std::string scriptName, const std::string scriptType)
    : tokens(tokens), input(input), position(0), outputMode("EVERYTHING"), allowJavaScript(allowJavaScript),
      globalScope(false), strictMode(false), hasLogFile(false),
      doExecute(doExecute), runAsync(runAsync), canAllowJS(allowJavaScript ? true : canAllowJS), scriptName(scriptName), scriptType(scriptType) {}

std::string Parser::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::tm timeinfo;

    #ifdef _WIN32
        localtime_s(&timeinfo, &time_t);
    #else
        localtime_r(&time_t, &timeinfo);  // POSIX (Linux/macOS/Emscripten)
    #endif

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

// logs
void Parser::addLog(const std::string& type, const std::string& message, size_t position) {
    std::string time = getCurrentTimestamp();
    logs.push_back({type, message, position, time});
    if (hasLogFile && type == "LOG") {
        appendToLogFile("[" + time + "] " + message);
    }
}
void Parser::setLogFile(const std::string& path) {
    logFilePath = path;
    hasLogFile = true;
}
void Parser::appendToLogFile(const std::string& content) {
    logFileContent += content + "\n";
}
void Parser::addImportLog(const std::string& path, const std::string& script, const std::string& type) {
    std::vector<std::string> log;
    log.push_back(path);
    log.push_back(script);
    log.push_back(type);
    importLogs.push_back(log);
}

ParserToken Parser::currentToken() const {
    if (position >= tokens.size()) {
        return {"EOF", "", 0};
    }
    return tokens[position];
}

ParserToken Parser::peekToken(size_t offset) const {
    if (position + offset >= tokens.size()) {
        return {"EOF", "", 0};
    }
    return tokens[position + offset];
}

void Parser::advance() {
    if (position < tokens.size()) {
        position++;
    }
}

bool Parser::match(const std::string& type) const {
    return currentToken().type == type;
}

bool Parser::match(const std::string& type, const std::string& value) const {
    return currentToken().type == type && currentToken().value == value;
}

bool Parser::isEnd() const {
    return position >= tokens.size();
}

void Parser::skipCommas() {
    while (match(",")) advance();
}

ParseResult Parser::parse(bool doExecute) {
    ParseResult result;

    try {
        while (!isEnd()) {
            skipCommas();
            if (isEnd()) break;

            if (match("keyword")) {
                std::string keyword = currentToken().value;

                if (keyword == "TYPE") {
                    ast.push_back(parseTypeCommand());
                } else if (keyword == "OUTPUT") {
                    ast.push_back(parseOutputCommand());
                } else if (keyword == "RETURN" || keyword == "RT") {
                    ast.push_back(parseReturnCommand());
                } else if (keyword == "ALLOW" || keyword == "DISALLOW") {
                    ast.push_back(parseAllowCommand());
                } else if (keyword == "IMPORT") {
                    ast.push_back(parseImportCommand());
                } else if (keyword == "ECHO" || keyword == "LOGFILE" || keyword == "LOG") {
                    ast.push_back(parseCommand(doExecute));
                } else {
                    ast.push_back(parseStatement(doExecute));
                }
            } else if (match("identifier")) {
                ast.push_back(parseStatement(doExecute));
            } else if (match(".")) {
                advance();
                if (!isEnd()) {
                    throw std::runtime_error("After end of script - Unexpected token \"" + tokens[position + 1].value + "\" at " + Utility::position(position + 1, input) + ".");
                }
                break;
            } else if (match("JavaScript")) {
                if (doExecute && allowJavaScript) {
                    #ifdef __EMSCRIPTEN__

                    Value result = runJavaScript(currentToken().value, Utility::position(position, input), false);
                    addLog("JAVASCRIPT", Utility::value2string(result), position);
                    if (result.type != DataType::NULL_TYPE) {
                        std::cout << Utility::value2string(result) << std::endl;
                    }

                    #else

                    std::pair<std::string, bool> jsresult = JavaScript::Eval(currentToken().value);
                    if (jsresult.second) {
                        throw std::runtime_error("JavaScript error at " + Utility::position(position, input) + ":\n" + jsresult.first);
                    } else {
                        addLog("JAVASCRIPT", jsresult.first, position);
                        std::cout << jsresult.first << std::endl;
                    }

                    #endif
                } else if (!allowJavaScript) {
                    #ifdef __EMSCRIPTEN__
                    warn_js_disabled_by_justc(Utility::position(currentToken().start, input).c_str(), currentToken().value.c_str(), getCurrentTimestamp().c_str());
                    #endif
                }
                advance();
            } else if (match("Luau")) {
                #ifdef __EMSCRIPTEN__
                bool error = use_luau(currentToken().value.c_str(), getCurrentTimestamp().c_str(), Utility::position(position, input).c_str());
                if (error) throw std::runtime_error("Luau error at " + Utility::position(position, input));
                #else
                std::cout << currentToken().value << std::endl;
                #endif
                advance();
            } else {
                throw std::runtime_error("Unexpected token \"" + currentToken().value + "\" at " + Utility::position(currentToken().start, input) + ".");
            }

            skipCommas();
        }
        position -= 1;

        buildDependencyGraph();

        if (detectCycles()) {
            throw std::runtime_error("Circular dependency detected");
        }

        evaluateAllVariables();

        if (outputMode == "SPECIFIED") {
            if (outputVariables.empty()) {
                throw std::runtime_error("OUTPUT SPECIFIED requires RETURN command with variables");
            }
            for (const auto& varName : outputVariables) {
                auto it = variables.find(varName);
                if (it != variables.end()) {
                    size_t index = &varName - &outputVariables[0];
                    std::string outputName = (index < outputNames.size()) ? outputNames[index] : varName;
                    if (outputName != "_") {
                        result.returnValues[outputName] = convertToDecimal(it->second);
                    } else {
                        result.returnValues[varName] = convertToDecimal(it->second);
                    }
                }
            }
        } else if (outputMode == "EVERYTHING") {
            if (!outputVariables.empty()) {
                throw std::runtime_error("RETURN command not allowed with OUTPUT EVERYTHING");
            }
            for (const auto& pair : variables) {
                result.returnValues[pair.first] = convertToDecimal(pair.second);
            }
        } else if (outputMode == "DISABLED") {
            if (!outputVariables.empty()) {
                throw std::runtime_error("RETURN command not allowed with OUTPUT DISABLED");
            }
        }

        result.logs = logs;
        result.logFilePath = hasLogFile ? logFilePath : "";
        result.logFileContent = hasLogFile ? logFileContent : "";
        result.importLogs = importLogs;

    } catch (const std::exception& e) {
        result.error = e.what();
        addLog("ERROR", e.what(), currentToken().start);
    }

    return result;
}

Value Parser::convertToDecimal(const Value& value) {
    if (value.type == DataType::HEXADECIMAL ||
        value.type == DataType::BINARY ||
        value.type == DataType::OCTAL) {
        Value result;
        result.type = DataType::NUMBER;
        result.number_value = value.number_value;
        result.name = value.name;
        return result;
    }
    return value;
}

ASTNode Parser::parseTypeCommand() {
    ASTNode node("TYPE_COMMAND", "", currentToken().start);
    advance();

    if (match("keyword")) {
        std::string type = currentToken().value;
        if (type == "GLOBAL") {
            globalScope = true;
        } else if (type == "LOCAL") {
            globalScope = false;
        } else if (type == "STRICT") {
            strictMode = true;
        }
        node.value = stringToValue(type);
        advance();
    }

    return node;
}

ASTNode Parser::parseOutputCommand() {
    ASTNode node("OUTPUT_COMMAND", "", currentToken().start);
    advance();

    if (match("keyword")) {
        std::string mode = currentToken().value;
        if (mode == "SPECIFIED" || mode == "EVERYTHING" || mode == "DISABLED") {
            outputMode = mode;
            node.value = stringToValue(outputMode);
            advance();
        } else {
            throw std::runtime_error("Invalid OUTPUT mode: " + mode);
        }
    }

    return node;
}

ASTNode Parser::parseReturnCommand() {
    ASTNode node("RETURN_COMMAND", "", currentToken().start);
    advance();

    if (match("[")) {
        advance();
        while (!match("]") && !isEnd()) {
            if (match("identifier")) {
                outputVariables.push_back(currentToken().value);
                advance();
            }
            if (match(",")) advance();
        }
        if (match("]")) advance();
    }

    if (match("keyword", "AS")) {
        advance();
        if (match("[")) {
            advance();
            while (!match("]") && !isEnd()) {
                if (match("identifier")) {
                    outputNames.push_back(currentToken().value);
                    advance();
                }
                if (match(",")) advance();
            }
            if (match("]")) advance();
        }
    }

    return node;
}

ASTNode Parser::parseAllowCommand() {
    ASTNode node("ALLOW_COMMAND", "", currentToken().start);
    std::string command = currentToken().value;
    advance();

    if (match("keyword", "JAVASCRIPT")) {
        if (!canAllowJS && command == "ALLOW") {
            #ifdef __EMSCRIPTEN__
            warn_cant_enable_js(Utility::position(currentToken().start, input).c_str(), getCurrentTimestamp().c_str(), scriptName.c_str(), scriptType.c_str());
            #endif
            addLog("WARN", "Attempt to allow JavaScript at <import ", currentToken().start);
        } else allowJavaScript = (command == "ALLOW");
        node.value = booleanToValue(allowJavaScript);
        advance();
    }

    return node;
}

ASTNode Parser::parseImportCommand() {
    ASTNode node("IMPORT_COMMAND", "", currentToken().start);
    advance();

    // TODO: import logic
    if (match("keyword", "JUSTC")) {
        advance();
        if (match("(")) {
            advance();
            std::string path;
            bool mode = true; // true = "export", false = "return"
            if (match("string") || match("path") || match("identifier")) {
                path = currentToken().value;
                advance();
                while (match("/") || match("path") || match("string") || match("identifier") || match(".")) {
                    if (isEnd()) {
                        throw std::runtime_error("Unexpected EOF.");
                    }
                    path += currentToken().value;
                    advance();
                }
            } else throw std::runtime_error("Expected <path>, got <" + currentToken().type + "> at " + Utility::position(position, input));
            if (match(")")) {advance();}
            else throw std::runtime_error("Expected \")\", got \"" + currentToken().value + "\" at " + Utility::position(position, input));
            // if (match("keyword", "REQUIRE") || match("keyword", "EXECUTE"));

            std::pair<ParseResult, std::string> imports = Import::JUSTC(path, Utility::position(position, input), doExecute, runAsync, allowJavaScript, mode);
            addImportLog(path, imports.second, "JUSTC");
            for (const auto& pair : imports.first.returnValues) {
                ASTNode node = ASTNode("VARIABLE_DECLARATION", pair.first, position);
                node.value = pair.second;
                ast.push_back(node);
            }
            for (size_t i = 0; i < imports.first.importLogs.size(); i++) {
                std::vector<std::string> importLog = imports.first.importLogs[i];
                std::string _path = importLog[0];
                std::string _script = importLog[1];
                std::string _type = importLog[2];
                addImportLog(_path, _script, _type);
            }
        } else {
            throw std::runtime_error("Expected \"(\", got \"" + currentToken().value + "\" at " + Utility::position(position, input));
        }
    }

    return node;
}

ASTNode Parser::parseStatement(bool doExecute) {
    if (match("identifier")) {
        return parseVariableDeclaration(doExecute);
    } else {
        return parseCommand(doExecute);
    }
}

ASTNode Parser::parseVariableDeclaration(bool doExecute) {
    std::string identifier = currentToken().value;
    size_t startPos = currentToken().start;
    ASTNode node("VARIABLE_DECLARATION", identifier, startPos);
    advance();

    std::string assignOp;
    std::string typeDecl;
    if (match(":")) {
        advance();
        typeDecl = currentToken().value;
        if (!match("identifier")) {
            throw std::runtime_error("Invalid or unexpected token \"" + typeDecl + "\" at " + Utility::position(position, input) + ".");
        }
        node.typeDeclaration = Utility::typeDeclaration2dataType(typeDecl, Utility::position(position, input));
        advance();
    }

    if (match("keyword", "is") || match("=")) {
        assignOp = currentToken().value;
        advance();

        Value exprValue = parseExpression(doExecute);
        node.value = exprValue;
        extractReferences(exprValue, node.references);
    }
    else if (match("keyword", "isn't") || match("!=")) {
        assignOp = currentToken().value;
        advance();

        Value exprValue = parseExpression(doExecute);
        exprValue = handleInequality(exprValue);
        node.value = exprValue;
        extractReferences(exprValue, node.references);
    }
    else if (match("keyword", "isif") || match("?")) {
        advance();
        Value conditionalValue = parseConditional(doExecute);
        node.value = conditionalValue;
        extractReferences(conditionalValue, node.references);
    }
    else if (position >= 2 && (
        tokens[position - 2].value == "ECHO" ||
        tokens[position - 2].value == "LOG"  ||
        tokens[position - 2].value == "LOGFILE"
    )) {
        position -= 2;
        parseCommand(doExecute);
    }
    else {
        if (isEnd()) {
            throw std::runtime_error("Expected assignment operator at " + Utility::position(position, input) + ", got EOF.");
        }
        throw std::runtime_error("Expected assignment operator at " + Utility::position(position, input) + ", got \"" + currentToken().value +"\".");
    }

    variables[identifier] = Value();

    return node;
}

Value Parser::parseExpression(bool doExecute) {
    return parseConditional(doExecute);
}

Value Parser::parseConditional(bool doExecute) {
    Value condition = parseLogicalOR(doExecute);

    if (match("keyword", "then") || match("==")) {
        std::string thenOp = currentToken().value;
        advance();

        Value thenValue = parseExpression(doExecute);

        if (match("keyword", "else") || match("?=")) {
            std::string elseOp = currentToken().value;
            advance();

            Value elseValue = parseExpression(doExecute);

            return handleConditional(condition, thenValue, elseValue, thenOp, elseOp);
        } else {
            throw std::runtime_error("Expected 'else' after 'then'");
        }
    }

    if (match("keyword", "elseif") || match("??")) {
        std::string elseifOp = currentToken().value;
        advance();

        Value elseifCondition = parseExpression(doExecute);

        if (match("keyword", "then") || match("==")) {
            std::string thenOp = currentToken().value;
            advance();

            Value thenValue = parseExpression(doExecute);

            if (match("keyword", "else") || match("?=")) {
                std::string elseOp = currentToken().value;
                advance();

                Value elseValue = parseExpression(doExecute);

                Value nestedConditional = handleConditional(elseifCondition, thenValue, elseValue, thenOp, elseOp);
                return handleConditional(condition, thenValue, nestedConditional, thenOp, elseOp);
            } else {
                throw std::runtime_error("Expected 'else' after 'then' in elseif");
            }
        } else {
            throw std::runtime_error("Expected 'then' after 'elseif'");
        }
    }

    return condition;
}

Value Parser::parseLogicalOR(bool doExecute) {
    Value left = parseLogicalAND(doExecute);

    while (match("keyword", "or") || match("||") ||
           match("keyword", "orn't") || match("!|")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseLogicalAND(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parseLogicalAND(bool doExecute) {
    Value left = parseEquality(doExecute);

    while (match("keyword", "and") || match("&") ||
           match("keyword", "andn't") || match("!&")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseEquality(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parseEquality(bool doExecute) {
    Value left = parseComparison(doExecute);

    while (match("keyword", "is") || match("=") ||
           match("keyword", "isn't") || match("!=")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseComparison(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parseComparison(bool doExecute) {
    Value left = parseTerm(doExecute);

    while (match("<") || match(">") || match("<=") || match(">=")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseTerm(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parseTerm(bool doExecute) {
    Value left = parseFactor(doExecute);

    while (match("+") || match("minus") || match("..")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseFactor(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parseFactor(bool doExecute) {
    Value left = parsePower(doExecute);

    while (match("*") || match("/") || match("%")) {
        std::string op = currentToken().value;
        advance();

        Value right = parsePower(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parsePower(bool doExecute) {
    Value left = parseUnary(doExecute);

    while (match("^")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseUnary(doExecute);
        left = evaluateExpression(left, op, right);
    }

    return left;
}

Value Parser::parseUnary(bool doExecute) {
    if (match("minus") || match("+") || match("!") || match("-")) {
        std::string op = currentToken().value;
        advance();

        Value right = parseUnary(doExecute);
        return evaluateExpression(Value(), op, right);
    }

    return parsePrimary(doExecute);
}

Value Parser::astNodeToValue(const ASTNode& node) {
    if (node.type == "VARIABLE_DECLARATION") {
        return evaluateASTNode(node);
    }
    else if (node.type == "COMMAND") {
        return stringToValue(node.identifier);
    }
    else {
        return node.value;
    }
}

Value Parser::parsePrimary(bool doExecute) {
    if (match("number")) {
        double num = parseNumber(currentToken().value);
        advance();
        return numberToValue(num);
    }
    else if (match("hex")) {
        std::string hexStr = currentToken().value;
        advance();
        return hexToValue(hexStr);
    }
    else if (match("binary")) {
        std::string binStr = currentToken().value;
        advance();
        return binaryToValue(binStr);
    }
    else if (match("string")) {
        std::string str = currentToken().value;
        advance();
        return stringToValue(str);
    }
    else if (match("link")) {
        std::string link = currentToken().value;
        advance();
        return linkToValue(link);
    }
    else if (match("boolean")) {
        auto toLower = [](const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            return result;
        };

        std::string tokenValue = currentToken().value;
        bool b = (toLower(tokenValue) == "true" ||
                  toLower(tokenValue) == "yes" ||
                  toLower(tokenValue) == "y");
        advance();
        return booleanToValue(b);
    }
    else if (match("null")) {
        Value result;
        result.type = DataType::NULL_TYPE;
        result.name = "null";
        advance();
        return result;
    }
    else if (match("identifier")) {
        std::string varName = currentToken().value;

        if (varName == "$TIME" || varName == "$VERSION" || varName == "$LATEST" ||
            varName == "$DBID" || varName == "$SHA" || varName == "$NAV" ||
            varName == "$PAGES" || varName == "$CSS" || varName == "$PI" ||
            varName == "$BACKSLASH" || varName == "$JUST_VERSION") {
            advance();
            return executeFunction(varName.substr(1), {}, currentToken().start);
        }

        if (peekToken().type == "(") {
            return parseFunctionCall(doExecute);
        }

        Value result;
        result.type = DataType::VARIABLE;
        result.string_value = varName;
        advance();
        while (match(".") && tokens[position + 1].type == "identifier") {
            advance();
            result.string_value += "." + currentToken().value;
            advance();
            if (isEnd()) {
                throw std::runtime_error("Unexpected EOF");
            }
        }
        return result;
    }
    else if (match("keyword") && peekToken().type == "(") {
        return parseFunctionCall(doExecute);
    }
    else if (match("(")) {
        advance();
        Value result = parseExpression(doExecute);
        if (!match(")")) {
            throw std::runtime_error("Expected \")\" at " + Utility::position(position, input) + ".");
        }
        advance();
        return result;
    }
    else if ((match(".") && tokens[position + 1].type != "number") || match(",")) {
        Value result;
        result.type = DataType::NULL_TYPE;
        result.string_value = "null";
        result.name = "null";
        return result;
    }
    else if (match("keyword") || match("?") || match("!=") || match("=")) {
        return astNodeToValue(parseStatement(doExecute));
    }
    else if (match(".") && position + 1 < tokens.size() && tokens[position + 1].type == "number") {
        advance();
        double num = parseNumber("0." + currentToken().value);
        advance();
        return numberToValue(num);
    }
    else if (match("|")) {
        advance();
        std::stringstream object;
        while (!match(".")) {
            object << currentToken().value;
            advance();
            if (isEnd()) {
                throw std::runtime_error("Expected \".\" to close object, got EOF at " + Utility::position(position, input) + ".");
            }
        }
        object << ".";
        std::string objectstr = object.str();
        advance();
        Value result = stringToValue(objectstr);
        result.type = DataType::JUSTC_OBJECT;
        result.name = objectstr;
        return result;
    }
    else if (match("JavaScript") && doExecute && allowJavaScript) {
        #ifdef __EMSCRIPTEN__

        Value result = runJavaScript(currentToken().value, Utility::position(currentToken().start, input), true);
        addLog("JAVASCRIPT", Utility::value2string(result), currentToken().start);
        advance();
        return result;

        #else

        std::pair<std::string, bool> jsresult = JavaScript::Eval(currentToken().value);
        if (jsresult.second) {
            throw std::runtime_error("JavaScript error at " + Utility::position(currentToken().start, input) + ":\n" + jsresult.first);
        } else {
            addLog("JAVASCRIPT", jsresult.first, currentToken().start);
        }
        advance();
        return stringToValue(jsresult.first);

        #endif
    }
    else if (match("JavaScript")) {
        #ifdef __EMSCRIPTEN__
        if (!allowJavaScript) warn_js_disabled_by_justc(Utility::position(currentToken().start, input).c_str(), currentToken().value.c_str(), getCurrentTimestamp().c_str());
        else warn_js_disabled(Utility::position(currentToken().start, input).c_str(), currentToken().value.c_str(), getCurrentTimestamp().c_str());
        #endif
        advance();
        return Value::createNull();
    }

    throw std::runtime_error("Invalid or unexpected token \"" + currentToken().value + "\" at " + Utility::position(position, input) + ".");
}

Value Parser::parseFunctionCall(bool doExecute) {
    std::string funcName = currentToken().value;
    size_t startPos = currentToken().start;
    advance();

    if (!match("(")) {
        throw std::runtime_error("Expected \"(\" after function name at " + Utility::position(startPos, input) + ".");
    }
    advance();

    std::vector<Value> args;
    while (!match(")") && !isEnd()) {
        args.push_back(parseExpression(doExecute));
        if (match(",")) advance();
    }

    if (!match(")")) {
        throw std::runtime_error("Expected \")\" after function arguments at" + Utility::position(startPos, input) + ".");
    }
    advance();

    return executeFunction(funcName, args, startPos);
}

ASTNode Parser::parseCommand(bool doExecute) {
    ASTNode node("COMMAND", currentToken().value, currentToken().start);
    std::string command = currentToken().value;
    advance();
    std::vector<Value> args;

    if (doExecute && (command == "ECHO" || command == "LOGFILE" || command == "LOG") && !match("(")) {
        while (!match(",") && !match(".") && !isEnd()) {
            args.push_back(parseExpression(doExecute));
        }
        if (match(",")) advance();

        if (command == "ECHO") {
            for (const auto& arg : args) {
                std::string message = arg.toString();
                auto varval = resolveVariableValue(message, false);
                if (varval.type == DataType::UNKNOWN) {
                    addLog("ECHO", message, node.startPos);
                    std::cout << message << std::endl;
                } else {
                    addLog("ECHO", Utility::value2string(varval), node.startPos);
                    std::cout << Utility::value2string(varval) << std::endl;
                }
            }
        } else if (command == "LOGFILE") {
            if (!args.empty()) {
                std::string path = args[0].toString();
                setLogFile(path);
            }
        } else if (command == "LOG") {
            for (const auto& arg : args) {
                std::string message = arg.toString();
                auto varval = resolveVariableValue(message, false);
                if (varval.type == DataType::UNKNOWN) {
                    addLog("LOG", message, node.startPos);
                } else {
                    addLog("LOG", Utility::value2string(varval), node.startPos);
                }
            }
        }
        return node;
    }

    if (match("(")) {
        advance();
        while (!match(")") && !isEnd()) {
            args.push_back(parseExpression(doExecute));
            if (match(",")) advance();
        }
        if (match(")")) advance();
    }

    if (doExecute) {
        if (command == "ECHO") {
            for (const auto& arg : args) {
                std::string message = arg.toString();
                auto varval = resolveVariableValue(message, false);
                if (varval.type == DataType::UNKNOWN) {
                    addLog("ECHO", message, node.startPos);
                    std::cout << message << std::endl;
                } else {
                    addLog("ECHO", Utility::value2string(varval), node.startPos);
                    std::cout << Utility::value2string(varval) << std::endl;
                }
            }
        } else if (command == "LOGFILE") {
            if (!args.empty()) {
                std::string path = args[0].toString();
                setLogFile(path);
            }
        } else if (command == "LOG") {
            for (const auto& arg : args) {
                std::string message = arg.toString();
                auto varval = resolveVariableValue(message, false);
                if (varval.type == DataType::UNKNOWN) {
                    addLog("LOG", message, node.startPos);
                } else {
                    addLog("LOG", Utility::value2string(varval), node.startPos);
                }
            }
        }
    }

    return node;
}

Value Parser::onHTTPDisabled(size_t startPos, std::string args0string_value) {
    #ifdef __EMSCRIPTEN__
    warn_http_disabled(Utility::position(startPos, input).c_str(), args0string_value.c_str(), getCurrentTimestamp().c_str());
    #endif

    Value result;
    result.type = DataType::ERROR;
    result.string_value = "HTTP requests are disabled";
    result.name = "<" + args0string_value + ">";
    return result;
}

Value Parser::executeFunction(const std::string& funcName, const std::vector<Value>& args, size_t startPos) {
    if (funcName == "TIME") {
        long timestamp = getCurrentTime();
        return numberToValue(timestamp);
    }
    else if (funcName == "PI") {
        return numberToValue(3.14159265358979323846);
    }
    else if (funcName == "BACKSLASH") {
        return stringToValue("\\");
    }
    else if (funcName == "VERSION") {
        return stringToValue(JUSTC_VERSION);
    }

    // built-in
    if (funcName == "VALUE") return functionVALUE(args);
    if (funcName == "STRING") return functionSTRING(args);
    if (funcName == "LINK") return functionLINK(args);
    if (funcName == "NUMBER") return functionNUMBER(args);
    if (funcName == "BINARY") return functionBINARY(args);
    if (funcName == "OCTAL") return functionOCTAL(args);
    if (funcName == "HEXADECIMAL") return functionHEXADECIMAL(args);
    if (funcName == "TYPEID") return functionTYPEID(args);
    if (funcName == "TYPEOF") return functionTYPEOF(args);
    if (funcName == "ECHO") return functionECHO(args);
    if (funcName == "JSON") return functionJSON(args);
    if (funcName == "HTTPJSON") {
        if (!doExecute) {
            return onHTTPDisabled(startPos, args[0].string_value);
        }
        if (runAsync) {
            auto future = functionHTTPJSONAsync(args);
            return future.get();
        }
        return functionHTTPJSON(args);
    }
    if (funcName == "HTTPTEXT") {
        if (!doExecute) {
            return onHTTPDisabled(startPos, args[0].string_value);
        }
        if (runAsync) {
            auto future = functionHTTPTEXTAsync(startPos, args);
            return future.get();
        }
        return functionHTTPTEXT(startPos, args);
    }
    if (funcName == "JUSTC") return functionJUSTC(args);
    if (funcName == "HTTPJUSTC") {
        if (!doExecute) {
            return onHTTPDisabled(startPos, args[0].string_value);
        }
        if (runAsync) {
            auto future = functionHTTPJUSTCAsync(args);
            return future.get();
        }
        return functionHTTPJUSTC(args);
    }
    if (funcName == "PARSEJUSTC") return functionPARSEJUSTC(args);
    if (funcName == "PARSEJSON") return functionPARSEJSON(args);
    if (funcName == "FILE") {
        if (runAsync) {
            auto future = functionFILEAsync(args);
            return future.get();
        }
        return functionFILE(args);
    }
    if (funcName == "SIZE") return functionSTAT(args);
    if (funcName == "ENV") return functionENV(args);
    if (funcName == "CONFIG") return functionCONFIG(args);

    // math
    if (funcName == "V") return functionV(args);
    if (funcName == "D") return functionD(args);
    if (funcName == "SQ") return functionSQ(args);
    if (funcName == "CU") return functionCU(args);
    if (funcName == "P") return functionP(args);
    if (funcName == "M") return functionM(args);
    if (funcName == "S") return functionS(args);
    if (funcName == "C") return functionC(args);
    if (funcName == "T") return functionT(args);
    if (funcName == "N") return functionN(args);
    if (funcName == "ABSOLUTE") return functionABSOLUTE(args);
    if (funcName == "CEIL") return functionCEIL(args);
    if (funcName == "FLOOR") return functionFLOOR(args);

    throw std::runtime_error("Unknown function: " + funcName);
}

Value Parser::concatenateStrings(const Value& left, const Value& right) {
    Value result;

    if (
        ( left.type  != DataType::STRING  &&
          left.type  != DataType::UNKNOWN ) ||
        ( right.type != DataType::STRING  &&
          right.type != DataType::UNKNOWN )
    ) {
        std::string error = "Cannot concatenate string with ";
        if (left.type == DataType::STRING || left.type == DataType::UNKNOWN) {
            throw std::runtime_error(error + dataTypeToString(right.type) + " at " + Utility::position(position, input) + ".");
        } else if (right.type == DataType::STRING || right.type == DataType::UNKNOWN) {
            throw std::runtime_error(error + dataTypeToString(left.type)  + " at " + Utility::position(position, input) + ".");
        } else {
            throw std::runtime_error("Unexpected operator \"..\" at " + Utility::position(position, input) + ". Did you mean " + left.name + " + " + right.name + "?");
        }
    } else if (left.type == DataType::UNKNOWN && right.type == DataType::UNKNOWN) {
        result = stringToValue(left.name + right.name);                                     // "abc .. def" = ""abcdef"", where both "abc" and "def" are not defined.
    } else if (left.type == DataType::UNKNOWN) {
        result = stringToValue(left.name + Utility::value2string(right));                   // "abc .. "def"" = ""abcdef"", where "abc" is not defined.
    } else if (right.type == DataType::UNKNOWN) {
        result = stringToValue(Utility::value2string(left) + right.name);                   // ""abc" .. def" = ""abcdef"", where "def" is not defined.
    } else {
        result = stringToValue(Utility::value2string(left) + Utility::value2string(right)); // ""abc" .. "def"" = ""abcdef"".
    }

    return result;
}
Value Parser::evaluateExpression(const Value& left, const std::string& op, const Value& right) {
    Value result;

    if (op == "+") {
        if (
            (left.type == DataType::STRING  && right.type == DataType::STRING ) ||
            (left.type == DataType::UNKNOWN && right.type == DataType::UNKNOWN) ||
            (left.type == DataType::UNKNOWN && right.type == DataType::STRING ) ||
            (left.type == DataType::STRING  && right.type == DataType::UNKNOWN)
        ) {
            throw std::runtime_error("Unexpected operator \"+\" at " + Utility::position(position, input) + ". Did you mean '" + left.name + " .. " + right.name + "'?");
        } else if (left.type == DataType::STRING) {
            throw std::runtime_error("Cannot add string to " + Utility::value2string(right) + " at " + Utility::position(position, input) + ".");
        } else if (right.type == DataType::STRING) {
            throw std::runtime_error("Cannot add " + Utility::value2string(left) + " to string at " + Utility::position(position, input) + ".");
        } else if (left.type == DataType::NUMBER && right.type == DataType::NUMBER) {
            result = numberToValue(left.toNumber() + right.toNumber());
        } else if (left.type == DataType::UNKNOWN) {
            result = stringToValue(left.name + Utility::value2string(right));
        } else if (right.type == DataType::UNKNOWN) {
            result = stringToValue(Utility::value2string(left) + right.name);
        } else {
            result = stringToValue(left.toString() + right.toString());
        }
    }
    else if (op == "minus" || op == "-") {
        if (left.type == DataType::UNKNOWN) {
            result = numberToValue(-right.toNumber());
        } else if (Utility::checkNumbers(left, right)) {
            result = numberToValue(left.toNumber() - right.toNumber());
        } else {
            throw std::runtime_error("Unexpected operator \"-\" at " + Utility::position(position, input) + ".");
        }
    }
    else if (op == "*" && Utility::checkNumbers(left, right)) {
        result = numberToValue(left.toNumber() * right.toNumber());
    }
    else if (op == "/" && Utility::checkNumbers(left, right)) {
        double divisor = right.toNumber();
        if (divisor == 0) {
            result.type = DataType::INFINITE;
            result.name = "infinity";
        } else {
            result = numberToValue(left.toNumber() / divisor);
        }
    }
    else if (op == "^" && Utility::checkNumbers(left, right)) {
        result = numberToValue(std::pow(left.toNumber(), right.toNumber()));
    }
    else if (op == "%" && Utility::checkNumbers(left, right)) {
        result = numberToValue(std::fmod(left.toNumber(), right.toNumber()));
    }
    else if (op == "..") {
        result = concatenateStrings(left, right);
    }

    else if (op == "=" || op == "is") {
        result = booleanToValue(left.toNumber() == right.toNumber());
    }
    else if (op == "!=" || op == "isn't") {
        result = booleanToValue(left.toNumber() != right.toNumber());
    }
    else if (op == "<" && Utility::checkNumbers(left, right)) {
        result = booleanToValue(left.toNumber() < right.toNumber());
    }
    else if (op == ">" && Utility::checkNumbers(left, right)) {
        result = booleanToValue(left.toNumber() > right.toNumber());
    }
    else if (op == "<=" && Utility::checkNumbers(left, right)) {
        result = booleanToValue(left.toNumber() <= right.toNumber());
    }
    else if (op == ">=" && Utility::checkNumbers(left, right)) {
        result = booleanToValue(left.toNumber() >= right.toNumber());
    }

    else if (op == "&" || op == "and") {
        result = booleanToValue(left.toBoolean() && right.toBoolean());
    }
    else if (op == "!&" || op == "andn't") {
        result = booleanToValue(!(left.toBoolean() && right.toBoolean()));
    }
    else if (op == "||" || op == "or") {
        result = booleanToValue(left.toBoolean() || right.toBoolean());
    }
    else if (op == "!|" || op == "orn't") {
        result = booleanToValue(!(left.toBoolean() || right.toBoolean()));
    }
    else if (op == "!") {
        result = booleanToValue(!right.toBoolean());
    }

    else {
        throw std::runtime_error("Unexpected operator \"" + op + "\" at " + Utility::position(position, input) + ".");
    }

    return result;
}

Value Parser::handleInequality(const Value& value) {
    Value result;

    switch (value.type) {
        case DataType::NUMBER:
            result = booleanToValue(value.toNumber() > 0);
            break;
        case DataType::LINK:
            result = stringToValue(value.toString());
            result.type = DataType::STRING;
            break;
        case DataType::BOOLEAN:
            result = booleanToValue(!value.toBoolean());
            break;
        default:
            result = booleanToValue(false);
            break;
    }

    return result;
}

Value Parser::handleConditional(const Value& condition, const Value& thenVal, const Value& elseVal,
                               const std::string& thenOp, const std::string& elseOp) {
    bool cond = condition.toBoolean();

    if (thenOp == "then't" || thenOp == "=!") {
        cond = !cond;
    }

    if (cond) {
        return thenVal;
    } else {
        if (elseOp == "elsen't" || elseOp == "?!") {
            return handleInequality(elseVal);
        }
        return elseVal;
    }
}

void Parser::buildDependencyGraph() {
    for (const auto& node : ast) {
        if (node.type == "VARIABLE_DECLARATION") {
            dependencies[node.identifier] = node.references;
        }
    }
}

bool Parser::detectCycles() {
    std::unordered_map<std::string, bool> visited;
    std::unordered_map<std::string, bool> recStack;
    std::vector<std::string> cyclePath;

    for (const auto& pair : dependencies) {
        if (dfsCycleDetection(pair.first, visited, recStack, cyclePath)) {
            return true;
        }
    }

    return false;
}

bool Parser::dfsCycleDetection(const std::string& node,
                              std::unordered_map<std::string, bool>& visited,
                              std::unordered_map<std::string, bool>& recStack,
                              std::vector<std::string>& cyclePath) {
    if (!visited[node]) {
        visited[node] = true;
        recStack[node] = true;
        cyclePath.push_back(node);

        for (const auto& neighbor : dependencies[node]) {
            if (!visited[neighbor] && dfsCycleDetection(neighbor, visited, recStack, cyclePath)) {
                return true;
            } else if (recStack[neighbor]) {
                cyclePath.push_back(neighbor);
                return true;
            }
        }
    }

    recStack[node] = false;
    if (!cyclePath.empty()) cyclePath.pop_back();
    return false;
}

Value Parser::resolveVariableValue(const std::string& varName, const bool unknownIsString) {
    auto it = variables.find(varName);
    if (it != variables.end() && it->second.type != DataType::UNKNOWN) {
        return it->second;
    }

    for (const auto& node : ast) {
        if (node.type == "VARIABLE_DECLARATION" && node.identifier == varName) {
            return evaluateASTNode(node);
        }
    }

    if (unknownIsString) {
        Value result;
        result.type = DataType::STRING;
        result.name = varName;
        result.string_value = varName;
        return result;
    }

    Value result;
    result.type = DataType::UNKNOWN;
    result.name = "unknown";
    return result;
}

void Parser::evaluateAllVariables() {
    if (runAsync && !dependencies.empty()) {
        evaluateAllVariablesAsync();
    } else {
        evaluateAllVariablesSync();
    }
}

std::runtime_error Parser::typeDeclarationError(const DataType left, const DataType right, const ASTNode node) {
    return std::runtime_error("Type declaration error: Cannot convert " + dataTypeToString(left) + " to " + dataTypeToString(right) + " at " + Utility::position(node.startPos, input) + ".");
}

Value Parser::applyTypeDeclaration(const Value value, const ASTNode node) {
    DataType typeDeclaration = node.typeDeclaration;
    Value result = value;
    if (typeDeclaration == result.type) return result;
    switch (typeDeclaration) {
        case DataType::UNKNOWN:
            break;
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::OCTAL:
        case DataType::BINARY:
            switch (result.type) {
                case DataType::NUMBER:
                case DataType::HEXADECIMAL:
                case DataType::OCTAL:
                case DataType::BINARY:
                    result = Utility::convert(result, typeDeclaration);
                    break;
                default:
                    throw typeDeclarationError(result.type, typeDeclaration, node);
                    break;
            }
            break;
        case DataType::STRING:
            result.type = DataType::STRING;
            result.string_value = Utility::value2string(value);
            break;
        case DataType::LINK:
            if (result.type == DataType::STRING) {
                if (isValidLink(result.string_value)) {
                    result.type = DataType::LINK;
                } else {
                    throw std::runtime_error("Type declaration error: Invalid link: " + result.string_value);
                }
            }
            break;
        case DataType::BOOLEAN:
            result.type = DataType::BOOLEAN;
            switch (result.type) {
                case DataType::NUMBER:
                case DataType::HEXADECIMAL:
                case DataType::OCTAL:
                case DataType::BINARY:
                    result.boolean_value = (value.number_value > 0);
                    break;
                case DataType::STRING:
                    result.boolean_value = value.toBoolean();
                    break;
                case DataType::NULL_TYPE:
                    result.boolean_value = false;
                    break;
                case DataType::INFINITE:
                    result.boolean_value = true;
                    break;
                default:
                    throw typeDeclarationError(result.type, typeDeclaration, node);
                    break;
            }
            break;
        default:
            throw typeDeclarationError(result.type, typeDeclaration, node);
            break;
    }
    return result;
}

Value Parser::evaluateASTNode(const ASTNode& node) {
    if (node.type == "VARIABLE_DECLARATION") {
        Value result = node.value;

        if (result.type == DataType::VARIABLE) {
            std::string refVar = result.string_value;
            if (refVar == node.identifier) {
                throw std::runtime_error("Variable cannot reference itself: " + node.identifier);
            }
            Value varval = resolveVariableValue(refVar, true);
            return applyTypeDeclaration(varval, node);
        }

        return applyTypeDeclaration(result, node);
    }

    return node.value;
}

void Parser::extractReferences(const Value& value, std::vector<std::string>& references) {
    if (value.type == DataType::VARIABLE) {
        references.push_back(value.string_value);
    }
    // TODO: get links from complex things
}

std::future<Value> Parser::functionHTTPJSONAsync(const std::vector<Value>& args) {
    return executeAsyncIfEnabled([this, args]() {
        return functionHTTPJSON(args);
    });
}

std::future<Value> Parser::functionHTTPTEXTAsync(size_t startPos, const std::vector<Value>& args) {
    return executeAsyncIfEnabled([this, startPos, args]() {
        return functionHTTPTEXT(startPos, args);
    });
}

std::future<Value> Parser::functionHTTPJUSTCAsync(const std::vector<Value>& args) {
    return executeAsyncIfEnabled([this, args]() {
        return functionHTTPJUSTC(args);
    });
}

std::future<Value> Parser::functionFILEAsync(const std::vector<Value>& args) {
    return executeAsyncIfEnabled([this, args]() {
        return functionFILE(args);
    });
}

Value Parser::functionVALUE(const std::vector<Value>& args) {
    if (args.size() != 1 || args[0].type != DataType::VARIABLE) {
        throw std::runtime_error("VALUE function requires one variable argument");
    }

    std::string varName = args[0].string_value;
    return resolveVariableValue(varName, true);
}

Value Parser::functionSTRING(const std::vector<Value>& args) {
    if (args.empty()) return stringToValue("");

    return stringToValue(args[0].toString());
}

Value Parser::functionLINK(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("LINK function requires one argument");

    std::string str = args[0].toString();
    if (!isValidLink(str)) {
        throw std::runtime_error("Invalid link: " + str);
    }

    return linkToValue(str);
}

Value Parser::functionNUMBER(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);

    return numberToValue(args[0].toNumber());
}

Value Parser::functionBINARY(const std::vector<Value>& args) {
    if (args.empty()) return binaryToValue("0");

    double num = args[0].toNumber();
    std::string binary;
    int intNum = static_cast<int>(num);

    if (intNum == 0) return binaryToValue("0");

    while (intNum > 0) {
        binary = (intNum % 2 == 0 ? "0" : "1") + binary;
        intNum /= 2;
    }

    return binaryToValue(binary);
}

Value Parser::functionOCTAL(const std::vector<Value>& args) {
    if (args.empty()) return octalToValue("0");

    double num = args[0].toNumber();
    std::stringstream ss;
    ss << std::oct << static_cast<int>(num);
    return octalToValue(ss.str());
}

Value Parser::functionHEXADECIMAL(const std::vector<Value>& args) {
    if (args.empty()) return hexToValue("0");

    double num = args[0].toNumber();
    std::stringstream ss;
    ss << std::hex << static_cast<int>(num);
    return hexToValue(ss.str());
}

Value Parser::functionTYPEID(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(static_cast<double>(DataType::UNKNOWN));

    return numberToValue(static_cast<double>(args[0].type));
}

Value Parser::functionTYPEOF(const std::vector<Value>& args) {
    if (args.empty()) return stringToValue("unknown");

    switch (args[0].type) {
        case DataType::JUSTC_OBJECT: return stringToValue("justc_object");
        case DataType::NUMBER: return stringToValue("number");
        case DataType::STRING: return stringToValue("string");
        case DataType::LINK: return stringToValue("link");
        case DataType::BOOLEAN: return stringToValue("boolean");
        case DataType::JSON_OBJECT: return stringToValue("json_object");
        case DataType::JSON_ARRAY: return stringToValue("json_array");
        case DataType::NULL_TYPE: return stringToValue("null");
        case DataType::HEXADECIMAL: return stringToValue("hexadecimal");
        case DataType::BINARY: return stringToValue("binary");
        case DataType::PATH: return stringToValue("path");
        case DataType::OCTAL: return stringToValue("octal");
        default: return stringToValue("unknown");
    }
}

Value Parser::functionECHO(const std::vector<Value>& args) {
    for (const auto& arg : args) {
        std::string message = arg.toString();
        addLog("ECHO", message, currentToken().start);
        std::cout << message << std::endl;
    }
    return Value();
}

Value Parser::functionJSON(const std::vector<Value>& args) { return Value(); }
Value Parser::functionHTTPJSON(const std::vector<Value>& args) { return Value(); }

Value Parser::functionHTTPTEXT(size_t startPos, const std::vector<Value>& args) {
    if (args.empty()) {
        throw std::runtime_error("Expected one argument at function HTTPTEXT at " + Utility::position(startPos, input) + ".");
    } else if (args[0].type != DataType::LINK) {
        throw std::runtime_error("Expected TYPEOF( argument 0 )=\"Link\" at function HTTPTEXT at " + Utility::position(startPos, input) + ", got \"" + dataTypeToString(args[0].type) + "\".");
    }

    std::string url = args[0].toString();

    Value result = Fetch::httpGet(url, "TEXT");
    if (result.type == DataType::STRING) {
        return result;
    } else {
        return stringToValue(result.toString());
    }
}

Value Parser::functionJUSTC(const std::vector<Value>& args) { return Value(); }
Value Parser::functionHTTPJUSTC(const std::vector<Value>& args) { return Value(); }
Value Parser::functionPARSEJUSTC(const std::vector<Value>& args) { return Value(); }
Value Parser::functionPARSEJSON(const std::vector<Value>& args) { return Value(); }
Value Parser::functionFILE(const std::vector<Value>& args) { return Value(); }
Value Parser::functionSTAT(const std::vector<Value>& args) { return Value(); }
Value Parser::functionENV(const std::vector<Value>& args) { return Value(); }
Value Parser::functionCONFIG(const std::vector<Value>& args) { return Value(); }

Value Parser::functionV(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(std::sqrt(args[0].toNumber()));
}

Value Parser::functionD(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(args[0].toNumber() * 2);
}

Value Parser::functionSQ(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    double num = args[0].toNumber();
    return numberToValue(num * num);
}

Value Parser::functionCU(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    double num = args[0].toNumber();
    return numberToValue(num * num * num);
}

Value Parser::functionP(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(1);
    return numberToValue(args[0].toNumber() + 1);
}

Value Parser::functionM(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(-1);
    return numberToValue(args[0].toNumber() - 1);
}

Value Parser::functionS(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(std::sin(args[0].toNumber()));
}

Value Parser::functionC(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(1);
    return numberToValue(std::cos(args[0].toNumber()));
}

Value Parser::functionT(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(std::tan(args[0].toNumber()));
}

Value Parser::functionN(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(-args[0].toNumber());
}

Value Parser::functionABSOLUTE(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(std::abs(args[0].toNumber()));
}

Value Parser::functionCEIL(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(std::ceil(args[0].toNumber()));
}

Value Parser::functionFLOOR(const std::vector<Value>& args) {
    if (args.empty()) return numberToValue(0);
    return numberToValue(std::floor(args[0].toNumber()));
}

Value Parser::stringToValue(const std::string& str) {
    Value result;
    result.type = DataType::STRING;
    result.string_value = str;
    result.name = "\"" + str + "\"";
    return result;
}

Value Parser::numberToValue(double num) {
    Value result;
    result.type = DataType::NUMBER;
    result.number_value = num;
    result.name = num;
    return result;
}

Value Parser::booleanToValue(bool b) {
    Value result;
    result.type = DataType::BOOLEAN;
    result.boolean_value = b;
    result.name = b;
    return result;
}

Value Parser::linkToValue(const std::string& link) {
    Value result;
    result.type = DataType::LINK;
    result.string_value = link;
    result.name = "<" + link + ">";
    return result;
}

Value Parser::pathToValue(const std::string& path) {
    Value result;
    result.type = DataType::PATH;
    result.string_value = path;
    result.name = path;
    return result;
}

Value Parser::hexToValue(const std::string& hexStr) {
    Value result;
    result.type = DataType::HEXADECIMAL;

    std::string cleanHex = hexStr;
    if (!cleanHex.empty() && (cleanHex[0] == '#' || cleanHex[0] == 'x' || cleanHex[0] == 'X')) {
        cleanHex = cleanHex.substr(1);
    }

    try {
        unsigned int num;
        std::stringstream ss;
        ss << std::hex << cleanHex;
        ss >> num;
        result.number_value = static_cast<double>(num);
    } catch (...) {
        result.number_value = 0.0;
    }

    result.name = Utility::double2hexString(result.number_value);
    return result;
}

Value Parser::binaryToValue(const std::string& binStr) {
    Value result;
    result.type = DataType::BINARY;

    std::string cleanBin = binStr;
    if (!cleanBin.empty() && (cleanBin[0] == 'b' || cleanBin[0] == 'B')) {
        cleanBin = cleanBin.substr(1);
    }

    try {
        unsigned int num = std::stoi(cleanBin, nullptr, 2);
        result.number_value = static_cast<double>(num);
    } catch (...) {
        result.number_value = 0.0;
    }

    result.name = Utility::double2binString(result.number_value);
    return result;
}

Value Parser::octalToValue(const std::string& octStr) {
    Value result;
    result.type = DataType::OCTAL;

    std::string cleanOct = octStr;
    if (!cleanOct.empty() && (cleanOct[0] == 'o' || cleanOct[0] == 'O')) {
        cleanOct = cleanOct.substr(1);
    }

    try {
        unsigned int num = std::stoi(cleanOct, nullptr, 8);
        result.number_value = static_cast<double>(num);
    } catch (...) {
        result.number_value = 0.0;
    }

    result.name = Utility::double2octString(result.number_value);
    return result;
}

void Parser::evaluateAllVariablesSync() {
    bool changed;
    int passes = 0;
    const int MAX_PASSES = 100;

    do {
        changed = false;
        passes++;
        std::vector<std::string> vars;

        for (auto& node : ast) {
            if (node.type == "VARIABLE_DECLARATION") {
                std::string varName = node.identifier;
                Value oldValue = variables[varName];
                Value newValue = evaluateASTNode(node);
                if (std::find(vars.begin(), vars.end(), varName) != vars.end()) {
                    throw std::runtime_error("Attempt to redefine \"" + varName + "\" at " + Utility::position(node.startPos, input) + ".");
                }
                vars.push_back(varName);

                if (newValue.type != DataType::UNKNOWN &&
                    (oldValue.type == DataType::UNKNOWN ||
                     oldValue.toString() != newValue.toString())) {
                    variables[varName] = newValue;
                    changed = true;
                }
            }
        }

    } while (changed && passes < MAX_PASSES);

    if (passes >= MAX_PASSES) {
        throw std::runtime_error("Cannot resolve variable dependencies - possible circular reference");
    }
}

void Parser::evaluateAllVariablesAsync() {
#ifndef __EMSCRIPTEN__
    std::unordered_map<std::string, std::future<Value>> futures;

    for (auto& node : ast) {
        if (node.type == "VARIABLE_DECLARATION") {
            std::string varName = node.identifier;
            if (dependencies[varName].empty()) {
                futures[varName] = executeAsyncIfEnabled([this, node]() {
                    return evaluateASTNode(node);
                });
            }
        }
    }

    for (auto it = futures.begin(); it != futures.end(); ++it) {
        variables[it->first] = it->second.get();
    }

    evaluateAllVariablesSync();
#else
    evaluateAllVariablesSync();
#endif
}

ParseResult Parser::parseTokens(const std::vector<ParserToken>& tokens, bool doExecute, bool runAsync, const std::string& input, const bool allowJavaScript, const bool canAllowJS, const std::string scriptName, const std::string scriptType) {
    Parser parser(tokens, doExecute, runAsync, input, allowJavaScript, canAllowJS, scriptName, scriptType);
    return parser.parse(doExecute);
}
