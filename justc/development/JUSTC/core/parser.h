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

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <cstdint>
#include <variant>
#include "lexer.h"
#include "version.h"
#include <functional>

struct Value;
class Parser;

struct ObjectContext {
    std::shared_ptr<Parser> parser;
    std::string outputMode;
    std::vector<std::string> outputVariables;
    std::unordered_map<std::string, Value> variables;
    bool allowJavaScript;
    bool allowLuau;

    std::shared_ptr<ObjectContext> parent;
    std::unordered_map<std::string, std::shared_ptr<ObjectContext>> childObjects;
};

enum class DataType {
    JUSTC_OBJECT =  0,
    NUMBER       =  1,
    STRING       =  2,
    LINK         =  3,
    BOOLEAN      =  4,
    JSON_OBJECT  =  5,
    JSON_ARRAY   =  6,
    NULL_TYPE    =  7,
    HEXADECIMAL  =  9,
    BINARY       = 11,
    PATH         = 12,
    ERROR        = 13,
    VARIABLE     = 14,
    FUNCTION     = 15,
    NOT_A_NUMBER = 17,
    INFINITE     = 18,
    SYNTAX_ERROR = 19,
    OCTAL        = 20,
    CLASS        = 21,
    SPACE        = 22,
    BINARY_DATA  = 23,
    BIGNUM       = 24,
    INTEGER      = 25,
    BASE64       = 26,
    HTTP_ERROR   = 27,
    UNKNOWN      =-1
};

inline std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::JUSTC_OBJECT: return "Object";
        case DataType::NUMBER:       return "Number";
        case DataType::STRING:       return "String";
        case DataType::LINK:         return "Link";
        case DataType::BOOLEAN:      return "Boolean";
        case DataType::JSON_OBJECT:  return "Object";
        case DataType::JSON_ARRAY:   return "Array";
        case DataType::NULL_TYPE:    return "Null";
        case DataType::HEXADECIMAL:  return "Hexadecimal number";
        case DataType::BINARY:       return "Binary number";
        case DataType::PATH:         return "Path";
        case DataType::ERROR:        return "Error";
        case DataType::VARIABLE:     return "Variable";
        case DataType::FUNCTION:     return "Function";
        case DataType::NOT_A_NUMBER: return "NaN";
        case DataType::INFINITE:     return "Infinity";
        case DataType::SYNTAX_ERROR: return "Syntax Error";
        case DataType::OCTAL:        return "Octal number";
        case DataType::CLASS:        return "Class";
        case DataType::SPACE:        return "Space";
        case DataType::BINARY_DATA:  return "Data";
        case DataType::BIGNUM:       return "Big number";
        case DataType::INTEGER:      return "Integer";
        case DataType::BASE64:       return "Base64 number";
        case DataType::HTTP_ERROR:   return "HTTP Error";
        case DataType::UNKNOWN:      return "unknown";
        default:                     return "invalid";
    }
};

struct FunctionInfo {
    std::string code;
    std::vector<std::string> paramNames;
    std::vector<DataType> paramTypes;
    std::vector<struct Value> defaultValues;
    bool hasVarArgs;
    bool isIsolated;

    FunctionInfo() : hasVarArgs(false), isIsolated(false) {}
};

struct Value {
    DataType type;

    union {
        double number_value;
        bool boolean_value;
    };
    std::string string_value;
    std::shared_ptr<void> complex_value;
    std::string name;
    std::unordered_map<std::string, Value> object_value;
    std::vector<unsigned char> binary_data;

    std::shared_ptr<ObjectContext> object_context;
    std::unordered_map<std::string, Value> properties;
    std::vector<Value> array_elements;
    DataType object_type;

    FunctionInfo function_info;
    std::shared_ptr<ObjectContext> closure_context;

    bool native;

    Value() : type(DataType::UNKNOWN), number_value(0), name("unknown"), object_type(DataType::UNKNOWN), native(false) {}
    Value(DataType t) : type(t), number_value(0), name(dataTypeToString(t)), object_type(DataType::UNKNOWN), native(false) {}
    Value(DataType t, std::string s) : type(t), string_value(s), name(dataTypeToString(t)), object_type(DataType::UNKNOWN), native(false) {}

