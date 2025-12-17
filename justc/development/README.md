> [!WARNING]
> ### JUSTC language is currently in development and is experimental!
> Documentation is coming after release of new Just an Ultimate Site Tool update. Currently, you can use [experimental JUSTC demo website](https://just.js.org/justc/development/test.html).

# <img align="top" src="https://just.js.org/justc/logo-50.svg" alt="JUSTC Logo" width="40" height="40"> JUSTC [![Compile](https://github.com/js-just/JUSTC/actions/workflows/compile.yml/badge.svg)](https://github.com/js-just/JUSTC/actions/workflows/compile.yml)
JUSTC (Just an Ultimate Site Tool Configuration language) is a powerful, small, safe, human-optimized, easy-to-use object notation (or configuration) language designed to replace JSON (JavaScript Object Notation) and also designed to be backwards compatible with JSON, with embeddable JavaScript and Luau.

# Installation and usage
```
npm i -g justc
```

CLI:
```
justc --help
```

> [!NOTE]
> You can remove `-g` flag if you don't want to install JUSTC globally.
> ```
> npm i justc
> ```
> CLI:
> ```
> npx justc --help
> ```

JavaScript (Node.js):
```js
const JUSTC = require("justc");

// example:
JUSTC.execute(`

    foo = "Hello ",
    bar = "World!",
    baz = value(foo)..value(bar),

    output specified,
    return [baz] as ['output'].

`).then(result => console.log(result));
```

JavaScript (Browsers):
```js
await JUSTC.initialize()

// example
const result = JUSTC.execute(`

    foo = "Hello ",
    bar = "World!",
    baz = value(foo)..value(bar),

    output specified,
    return [baz] as ['output'].

`);
console.log(result);
```

# Dependencies
JUSTC uses C++ as its implementation language. The entire project requires C++17. It should build (and compile to WebAssembly) without issues on Linux Ubuntu.[^1]

JUSTC depends on:
- [Luau](https://github.com/luau-lang/luau) by [Roblox](https://github.com/Roblox) [^2]
- [JSON](https://github.com/nlohmann/json) by [Niels Lohmann](https://github.com/nlohmann) [^3]
- [CPR](https://github.com/libcpr/cpr) [^4]
- [QuickJS CMake](https://github.com/robloach/quickjs-cmake) by [Rob Loach](https://github.com/RobLoach) [^5]
- [QuickJS](https://github.com/bellard/quickjs) by [bellard](https://github.com/bellard) [^6]

> [!NOTE]
> JUSTC WebAssembly module does not depend on QuickJS and/or QuickJS CMake. Instead, it uses JavaScript `eval` function.

[^1]: DOWNLOADING THE SOURCE CODE AND BUILDING IT ON YOUR MACHINE IS NOT RECOMMENDED AND IS AT YOUR OWN RISK. Instead, install the compiled JUSTC WebAssembly module as an NPM package.
[^2]: Luau is licensed under the MIT License: https://github.com/luau-lang/luau/blob/master/LICENSE.txt
[^3]: JSON by Niels Lohmann is licensed under the MIT License: https://github.com/nlohmann/json/blob/develop/LICENSE.MIT
[^4]: CPR is licensed under the MIT License: https://github.com/libcpr/cpr/blob/master/LICENSE
[^5]: QuickJS CMake is licensed under the MIT License: https://github.com/RobLoach/quickjs-cmake/blob/master/LICENSE
[^6]: QuickJS is licensed under the MIT License: https://github.com/bellard/quickjs/blob/master/LICENSE

# License
JUSTC implementation is distributed under the terms of [MIT License](https://github.com/js-just/JUSTC/blob/main/LICENSE).

When JUSTC is integrated into external projects, we ask that you honor the license agreement and include JUSTC attribution into the user-facing product documentation. Attribution making use of the [JUSTC logo](https://github.com/js-just/website/blob/main/justc/logo.svg) is also encouraged when reasonable.
