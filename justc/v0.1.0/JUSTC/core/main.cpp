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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "json.hpp"
#include "version.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <tuple>
#include "utility.h"

void logError(const std::string error) {
    if (Utility::isGitHubActions()) std::cerr << "::error::" + error << std::endl;
    else std::cerr << error << std::endl;
}

void setupGlobalExceptionHandler() {
    std::set_terminate([]() {
        logError("Terminate called due to uncaught exception");

        auto current = std::current_exception();
        if (current) {
            try {
                std::rethrow_exception(current);
            } catch (const std::exception& e) {
                logError(std::string("Exception: ") + std::string(e.what()));
            } catch (...) {
                logError("Unknown exception type");
            }
        } else {
            logError("No current exception available");
        }

        std::abort();
    });
}

class OutputRedirector {
private:
    std::streambuf* originalCout;
    std::streambuf* originalCerr;
    std::stringstream silentBuffer;
    bool silent;
    bool redirectErrors;

public:
    OutputRedirector(bool silentMode, bool redirectErrors = false)
        : silent(silentMode), redirectErrors(redirectErrors) {
        if (silent) {
            originalCout = std::cout.rdbuf();
            std::cout.rdbuf(silentBuffer.rdbuf());

            if (redirectErrors) {
                originalCerr = std::cerr.rdbuf();
                std::cerr.rdbuf(silentBuffer.rdbuf());
            }
        }
    }

    ~OutputRedirector() {
        restore();
    }

    void restore() {
        if (silent) {
            std::cout.rdbuf(originalCout);
            if (redirectErrors) {
                std::cerr.rdbuf(originalCerr);
            }
        }
    }

    void outputError(const std::string& error) {
        if (silent) {
            restore();
            std::cerr << error << std::endl;
            std::cout.rdbuf(silentBuffer.rdbuf());
            if (redirectErrors) {
                std::cerr.rdbuf(silentBuffer.rdbuf());
            }
        } else {
            logError(std::string(error));
        }
    }
};

void printUsage() {
    std::cout << "" << std::endl;
    std::cout << "Just an Ultimate Site Tool Configuration language (JUSTC) v" + JUSTC_VERSION << std::endl;
    std::cout << "https://just.js.org/justc" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Usage: justc   [ flags ( [ arguments ] ) ]   [ input.justc | input.json ] ( [ output.json | output.justc | output.xml | output.yaml ] )" << std::endl;
    std::cout << "       justc <file.justc>               - Execute JUSTC file" << std::endl;
    std::cout << "       justc <file.justc> <file.json>   - Execute JUSTC file and output to JSON file" << std::endl;
    std::cout << "       justc -e \"<code>\"                - Execute JUSTC code" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Flags:" << std::endl;
    std::cout << "  -a, --async                           - Evaluate script variables asynchronously (may run faster)" << std::endl;
    std::cout << "  -e, --eval                            - Execute (Evaluate) script (not file)" << std::endl;
    std::cout << "  -E, --execute                         - Execute JUSTC from lexer output tokens as JSON" << std::endl;
    std::cout << "  -h, --help                            - Print JUSTC command line options (this list)" << std::endl;
    std::cout << "  -j, --json                            - Output as JSON (default)" << std::endl;
    std::cout << "  -J, --justc                           - Output as JUSTC, input should be JSON" << std::endl;
    std::cout << "  -l, --lexer                           - Run lexer only / Tokenize" << std::endl;
    std::cout << "  -P, --parser                          - Run parser only / Parse JUSTC (not execute) from lexer output tokens as JSON" << std::endl;
    std::cout << "  -p, --parse                           - Parse JUSTC (not execute) / No HTTP requests, some commands will not be executed" << std::endl;
    std::cout << "  -r, --result                          - Print result" << std::endl;
    std::cout << "  -s <commit sha>, --sha <commit sha>   - Set commit SHA" << std::endl;
    std::cout << "  -S, --silent                          - Suppress all output except errors and results" << std::endl;
    std::cout << "  -v, --version                         - Print JUSTC version" << std::endl;
    std::cout << "  -x, --xml                             - Output as XML" << std::endl;
    std::cout << "  -y, --yaml                            - Output as YAML" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "\"Run parser only\" means that JUSTC (will not be executed) will only be compiled to JSON - no logs and/or HTTP requests." << std::endl;
    std::cout << "\"Run lexer only\" means that JUSTC won't be executed and/or parsed, JUSTC will only be tokenized." << std::endl;
    std::cout << "" << std::endl;
}
void throwError(const std::string& error, OutputRedirector* redirector = nullptr) {
    if (Utility::isGitHubActions()) {
        if (redirector) {
            redirector->outputError("::error::" + error);
        } else {
            std::cerr << "::error::" << error << std::endl;
        }
    } else {
        if (redirector) {
            redirector->outputError(error);
        } else {
            std::cerr << error << std::endl;
        }
    }
    std::exit(1);
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throwError("Unable to read the file: " + filename);
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}
void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throwError("Unable to write the file: " + filename);
    }
    file << content;
    if (!file.good()) {
        throwError("Error occurred while writing to file: " + filename);
    }
}