    std::string toString() const;
    double toNumber() const;
    bool toBoolean() const;

    std::wstring toWString() const {
        try {
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string_value);
        } catch (...) {
            return L"";
        }
    }

    bool isObject() const {
        return type == DataType::JUSTC_OBJECT ||
               type == DataType::JSON_OBJECT ||
               type == DataType::JSON_ARRAY;
    }
    Value* getProperty(const std::string& name) {
        if (properties.find(name) != properties.end()) {
            return &properties[name];
        }
        return nullptr;
    }
    Value* getArrayElement(size_t index) {
        if (type == DataType::JSON_ARRAY && index < array_elements.size()) {
            return &array_elements[index];
        }
        return nullptr;
    }

    Value getProperty(const std::string& name, Value placeholder) {
        if (properties.find(name) != properties.end()) {
            return properties[name];
        }
        return placeholder;
    }

    static Value createNumber(double num);
    static Value createString(const std::string& str);
    static Value createBoolean(bool b);
    static Value createNull();
    static Value createLink(const std::string& link);
    static Value createPath(const std::string& path);
    static Value createVariable(const std::string& varName);
    static Value createHexadecimal(double num);
    static Value createBinary(double num);
    static Value createOctal(double num);
    static Value createBinaryData(const std::vector<unsigned char>& data);
    static Value createJustcObject(const std::shared_ptr<ObjectContext>& context);
    static Value createJsonObject(const std::unordered_map<std::string, Value>& obj);
    static Value createJsonArray(const std::vector<Value>& arr);

    static Value createString(const std::wstring& wstr) {
        Value result;
        result.type = DataType::STRING;
        try {
            result.string_value = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
            result.name = "\"" + result.string_value + "\"";
        } catch (...) {
            result.string_value = "";
            result.name = "\"\"";
        }
        return result;
    }
};

struct LogEntry {
    std::string type;
    std::string message;
    size_t position;
    std::string timestamp;

    LogEntry(const std::string& t, const std::string& m, size_t p, const std::string& ts = "")
        : type(t), message(m), position(p), timestamp(ts) {}
};

struct ParseResult {
    std::unordered_map<std::string, Value> returnValues;
    std::vector<LogEntry> logs;
    std::string logFilePath;
    std::string logFileContent;
    std::string error;
    std::vector<std::vector<std::string>> importLogs;
    bool array;

    std::shared_ptr<std::unordered_map<std::string, Value>> variables;
    std::shared_ptr<std::unordered_map<std::string, bool>> constants;
    std::shared_ptr<std::unordered_map<std::string, std::vector<std::string>>> dependencies;

    ParseResult() : logFilePath(""), logFileContent(""), error(""), array(false) {}
};

struct JSONObject {
    std::unordered_map<std::string, Value> properties;
};

struct JSONArray {
    std::vector<Value> elements;
};

struct JUSTCObject {
    std::unordered_map<std::string, Value> variables;
    std::vector<std::string> outputOrder;
};

struct ASTNode {
    std::string type;
    std::string identifier;
    Value value;
    std::vector<std::string> references;
    std::vector<std::string> tokens;
    size_t startPos;
    DataType typeDeclaration;
    bool constant;
    bool local;

    ASTNode(const std::string& t, const std::string& id = "", size_t start = 0)
        : type(t), identifier(id), startPos(start), typeDeclaration(DataType::UNKNOWN), local(false) {}
};

enum class CharType {
    GRAPHEME  = 0,
    CODEPOINT = 1,
    BYTE      = 2
};

struct Mutated {
    Value value;
    size_t startPos;
    bool applied = false;

    Mutated() : value(), startPos(0), applied(false) {}
    Mutated(Value v, size_t p) : value(v), startPos(p), applied(false) {}
};

using Function = std::function<Value(const std::vector<Value>&)>;

