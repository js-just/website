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
#include <stdexcept>
#include <cstring>

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
        throw std::runtime_error(std::string("Lua initialization failed: ") + e.what());
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

    std::cout << "DEBUG: Executing Lua script: " << script << std::endl;

    int load_result = luaL_loadstring(L, script.c_str());
    if (load_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "Lua load error: " << (error ? error : "Unknown load error") << std::endl;
        lua_pop(L, 1);
        return false;
    }

    int call_result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (call_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "Lua execution error: " << (error ? error : "Unknown execution error") << std::endl;
        std::cerr << "Error code: " << call_result << std::endl;
        lua_pop(L, 1);
        return false;
    }

    std::cout << "DEBUG: Lua script executed successfully" << std::endl;
    return true;
}

bool RunLua::executeFile(const std::string& filename) {
    if (!L) {
        std::cerr << "Lua state is not initialized" << std::endl;
        return false;
    }

    int result = luaL_dofile(L, filename.c_str());
    if (result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "Lua file error: " << (error ? error : "Unknown file error") << std::endl;
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

int lua_print_wrapper(lua_State* L) {
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++) {
        if (lua_isstring(L, i)) {
            std::cout << lua_tostring(L, i) << " ";
        }
    }
    std::cout << std::endl;
    return 0;
}

int justc_version_lua(lua_State* L) {
    lua_pushstring(L, "1.0.0");
    return 1;
}

void RunLua::initJUSTC() {
    registerFunction("print", lua_print_wrapper);
    registerFunction("justc_version", justc_version_lua);

    lua_pushnil(L);
    lua_setglobal(L, "dofile");

    lua_pushnil(L);
    lua_setglobal(L, "loadfile");

    lua_pushnil(L);
    lua_setglobal(L, "io");

    lua_pushnil(L);
    lua_setglobal(L, "os");
}
