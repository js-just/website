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

declare module 'justc' {
    interface JUSTCError extends Error {
        constructor(message?: string): JUSTCError;
    }

    interface LexerResult {
        input: string;
        tokens: Array<{
            type: string;
            start: number;
            value: any;
        }>;
        version: string;
        error?: string;
        lexer?: boolean;
    }

    interface ParserResult {
        return?: any;
        error?: string;
        parser?: boolean;
    }

    interface ParseResult {
        return?: any;
        error?: string;
        imported?: Array<[string, string, string]>;
        logs?: Array<{
            type: string;
            time: string;
            message: string;
        }>;
        logfile?: {
            file: string;
        };
    }

    interface VirtualFileSystem {
        files: Map<string, any>;
        createFile(filename: string, content: string, options?: {
            mimeType?: string;
            language?: string;
            syntaxHighlighting?: boolean;
        }): string;
        getFile(filename: string): any;
        listFiles(): string[];
    }

    interface JUSTCOutput {
        /**
         * Parse JUSTC code without execution
         * @param code JUSTC code to parse
         * @param outputMode Output format ('json', 'xml', 'yaml')
         * @returns Parsed result
         * @since 0.1.0
         */
        parse(code: string, outputMode?: 'json' | 'xml' | 'yaml'): any;

        /**
         * Parse and execute JUSTC code
         * @param code JUSTC code to execute
         * @param outputMode Output format ('json', 'xml', 'yaml')
         * @returns Execution result
         * @since 0.1.0
         */
        execute(code: string, outputMode?: 'json' | 'xml' | 'yaml'): any;

        /**
         * Initialize JUSTC WebAssembly module
         * @since 0.1.0
         */
        initialize(): Promise<void>;

        /**
         * Convert JavaScript object to JUSTC string format
         * @param JavaScriptObjectNotation JavaScript object to convert
         * @returns JUSTC string representation
         * @since 0.1.0
         */
        stringify(JavaScriptObjectNotation: object): string;

        /**
         * Asynchronously parse JUSTC code (experimental)
         * @param code JUSTC code to parse
         * @returns Promise with parsed result
         * @since 0.1.0
         */
        parseAsync?(...code: string[]): Promise<any[]>;

        /**
         * Asynchronously execute JUSTC code (experimental)
         * @param code JUSTC code to execute
         * @returns Promise with execution result
         * @since 0.1.0
         */
        executeAsync?(...code: string[]): Promise<any[]>;
    }

    interface JUSTCPublic extends JUSTCOutput {
        readonly [Symbol.toStringTag]: 'JUSTC';

        /**
         * Get JUSTC version.
         * @since 0.1.0
         */
        readonly version: string;

        /**
         * Request permissions for internal functions.
         * @param what Internal function name(s).
         * @returns Requested internal function(s) if exists and available.
         * @since 0.1.0
         */
        requestPermissions(what: string | string[]): ((...args: any[]) => any) | ((...args: any[]) => any)[];
    }

    interface JUSTCGlobal {
        /**
         * Main JUSTC interface - getter returns frozen JUSTC object
         * @since 0.1.0
         */
        JUSTC: JUSTCPublic;

        /**
         * Shortcut for `JUSTC.execute` - getter returns execute function
         * @since 0.1.0
         */
        $JUSTC: (code: string, outputMode?: 'json' | 'xml' | 'yaml') => any;
    }

    // Browser global
    declare global {
        interface Window extends JUSTCGlobal {}
        var JUSTC: JUSTCPublic;
        var $JUSTC: (code: string, outputMode?: 'json' | 'xml' | 'yaml') => any;
    }

    // CommonJS/Node.js export
    const JUSTC: {
        /**
         * Parse `JUSTC` code without execution.
         * @returns Output `JSON` as `Object` or output `XML`/`YAML` as `string`.
         * @since 0.1.0
         */
        parse(code: string, outputMode?: 'json' | 'xml' | 'yaml'): Promise<string> | Promise<any>;

        /**
         * Parse and execute `JUSTC` code.
         * @returns Output `JSON` as `Object` or output `XML`/`YAML` as `string`.
         * @since 0.1.0
         */
        execute(code: string, outputMode?: 'json' | 'xml' | 'yaml'): Promise<string> | Promise<any>;

        /**
         * Initialize JUSTC WebAssembly module.
         * @since 0.1.0
         */
        initialize(): Promise<void>;

        /**
         * Convert `JSON` to `JUSTC`.
         * @since 0.1.0
         */
        stringify(JavaScriptObjectNotation: object): Promise<string>;

        /**
         * Get current `JUSTC` version.
         * @since 0.1.0
         */
        readonly version: Promise<string> | string;

        /**
         * Request permissions for internal functions.
         * @param what Internal function name(s).
         * @returns Requested internal function(s) if exists and available.
         * @since 0.1.0
         */
        requestPermissions(what: string | string[]): Promise<((...args: any[]) => any) | ((...args: any[]) => any)[]>;

        /**
         * Define JUSTC WebAssembly module if it has not been defined automatically.
         * @since 0.1.0
         */
        defineWASM(WASMmodule: any): void;

        readonly [Symbol.toStringTag]: 'JUSTC';
    };

    export = JUSTC;
}
