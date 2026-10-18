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

#ifndef JUSTC_TYPES_H
#define JUSTC_TYPES_H

#include <variant>
#include <vector>
#include <string>
#include <map>

namespace JUSTC {

struct Value;

using Object = std::map<std::string, Value>;
using Array = std::vector<Value>;

struct Value {
    std::variant<
        std::monostate,
        bool,
        double,
        std::string,
        Array,
        Object
    > data;

    Value() = default;

    Value(bool v) : data(v) {}
    Value(int v) : data((double)v) {}
    Value(double v) : data(v) {}
    Value(const char* v) : data(std::string(v)) {}
    Value(const std::string& v) : data(v) {}
    Value(const Array& v) : data(v) {}
    Value(const Object& v) : data(v) {}

    bool isNull() const {
        return std::holds_alternative<std::monostate>(data);
    }

    bool isBool() const {
        return std::holds_alternative<bool>(data);
    }

    bool isNumber() const {
        return std::holds_alternative<double>(data);
    }

    bool isString() const {
        return std::holds_alternative<std::string>(data);
    }

    bool isArray() const {
        return std::holds_alternative<Array>(data);
    }

    bool isObject() const {
        return std::holds_alternative<Object>(data);
    }

    template<typename T>
    const T& get() const {
        return std::get<T>(data);
    }
};

}

#endif
