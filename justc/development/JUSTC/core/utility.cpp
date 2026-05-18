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

#ifdef __EMSCRIPTEN__

#include "utility.emscripten.h"
#include "../boost/multiprecision/cpp_dec_float.hpp"
#include "../boost/multiprecision/cpp_int.hpp"

#else

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#endif

namespace boost {
namespace multiprecision {
    template<typename Backend, expression_template_option ET>
    inline number<Backend, ET> fmod(const number<Backend, ET>& a, const number<Backend, ET>& b) {
        using value_type = number<Backend, ET>;
        if constexpr (std::is_floating_point<typename Backend::value_type>::value) {
            value_type int_part;
            return modf(a / b, &int_part) * b;
        }
        else {
            return a % b;
        }
    }
}
}

template<>
std::string Utility::numberToString<JUSTCnum>(const JUSTCnum& num) {
    return numToString(num);
}

std::string Utility::numberValue2string(const Value& value) {
    return std::visit([](auto&& num) {
        return numberToString(num);
    }, value.number_value);
}

std::string Utility::value2string(const Value& value) {
    switch (value.type) {
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
        case DataType::BIGNUM:
        case DataType::LARGENUM:
        case DataType::HUGENUM:
        case DataType::GIANTNUM:
        case DataType::COLOSSALNUM:
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
        left.type == DataType::OCTAL ||
        left.type == DataType::BIGNUM ||
        left.type == DataType::LARGENUM ||
        left.type == DataType::HUGENUM ||
        left.type == DataType::GIANTNUM ||
        left.type == DataType::COLOSSALNUM
    ) && (
        right.type == DataType::NUMBER ||
        right.type == DataType::HEXADECIMAL ||
        right.type == DataType::BINARY ||
        right.type == DataType::OCTAL ||
        right.type == DataType::BIGNUM ||
        right.type == DataType::LARGENUM ||
        right.type == DataType::HUGENUM ||
        right.type == DataType::GIANTNUM ||
        right.type == DataType::COLOSSALNUM
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
        { "number",      DataType::NUMBER       },     { "num",      DataType::NUMBER       },
        { "string",      DataType::STRING       },     { "str",      DataType::STRING       },
        { "boolean",     DataType::BOOLEAN      },     { "bool",     DataType::BOOLEAN      },
        { "null",        DataType::NULL_TYPE    },     { "nil",      DataType::NULL_TYPE    },
        { "link",        DataType::LINK         },
        { "path",        DataType::PATH         },
        { "binary",      DataType::BINARY       },     { "bin",      DataType::BINARY       },
        { "octal",       DataType::OCTAL        },     { "oct",      DataType::OCTAL        },
        { "hexadecimal", DataType::HEXADECIMAL  },     { "hex",      DataType::HEXADECIMAL  },
        { "object",      DataType::JUSTC_OBJECT },     { "obj",      DataType::JUSTC_OBJECT },
        { "json",        DataType::JSON_OBJECT  },
        { "array",       DataType::JSON_ARRAY   },
        { "nan",         DataType::NOT_A_NUMBER },
        { "infinity",    DataType::INFINITE     },     { "inf",      DataType::INFINITE     },
        { "data",        DataType::BINARY_DATA  },
        { "auto",        DataType::UNKNOWN      },
        { "bignum",      DataType::BIGNUM       },     { "big",      DataType::BIGNUM       },
        { "largenum",    DataType::LARGENUM     },     { "large",    DataType::LARGENUM     },
        { "hugenum",     DataType::HUGENUM      },     { "huge",     DataType::HUGENUM      },
        { "giantnum",    DataType::GIANTNUM     },     { "giant",    DataType::GIANTNUM     },
        { "colossalnum", DataType::COLOSSALNUM  },     { "colossal", DataType::COLOSSALNUM  },
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
        case DataType::BIGNUM:
        case DataType::LARGENUM:
        case DataType::HUGENUM:
        case DataType::GIANTNUM:
        case DataType::COLOSSALNUM:
            break;
        case DataType::BINARY:
            result.name = double2binString(numToDouble(value.number_value));
            break;
        case DataType::HEXADECIMAL:
            result.name = double2hexString(numToDouble(value.number_value));
            break;
        case DataType::OCTAL:
            result.name = double2octString(numToDouble(value.number_value));
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

double Utility::numToDouble(const JUSTCnum& num) {
    return std::visit([](auto&& arg) -> double {
        return static_cast<double>(arg);
    }, num);
}

bool Utility::numIsZero(const JUSTCnum& num) {
    return std::visit([](auto&& arg) -> bool {
        return arg == 0;
    }, num);
}

int Utility::numToInt(const JUSTCnum& num) {
    return std::visit([](auto&& arg) -> int {
        return static_cast<int>(arg);
    }, num);
}

JUSTCnum Utility::doubleToJUSTCnum(double num, DataType type) {
    switch (type) {
        case DataType::BIGNUM:
            return BigNum(num);
        case DataType::LARGENUM:
            return LargeNum(num);
        case DataType::HUGENUM:
            return HugeNum(num);
        case DataType::GIANTNUM:
            return GiantNum(num);
        case DataType::COLOSSALNUM:
            return ColossalNum(num);
        default:
            return num;
    }
}

std::string Utility::numToString(const JUSTCnum& num) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, double>) {
            std::ostringstream out;
            out << std::setprecision(std::numeric_limits<double>::max_digits10) << arg;
            std::string s = out.str();
            if (s.find('.') != std::string::npos) {
                s.erase(s.find_last_not_of('0') + 1);
                if (s.back() == '.') s.pop_back();
            }
            return s;
        } else {
            return arg.str();
        }
    }, num);
}

