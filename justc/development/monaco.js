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
            "nor", "xor", "imply", "nimply",

            "true", "false", "yes", "no", "y", "n",
            "null", "nil",
        ],

        typeKeywords: [ // lowercase
            'string', 'number', 'link', 'binary', 'hexadecimal', 'octal', 'infinity', 'nan'
        ],

        operators: [
            '!=', '<=', '>=', '<<', '>>', '&&', '!&', '||', '!|', '=!', '?!', '..', '::', '??', '==', '?=',
            '=', '!', '&', '|', '/', ':', '*', '-', '+', '^', '<', '>', '?'
        ],

        symbols: /[=><!~?:&|+\-*\/\^%]+/,

        escapes: /\\(?:[abfnrtv\\"]|x[0-9A-Fa-f]{1,4}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})/,

        // should be lowercase
        builtinLowercase: /(echo|log|logfile|value|parseNum)(?=\()/,
        // optionally uppercase
        builtinUppercase: /(True|TRUE|False|FALSE|Yes|No|YES|NO|Y|N|Null|Nil|NULL|NIL|AND|OR|XOR|NOT|JUSTC|HTTP::(GET|POST|PUT|PATCH|DELETE|HEAD|OPTIONS)|Math::(Abs|Acos|Asin|Atan|Atan2|Ceil|Cos|Clamp|Cube|Double|Exp|Factorial|Floor|Hypot|IsPrime|Lerp|Log(10|)|Max|Min|Pow|Random|Round|Sign|Sin|Sqrt|Square|Tan|ToDegrees|ToRadians))/,

        tokenizer: {
            root: [
                [/@builtinLowercase/, 'type.builtin'],
                [/[a-z_][\w$']*/, {
                    cases: {
                    '@typeKeywords': 'keyword',
                    '@keywords': 'keyword',
                    }
                }],

                [/@builtinUppercase/, 'type.builtin'],

                { include: '@whitespace' },

                [/-\{/, 'comment', '@comment'],

                [/{{/, { token: 'keyword', next: '@jsEmbedded', nextEmbedded: 'javascript' }],

                [/[{}()\[\]]/, '@brackets'],

                [/<</, { token: 'keyword', next: '@luauEmbedded', nextEmbedded: 'lua' }],

                [/</, { token: 'string.quote', bracket: '@open', next: '@angleString' }],

                [/.*(?=\(.*?\))/, 'function.call'],

                [/@symbols/, {
                    cases: {
                    '@operators': 'operator',
                    '@default': ''
                    }
                }],

                [/(\$.*)(?=[,.]$)/, 'type.identifier'],

                [/\d*\.\d+([eE][\-+]?\d+)?/, 'number.float'],
                [/0[xX][0-9a-fA-F]+/, 'number.hex'],
                [/\d+/, 'number'],

                [/[,.]/, 'delimiter'],

                [/"([^"\\]|\\.)*$/, 'string.invalid'],
                [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],
            ],

            comment: [
                [/[^\-\}]+/, 'comment'],
                [/\}-/, 'comment', '@pop'],
                [/[\-\}]/, 'comment']
            ],

            string: [
                [/[^\\"]+/, 'string'],
                [/@escapes/, 'string.escape'],
                [/\\./, 'string.escape.invalid'],
                [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
            ],

            angleString: [
                [/[^<>]+/, 'string'],
                [/>/, { token: 'string.quote', bracket: '@close', next: '@pop' }],
                [/</, 'string']
            ],

            whitespace: [
                [/[ \t\r\n]+/, 'white'],
                [/--*\n/, 'comment', '@comment'],
                [/--.*$/, 'comment'],
            ],

            jsEmbedded: [
                [/}}/, { token: 'keyword', next: '@pop', nextEmbedded: '@pop' }],
                [/./, '']
            ],

            luauEmbedded: [
                [/>>/, { token: 'keyword', next: '@pop', nextEmbedded: '@pop' }],
                [/./, '']
            ],
        },
    };
}

monacoScript.onload = function() {
    require.config({ paths: { "vs": "https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/vs/" }});

    window.MonacoEnvironment = {
        getWorkerUrl: function(workerId, label) {
            return `data:text/javascript;charset=utf-8,${encodeURIComponent(`
                self.MonacoEnvironment = { baseUrl: "https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/" };
                importScripts("https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.29.1/min/vs/base/worker/workerMain.min.js");`
            )}`;
        }
    };

    require(["vs/editor/editor.main"], function () {
        monaco.languages.register({ id: 'justc' });
        monaco.languages.setMonarchTokensProvider('justc', monacoJUSTClang());

        var editor = monaco.editor.create(document.getElementById("editor"), {
            value: "-- Type JUSTC code here...",
            language: "justc"
        });

        const editorElement = document.getElementById("editor");
        window.addEventListener("resize", () => editor.layout({
            width: editorElement.offsetWidth,
            height: editorElement.offsetHeight
        }));

        Object.defineProperty(globalThis.window, 'JUSTC_MonacoEditor', {
            value: editor,
            configurable: false,
            enumerable: true,
            writable: false
        });
        document.documentElement.setAttribute('justc','');
    });
};

document.head.appendChild(monacoScript);
document.body.appendChild(monacoElement);
