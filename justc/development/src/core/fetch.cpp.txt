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

#include "fetch.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <sstream>
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

long getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

std::string Fetch::executeHttpRequest(const std::string& url) {
    char* result = (char*)EM_ASM_INT({
        return Asyncify.handleSleep(function(wakeUp) {
            try {
                var url = UTF8ToString($0);
                var version = UTF8ToString($1);
                var isBrowser = (typeof window !== 'undefined') && (typeof navigator !== 'undefined');

                var xhr = new XMLHttpRequest();
                xhr.open('GET', url, false);
                xhr.setRequestHeader('Accept', '*/*');
                if (!isBrowser) {
                    xhr.setRequestHeader('User-Agent', 'JUSTC/' + version);
                }
                xhr.setRequestHeader('X-JUSTC', 'JUSTC/' + version);

                xhr.onload = function() {
                    var response = xhr.responseText;
                    var length = lengthBytesUTF8(response) + 1;
                    var result = _malloc(length);
                    if (xhr.status >= 400 && xhr.status < 600) {
                        if ((length - 1) < 1) {
                            throw new Error("HTTP Request failed with status " + xhr.status + ".");
                        } else {
                            console.warn('[JUSTC] (' + (new Date()).toLocaleString('sv-SE', {
                                year: 'numeric',
                                month: '2-digit',
                                day: '2-digit',
                                hour: '2-digit',
                                minute: '2-digit',
                                second: '2-digit',
                                hour12: false
                            }) + ') HTTP Request succeeded, but with status', xhr.status);
                        }
                    }
                    stringToUTF8(response, result, length);
                    wakeUp(result);
                };

                xhr.onerror = function() {
                    throw new Error("HTTP Request failed with status " + xhr.status + ".");
                };

                xhr.send();
            } catch (e) {
                throw new Error("HTTP Request failed: " + e);
            }
        });
    }, url.c_str(), JUSTC_VERSION.c_str());

    std::string resultStr(result);
    free(result);

    return resultStr;
}

#else
#include <cpr/cpr.h>

std::string Fetch::executeHttpRequest(const std::string& url) {
    try {
        cpr::Response response = cpr::Get(
            cpr::Url{url},
            cpr::Header{
                {"User-Agent", "JUSTC/" + JUSTC_VERSION},
                {"Accept", "*/*"},
                {"X-JUSTC", "JUSTC/" + JUSTC_VERSION}
            },
            cpr::Timeout{10000}
        );

        if (response.status_code >= 400) {
            if (response.text.empty()) {
                throw std::runtime_error("HTTP Request failed with status " + std::to_string(response.status_code));
            } else {
                std::cerr << "[JUSTC] HTTP Request succeeded, but with status " << response.status_code << std::endl;
            }
        }

        if (response.error) {
            throw std::runtime_error("HTTP Request failed: " + response.error.message);
        }

        return response.text;

    } catch (const std::exception& e) {
        return "HTTP Error: " + std::string(e.what());
    }
}

#endif

void Fetch::processHttpRequests(const ParseResult& result) {
    for (auto& pair : result.returnValues) {
        if (pair.second.type == DataType::VARIABLE) {
            std::string varName = pair.second.string_value;
        }
        else if (pair.second.type == DataType::LINK) {
            std::string url = pair.second.string_value;
        }
    }
}

Value Fetch::fetchHttpContent(const std::string& url, const std::string& expectedType) {
    Value result;

    try {
        std::string content = executeHttpRequest(url);

        if (expectedType == "JSON" || expectedType == "HTTPJSON") {
            result.type = DataType::JSON_OBJECT;
            result.string_value = content;
        } else if (expectedType == "TEXT" || expectedType == "HTTPTEXT") {
            result.type = DataType::STRING;
            result.string_value = content;
        } else if (expectedType == "JUSTC" || expectedType == "HTTPJUSTC") {
            result.type = DataType::JUSTC_OBJECT;
            result.string_value = content;
        } else {
            result.type = DataType::STRING;
            result.string_value = content;
        }

    } catch (const std::exception& e) {
        std::runtime_error("HTTP request failed: " + std::string(e.what()));
    }

    return result;
}

Value Fetch::httpGet(const std::string& url, const std::string& format) {
    if (url == "test") {
        Value result;
        result.type = DataType::STRING;
        result.string_value = "Just an Ultimate Site Tool Configuration language";
        return result;
    }

    Value result = fetchHttpContent(url, format);
    return result;
}
