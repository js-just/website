#include "run.lua.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>

RunLua::RunLua() {
    std::cout << "DEBUG: Creating Lua state..." << std::endl;
    L = luaL_newstate();
    if (!L) {
        throw std::runtime_error("Failed to create Lua state");
    }

    std::cout << "DEBUG: Lua state created successfully" << std::endl;

    try {
        std::cout << "DEBUG: Opening Lua libraries..." << std::endl;
        luaL_openlibs(L);
        std::cout << "DEBUG: Lua libraries opened" << std::endl;

        initJUSTC();
        std::cout << "DEBUG: JUSTC initialization completed" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "DEBUG: Exception during Lua init: " << e.what() << std::endl;
        lua_close(L);
        throw std::runtime_error(std::string("Lua initialization failed: ") + e.what());
    } catch (...) {
        std::cerr << "DEBUG: Unknown exception during Lua init" << std::endl;
        lua_close(L);
        throw std::runtime_error("Unknown Lua initialization error");
    }
}

RunLua::~RunLua() {
    if (L) {
        std::cout << "DEBUG: Closing Lua state..." << std::endl;
        lua_close(L);
    }
}

bool RunLua::executeScript(const std::string& script) {
    if (!L) {
        std::cerr << "ERROR: Lua state is not initialized" << std::endl;
        return false;
    }

    std::cout << "DEBUG: Executing Lua script: " << script << std::endl;

    if (script.empty()) {
        std::cerr << "ERROR: Empty Lua script" << std::endl;
        return false;
    }

    std::cout << "DEBUG: Loading Lua script..." << std::endl;
    int load_result = luaL_loadstring(L, script.c_str());
    if (load_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::string error_msg = error ? error : "Unknown load error";
        std::cerr << "ERROR: Lua load failed (" << load_result << "): " << error_msg << std::endl;
        lua_pop(L, 1);
        return false;
    }
    std::cout << "DEBUG: Lua script loaded successfully" << std::endl;

    std::cout << "DEBUG: Executing Lua script with pcall..." << std::endl;
    int call_result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (call_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::string error_msg = error ? error : "Unknown execution error";
        std::cerr << "ERROR: Lua execution failed (" << call_result << "): " << error_msg << std::endl;

        switch (call_result) {
            case LUA_ERRRUN: std::cerr << "ERROR: Runtime error" << std::endl; break;
            case LUA_ERRMEM: std::cerr << "ERROR: Memory allocation error" << std::endl; break;
            case LUA_ERRERR: std::cerr << "ERROR: Error while running error handler" << std::endl; break;
            case LUA_ERRSYNTAX: std::cerr << "ERROR: Syntax error" << std::endl; break;
            default: std::cerr << "ERROR: Unknown error code" << std::endl; break;
        }

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
    std::string output;
    for (int i = 1; i <= n; i++) {
        if (lua_isstring(L, i)) {
            output += lua_tostring(L, i);
            if (i < n) output += "\t";
        }
    }
    std::cout << "LUA PRINT: " << output << std::endl;
    return 0;
}

int justc_version_lua(lua_State* L) {
    lua_pushstring(L, "1.0.0");
    return 1;
}

int lua_safe_arithmetic(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
        lua_pushstring(L, "Expected 2 arguments");
        return lua_error(L);
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
        lua_pushstring(L, "Expected numbers");
        return lua_error(L);
    }

    double a = lua_tonumber(L, 1);
    double b = lua_tonumber(L, 2);
    lua_pushnumber(L, a + b);
    return 1;
}

void RunLua::initJUSTC() {
    std::cout << "DEBUG: Initializing JUSTC Lua functions..." << std::endl;

    registerFunction("print", lua_print_wrapper);
    registerFunction("justc_version", justc_version_lua);
    registerFunction("add", lua_safe_arithmetic);

    std::cout << "DEBUG: JUSTC Lua functions registered" << std::endl;

    lua_pushnil(L);
    lua_setglobal(L, "dofile");

    lua_pushnil(L);
    lua_setglobal(L, "loadfile");

    lua_pushnil(L);
    lua_setglobal(L, "io");

    lua_pushnil(L);
    lua_setglobal(L, "os");

    lua_pushnil(L);
    lua_setglobal(L, "debug");

    std::cout << "DEBUG: Dangerous Lua functions disabled" << std::endl;

    std::string safe_env =
        "function safe_exec(code)\n"
        "  local loaded, err = load(code)\n"
        "  if not loaded then return nil, err end\n"
        "  return pcall(loaded)\n"
        "end\n";

    if (luaL_dostring(L, safe_env.c_str()) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "WARNING: Failed to set up safe environment: " << (error ? error : "Unknown error") << std::endl;
        lua_pop(L, 1);
    } else {
        std::cout << "DEBUG: Safe Lua environment set up" << std::endl;
    }
}
