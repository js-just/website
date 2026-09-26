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

const JUSTC = require('./index.js');
const fs = require('fs');
const path = require('path');

class OutputRedirector {
    constructor(silentMode) {
        this.silent = silentMode;
        this.originalConsole = {
            log: console.log,
            error: console.error,
            info: console.info,
            warn: console.warn
        };

        if (silentMode) {
            console.log = () => {};
            console.info = () => {};
            console.warn = () => {};
        }
    }

    restore() {
        if (this.silent) {
            console.log = this.originalConsole.log;
            console.error = this.originalConsole.error;
            console.info = this.originalConsole.info;
            console.warn = this.originalConsole.warn;
        }
    }

    outputError(error) {
        this.originalConsole.error(error);
    }
}

function logError(error) {
    if (process.env.GITHUB_ACTIONS) {
        console.error(`::error::${error}`);
    } else {
        console.error(error);
    }
}

function printUsage() {
    require('./help.js');
}

function throwError(error, redirector = null) {
    if (process.env.GITHUB_ACTIONS) {
        if (redirector) {
            redirector.outputError(`::error::${error}`);
        } else {
            console.error(`::error::${error}`);
        }
    } else {
        if (redirector) {
            redirector.outputError(error);
        } else {
            console.error(error);
        }
    }
    process.exit(1);
}

function readFile(filename) {
    try {
        return fs.readFileSync(filename, 'utf8');
    } catch (error) {
        throwError(`Unable to read the file: ${filename}`);
    }
}

function writeFile(filename, content) {
    try {
        fs.writeFileSync(filename, content, 'utf8');
    } catch (error) {
        throwError(`Unable to write the file: ${filename}`);
    }
}

async function main() {
    const args = process.argv.slice(2);

    if (args.length < 1) {
        printUsage();
        process.exit(1);
    }

    let outputRedirector = null;

    try {
        const flags = {
            mode: "execute",
            outputMode: "json",
            input: "",
            output: "",
            outputToConsole: false,
            executeJUSTC: true,
            helpandorversionflag: false,
            lexerTokensToParser: false,
            asynchronously: false,
            gotFileOrCode: false,
            outputToFile: false,
            noInput: false,
            silentMode: false,
            waitingForCommitSHA: false,
            commitSHA: ""
        };

        // Parse arguments
        for (let i = 0; i < args.length; i++) {
            const arg = args[i];

            if (flags.waitingForCommitSHA) {
                flags.commitSHA = arg;
                flags.waitingForCommitSHA = false;
            } else if (arg === "--help" || arg === "-h") {
                flags.helpandorversionflag = true;
                printUsage();
            } else if (arg === "--lexer" || arg === "-l") {
                flags.mode = "lexer";
            } else if (arg === "--result" || arg === "-r") {
                flags.outputToConsole = true;
            } else if ((arg === "-e" || arg === "--eval") && i + 1 < args.length) {
                flags.input = args[++i];
                flags.gotFileOrCode = true;
            } else if (arg[0] !== '-' && flags.gotFileOrCode) {
                flags.output = arg;
                flags.outputToFile = true;
            } else if (arg[0] !== '-') {
                flags.input = readFile(arg);
                flags.gotFileOrCode = true;
            } else if (arg === "--sha" || arg === "-s") {
                flags.waitingForCommitSHA = true;
            } else if (arg === "--version" || arg === "-v") {
                flags.helpandorversionflag = true;
            } else if (arg === "--parse" || arg === "-p") {
                flags.executeJUSTC = false;
            } else if (arg === "--parser" || arg === "-P") {
                flags.lexerTokensToParser = true;
                flags.mode = "parser";
            } else if (arg === "--execute" || arg === "-E") {
                flags.lexerTokensToParser = true;
                flags.mode = "parserExecute";
            } else if (arg === "--async" || arg === "-a") {
                flags.asynchronously = true;
            } else if (arg === "--json" || arg === "-j") {
                flags.outputMode = "json";
            } else if (arg === "--xml" || arg === "-x") {
                flags.outputMode = "xml";
            } else if (arg === "--yaml" || arg === "-y") {
                flags.outputMode = "yaml";
            } else if (arg === "--justc" || arg === "-J") {
                flags.mode = "stringify";
                flags.outputMode = "justc";
            } else if (arg === "--silent" || arg === "-S") {
                flags.silentMode = true;
            }
        }

        if (flags.silentMode) {
            outputRedirector = new OutputRedirector(true);
        }

        await JUSTC.initialize();

        if (flags.helpandorversionflag && args.includes("-v") || args.includes("--version")) {
            console.log(await JUSTC.version());
            if (outputRedirector) outputRedirector.restore();
            return;
        }

        if (flags.input === "" && !flags.helpandorversionflag) {
            throwError("No input provided", outputRedirector);
        } else if (flags.input === "" && flags.helpandorversionflag) {
            flags.noInput = true;
        }

        if (!flags.noInput) {
            let result;

            if (flags.mode === "lexer") {
                const lexer = await JUSTC.requestPermissions("core.lexer");
                result = lexer(flags.input);
                result = JSON.stringify(result, null, 2);
            } else if (flags.mode === "parser" || flags.mode === "parserExecute") {
                const parser = await JUSTC.requestPermissions("core.parser");
                const lexerOutput = JSON.parse(flags.input);
                result = parser(lexerOutput);
                result = JSON.stringify(result, null, 2);
            } else if (flags.mode === "stringify") {
                const obj = JSON.parse(flags.input);
                result = await JUSTC.stringify(obj);
                if (flags.outputMode !== "justc") {
                    throwError(`Cannot output JSON converted to JUSTC as "${flags.outputMode}".`, outputRedirector);
                }
            } else {
                // Default execution
                if (flags.executeJUSTC) {
                    result = await JUSTC.execute(flags.input, flags.outputMode);
                } else {
                    result = await JUSTC.parse(flags.input, flags.outputMode);
                }
                result = typeof result === 'object' ? JSON.stringify(result, null, 2) : String(result);
            }

            if (flags.outputToConsole) {
                if (outputRedirector) {
                    outputRedirector.restore();
                }
                console.log(result);
                if (outputRedirector) {
                    outputRedirector = new OutputRedirector(true);
                }
            }

            if (flags.outputToFile) {
                writeFile(flags.output, result);
            }
        }

        if (outputRedirector) {
            outputRedirector.restore();
        }
    } catch (error) {
        if (outputRedirector) {
            outputRedirector.restore();
        }
        logError(`Error: ${error.message}`);
        process.exit(1);
    }
}

// Handle uncaught exceptions
process.on('uncaughtException', (error) => {
    console.error('Uncaught Exception:', error);
    process.exit(1);
});

process.on('unhandledRejection', (reason, promise) => {
    console.error('Unhandled Rejection at:', promise, 'reason:', reason);
    process.exit(1);
});

main().catch(error => {
    console.error('Fatal error:', error);
    process.exit(1);
});
