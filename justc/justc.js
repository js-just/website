(async()=>{
    "use strict";

    const JUSTC = {};
    JUSTC.JUSTC = __justc__;
    JUSTC.Error = class extends Error {};
    JUSTC.ErrorEnabled = true;
    JUSTC.CoreLogsEnabled = false;
    JUSTC.Silent = false;

    JUSTC.Console = function(type, ...args) {
        if (!JUSTC.Silent) {
            console[type](...args);
        }
    };
    JUSTC.ErrorIfEnabled = function(...args) {
        if (JUSTC.ErrorEnabled) {
            throw new JUSTC.Error(...args);
        }
    };

    JUSTC.Errors = {
        initWasm: 'JUSTC WebAssembly module hasn\'t been initialized yet.',
        wrongInputType: 'Argument 0 should be a string.',
        wasmFailed: 'Failed to initialize JUSTC WebAssembly module:',
        wasmInitFailed: 'Unable to initialize JUSTC WebAssembly module.',
        executionError: 'JUSTC/core/browsers.cpp error:',
        arrayInput: 'Array cannot be converted to JUSTC.',
        objectInput: 'Provided input is not valid object.',
        arg0: 'Invalid argument 0. Run "JUSTC = \'help\'" for help.',
        lexerError: 'JUSTC/core/lexer.cpp error:',
        parseError: 'JUSTC/core/parser.cpp error:'
    };

    JUSTC.Core = {};
    JUSTC.CoreScript = function(code, name) {
        try {
            const resultptr = JUSTC.WASM.ccall(
                name,
                'number',
                ['string'],
                [code]
            );
            const resultjson = JUSTC.WASM.UTF8ToString(resultptr);
            JUSTC.WASM.ccall(
                'free_string',
                null,
                ['number'],
                [resultptr]
            );

            console.log(resultjson);
            return JSON.parse(resultjson);
        } catch (error) {
            throw new JUSTC.Error(JUSTC.Errors[name + 'Error'], error);
        }
    };
    JUSTC.Core.Lexer = function Lexer(code) {
        if (!JUSTC.WASM) throw new JUSTC.Error(JUSTC.Errors.initWasm);
        if (!code || typeof code != 'string' || code.length < 1) throw new JUSTC.Error(JUSTC.Errors.wrongInputType);
        const result = JUSTC.CoreScript(code, 'lexer');
        if (result.error) {
            throw new JUSTC.Error(result.error);
        } else {
            return result.return || {};
        }
    };
    JUSTC.Core.Parser = function Parser(code) {
        if (!JUSTC.WASM) throw new JUSTC.Error(JUSTC.Errors.initWasm);
        if (!code || typeof code != 'string' || code.length < 1) throw new JUSTC.Error(JUSTC.Errors.wrongInputType);
        const result = JUSTC.CoreScript(code, 'parse');
        console.log(result)
        if (result.error) {
            throw new JUSTC.Error(result.error);
        } else {
            return result.return || {};
        }
    };

    JUSTC.PrivateFunctions = {
        All: {
            Lexer: {
                NeedsWASM: true,
                Name: "core.lexer",
                Return: JUSTC.Core.Lexer
            },
            Parser: {
                NeedsWASM: true,
                Name: "core.parser",
                Return: JUSTC.Core.Parser
            }
        },
        Available: [],
        WhatToName: {
            "core.lexer": "Lexer",
            "core.parser": "Parser"
        }
    };

    JUSTC.Initialize = async function() {
        try {
            JUSTC.WASM = await JUSTC.JUSTC();
            if (JUSTC.CoreLogsEnabled) {
                JUSTC.Console("log", "JUSTC WebAssembly module initialized.");
            }
        } catch (error) {
            JUSTC.Console("error", JUSTC.Errors.wasmFailed, error);
        }
    };
    JUSTC.InitWASM = async function InitializeJUSTC(attempt = 0) {
        while (!JUSTC.WASM) {
            attempt++;
            await JUSTC.Initialize();
            if (attempt > 10) {
                throw new JUSTC.Error(JUSTC.Errors.wasmInitFailed);
            }
        };
        if (JUSTC.WASM) {
            for (const [unused, prfunc] of Object.entries(JUSTC.PrivateFunctions.All)) {
                if (prfunc.NeedsWASM && !JUSTC.PrivateFunctions.Available.includes(prfunc.Name)) {
                    JUSTC.PrivateFunctions.Available.push(prfunc.Name);
                }
            }
        }
    };
    JUSTC.DisplayLogs = function(result) {
        if (result.logfile && result.logfile.file && result.logfile.file != '') {
            throw new JUSTC.Error("Logfile cannot be created in browser.");
        };
        if (result.logs && Array.isArray(result.logs)) {
            result.logs.forEach(log => {
                JUSTC.Console("log", log.time, log.message);
            });
        }
    };

    JUSTC.Commands = {
        "EnableCoreLogs": function() { JUSTC.CoreLogsEnabled = true },
        "DisableCoreLogs": function() { JUSTC.CoreLogsEnabled = false },
        "SwitchCoreLogs": function() { JUSTC.CoreLogsEnabled = !JUSTC.CoreLogsEnabled },
        "ShowCoreErrors": function() { JUSTC.ErrorEnabled = true },
        "HideCoreErrors": function() { JUSTC.ErrorEnabled = false },
        "SwitchCoreErrors": function() { JUSTC.ErrorEnabled = !JUSTC.ErrorEnabled },
        "Silent": function() { JUSTC.Silent = true },
        "Help": function() { console.info("https://just.js.org/justc") },
    };

    JUSTC.Parse = function(code) {
        try {
            const resultPtr = JUSTC.WASM.ccall(
                'parse',
                'number',
                ['string'],
                [code]
            );
            
            const resultJson = JUSTC.WASM.UTF8ToString(resultPtr);
            JUSTC.WASM.ccall('free_string', null, ['number'], [resultPtr]);
            
            const result = JSON.parse(resultJson);
            
            return result;
        } catch (error) {
            console.error(JUSTC.Errors.executionError, error);
            throw error;
        }
    };

    JUSTC.CheckWASM = function() {
        if (!JUSTC.WASM) {
            throw new JUSTC.Error(JUSTC.Errors.initWasm);
        }
    };
    JUSTC.CheckInput = function(input) {
        if (!input || typeof input != 'string') {
            JUSTC.ErrorIfEnabled(String(input).length < 32 ? `"${String(input)}" is not valid JUSTC.` : ()=>{
                try {
                    if (!input.name) throw new JUSTC.Error('');
                    return `Provided ["${input.name}"] is not valid JUSTC.`;
                } catch (err) {
                    return "Provided code is not valid JUSTC.";
                }
            });
            return true;
        } else return false;
    };

    JUSTC.fromJSON = function(input) {
        if (Array.isArray(input)) {
            throw new JUSTC.Error(JUSTC.Errors.arrayInput);
        } else {
            const varNames = [];
            let output = '';
            for (const [name, value] of Object.entries(JSON.parse(JSON.stringify(input)))) {
                varNames.push(name);
                output += `${name}=${
                    typeof value === 'string' ? `"${value}"` :
                    value === true ? 'y' :
                    value === false ? 'n' :
                    value === null ? 'nil' :
                    value
                },`;
            };
            output += 'RT[';
            for (const name of varNames) {
                output += (varNames.indexOf(name) > 0 ? ',' : '') + name;
            };
            return output + '].';
        }
    };

    JUSTC.Output = {
        parse: function ParseJUSTC(code) {
            JUSTC.CheckInput(code);
            JUSTC.CheckWASM();

            const result = JUSTC.Parse(code);
            if (result.error) {
                throw new JUSTC.Error(result.error);
            } else {
                return result.return || {};
            }
        },
        execute: function ExecuteJUSTC(code) {
            JUSTC.CheckInput(code);
            JUSTC.CheckWASM();

            const result = JUSTC.Parse(code);
            if (result.error) {
                throw new JUSTC.Error(result.error);
            } else {
                JUSTC.DisplayLogs(result);
                return result.return || {};
            }
        },
        initialize: async function InitializeJUSTC() {
            await JUSTC.InitWASM();
        },
        stringify: function JSONtoJUSTC(JavaScriptObjectNotation) {
            if (typeof JavaScriptObjectNotation != 'object') throw new JUSTC.Error(JUSTC.Errors.objectInput);
            return JUSTC.fromJSON(JavaScriptObjectNotation);
        }
    };
    JUSTC.Public = {};
    for (const [name, value] of Object.entries(JUSTC.Output)) {
        Object.defineProperty(JUSTC.Public, name, {
            value, 
            writable: false,
            configurable: false,
            enumerable: true
        });
    };
    JUSTC.Private = function(what) {
        if (!what || typeof what != 'string' || what.length < 1) throw new JUSTC.Error(JUSTC.Errors.arg0);
        if (JUSTC.PrivateFunctions.Available.includes(what)) {
            return JUSTC.PrivateFunctions.All[JUSTC.PrivateFunctions.WhatToName[what]].Return;
        } else {
            throw new JUSTC.Error(`JUSTC.requestPermissions: "${what}" is either not available or does not exist.`);
        }
    };
    Object.defineProperty(JUSTC.Public, 'requestPermissions', {
        value: JUSTC.Private,
        writable: false,
        configurable: false,
        enumerable: false,
    });
    
    Object.defineProperty(globalThis.window, 'JUSTC', {
        get: function() {
            JUSTC.InitWASM();
            return Object.freeze(JUSTC.Public);
        },
        set: function(command) {
            if (typeof command === 'string' && typeof JUSTC.Commands[command] === 'function') {
                JUSTC.Commands[command]();
            } else if (String(command).toLowerCase().includes('help')) {
                JUSTC.Commands.Help();
            } else {
                JUSTC.ErrorIfEnabled('JUSTC cannot be redefined.');
            }
        },
        configurable: false,
    })
})()
