const monacoScript = document.createElement('script');
monacoScript.src = 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/vs/loader.min.js';
const monacoElement = document.createElement('div');
monacoElement.id = "editor";
monacoElement.style = "height: 80vh; outline: 2px solid gray; overflow-y: clip; border-radius: 2px;";

function monacoJUSTClang() {
    return {
        keywords: [ // lowercase
            "type", "global", "local", "strict",
            "import", "export", "exports", "require",
            "run", "output", "return", "specified",
            "everything", "disabled", "as", "allow",
            "disallow", "JavaScript", "safe", "Luau",
            "class", "function", "public", "private",
            "static", "const", "define", "undefine",
            "echo", "log", "logfile", "space", "var",
            "new",

            "is", "isn't", "isif", "then", "elseif", "else",
            "isifn't", "elseifn't", "then't", "elsen't",
            "or", "orn't", "and", "andn't", "not", "nand",
            "nor", "xor", "xnor", "imply", "nimply",

            "true", "false", "yes", "no", "y", "n",
            "null", "nil",
        ],

        typeKeywords: [ // lowercase
            'string', 'number', 'link', 'binary', 'hexadecimal',
            'octal', 'infinity', 'nan', 'boolean', 'path', 'variable'
        ],

        operators: [
            '!=', '<=', '>=', '<<', '>>', '&&', '!&', '||', '!|',
            '=!', '?!', '..', '::', '??', '==', '?=', '**',
            '=', '!', '&', '|', '/', ':', '*', '-', '+', '^',
            '<', '>', '?', '%', '~'
        ],

        symbols: /[=><!~?:&|+\-*\/\^%]+/,

        escapes: /\\(?:[abfnrtv\\"']|x[0-9A-Fa-f]{1,4}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})/,

        builtinFunctions: /\b(echo|log|logfile|value|string|number|link|binary|octal|hexadecimal|typeid|typeof|JSON|file|size|env|config|JUSTC|PARSEJUSTC|PARSEJSON|TIME|PI|BACKSLASH|VERSION|HTTP::(GET|POST|PUT|PATCH|DELETE|HEAD|OPTIONS)|Math::(Abs|Acos|Asin|Atan|Atan2|Ceil|Cos|Clamp|Cube|Double|Exp|Factorial|Floor|Hypot|IsPrime|Lerp|Log|Log10|Max|Min|Pow|Random|Round|Sign|Sin|Sqrt|Square|Tan|ToDegrees|ToRadians))\b/i,

        constants: /\b(True|TRUE|False|FALSE|Yes|YES|No|NO|Y|N|Null|NULL|Nil|NIL|Infinity|NaN|undefined)\b/,

        tokenizer: {
            root: [
                // Comments
                [/--.*$/, 'comment'],
                [/-\{/, { token: 'comment', next: '@multiLineComment' }],

                // Strings
                [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],
                [/'/, { token: 'string.quote', bracket: '@open', next: '@singleQuoteString' }],

                // Links
                [/</, { token: 'string.link', next: '@link' }],

                // JavaScript
                [/\{\{/, { token: 'keyword.js', next: '@jsEmbedded', nextEmbedded: 'javascript' }],

                // Luau
                [/<</, { token: 'keyword.luau', next: '@luauEmbedded', nextEmbedded: 'lua' }],

                // Numbers
                [/0[xX][0-9a-fA-F_]+/, 'number.hex'],
                [/0[oO][0-7_]+/, 'number.octal'],
                [/\d[\d_]*(\.\d[\d_]*)?([eE][+-]?\d[\d_]*)?/, 'number'],
                [/[xX#][0-9a-fA-F_]+/, 'number.hex'],
                [/[bB][01_]+/, 'number.binary'],
                [/[oO][0-7_]+/, 'number.octal'],

                // Built-ins
                [/@builtinFunctions(?=\s*\()/, 'type.identifier'],

                // Constants
                [/@constants/, 'constant'],

                // Keywords
                [/[a-zA-Z_][\w']*/, {
                    cases: {
                        '@keywords': 'keyword',
                        '@typeKeywords': 'type',
                        '@default': 'identifier'
                    }
                }],

                // $
                [/\$[a-zA-Z_][\w']*/, 'variable'],

                // Operators
                [/@symbols/, {
                    cases: {
                        '@operators': 'operator',
                        '@default': ''
                    }
                }],

                // Delimiters
                [/[()\[\]{}]/, '@brackets'],
                [/[,.:;]/, 'delimiter'],

                { include: '@whitespace' },
            ],

            string: [
                [/[^\\"]+/, 'string'],
                [/@escapes/, 'string.escape'],
                [/\\./, 'string.escape.invalid'],
                [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
            ],

            singleQuoteString: [
                [/[^\\']+/, 'string'],
                [/@escapes/, 'string.escape'],
                [/\\./, 'string.escape.invalid'],
                [/'/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
            ],

            link: [
                [/[^>]+/, 'string.link'],
                [/>/, { token: 'string.link', bracket: '@close', next: '@pop' }]
            ],

            multiLineComment: [
                [/[^-{}]+/, 'comment'],
                [/-\}/, { token: 'comment', next: '@pop' }],
                [/[-{}]/, 'comment']
            ],

            jsEmbedded: [
                [/\}\}/, { token: 'keyword.js', next: '@pop', nextEmbedded: '@pop' }],
                [/./, '']
            ],

            luauEmbedded: [
                [/>>/, { token: 'keyword.luau', next: '@pop', nextEmbedded: '@pop' }],
                [/./, '']
            ],

            whitespace: [
                [/[ \t\r\n]+/, 'white'],
            ],
        },
    };
}

monacoScript.onload = function() {
    require.config({
        paths: {
            "vs": "https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/vs/"
        }
    });

    window.MonacoEnvironment = {
        getWorkerUrl: function(workerId, label) {
            return `data:text/javascript;charset=utf-8,${encodeURIComponent(`
                self.MonacoEnvironment = {
                    baseUrl: "https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/"
                };
                importScripts("https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/vs/base/worker/workerMain.js");
            `)}`;
        }
    };

    require(["vs/editor/editor.main"], function() {
        monaco.languages.register({ id: 'justc' });
        monaco.languages.setMonarchTokensProvider('justc', monacoJUSTClang());

        monaco.languages.setLanguageConfiguration('justc', {
            brackets: [
                ['(', ')'],
                ['[', ']'],
                ['{', '}'],
                ['"', '"'],
                ["'", "'"],
                ['<', '>']
            ],
            autoClosingPairs: [
                { open: '(', close: ')' },
                { open: '[', close: ']' },
                { open: '{', close: '}' },
                { open: '"', close: '"' },
                { open: "'", close: "'" },
                { open: '<', close: '>' },
                { open: '{{', close: '}}' },
                { open: '<<', close: '>>' }
            ],
            surroundingPairs: [
                { open: '(', close: ')' },
                { open: '[', close: ']' },
                { open: '{', close: '}' },
                { open: '"', close: '"' },
                { open: "'", close: "'" },
                { open: '<', close: '>' }
            ],
            comments: {
                lineComment: '--',
                blockComment: ['-{', '}-']
            }
        });

        var editor = monaco.editor.create(document.getElementById("editor"), {
            value: `-- Type JUSTC code here...`,
            language: "justc",
            theme: "vs-dark",
            fontSize: 14,
            //wordWrap: "on",
            minimap: { enabled: true },
            scrollBeyondLastLine: false,
            automaticLayout: true
        });

        const editorElement = document.getElementById("editor");
        const resizeObserver = new ResizeObserver(() => {
            editor.layout({
                width: editorElement.clientWidth,
                height: editorElement.clientHeight
            });
        });
        resizeObserver.observe(editorElement);

        window.JUSTC_MonacoEditor = editor;

        document.documentElement.setAttribute('data-justc-editor', 'true');

        console.log("JUSTC Monaco Editor loaded successfully!");
    });
};

monacoScript.onerror = function() {
    console.error("Failed to load Monaco Editor script");
    monacoElement.innerHTML = "<p style='color: red; padding: 20px;'>Failed to load code editor. Please check your internet connection.</p>";
};

document.head.appendChild(monacoScript);
document.body.appendChild(monacoElement);
