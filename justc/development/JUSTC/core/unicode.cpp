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

#include <algorithm>
#include <vector>
#include <string>
#include <string_view>
#include "unicode.hpp"

#ifdef __EMSCRIPTEN__

    #include "unicode.emscripten.h"
    #include <cstdlib>

#else

    #include <unicode/unistr.h>
    #include <unicode/normalizer2.h>
    #include <unicode/uchar.h>
    #include <unicode/brkiter.h>
    #include <unicode/utf8.h>
    #include <memory>

#endif

namespace Unicode {

#ifdef __EMSCRIPTEN__

    static std::string fromJS(char* ptr) {
        if (!ptr) return {};

        std::string result(ptr);

        free(ptr);

        return result;
    }

#endif

std::string Upper(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_upper_js(std::string(str).c_str()));

    #else

        icu::UnicodeString s = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        s.toUpper();

        std::string result;
        s.toUTF8String(result);

        return result;

    #endif
}

std::string Lower(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_lower_js(std::string(str).c_str()));

    #else

        icu::UnicodeString s = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        s.toLower();

        std::string result;
        s.toUTF8String(result);

        return result;

    #endif
}

std::string NormalizeNFC(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_nfc_js(std::string(str).c_str()));

    #else

        UErrorCode status = U_ZERO_ERROR;

        const auto* normalizer = icu::Normalizer2::getNFCInstance(status);

        icu::UnicodeString source = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        icu::UnicodeString result;

        normalizer->normalize(
            source,
            result,
            status
        );

        std::string output;
        result.toUTF8String(output);

        return output;

    #endif
}

std::string NormalizeNFD(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_nfd_js(std::string(str).c_str()));

    #else

        UErrorCode status = U_ZERO_ERROR;

        const auto* normalizer = icu::Normalizer2::getNFDInstance(status);

        icu::UnicodeString source = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        icu::UnicodeString result;

        normalizer->normalize(
            source,
            result,
            status
        );

        std::string output;
        result.toUTF8String(output);

        return output;

    #endif
}

std::string NormalizeNFKC(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_nfkc_js(std::string(str).c_str()));

    #else

        UErrorCode status = U_ZERO_ERROR;

        const auto* normalizer = icu::Normalizer2::getNFKCInstance(status);

        icu::UnicodeString source = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        icu::UnicodeString result;

        normalizer->normalize(
            source,
            result,
            status
        );

        std::string output;
        result.toUTF8String(output);

        return output;

    #endif
}

std::string NormalizeNFKD(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_nfkd_js(std::string(str).c_str()));

    #else

        UErrorCode status = U_ZERO_ERROR;

        const auto* normalizer = icu::Normalizer2::getNFKDInstance(status);

        icu::UnicodeString source = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        icu::UnicodeString result;

        normalizer->normalize(
            source,
            result,
            status
        );

        std::string output;
        result.toUTF8String(output);

        return output;

    #endif
}

bool EqualsIgnoreCase(std::string_view left, std::string_view right){
    #ifdef __EMSCRIPTEN__

        return unicode_equals_ignore_case_js(
            std::string(left).c_str(),
            std::string(right).c_str()
        );

    #else

        return Lower(left) == Lower(right);

    #endif
}

std::string ByteReverse(std::string_view str) {
    std::string res(str);
    std::reverse(res.begin(), res.end());
    return res;
}
size_t ByteLength(std::string_view str) {
    return str.size();
}
std::string ByteSlice(std::string_view str, int64_t start, int64_t end) {
    int64_t len = static_cast<int64_t>(str.size());

    if (start < 0) start += len;
    if (end < 0)   end += len;

    start = std::clamp(start, int64_t(0), len);
    end   = std::clamp(end, int64_t(0), len);

    bool reverse = false;
    if (start > end) {
        std::swap(start, end);
        reverse = true;
    }

    if (start == end) {
        return "";
    }

    std::string result{str.substr(start, end - start)};

    if (reverse) {
        std::reverse(result.begin(), result.end());
    }

    return result;
}

size_t CodePointLength(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return static_cast<size_t>(unicode_codepoint_length_js(std::string(str).c_str()));

    #else

        icu::UnicodeString s = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        return static_cast<size_t>(s.countChar32());

    #endif
}
std::string CodePointReverse(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_codepoint_reverse_js(std::string(str).c_str()));

    #else

        icu::UnicodeString s = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        icu::UnicodeString result;

        for (int32_t i = s.length(); i > 0;) {
            UChar32 cp = s.char32At(s.moveIndex32(i, -1));

            i = s.moveIndex32(i, -1);

            result.append(cp);
        }

        std::string out;
        result.toUTF8String(out);

        return out;

    #endif
}
std::string CodePointSlice(std::string_view str, int64_t start, int64_t end) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_codepoint_slice_js(
            std::string(str).c_str(),
            static_cast<int>(start),
            static_cast<int>(end)
        ));

    #else

        icu::UnicodeString s = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        int64_t len = static_cast<int64_t>(s.countChar32());

        if (start < 0) start += len;
        if (end < 0) end += len;

        start = std::clamp(start, int64_t(0), len);
        end = std::clamp(end, int64_t(0), len);

        bool reverse = false;

        if (start > end) {
            std::swap(start, end);
            reverse = true;
        }

        int32_t begin = s.moveIndex32(0, static_cast<int32_t>(start));

        int32_t finish = s.moveIndex32(begin, static_cast<int32_t>(end - start));

        icu::UnicodeString part = s.tempSubStringBetween(begin, finish);

        std::string result;
        part.toUTF8String(result);

        if (reverse) return CodePointReverse(result);

        return result;

    #endif
}