DataType Utility::getLargestType(DataType a, DataType b) {
    static const std::vector<DataType> typePriority = {
        DataType::NUMBER,
        DataType::BIGNUM,
        DataType::LARGENUM,
        DataType::HUGENUM,
        DataType::GIANTNUM,
        DataType::COLOSSALNUM
    };

    auto findPriority = [](DataType t) -> int {
        for (size_t i = 0; i < typePriority.size(); ++i) {
            if (typePriority[i] == t) return static_cast<int>(i);
        }
        return -1;
    };

    int prioA = findPriority(a);
    int prioB = findPriority(b);

    if (prioA >= prioB) return a;
    return b;
}

template<typename T, typename = std::enable_if_t<
    std::is_same_v<std::decay_t<T>, BigNum> ||
    std::is_same_v<std::decay_t<T>, LargeNum> ||
    std::is_same_v<std::decay_t<T>, HugeNum> ||
    std::is_same_v<std::decay_t<T>, GiantNum> ||
    std::is_same_v<std::decay_t<T>, ColossalNum>
>>
static JUSTCnum promoteToType(T&& value, DataType targetType) {
    using ValueType = std::decay_t<T>;
    ValueType evaluated = std::forward<T>(value);

    switch (targetType) {
        case DataType::NUMBER:
            return static_cast<double>(evaluated);
        case DataType::BIGNUM:
            return BigNum(evaluated);
        case DataType::LARGENUM:
            return LargeNum(evaluated);
        case DataType::HUGENUM:
            return HugeNum(evaluated);
        case DataType::GIANTNUM:
            return GiantNum(evaluated);
        case DataType::COLOSSALNUM:
            return ColossalNum(evaluated);
        default:
            return static_cast<double>(evaluated);
    }
}

JUSTCnum Utility::promoteToType(const JUSTCnum& num, DataType targetType) {
    return std::visit([targetType](auto&& value) -> JUSTCnum {
        using T = std::decay_t<decltype(value)>;

        if constexpr (!std::is_arithmetic_v<T>) {
            auto evaluated = T(value);
            switch (targetType) {
                case DataType::NUMBER:
                    return static_cast<double>(evaluated);
                case DataType::BIGNUM:
                    return BigNum(evaluated);
                case DataType::LARGENUM:
                    return LargeNum(evaluated);
                case DataType::HUGENUM:
                    return HugeNum(evaluated);
                case DataType::GIANTNUM:
                    return GiantNum(evaluated);
                case DataType::COLOSSALNUM:
                    return ColossalNum(evaluated);
                default:
                    return static_cast<double>(evaluated);
            }
        } else {
            switch (targetType) {
                case DataType::NUMBER:
                    return static_cast<double>(value);
                case DataType::BIGNUM:
                    return BigNum(value);
                case DataType::LARGENUM:
                    return LargeNum(value);
                case DataType::HUGENUM:
                    return HugeNum(value);
                case DataType::GIANTNUM:
                    return GiantNum(value);
                case DataType::COLOSSALNUM:
                    return ColossalNum(value);
                default:
                    return static_cast<double>(value);
            }
        }
    }, num);
}

template<typename Numeric>
static JUSTCnum promoteToType(Numeric value, DataType targetType) {
    switch (targetType) {
        case DataType::NUMBER: return static_cast<double>(value);
        case DataType::BIGNUM: return BigNum(value);
        case DataType::LARGENUM: return LargeNum(value);
        case DataType::HUGENUM: return HugeNum(value);
        case DataType::GIANTNUM: return GiantNum(value);
        case DataType::COLOSSALNUM: return ColossalNum(value);
        default: return static_cast<double>(value);
    }
}

template<typename Expr>
static JUSTCnum evaluateToType(Expr&& expr, DataType targetType) {
    switch (targetType) {
        case DataType::NUMBER:
            return static_cast<double>(expr);
        case DataType::BIGNUM:
            return BigNum(expr);
        case DataType::LARGENUM:
            return LargeNum(expr);
        case DataType::HUGENUM:
            return HugeNum(expr);
        case DataType::GIANTNUM:
            return GiantNum(expr);
        case DataType::COLOSSALNUM:
            return ColossalNum(expr);
        default:
            return static_cast<double>(expr);
    }
}

