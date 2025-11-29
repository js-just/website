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

/*

Just an Ultimate Site Tool Configuration language v0.01.000 (development)

*/

console.log("\nJust an Ultimate Site Tool Configuration language (JUSTC) v0.01.000\nhttps://just.js.org/justc\n\nUsage: justc   [ flags ( [ arguments ] ) ]   [ input.justc | input.json ] ( [ output.json | output.justc | output.xml | output.yaml ] )\n       justc <file.justc>               - Execute JUSTC file\n       justc <file.justc> <file.json>   - Execute JUSTC file and output to JSON file\n       justc -e \"<code>\"                - Execute JUSTC code\n\nFlags:\n  -a, --async                           - Evaluate script variables asynchronously (may run faster)\n  -e, --eval                            - Execute (Evaluate) script (not file)\n  -E, --execute                         - Execute JUSTC from lexer output tokens as JSON\n  -h, --help                            - Print JUSTC command line options (this list)\n  -j, --json                            - Output as JSON (default)\n  -J, --justc                           - Output as JUSTC, input should be JSON\n  -l, --lexer                           - Run lexer only / Tokenize\n  -P, --parser                          - Run parser only / Parse JUSTC (not execute) from lexer output tokens as JSON\n  -p, --parse                           - Parse JUSTC (not execute) / No HTTP requests, some commands will not be executed\n  -r, --result                          - Print result\n  -s <commit sha>, --sha <commit sha>   - Set commit SHA\n  -S, --silent                          - Suppress all output except errors and results\n  -v, --version                         - Print JUSTC version\n  -x, --xml                             - Output as XML\n  -y, --yaml                            - Output as YAML\n\n\"Run parser only\" means that JUSTC (will not be executed) will only be compiled to JSON - no logs and/or HTTP requests.\n\"Run lexer only\" means that JUSTC won't be executed and/or parsed, JUSTC will only be tokenized.");
