const monacoScript = document.createElement('script');
monacoScript.src = 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/vs/loader.min.js';
const monacoElement = document.createElement('div');
monacoElement.id = "editor";
monacoElement.style = "height: 80vh; outline: 2px solid gray; overflow-y: clip; border-radius: 2px;";

function monacoJUSTClang() {
    return {
        keywords: [
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
            "null", "nil"
        ],

        typeKeywords: [
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

        numberBeforeShift: /[\d\w\)\]\}]\s*<<$/,

        luauEmbeddingStart: /(?:^|[^\w\d\)\]\}\s])<<$/,

        tokenizer: {
            root: [
                [/@numberBeforeShift/, { token: '@rematch', next: '@shiftOperator' }],
                [/@luauEmbeddingStart/, { token: 'keyword.luau', next: '@luauEmbedded', nextEmbedded: 'lua' }],

                [/--.*$/, 'comment'],
                [/-\{/, { token: 'comment', next: '@multiLineComment' }],

                [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],
                [/'/, { token: 'string.quote', bracket: '@open', next: '@singleQuoteString' }],

                [/<(?![<])/, { token: 'string.link', next: '@link' }],

                [/\{\{/, { token: 'keyword.js', next: '@jsEmbedded', nextEmbedded: 'javascript' }],

                [/0[xX][0-9a-fA-F_]+/, 'number.hex'],
                [/[xX#][0-9a-fA-F_]+/, 'number.hex'],
                [/[bB][01_]+/, 'number.binary'],
                [/[oO][0-7_]+/, 'number.octal'],
                [/\d[\d_]*(\.\d[\d_]*)?([eE][+-]?\d[\d_]*)?/, 'number'],

                [/@builtinFunctions(?=\s*\()/, 'type.identifier'],

                [/@constants/, 'constant'],

                [/[a-zA-Z_][\w']*/, {
                    cases: {
                        '@keywords': 'keyword',
                        '@typeKeywords': 'type',
                        '@default': 'identifier'
                    }
                }],

                [/\$[a-zA-Z_][\w']*/, 'variable'],

                [/<</, { token: '@rematch', next: '@shiftOperator' }],
                [/>>/, 'operator'],
                [/[=><!~?:&|+\-*\/\^%]/, {
                    cases: {
                        '@operators': 'operator',
                        '@default': ''
                    }
                }],

                [/[()\[\]{}]/, '@brackets'],
                [/[,.:;]/, 'delimiter'],

                { include: '@whitespace' },
            ],

            shiftOperator: [
                [/<</, { token: 'operator', next: '@pop' }],
                [/./, { token: '@rematch', next: '@pop' }]
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
};

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
                { open: '{{', close: '}}', notIn: ['string', 'comment'] },
                { open: '<<', close: '>>', notIn: ['string', 'comment'] },
                { open: '-{', close: '}-', notIn: ['string', 'comment'] },
                { open: '(', close: ')' },
                { open: '[', close: ']' },
                { open: '{', close: '}' },
                { open: '"', close: '"', notIn: ['string'] },
                { open: "'", close: "'", notIn: ['string'] },
                { open: '<', close: '>' },
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
            fontSize: 14,
            minimap: { enabled: true },
            scrollBeyondLastLine: false,
            automaticLayout: true,
            lineNumbers: "on",
            folding: true,
            lineDecorationsWidth: 10,
            lineNumbersMinChars: 3
        });

        const editorElement = document.getElementById("editor");
        const resizeObserver = new ResizeObserver(() => {
            editor.layout({
                width: editorElement.clientWidth,
                height: editorElement.clientHeight
            });
        });
        resizeObserver.observe(editorElement);

        document.documentElement.setAttribute('justc','');
        window.JUSTC_MonacoEditor = editor;
    });
};

monacoScript.onerror = function() {
    monacoElement.innerHTML = "<p style='color: red; padding: 20px;'>Failed to load code editor. Please check your internet connection.</p>";
};

document.head.appendChild(monacoScript);
document.body.appendChild(monacoElement);
