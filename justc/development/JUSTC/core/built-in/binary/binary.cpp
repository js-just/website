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

#include "binary.hpp"
#include "../../parser.h"

Value Binary::ToText(const std::vector<Value>& args) {
        if (args.empty() || args[0].type != DataType::BINARY_DATA) {
        throw std::runtime_error("Binary::ToText requires Binary Data as first argument");
    }

    const auto& binary = args[0].binary_data;
    std::string encoding = "utf-8";

    if (args.size() > 1) {
        encoding = args[1].toString();
    }

    std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);

    try {
        if (encoding == "utf-8" || encoding == "utf8") {
            std::string result(binary.begin(), binary.end());
            for (size_t i = 0; i < result.size(); i++) {
                unsigned char c = result[i];
                if (c > 127) { // validate sequence
                    if ((c & 0xE0) == 0xC0) { // 2-byte
                        if (i + 1 >= result.size() || (result[i+1] & 0xC0) != 0x80) {
                            throw std::runtime_error("Invalid UTF-8 sequence");
                        }
                        i++;
                    } else if ((c & 0xF0) == 0xE0) { // 3-byte
                        if (i + 2 >= result.size() || (result[i+1] & 0xC0) != 0x80 ||
                            (result[i+2] & 0xC0) != 0x80) {
                            throw std::runtime_error("Invalid UTF-8 sequence");
                        }
                        i += 2;
                    } else if ((c & 0xF8) == 0xF0) { // 4-byte
                        if (i + 3 >= result.size() || (result[i+1] & 0xC0) != 0x80 ||
                            (result[i+2] & 0xC0) != 0x80 || (result[i+3] & 0xC0) != 0x80) {
                            throw std::runtime_error("Invalid UTF-8 sequence");
                        }
                        i += 3;
                    } else {
                        throw std::runtime_error("Invalid UTF-8 sequence");
                    }
                }
            }
            return Parser::stringToValue(result);
        } else if (encoding == "ascii") {
            std::string result;
            for (unsigned char c : binary) {
                if (c > 127) {
                    throw std::runtime_error("Non-ASCII character in binary data");
                }
                result.push_back(static_cast<char>(c));
            }
            return Parser::stringToValue(result);
        } else if (encoding == "base64") {
            const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            std::string result;
            int i = 0;
            int j = 0;
            unsigned char char_array_3[3];
            unsigned char char_array_4[4];

            for (unsigned char c : binary) {
                char_array_3[i++] = c;
                if (i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for (i = 0; i < 4; i++) {
                        result += base64_chars[char_array_4[i]];
                    }
                    i = 0;
                }
            }

            if (i > 0) {
                for (j = i; j < 3; j++) {
                    char_array_3[j] = '\0';
                }

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; j < i + 1; j++) {
                    result += base64_chars[char_array_4[j]];
                }

                while (i++ < 3) {
                    result += '=';
                }
            }

            return Parser::stringToValue(result);
        } else {
            throw std::runtime_error("Unsupported encoding: " + encoding);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Binary::ToText: " + std::string(e.what()));
    }
}