JUSTCnum Utility::add(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([resultType](auto&& x, auto&& y) -> JUSTCnum {
        return evaluateToType(x + y, resultType);
    }, aPromoted, bPromoted);
}

JUSTCnum Utility::subtract(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([resultType](auto&& x, auto&& y) -> JUSTCnum {
        return evaluateToType(x - y, resultType);
    }, aPromoted, bPromoted);
}

JUSTCnum Utility::multiply(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([resultType](auto&& x, auto&& y) -> JUSTCnum {
        return evaluateToType(x * y, resultType);
    }, aPromoted, bPromoted);
}

JUSTCnum Utility::divide(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([resultType](auto&& x, auto&& y) -> JUSTCnum {
        return evaluateToType(x / y, resultType);
    }, aPromoted, bPromoted);
}

JUSTCnum Utility::mod(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    auto visitor = [resultType](auto&& x, auto&& y) -> JUSTCnum {
        using T1 = std::decay_t<decltype(x)>;
        using T2 = std::decay_t<decltype(y)>;

        if constexpr (std::is_same_v<T1, T2>) {
            if constexpr (std::is_same_v<T1, double>) {
                double result = std::fmod(x, y);
                switch (resultType) {
                    case DataType::NUMBER: return result;
                    case DataType::BIGNUM: return BigNum(result);
                    case DataType::LARGENUM: return LargeNum(result);
                    case DataType::HUGENUM: return HugeNum(result);
                    case DataType::GIANTNUM: return GiantNum(result);
                    case DataType::COLOSSALNUM: return ColossalNum(result);
                    default: return result;
                }
            } else {
                auto result = boost::multiprecision::fmod(x, y);
                std::stringstream ss;
                ss << result;
                T1 evaluated;
                ss >> evaluated;
                switch (resultType) {
                    case DataType::NUMBER: return static_cast<double>(evaluated);
                    case DataType::BIGNUM: return BigNum(evaluated);
                    case DataType::LARGENUM: return LargeNum(evaluated);
                    case DataType::HUGENUM: return HugeNum(evaluated);
                    case DataType::GIANTNUM: return GiantNum(evaluated);
                    case DataType::COLOSSALNUM: return ColossalNum(evaluated);
                    default: return static_cast<double>(evaluated);
                }
            }
        } else {
            double xd = static_cast<double>(x);
            double yd = static_cast<double>(y);
            double result = std::fmod(xd, yd);
            switch (resultType) {
                case DataType::NUMBER: return result;
                case DataType::BIGNUM: return BigNum(result);
                case DataType::LARGENUM: return LargeNum(result);
                case DataType::HUGENUM: return HugeNum(result);
                case DataType::GIANTNUM: return GiantNum(result);
                case DataType::COLOSSALNUM: return ColossalNum(result);
                default: return result;
            }
        }
    };

    return std::visit(visitor, aPromoted, bPromoted);
}

JUSTCnum Utility::power(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([resultType](auto&& x, auto&& y) -> JUSTCnum {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, double>) {
            return evaluateToType(std::pow(x, static_cast<double>(y)), resultType);
        } else {
            return evaluateToType(boost::multiprecision::pow(x, y), resultType);
        }
    }, aPromoted, bPromoted);
}

JUSTCnum Utility::longToJUSTCnum(long value, DataType type) {
    switch (type) {
        case DataType::BIGNUM:
            return BigNum(value);
        case DataType::LARGENUM:
            return LargeNum(value);
        case DataType::HUGENUM:
            return HugeNum(value);
        case DataType::GIANTNUM:
            return GiantNum(value);
        case DataType::COLOSSALNUM:
            return ColossalNum(value);
        default:
            return static_cast<double>(value);
    }
}

bool Utility::equals(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([](auto&& x, auto&& y) -> bool {
        return x == y;
    }, aPromoted, bPromoted);
}
bool Utility::lessThan(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([](auto&& x, auto&& y) -> bool {
        return x < y;
    }, aPromoted, bPromoted);
}
bool Utility::greaterThan(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([](auto&& x, auto&& y) -> bool {
        return x > y;
    }, aPromoted, bPromoted);
}
bool Utility::lessOrEqual(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([](auto&& x, auto&& y) -> bool {
        return x <= y;
    }, aPromoted, bPromoted);
}
bool Utility::greaterOrEqual(const JUSTCnum& a, const JUSTCnum& b, DataType aType, DataType bType) {
    DataType resultType = getLargestType(aType, bType);
    JUSTCnum aPromoted = promoteToType(a, resultType);
    JUSTCnum bPromoted = promoteToType(b, resultType);

    return std::visit([](auto&& x, auto&& y) -> bool {
        return x >= y;
    }, aPromoted, bPromoted);
}
