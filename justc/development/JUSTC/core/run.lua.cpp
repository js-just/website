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
#include <iostream>
#include "version.h"
#include <string>
#include <cstring>
#include <stdexcept>

RunLua::RunLua() {
    L = luaL_newstate();
    if (!L) {
        throw std::runtime_error("Failed to create Lua state");
    }

    try {
        luaL_openlibs(L);
        initJUSTC();
    } catch (const std::exception& e) {
        lua_close(L);
        throw;
    }
}

RunLua::~RunLua() {
    if (L) {
        lua_close(L);
    }
}

bool RunLua::executeScript(const std::string& script) {
    if (!L) {
        std::cerr << "Lua state is not initialized" << std::endl;
        return false;
    }

    try {
        int result = luaL_dostring(L, script.c_str());
        if (result != LUA_OK) {
            const char* error = lua_tostring(L, -1);
            std::cerr << "Lua error: " << (error ? error : "Unknown error") << std::endl;
            lua_pop(L, 1);
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in Lua execution: " << e.what() << std::endl;
        return false;
    }
}

bool RunLua::executeFile(const std::string& filename) {
    if (!L) return false;

    int result = luaL_dofile(L, filename.c_str());
    if (result != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void RunLua::registerFunction(const std::string& name, lua_CFunction func) {
    if (L) {
        lua_register(L, name.c_str(), func);
    }
}

int justc_version_lua(lua_State* L) {
    lua_pushstring(L, JUSTC_VERSION.c_str());
    return 1;
}

void RunLua::initJUSTC() {
    registerFunction("justc_version", justc_version_lua);
}