Value Binary::FromText(const std::vector<Value>& args) {
    if (args.empty()) {
        throw std::runtime_error("Binary::FromText requires at least one argument");
    }

    std::string text = args[0].toString();
    std::string encoding = "utf-8";

    if (args.size() > 1) {
        encoding = args[1].toString();
    }

    std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);

    try {
        std::vector<unsigned char> binary;

        if (encoding == "utf-8" || encoding == "utf8") {
            binary.assign(text.begin(), text.end());
        } else if (encoding == "ascii") {
            for (char c : text) {
                if (static_cast<unsigned char>(c) > 127) {
                    throw std::runtime_error("Non-ASCII character in text");
                }
                binary.push_back(static_cast<unsigned char>(c));
            }
        } else if (encoding == "base64") {
            const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            auto is_base64 = [&](unsigned char c) {
                return (isalnum(c) || (c == '+') || (c == '/'));
            };

            size_t in_len = text.size();
            size_t i = 0;
            size_t j = 0;
            int in_ = 0;
            unsigned char char_array_4[4], char_array_3[3];

            while (in_len-- && (text[in_] != '=') && is_base64(text[in_])) {
                char_array_4[i++] = text[in_]; in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++) {
                        char_array_4[i] = static_cast<unsigned char>(
                            base64_chars.find(char_array_4[i]));
                    }

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; i < 3; i++) {
                        binary.push_back(char_array_3[i]);
                    }
                    i = 0;
                }
            }

            if (i > 0) {
                for (j = i; j < 4; j++) {
                    char_array_4[j] = 0;
                }

                for (j = 0; j < 4; j++) {
                    char_array_4[j] = static_cast<unsigned char>(
                        base64_chars.find(char_array_4[j]));
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; j < i - 1; j++) {
                    binary.push_back(char_array_3[j]);
                }
            }
        } else {
            throw std::runtime_error("Unsupported encoding: " + encoding);
        }

        return Value::createBinaryData(binary);
    } catch (const std::exception& e) {
        throw std::runtime_error("Binary::FromText: " + std::string(e.what()));
    }
}

Value Binary::ToDataURL(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != DataType::BINARY_DATA) {
        throw std::runtime_error("Binary::ToDataURL requires binary data as first argument");
    }

    const auto& binary = args[0].binary_data;
    std::string mime_type = "application/octet-stream";

    if (args.size() > 1) {
        mime_type = args[1].toString();
    }

    try {
        Value base64Value = Binary::ToText({args[0], Value::createString("base64")});
        std::string base64 = base64Value.toString();

        std::string data_url = "data:" + mime_type + ";base64," + base64;
        return Parser::stringToValue(data_url);
    } catch (const std::exception& e) {
        throw std::runtime_error("Binary::ToDataURL: " + std::string(e.what()));
    }
}

Value Binary::FromDataURL(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != DataType::LINK) {
        throw std::runtime_error("Binary::FromDataURL requires a data Link argument");
    }

    std::string data_url = args[0].toString();

    if (data_url.substr(0, 5) != "data:") {
        throw std::runtime_error("Binary::FromDataURL: Not a valid data URL");
    }

    try {
        size_t base64_pos = data_url.find(";base64,");
        if (base64_pos == std::string::npos) {
            throw std::runtime_error("Data URL must be base64 encoded");
        }

        std::string base64_data = data_url.substr(base64_pos + 8);

        Value binaryValue = Binary::FromText({
            Value::createString(base64_data),
            Value::createString("base64")
        });

        return binaryValue;
    } catch (const std::exception& e) {
        throw std::runtime_error("Binary::FromDataURL: " + std::string(e.what()));
    }
}

Value Binary::Data(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != DataType::BINARY) {
        throw std::runtime_error("Binary::Data requires a data Binary Number argument");
    }

    try {
        double num = args[0].toNumber();

        size_t byteCount = 4;
        if (args.size() > 1) {
            byteCount = static_cast<size_t>(args[1].toNumber());
            if (byteCount < 1 || byteCount > 8) {
                throw std::runtime_error("Byte count must be between 1 and 8");
            }
        }

        unsigned long long intValue;
        if (byteCount <= 4) {
            intValue = static_cast<unsigned int>(num);
        } else {
            intValue = static_cast<unsigned long long>(num);
        }

        std::vector<unsigned char> binary;
        for (size_t i = 0; i < byteCount; i++) {
            unsigned char byte = static_cast<unsigned char>((intValue >> (8 * (byteCount - i - 1))) & 0xFF);
            binary.push_back(byte);
        }

        return Value::createBinaryData(binary);

    } catch (const std::exception& e) {
        throw std::runtime_error("Binary::Data: " + std::string(e.what()));
    }
}
