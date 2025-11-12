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

#include "run.lua.hpp"
#define LUA_IMPL
#include <minilua/minilua.h>
#include <string>
#include <cstring>
#include <stdexcept>

void RunLua::runScript(const std::string& code) {
    lua_State *L = luaL_newstate();
    if (L == NULL)
        throw std::runtime_error("Failed to create Lua state");

    luaL_openlibs(L);

    int load_result = luaL_loadstring(L, code.c_str());
    if (load_result != LUA_OK) {
        std::string error_msg = lua_tostring(L, -1);
        lua_close(L);
        throw std::runtime_error("Lua load error: " + error_msg);
    }

    int call_result = lua_pcall(L, 0, 0, 0);
    if (call_result != LUA_OK) {
        std::string error_msg = lua_tostring(L, -1);
        lua_close(L);
        throw std::runtime_error("Lua runtime error: " + error_msg);
    }

    lua_close(L);
}
