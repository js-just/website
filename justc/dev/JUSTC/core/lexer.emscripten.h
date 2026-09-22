/*

MIT License

Copyright (c) 2025-2026 JustStudio. <https://juststudio.is-a.dev/>

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

#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, warn_lexer_lang, (const char* timestamp, const char* position, const char* lang), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ')', UTF8ToString(lang), 'may be corrupted in the lexer output.', '('+UTF8ToString(position)+')');
});
EM_JS(void, warn_lexer_goto, (const char* timestamp, const char* position), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') Found goto loop at ' + UTF8ToString(position)+'.');
});

#endif
