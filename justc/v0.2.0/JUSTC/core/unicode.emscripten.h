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

EM_JS(char*, unicode_upper_js, (const char* input), {
    const s = UTF8ToString(input);
    const r = s.toUpperCase();

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);

    stringToUTF8(r, ptr, len);

    return ptr;
});

EM_JS(char*, unicode_lower_js, (const char* input), {
    const s = UTF8ToString(input);
    const r = s.toLowerCase();

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);

    stringToUTF8(r, ptr, len);

    return ptr;
});

EM_JS(char*, unicode_nfc_js, (const char* input), {
    const r = UTF8ToString(input).normalize("NFC");

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);

    stringToUTF8(r, ptr, len);

    return ptr;
});
EM_JS(char*, unicode_nfd_js, (const char* input), {
    const r = UTF8ToString(input).normalize("NFD");

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);

    stringToUTF8(r, ptr, len);

    return ptr;
});
EM_JS(char*, unicode_nfkc_js, (const char* input), {
    const r = UTF8ToString(input).normalize("NFKC");

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);

    stringToUTF8(r, ptr, len);

    return ptr;
});
EM_JS(char*, unicode_nfkd_js, (const char* input), {
    const r = UTF8ToString(input).normalize("NFKD");

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);

    stringToUTF8(r, ptr, len);

    return ptr;
});

EM_JS(int, unicode_equals_ignore_case_js, (const char* left, const char* right), {
    const a = UTF8ToString(left);
    const b = UTF8ToString(right);

    return a.toLocaleLowerCase() === b.toLocaleLowerCase();
});

EM_JS(int, unicode_codepoint_length_js, (const char* input), {
    return Array.from(UTF8ToString(input)).length;
});
EM_JS(char*, unicode_codepoint_reverse_js, (const char* input), {
    const r = Array.from(UTF8ToString(input)).reverse().join("");

    const len = lengthBytesUTF8(r) + 1;
    const ptr = _malloc(len);
    stringToUTF8(r, ptr, len);

    return ptr;
});
EM_JS(char*, unicode_codepoint_slice_js, (const char* input, int start, int end), {
    const arr = Array.from(UTF8ToString(input));
    const len = arr.length;

    if (start < 0) start += len;
    if (end < 0) end += len;

    start = Math.max(0, Math.min(start, len));
    end = Math.max(0, Math.min(end, len));

    let reverse = false;
    if (start > end) {
        const tmp = start;
        start = end;
        end = tmp;
        reverse = true;
    }

    let result = arr.slice(start, end);
    if (reverse) result.reverse();
    result = result.join("");

    const bytes = lengthBytesUTF8(result) + 1;
    const ptr = _malloc(bytes);
    stringToUTF8(result, ptr, bytes);

    return ptr;
});

EM_JS(int, unicode_grapheme_length_js, (const char* input), {
    const arr = Array.from(new Intl.Segmenter(
        undefined,
        {granularity: "grapheme"}
    ).segment(UTF8ToString(input)));

    return arr.length;
});
EM_JS(char*, unicode_grapheme_reverse_js, (const char* input), {
    const arr = Array.from(new Intl.Segmenter(
        undefined,
        {granularity: "grapheme"}
    ).segment(UTF8ToString(input)),
        x => x.segment
    );
    const result = arr.reverse().join("");

    const bytes = lengthBytesUTF8(result) + 1;
    const ptr = _malloc(bytes);
    stringToUTF8(result, ptr, bytes);

    return ptr;
});
EM_JS(char*, unicode_grapheme_slice_js, (const char* input, int start, int end), {
    const arr = Array.from(new Intl.Segmenter(
        undefined,
        {granularity: "grapheme"}
    ).segment(UTF8ToString(input)),
        x => x.segment
    );
    const len = arr.length;

    if (start < 0) start += len;
    if (end < 0) end += len;

    start = Math.max(0, Math.min(start, len));
    end = Math.max(0, Math.min(end, len));

    let reverse = false;
    if (start > end) {
        const tmp = start;
        start = end;
        end = tmp;
        reverse = true;
    }

    let result = arr.slice(start, end);
    if (reverse) result.reverse();
    result = result.join("");

    const bytes = lengthBytesUTF8(result) + 1;
    const ptr = _malloc(bytes);
    stringToUTF8(result, ptr, bytes);

    return ptr;
});

EM_JS(int, unicode_is_whitespace_js, (const char* input), {
    const s = UTF8ToString(input);

    if (s.length === 0) return 1;

    for (const ch of s) {
        const cp = ch.codePointAt(0);

        if (!/\\p{White_Space}/u.test(ch)) {
            return 0;
        }
    }

    return 1;
});

#endif