class Parser {
private:
    bool doExecute;
    bool runAsync;
    std::vector<ParserToken> tokens;
    std::vector<ASTNode> ast;
    size_t position;
    std::string input;

    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, Mutated> mutated;
    std::unordered_map<std::string, bool> constVars;
    std::unordered_map<std::string, std::vector<std::string>> dependencies;
    std::vector<std::string> outputVariables;
    std::vector<std::string> outputNames;
    Value returnValue;
    std::string outputMode;
    bool allowJavaScript;
    bool globalScope;
    bool strictMode;
    bool canAllowJS;
    bool allowLuau;
    bool canAllowLuau;

    std::vector<LogEntry> logs;
    std::string logFilePath;
    std::string logFileContent;
    bool hasLogFile;

    std::string scriptName;
    std::string scriptType;

    bool asJSON;
    bool isJSONArray;
    std::string endOfScript;
    std::vector<Value> arrayItems;

    bool isFunction;

    CharType chartype;

    std::unordered_map<std::string, Function> userFunctions;
    std::unordered_map<std::string, bool> userFunctionsConst;
    std::vector<Function> variableUpdateListeners;

    ParserToken currentToken() const;
    ParserToken peekToken(size_t offset = 1) const;
    void advance();
    bool match(const std::string& type) const;
    bool match(const std::string& type, const std::string& value) const;
    bool isEnd() const;
    void skipCommas();

