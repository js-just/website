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

#include "to.xml.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "parser.h"
#include <cmath>
#include "utility.h"
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "to.json.h"
#endif

std::string XmlSerializer::escapeXmlString(const std::string& str) {
    std::stringstream ss;
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        switch (c) {
            case  '&': ss << "&amp;";  break;
            case  '<': ss << "&lt;";   break;
            case  '>': ss << "&gt;";   break;
            case  '"': ss << "&quot;"; break;
            case '\'': ss << "&apos;"; break;
            default:
                if (static_cast<unsigned char>(c) >= 0x20 || c == '\n' || c == '\r' || c == '\t') {
                    ss << c;
                }
                break;
        }

        if (i > 100000) {
            #ifdef __EMSCRIPTEN__
            EM_ASM({
                console.warn("[JUSTC] (" + $0 + ") string is too long. It will be truncated in the XML output.");
            }, Parser::getCurrentTimestamp().c_str());
            #else
            std::cout << "JUSTC: Warning: string is too long. It will be truncated in the XML output." << std::endl;
            #endif
            break;
        }
    }
    return ss.str();
}

std::string XmlSerializer::valueToXml(const Value& value) {
    switch (value.type) {
        case DataType::NUMBER:
        case DataType::HEXADECIMAL:
        case DataType::BINARY:
        case DataType::OCTAL:
            return Utility::numberValue2string(value);
        case DataType::STRING:
        case DataType::LINK:
        case DataType::PATH:
        case DataType::VARIABLE:
            return escapeXmlString(value.string_value);
        case DataType::BOOLEAN:
            return value.boolean_value ? "true" : "false";
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

std::string XmlSerializer::tokensToXml(const std::vector<ParserToken>& tokens) {
    std::stringstream xml;
    xml << "<tokens>";

    for (const auto& token : tokens) {
        xml << "<token>";
        xml << "<type>" << escapeXmlString(token.type) << "</type>";
        xml << "<value>" << escapeXmlString(token.value) << "</value>";
        xml << "<start>" << token.start << "</start>";
        xml << "</token>";
    }

    xml << "</tokens>";
    return xml.str();
}

std::string XmlSerializer::serialize(const ParseResult& result) {
    std::stringstream xml;

    #ifdef __EMSCRIPTEN__

    if (!result.error.empty()) {
        return "{\"error\":\"" + JsonSerializer::escapeJsonString(result.error) + "\"}";
    } else {
        std::stringstream json;
        json << "{";

        std::stringstream valuesXml;
        valuesXml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        valuesXml << "<justc version=\"" << escapeXmlString(JUSTC_VERSION) << "\">";
        for (const auto& pair : result.returnValues) {
            valuesXml << "<" << escapeXmlString(pair.first) << ">";
            valuesXml << valueToXml(pair.second);
            valuesXml << "</" << escapeXmlString(pair.first) << ">";
        }
        valuesXml << "</justc>";

        json << "\"type\":\"xml\",\"return\":\"" << JsonSerializer::escapeJsonString(valuesXml.str()) << "\",";
        json << "\"logs\":" << JsonSerializer::serialize(result.logs) << ",";

        // logfile object
        json << "\"logfile\":{";
        json << "\"file\":\"" << JsonSerializer::escapeJsonString(result.logFilePath) << "\",";
        json << "\"logs\":\"" << JsonSerializer::escapeJsonString(result.logFileContent) << "\"";
        json << "},";

        // import logs array
        json << "\"imported\":";
        json << JsonSerializer::serialize(result.importLogs);

        json << "}";
        return json.str();
    }

    #else

    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml << "<justc version=\"" << escapeXmlString(JUSTC_VERSION) << "\">";

    for (const auto& pair : result.returnValues) {
        xml << "<" << escapeXmlString(pair.first) << ">";
        xml << valueToXml(pair.second);
        xml << "</" << escapeXmlString(pair.first) << ">";
    }

    xml << "</justc>";
    return xml.str();

    #endif
}

std::string XmlSerializer::serialize(const std::vector<ParserToken>& tokens, const std::string& input) {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml << "<justc outputType=\"lexer\" version=\"" << escapeXmlString(JUSTC_VERSION) << "\">";
    xml << "<input>" << escapeXmlString(input) << "</input>";
    xml << tokensToXml(tokens);
    xml << "</justc>";
    return xml.str();
}
