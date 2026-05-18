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

#ifndef NUMBER_HPP
#define NUMBER_HPP

#ifndef __EMSCRIPTEN__

#include <boost/multiprecision/cpp_dec_float.hpp>

#else

#include "../boost/multiprecision/cpp_dec_float.hpp"

#endif

#include <variant>

using BigNum =
    boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<50>
    >;
using LargeNum =
    boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<100>
    >;
using HugeNum =
    boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<1000>
    >;
using GiantNum =
    boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<10000>
    >;
using ColossalNum =
    boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<100000>
    >;


using JUSTCnum = std::variant<
    double,
    BigNum,
    LargeNum,
    HugeNum,
    GiantNum,
    ColossalNum
>;

#endif