struct cmdFlags {
    std::string mode = "execute";
    std::string outputMode = "json";
    std::string input;
    std::string output;
    bool outputToConsole = false;
    bool executeJUSTC = true;
    bool helpandorversionflag = false;
    bool lexerTokensToParser = false;
    bool asynchronously = false;
    bool gotFileOrCode = false;
    bool outputToFile = false;
    bool noInput = false;
    bool silentMode = false;

    bool waitingForVersion = false;
    bool waitingForConfig = false;
    bool waitingForCommitSHA = false;
    bool waitingForNav = false;
    bool waitingForPages = false;
    bool waitingForCSS = false;

    std::string justversion;
    std::string configpath;
    std::string commitSHA;
    std::string builtinvarNAV;
    std::string builtinvarPAGES;
    std::string builtinvarCSS;
};
template<typename... Args>
std::string outputString(cmdFlags flags, Args... args) {
    if (flags.outputMode == "xml") {
        return XmlSerializer::serialize(args...);
    } else if (flags.outputMode == "yaml") {
        return YamlSerializer::serialize(args...);
    } else {
        return JsonSerializer::serialize(args...);
    }
}
int main(int argc, char* argv[]) {
    setupGlobalExceptionHandler();

    if (argc < 1) {
        printUsage();
        return 1;
    }

    OutputRedirector* outputRedirector = nullptr;

    try {
        cmdFlags flags;

        // parse arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (flags.waitingForVersion) {
                flags.justversion = arg;
            }
            else if (flags.waitingForConfig) {
                flags.configpath = arg;
            }
            else if (flags.waitingForCommitSHA) {
                flags.commitSHA = arg;
            }
            else if (flags.waitingForNav) {
                flags.builtinvarNAV = arg;
            }
            else if (flags.waitingForPages) {
                flags.builtinvarPAGES = arg;
            }
            else if (flags.waitingForCSS) {
                flags.builtinvarCSS = arg;
            }

            else if (arg == "--help" || arg == "-h") {
                flags.helpandorversionflag = true;
                printUsage();
            }
            else if (arg == "--lexer" || arg == "-l") {
                flags.mode = "lexer";
            }
            else if (arg == "--result" || arg == "-r") {
                flags.outputToConsole = true;
            }
            else if ((arg == "-e" || arg == "--eval") && i + 1 < argc) {
                flags.input = argv[++i];
                flags.gotFileOrCode = true;
            }
            else if (arg[0] != '-' && flags.gotFileOrCode) {
                flags.output = arg;
                flags.outputToFile = true;
            }
            else if (arg[0] != '-') {
                flags.input = readFile(arg);
                flags.gotFileOrCode = true;
            }
            else if (arg == "--sha" || arg == "-s") {
                flags.waitingForCommitSHA = true;
            }
            else if (arg == "--version" || arg == "-v") {
                flags.helpandorversionflag = true;
                std::cout << JUSTC_VERSION << std::endl;
            }
            else if (arg == "--parse" || arg == "-p") {
                flags.executeJUSTC = false;
            }
            else if (arg == "--parser" || arg == "-P") {
                flags.lexerTokensToParser = true;
                flags.mode = "parser";
            }
            else if (arg == "--execute" || arg == "-E") {
                flags.lexerTokensToParser = true;
                flags.mode = "parserExecute";
            }
            else if (arg == "--async" || arg == "-a") {
                flags.asynchronously = true;
            }
            else if (arg == "--json" || arg == "-j") {
                flags.outputMode = "json";
            }
            else if (arg == "--xml" || arg == "-x") {
                flags.outputMode = "xml";
            }
            else if (arg == "--yaml" || arg == "-y") {
                flags.outputMode = "yaml";
            }
            else if (arg == "--justc" || arg == "-J") {
                flags.mode = "stringify";
                flags.outputMode = "justc";
            }
            else if (arg == "--silent" || arg == "-S") {
                flags.silentMode = true;
            }

            // hidden flags. IMPORTANT: DO NOT USE THESE FLAGS! THESE FLAGS ARE ONLY FOR JUST AN ULTIMATE SITE TOOL ENVIRONMENT.
            flags.waitingForVersion = false;
            flags.waitingForConfig = false;
            flags.waitingForCommitSHA = false;
            flags.waitingForNav = false;
            flags.waitingForPages = false;
            flags.waitingForCSS = false;
            if (arg == "--justversion") {
                flags.waitingForVersion = true;
            }
            else if (arg == "--configfile") {
                flags.waitingForConfig = true;
            }
            else if (arg == "--setnav") {
                flags.waitingForNav = true;
            }
            else if (arg == "--setpages") {
                flags.waitingForPages = true;
            }
            else if (arg == "--setcss") {
                flags.waitingForCSS = true;
            }
        }

        if (flags.silentMode) {
            outputRedirector = new OutputRedirector(true, false);
        }

        if (flags.input.empty() && !flags.helpandorversionflag) {
            throwError("No input provided", outputRedirector);
            return 1;
        } else if (flags.input.empty() && flags.helpandorversionflag) {
            flags.noInput = true;
        }

        if (!flags.noInput) {
            std::string json;
            if (flags.mode == "lexer") {
                auto lexerResult = Lexer::parse(flags.input, true);
                json = outputString(flags, lexerResult.second, lexerResult.first);
            }
            else if (flags.mode == "parser" || flags.mode == "parserExecute") {
                std::vector<ParserToken> lexerResult;
                std::string input = "";
                JsonParser::parseJsonTokens(flags.input.c_str(), lexerResult, input);
                auto parseResult = Parser::parseTokens(lexerResult, flags.mode == "parserExecute", flags.asynchronously, input);
                json = outputString(flags, parseResult);
            }
            else if (flags.mode == "stringify") {
                json = JsonParser::stringify(flags.input);
                if (flags.outputMode != "justc") {
                    throwError("Cannot output JSON converted to JUSTC as \"" + flags.outputMode + "\".", outputRedirector);
                }
            }
            else {
                auto lexerResult = Lexer::parse(flags.input);
                auto parseResult = Parser::parseTokens(lexerResult.second, flags.executeJUSTC, flags.asynchronously, flags.input);
                json = outputString(flags, parseResult);
            }

            if (flags.outputToConsole) {
                if (outputRedirector) {
                    outputRedirector->restore();
                }
                std::cout << json << std::endl;
                if (outputRedirector) {
                    outputRedirector = new OutputRedirector(true, false);
                }
            }
            if (flags.outputToFile) {
                writeFile(flags.output, json);
            }
        }

        if (outputRedirector) {
            outputRedirector->restore();
            delete outputRedirector;
        }
    } catch (const std::exception& e) {
        if (outputRedirector) {
            outputRedirector->restore();
            delete outputRedirector;
        }
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error." << std::endl;
        return 1;
    }

    return 0;
}
