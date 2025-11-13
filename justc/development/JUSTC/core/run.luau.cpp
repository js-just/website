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

#include "run.luau.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>

#include "lua.h"
#include "lualib.h"
#include "luacode.h"

#ifdef __EMSCRIPTEN__
#include "run.luau.emscripten.hpp"
#endif

#define __JUSTC_LUAU_DEBUG__
void RunLuau::debug(const std::string log) {
    #ifdef __JUSTC_LUAU_DEBUG__
    #ifdef __EMSCRIPTEN__
    debug_luau(log.c_str());
    #else
    std::cout << "Luau debug: " << log << std::endl;
    #endif
    #endif
}

class LuaStateManager {
private:
    lua_State* L;

public:
    LuaStateManager() {
        L = luaL_newstate();
        if (!L) {
            throw std::runtime_error("Failed to create Lua state");
        }
        RunLuau::debug("Lua state created successfully.");

        luaL_openlibs(L);
        RunLuau::debug("openlibs()");

        RunLuau::debug("Redefining unsafe environment...");

        lua_pushnil(L);
        lua_setglobal(L, "dofile");
        RunLuau::debug("dofile = nil");

        lua_pushnil(L);
        lua_setglobal(L, "loadfile");
        RunLuau::debug("loadfile = nil");

        lua_pushnil(L);
        lua_setglobal(L, "io");
        RunLuau::debug("io = nil");

        lua_pushnil(L);
        lua_setglobal(L, "os");
        RunLuau::debug("os = nil");

        lua_pushnil(L);
        lua_setglobal(L, "package");
        RunLuau::debug("package = nil");

        lua_pushnil(L);
        lua_setglobal(L, "debug");
        RunLuau::debug("debug = nil");

        RunLuau::debug("Redefined unsafe environment successfully.");
    }

    ~LuaStateManager() {
        if (L) {
            RunLuau::debug("Closing Lua state...");
            lua_close(L);
            RunLuau::debug("Lua state closed successfully.");
        }
    }

    lua_State* getState() {
        RunLuau::debug("getState()");
        return L;
    }
};

void RunLuau::runScript(const std::string& code) {
    debug("Creating Lua state...");
    LuaStateManager luaManager;
    lua_State* L = luaManager.getState();

    debug("Compiling Luau code...");
    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(code.c_str(), code.size(), NULL, &bytecodeSize);
    if (!bytecode) {
        throw std::runtime_error("Failed to compile Luau code");
    }
    debug("Luau code compiled successfully.");

    debug("luau_load()");
    int result = luau_load(L, "user_code", bytecode, bytecodeSize, 0);
    free(bytecode);

    if (result != 0) {
        debug("Failed to compile. Getting compilation error...");
        const char* error = lua_tostring(L, -1);
        debug("Compilation error: " + std::string(error));
        throw std::runtime_error(std::string("Compilation error: ") + (error ? error : "Unknown error"));
    }

    debug("Executing Luau code...");
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        debug("Failed to execute. Getting runtime error...");
        const char* error = lua_tostring(L, -1);
        debug("Runtime error: " + std::string(error));
        throw std::runtime_error(std::string("Runtime error: ") + (error ? error : "Unknown error"));
    }

    debug("Done!");

    lua_close(L);
}

std::string RunLuau::runScriptWithResult(const std::string& code) {
    LuaStateManager luaManager;
    lua_State* L = luaManager.getState();

    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(code.c_str(), code.size(), NULL, &bytecodeSize);
    if (!bytecode) {
        throw std::runtime_error("Failed to compile Lua code");
    }

    int result = luau_load(L, "user_code", bytecode, bytecodeSize, 0);
    free(bytecode);

    if (result != 0) {
        const char* error = lua_tostring(L, -1);
        throw std::runtime_error(std::string("Compilation error: ") + (error ? error : "Unknown error"));
    }

    result = lua_pcall(L, 0, 1, 0);
    if (result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        throw std::runtime_error(std::string("Runtime error: ") + (error ? error : "Unknown error"));
    }

    std::string output;
    if (lua_isstring(L, -1)) {
        output = lua_tostring(L, -1);
    } else if (lua_isnumber(L, -1)) {
        output = std::to_string(lua_tonumber(L, -1));
    } else if (lua_isboolean(L, -1)) {
        output = lua_toboolean(L, -1) ? "true" : "false";
    } else if (lua_isnil(L, -1)) {
        output = "nil";
    } else {
        output = lua_typename(L, lua_type(L, -1));
    }

    lua_pop(L, 1);
    lua_close(L);
    return output;
}

bool RunLuau::compileScript(const std::string& code, std::string& error) {
    LuaStateManager luaManager;
    lua_State* L = luaManager.getState();

    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(code.c_str(), code.size(), NULL, &bytecodeSize);

    if (!bytecode) {
        error = "Compilation failed";
        return false;
    }

    free(bytecode);
    return true;
}
