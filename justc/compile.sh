#!/bin/bash

SAFE_DIR=$(echo "$OUTPUT_DIR" | sed 's|/|_|g') || "${{ env.DEFAULT_DIR }}"
mkdir -p "browsers/$SAFE_DIR"

echo $OUTPUT_DIR
echo $SAFE_DIR

emcc core/browsers.cpp core/lexer.cpp core/parser.cpp core/json_serializer.cpp core/keywords.cpp \
    -o browsers/$SAFE_DIR/justc.core.js \
    -s EXPORTED_FUNCTIONS='["_lexer","_parser","_parse","_free_string","_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8"]' \
    -s MODULARIZE=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INVOKE_RUN=0 \
    -std=c++11 \
    -s DISABLE_EXCEPTION_CATCHING=0 \
    -s EXPORT_NAME='__justc__' \
    -s ASSERTIONS=0 \
    -Os

mv browsers/$SAFE_DIR/justc.core.wasm browsers/$SAFE_DIR/justc.wasm
sed -i 's/justc\.core\.wasm/justc.wasm/g' browsers/$SAFE_DIR/justc.core.js
mv browsers/$SAFE_DIR/core.js browsers/$SAFE_DIR/justc.js
