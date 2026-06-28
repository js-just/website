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

#include "justb.hpp"
#include "../justb.hpp"
#include <cereal/archives/binary.hpp>
#include <cereal/types/unordered_map.hpp>

ParseResult JustbLoader::load(const std::string& inputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open JUSTB file");
    return load(in);
}

ParseResult JustbLoader::load(std::istream& in) {
    JUSTB::Header header;
    if (!JUSTB::readHeader(in, header) || !JUSTB::validateHeader(header)) {
        throw std::runtime_error("Invalid JUSTB header");
    }

    ParseResult result;
    {
        cereal::BinaryInputArchive archive(in);
        archive(result.returnValues);
    }
    return result;
}