size_t GraphemeLength(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return static_cast<size_t>(unicode_grapheme_length_js(std::string(str).c_str()));

    #else

        UErrorCode status = U_ZERO_ERROR;

        icu::UnicodeString s = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        std::unique_ptr<icu::BreakIterator> it(icu::BreakIterator::createCharacterInstance(
            icu::Locale::getRoot(),
            status
        ));

        it->setText(s);

        size_t count = 0;

        for (int32_t pos = it->first(); pos != icu::BreakIterator::DONE; pos = it->next()) {
            ++count;
        }

        return count ? count - 1 : 0;

    #endif
}
std::string GraphemeReverse(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return fromJS(unicode_grapheme_reverse_js(std::string(str).c_str()));

    #else

        UErrorCode status = U_ZERO_ERROR;

        icu::UnicodeString text = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        std::unique_ptr<icu::BreakIterator> iterator(icu::BreakIterator::createCharacterInstance(
            icu::Locale::getRoot(),
            status
        ));

        if (U_FAILURE(status) || !iterator) return std::string(str);

        iterator->setText(text);

        std::vector<icu::UnicodeString> graphemes;

        int32_t start = iterator->first();

        for (int32_t end = iterator->next(); end != icu::BreakIterator::DONE; start = end, end = iterator->next()) {
            graphemes.emplace_back(text.tempSubStringBetween(
                start,
                end
            ));
        }

        std::reverse(graphemes.begin(), graphemes.end());

        icu::UnicodeString result;

        for (const auto& g : graphemes) result.append(g);

        std::string output;
        result.toUTF8String(output);

        return output;

    #endif
}
std::string GraphemeSlice(std::string_view str, int64_t start, int64_t end) {
    #if __EMSCRIPTEN__

        return fromJS(unicode_grapheme_slice_js(
            std::string(str).c_str(),
            static_cast<int>(start),
            static_cast<int>(end)
        ));

    #else

        UErrorCode status = U_ZERO_ERROR;

        icu::UnicodeString text = icu::UnicodeString::fromUTF8(icu::StringPiece(
            str.data(),
            static_cast<int32_t>(str.size())
        ));

        std::unique_ptr<icu::BreakIterator> iterator(icu::BreakIterator::createCharacterInstance(
            icu::Locale::getRoot(),
            status
        ));

        if (U_FAILURE(status) || !iterator) return {};

        iterator->setText(text);

        std::vector<int32_t> boundaries;

        for (int32_t pos = iterator->first(); pos != icu::BreakIterator::DONE; pos = iterator->next()) {
            boundaries.push_back(pos);
        }

        const int64_t len = static_cast<int64_t>(
            boundaries.size() > 0 ? boundaries.size() - 1 : 0
        );

        if (start < 0) start += len;

        if (end < 0) end += len;

        start = std::clamp(
            start,
            int64_t(0),
            len
        );

        end = std::clamp(
            end,
            int64_t(0),
            len
        );

        bool reverse = false;

        if (start > end) {
            std::swap(start, end);
            reverse = true;
        }

        if (start == end) return {};

        const int32_t begin = boundaries[
            static_cast<size_t>(start)
        ];

        const int32_t finish = boundaries[
            static_cast<size_t>(end)
        ];

        icu::UnicodeString result = text.tempSubStringBetween(
            begin,
            finish
        );

        std::string output;

        result.toUTF8String(output);

        if (reverse) return GraphemeReverse(output);

        return output;

    #endif
}

bool IsWhitespace(std::string_view str) {
    #ifdef __EMSCRIPTEN__

        return unicode_is_whitespace_js(std::string(str).c_str());

    #else

        if (str.empty()) return true;

        const char* data = str.data();
        int32_t length = static_cast<int32_t>(str.size());

        int32_t i = 0;

        while (i < length) {
            UChar32 cp;
            U8_NEXT(data, i, length, cp);

            if (cp < 0 || !u_hasBinaryProperty(cp, UCHAR_WHITE_SPACE)) {
                return false;
            }
        }

        return true;

    #endif
}

}