    std::string toLower(const std::string& str) const {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::vector<std::vector<std::string>> importLogs;

    std::unordered_map<uint64_t, std::unordered_map<std::string, Value>> localScopes;
    std::unordered_map<uint64_t, std::unordered_map<std::string, bool>> localConstVars;
    std::vector<uint64_t> scopeStack;
    uint64_t currentScope;
    uint64_t rootIndex;

    // logs
    void addLog(const std::string& type, const std::string& message, size_t position = 0);
    void setLogFile(const std::string& path);
    void appendToLogFile(const std::string& content);
    void addImportLog(const std::string& path, const std::string& script, const std::string& type);

    Value astNodeToValue(const ASTNode& node);

    Value parseExpression(bool doExecute, bool identifierMode = false);
    Value parsePrimary(bool doExecute);
    Value parseConditional(bool doExecute, bool identifierMode = false);
    Value parseBitwiseOR(bool doExecute, bool identifierMode = false);
    Value parseBitwiseXOR(bool doExecute, bool identifierMode);
    Value parseBitwiseAND(bool doExecute, bool identifierMode);
    Value parseBitwiseSHIFT(bool doExecute, bool identifierMode);
    Value parseBitwiseNOT(bool doExecute, bool identifierMode);
    Value parsePipeline(bool doExecute, bool identifierMode);
    Value parseElvisOrNullCoalescing(bool doExecute, bool identifierMode);
    Value parseLogicalOR(bool doExecute, bool identifierMode);
    Value parseLogicalXOR(bool doExecute, bool identifierMode);
    Value parseLogicalAND(bool doExecute, bool identifierMode);
    Value parseLogicalIMPLY(bool doExecute, bool identifierMode);
    Value parseEquality(bool doExecute, bool identifierMode);
    Value parseComparison(bool doExecute, bool identifierMode);
    Value parseTerm(bool doExecute, bool identifierMode);
    Value parseFactor(bool doExecute, bool identifierMode);
    Value parsePower(bool doExecute, bool identifierMode);
    Value parseUnary(bool doExecute, bool identifierMode = false);
    Value parseFunctionCall(bool doExecute);
    Value parseSpaceCall(bool doExecute);
    std::vector<Value> parseLambda(bool doExecute, size_t pos);

    ASTNode typeDeclarationNode(std::string typeDecl, size_t pos);

    Value parseJustcObject(bool doExecute);
    Value parseJsonObject(bool doExecute);
    Value parseJsonArray(bool doExecute);
    Value parseObjectPropertyAccess(bool doExecute);
    std::shared_ptr<ObjectContext> createObjectContext(bool inheritFromParent);

    Value accessProperty(const Value& obj, const std::string& propName);
    Value accessIndex(const Value& arr, size_t index);
    std::vector<Value> parseArguments(bool doExecute);

    std::string readVariableName();
    void checkVariableNameAvailable(std::string name);

    ASTNode parseStatement(bool doExecute);
    bool CanIgnoreNoAssigmentOperator();
    Value makeValue(Value value, bool b);
    ASTNode parseGlobal(bool doExecute, bool constant = false);
    ASTNode parseVariableDeclaration(bool doExecute, bool constant = false, bool local = false, bool global = false);
    ASTNode parseCommand(bool doExecute);
    ASTNode parseScopeCommand();
    ASTNode parseOutputCommand();
    ASTNode parseReturnCommand();
    ASTNode parseAllowCommand();
    ASTNode parseImportCommand();

    Value executeFunction(const std::string& funcName, const std::vector<Value>& args, size_t startPos);
    Value concatenateStrings(const Value& left, const Value& right);
    Value evaluateExpression(const Value& left, const std::string& op, const Value& right, bool doExecute);
    Value handleInequality(const Value& value);
    Value handleConditional(const Value& condition, const Value& thenVal, const Value& elseVal,
                           const std::string& thenOp, const std::string& elseOp);

    void buildDependencyGraph();
    bool detectCycles();
    bool dfsCycleDetection(const std::string& node,
                          std::unordered_map<std::string, bool>& visited,
                          std::unordered_map<std::string, bool>& recStack,
                          std::vector<std::string>& cyclePath);

    Value resolveVariableValue(const std::string& varName, const bool unknownIsString);
    void evaluateAllVariables();
    void evaluateAllVariablesSync();
    void evaluateAllVariablesAsync();
    std::runtime_error typeDeclarationError(const DataType left, const DataType right, const ASTNode node);
    Value applyTypeDeclaration(const Value value, const ASTNode node);
    Value evaluateASTNode(const ASTNode& node);
    void extractReferences(const Value& value, std::vector<std::string>& references);

    Value numberToValue(double num);
    Value booleanToValue(bool b);
    Value linkToValue(const std::string& link);
    Value pathToValue(const std::string& path);
    Value hexToValue(const std::string& hexStr);
    Value binaryToValue(const std::string& binStr);
    Value octalToValue(const std::string& octStr);

    Value convertToDecimal(const Value& value);

    template<class Func>
    auto executeAsyncIfEnabled(Func&& func) -> std::future<decltype(func())> {
        typedef decltype(func()) ResultType;

        if (runAsync) {
    #ifdef __EMSCRIPTEN__
            return std::async(std::launch::deferred, std::forward<Func>(func));
    #else
            return std::async(std::launch::async, std::forward<Func>(func));
    #endif
        } else {
            ResultType result = func();
            return std::async(std::launch::deferred, [result]() { return result; });
        }
    }

    Value isolated(const std::string& code, bool doExecute, size_t startPos, const std::unordered_map<std::string, Value>* context = nullptr, const std::string name = "auto", bool merge = false, bool silent = false);
    Value shared(const std::string& code, bool doExecute, size_t startPos, const std::unordered_map<std::string, Value>* context, const std::string name = "auto", bool merge = true, bool silent = false);

    Value parseFunctionDeclaration(bool doExecute, std::string funcName = "anonymous", bool requireName = true);
    Value emptyJUSTC();

    Value parseCondition(bool doExecute, bool wasIsolated = false);
    Value i2v(Value fromIsolated);
    std::string t2i(ParserToken toIsolated);

    // built-in
    std::future<Value> functionHTTPAsync(size_t startPos, const std::string& method, const std::vector<Value>& args);
    std::future<Value> functionFILEAsync(const std::vector<Value>& args);
    Value functionVALUE(const std::vector<Value>& args);
    Value functionSTRING(const std::vector<Value>& args);
    Value functionLINK(const std::vector<Value>& args);
    Value functionBINARY(const std::vector<Value>& args);
    Value functionOCTAL(const std::vector<Value>& args);
    Value functionHEXADECIMAL(const std::vector<Value>& args);
    Value functionTYPEID(const std::vector<Value>& args);
    Value functionTYPEOF(const std::vector<Value>& args);
    Value functionECHO(const std::vector<Value>& args);
    Value functionJSON(const std::vector<Value>& args);
    Value functionHTTP(size_t startPos, const std::string& method, const std::vector<Value>& args);
    Value functionJUSTC(const std::vector<Value>& args, size_t startPos);
    Value functionJUSTC2(const std::string& code, bool doExecute, size_t startPos);
    Value functionFILE(const std::vector<Value>& args);
    Value functionSTAT(const std::vector<Value>& args);
    Value functionENV(const std::vector<Value>& args);
    Value functionCONFIG(const std::vector<Value>& args);
    Value functionJUSTO(const std::vector<Value>& args);
    Value toJUSTO(const std::vector<Value>& args);

    Value callFunction(const Value& function, const std::vector<Value>& args, size_t startPos, bool doExecute);

    Value onExecDisabled(size_t startPos, std::string name);
    void parseOutputCommandError(const std::string mode);
    void parseScopeCommandError(const std::string scope);
    void parseAllowCommandError();

    Value evaluateLengthOperator(const Value& value);

    static std::vector<double> values2numbers(const std::vector<Value>& values) {
        std::vector<double> result;
        result.reserve(values.size());

        for (size_t i = 0; i < values.size(); ++i) {
            const auto& value = values[i];
            switch (value.type) {
                case DataType::NUMBER:
                case DataType::HEXADECIMAL:
                case DataType::BINARY:
                case DataType::OCTAL:
                    result.push_back(value.number_value);
                    break;
                default:
                    throw std::runtime_error(
                        "Expected number at argument " + std::to_string(i) +
                        ", got <" + dataTypeToString(value.type) + ">"
                    );
                    break;
            }
        }
        return result;
    }

    std::vector<std::string> builtins;
    void initializeBuiltIns();
    bool isBuiltinVariable(const std::string& name) const;
    void handleBuiltinVariableAssignment(const std::string& name, const Value& value, size_t startPos);
    void removeBuiltinVariablesFromOutput();
    void builtinObject(const std::string& name, std::unordered_map<std::string, Value> props);
    Value builtinObjectFunction(const std::string& name);

    void updateCharType(const std::string& newType, size_t startPos);

    void triggerVariableUpdate(const std::string& name, const Value& value);
    Value merger(const std::vector<Value>& args);

    std::string getCurrentScopeName() const;
    uint64_t getCurrentScope() const { return currentScope; }
    uint64_t getRootScope() const { return rootIndex; }
    void enterScope();
    void exitScope();
    void setLocal(uint64_t scope, const std::string& name, const Value& value, bool isConst = false);
    Value getLocal(uint64_t scope, const std::string& name) const;
    bool hasLocal(uint64_t scope, const std::string& name) const;
    bool isLocalConst(uint64_t scope, const std::string& name) const;
    Value resolveVariableValueWithScopes(const std::string& varName, const bool unknownIsString);

public:
    static std::string getCurrentTimestamp();
    static Value stringToValue(const std::string& str);
    Parser(const std::vector<ParserToken>& tokens, bool doExecute = true, bool runAsync = false, const std::string& input = "", const bool allowJavaScript = true, const bool canAllowJS = true, const std::string scriptName = "", const std::string scriptType = "script", const bool allowLuau = true, const bool canAllowLuau = true, const bool isFunction = false, const std::unordered_map<std::string, Value>* initialContext = nullptr, const CharType chartype = CharType::GRAPHEME);
    ParseResult parse(bool doExecute = true);
    static ParseResult parseTokens(const std::vector<ParserToken>& tokens, bool doExecute = true, bool runAsync = false, const std::string& input = "", const bool allowJavaScript = true, const bool canAllowJS = true, const std::string scriptName = "", const std::string scriptType = "script", const bool allowLuau = true, const bool canAllowLuau = true);

    void registerFunction(const std::string& name, Function func, bool isConst = true);
    void registerFunctions(const std::unordered_map<std::string, Function>& functions, bool isConst = true);
    void unregisterFunction(const std::string& name);
    bool hasFunction(const std::string& name) const;
    void clearUserFunctions();

    void variableUpdateListener(Function func);

    void registerGlobal(const std::string& name, const Value& value, bool isConst = true);
    Value getGlobal(const std::string& name);
    bool hasGlobal(const std::string& name);
    void unregisterGlobal(const std::string& name);
    void clearGlobals();

    Value ParseJUSTO(const std::string& code);
};

#endif
