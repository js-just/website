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

#include <string>
#include <string_view>
#include <cstddef>

namespace Unicode {

std::string Upper(std::string_view str);
std::string Lower(std::string_view str);

std::string NormalizeNFC(std::string_view str);
std::string NormalizeNFD(std::string_view str);
std::string NormalizeNFKC(std::string_view str);
std::string NormalizeNFKD(std::string_view str);

bool EqualsIgnoreCase(
    std::string_view left,
    std::string_view right
);

bool IsWhitespace(std::string_view str);

size_t ByteLength(std::string_view str);
std::string ByteSlice(
    std::string_view str,
    int64_t start,
    int64_t end
);
std::string ByteReverse(
    std::string_view str
);

size_t CodePointLength(std::string_view str);
std::string CodePointSlice(
    std::string_view str,
    int64_t start,
    int64_t end
);
std::string CodePointReverse(
    std::string_view str
);

size_t GraphemeLength(std::string_view str);
std::string GraphemeSlice(
    std::string_view str,
    int64_t start,
    int64_t end
);
std::string GraphemeReverse(
    std::string_view str
);

}
