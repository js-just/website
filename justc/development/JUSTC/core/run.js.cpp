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

#include "run.js.hpp"
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <utility>
#include <stdexcept>
extern "C" {
    #include <quickjs.h>
}

JSValue JavaScript::Print(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if (argc > 0) {
        void* userdata = JS_GetContextOpaque(ctx);
        if (userdata != nullptr) {
            std::stringstream* output = static_cast<std::stringstream*>(userdata);
            size_t len;
            const char *str = JS_ToCStringLen(ctx, &len, argv[0]);
            *output << str << "\n";
            JS_FreeCString(ctx, str);
        }
    }
    return JS_UNDEFINED;
}

void JavaScript::DefineConsole(JSRuntime *qjs_rt, JSContext *qjs_ctx, void* userdata) {
    JSValue global_obj = JS_GetGlobalObject(qjs_ctx);
    JSValue console_obj = JS_NewObject(qjs_ctx);

    std::vector<std::string> console_functions = {"log", "info", "warn", "error", "debug"};
    for (const std::string& funcName : console_functions) {
        JS_SetPropertyStr(qjs_ctx, console_obj, funcName.c_str(),
                         JS_NewCFunction(qjs_ctx, Print, funcName.c_str(), 1));
    }

    JS_SetPropertyStr(qjs_ctx, global_obj, "console", console_obj);
    JS_FreeValue(qjs_ctx, global_obj);
}

std::pair<std::string, bool> JavaScript::Eval(const std::string& script, bool Console) {
    std::stringstream output;
    bool crashed = false;

    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    if (!rt || !ctx) {
        throw std::runtime_error("Failed to initialize QuickJS runtime or context.");
    }

    JS_SetContextOpaque(ctx, &output);

    if (Console) {
        DefineConsole(rt, ctx, &output);
    }

    JSValue result = JS_Eval(ctx, script.c_str(), script.length(), "<input>", JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(result)) {
        JSValue exception_val = JS_GetException(ctx);
        const char* exception_str = JS_ToCString(ctx, exception_val);
        output << exception_str;
        crashed = true;
        JS_FreeCString(ctx, exception_str);
        JS_FreeValue(ctx, exception_val);
    } else {
        const char* result_str = JS_ToCString(ctx, result);
        output << result_str;
        JS_FreeCString(ctx, result_str);
    }

    JS_FreeValue(ctx, result);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return {output.str(), crashed};
}
