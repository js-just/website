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

#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, warn_unsupported_js_type, (const char* timestamp, const char* value, const char* position), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') Unsupported JavaScript output type ("' + UTF8ToString(value) + '") was converted to a string at', UTF8ToString(position));
});

EM_JS(void, warn_js_disabled, (const char* position, const char* token_value, const char* timestamp), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') Running lexer and parser only - Cannot run JavaScript', '"' + UTF8ToString(token_value) + '"', 'at', UTF8ToString(position));
});
EM_JS(void, warn_js_disabled_by_justc, (const char* position, const char* token_value, const char* timestamp), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') JavaScript disallowed - Cannot run JavaScript', '"' + UTF8ToString(token_value) + '"', 'at', UTF8ToString(position));
});
EM_JS(void, warn_cant_enable_js, (const char* position, const char* timestamp, const char* filename, const char* filetype), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') Attempt to allow JavaScript at <import', UTF8ToString(filetype), '"'+UTF8ToString(filename)+'">', 'at', UTF8ToString(position));
});

EM_JS(void, warn_http_disabled, (const char* position, const char* url, const char* timestamp), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') Running lexer and parser only - Cannot fetch', '"' + UTF8ToString(url) + '"', 'at', UTF8ToString(position), '\nUse JUSTC.execute for HTTP requests.');
});

#ifdef __JUSTC_WEB__
EM_JS(int, use_luau, (const char* script, const char* timestamp, const char* position), {
    return __justc__luau__().then(function(luau) {
        var err = luau.ccall('executeScript', 'string', ['string'], [UTF8ToString(script)]);
        if (err) {
            var err_text = err.replace('stdin:', '');
            console.error('[JUSTC] (' + UTF8ToString(timestamp) + ') Luau error:', err_text + '\nat Luau, line', parseInt(err_text) - 1, '\nat' + UTF8ToString(position));
            return 1;
        }
        return 0;
    }).catch(function(e) {
        console.error('[JUSTC] (' + UTF8ToString(timestamp) + ') Luau loading error:', e, 'at' + UTF8ToString(position));
        return 1;
    });
});
#else
EM_JS(int, use_luau, (const char* script, const char* timestamp, const char* position), {
    console.warn('[JUSTC] (' + UTF8ToString(timestamp) + ') Luau debug:', UTF8ToString(script) + '\nat', UTF8ToString(position));
    return 0;
});
#endif

#endif
