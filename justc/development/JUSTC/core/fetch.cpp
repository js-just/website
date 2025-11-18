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
#include <string>
#include <algorithm>
#include <cctype>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>

using namespace emscripten;

long getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

std::pair<std::string, std::pair<std::string, std::string>> Fetch::executeHttpRequest(const std::string& url, const std::string& method, const std::string& body, const std::unordered_map<std::string, std::string>& headers) {
    val global = val::global();
    val XMLHttpRequest = global["XMLHttpRequest"];
    val xhr = XMLHttpRequest.new_();

    xhr.call<void>("open", val(method), val(url), val(false));

    xhr.call<void>("setRequestHeader", val("Accept"), val("*/*"));
    xhr.call<void>("setRequestHeader", val("X-JUSTC"), val("JUSTC/" + JUSTC_VERSION));

    for (const auto& pair : headers) {
        std::string key_lower = pair.first;
        std::transform(key_lower.begin(), key_lower.end(), key_lower.begin(), ::tolower);

        if (key_lower == "user-agent" || key_lower == "x-justc") {
            throw std::runtime_error("HTTP: Attempt to set \"" + pair.first + "\" header.");
        }

        try {
            xhr.call<void>("setRequestHeader", val(pair.first), val(pair.second));
        } catch (const std::exception& e) {
            std::cerr << "[JUSTC] (" << Parser::getCurrentTimestamp() << ") HTTP: Failed to set header "
                      << pair.first << " with value " << pair.second << ": " << e.what() << std::endl;
        }
    }

    if (body.empty()) {
        xhr.call<void>("send");
    } else {
        xhr.call<void>("send", val(body));
    }

    int status = xhr["status"].as<int>();
    std::string response_text = xhr["responseText"].as<std::string>();

    std::string response_headers_str;
    val headers_str = xhr.call<val>("getAllResponseHeaders");
    if (!headers_str.isUndefined() && !headers_str.isNull()) {
        response_headers_str = headers_str.as<std::string>();
    }

    if (status >= 400 && status < 600 && method != "HEAD") {
        if (response_text.empty()) {
            throw std::runtime_error("HTTP: Request failed with status " + std::to_string(status));
        } else {
            std::cerr << "[JUSTC] (" << Parser::getCurrentTimestamp() << ") HTTP: Request succeeded, but with status "
                      << status << std::endl;
        }
    }

    return std::make_pair(response_text, std::make_pair(std::to_string(status), response_headers_str));
}

#else
#include <cpr/cpr.h>

std::pair<std::string, std::pair<std::string, std::string>> Fetch::executeHttpRequest(const std::string& url, const std::string& method, const std::string& body, const std::unordered_map<std::string, std::string>& headers) {
    cpr::Header reqHeaders = {
        {"User-Agent", "JUSTC/" + JUSTC_VERSION},
        {"Accept", "*/*"},
        {"X-JUSTC", "JUSTC/" + JUSTC_VERSION}
    };

    for (const auto& pair : headers) {
        std::string pairfirst = pair.first;
        std::string pf;
        pf.resize(pairfirst.size());
        std::transform(pairfirst.begin(), pairfirst.end(),
                pf.begin(),
                [](unsigned char c){ return std::tolower(c); });
        if (pf == "user-agent" || pf == "x-justc") throw std::runtime_error("HTTP: Attempt to set \"" + pair.first + "\" header.");
        reqHeaders[pair.first] = pair.second;
    }

    cpr::Response response;

    if (method == "GET") {
        response = cpr::Get(
            cpr::Url{url},
            reqHeaders,
            cpr::Timeout{10000}
        );
    } else if (method == "POST") {
        response = cpr::Post(
            cpr::Url{url},
            reqHeaders,
            cpr::Body{body},
            cpr::Timeout{10000}
        );
    } else if (method == "PUT") {
        response = cpr::Put(
            cpr::Url{url},
            reqHeaders,
            cpr::Body{body},
            cpr::Timeout{10000}
        );
    } else if (method == "PATCH") {
        response = cpr::Patch(
            cpr::Url{url},
            reqHeaders,
            cpr::Body{body},
            cpr::Timeout{10000}
        );
    } else if (method == "DELETE") {
        response = cpr::Delete(
            cpr::Url{url},
            reqHeaders,
            cpr::Timeout{10000}
        );
    } else if (method == "HEAD") {
        response = cpr::Head(
            cpr::Url{url},
            reqHeaders,
            cpr::Timeout{10000}
        );
    } else if (method == "OPTIONS") {
        response = cpr::Options(
            cpr::Url{url},
            reqHeaders,
            cpr::Timeout{10000}
        );
    } else {
        throw std::runtime_error("HTTP: Invalid method.");
    }

    if (response.status_code >= 400 && response.status_code < 600 && method != "HEAD") {
        if (response.text.empty()) {
            throw std::runtime_error("HTTP: Request failed with status " + std::to_string(response.status_code));
        } else {
            std::cerr << "[JUSTC] HTTP: Request succeeded, but with status " << response.status_code << std::endl;
        }
    }

    if (response.error) {
        throw std::runtime_error("HTTP: Request failed: " + response.error.message);
    }

    std::string responseHeaders;
    for (const auto& header : response.header) {
        responseHeaders += header.first + ":" + header.second + "\n";
    }
    std::pair<std::string, std::pair<std::string, std::string>> output(response.text, std::make_pair(std::to_string(response.status_code), responseHeaders));
    return output;
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

Value Fetch::request(const std::string& url, const std::string& method, const std::unordered_map<std::string, std::string>& headers, const std::string& body) {
    Value result;

    try {
        auto content = executeHttpRequest(url, method, body, headers);

        result.type = DataType::JUSTC_OBJECT;
        result.object_value = std::unordered_map<std::string, Value>{
            {"text", Value(DataType::STRING, content.first)},
            {"status", Value(DataType::STRING, content.second.first)},
            {"headers", Value(DataType::STRING, content.second.second)}
        };
        result.name = "HTTP.Response";
    } catch (const std::exception& e) {
        throw std::runtime_error("HTTP request failed: " + std::string(e.what()));
    }

    return result;
}
