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

#ifndef DATATYPE_HPP
#define DATATYPE_HPP

enum class DataType {
    JUSTC_OBJECT =  0,
    NUMBER       =  1,
    STRING       =  2,
    LINK         =  3,
    BOOLEAN      =  4,
    JSON_OBJECT  =  5,
    JSON_ARRAY   =  6,
    NULL_TYPE    =  7,
    HEXADECIMAL  =  9,
    BINARY       = 11,
    PATH         = 12,
    ERROR        = 13,
    VARIABLE     = 14,
    FUNCTION     = 15,
    NOT_A_NUMBER = 17,
    INFINITE     = 18,
    SYNTAX_ERROR = 19,
    OCTAL        = 20,
    CLASS        = 21,
    SPACE        = 22,
    BINARY_DATA  = 23,
    BIGNUM       = 24,
    LARGENUM     = 25,
    HUGENUM      = 26,
    GIANTNUM     = 27,
    COLOSSALNUM  = 28,
    UNKNOWN      = -1
};

#endif
