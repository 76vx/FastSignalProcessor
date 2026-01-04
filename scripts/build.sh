#!/bin/bash

set -e

readonly SOURCE_DIR="src"
readonly NATIVE_DIR="native"
readonly BIN_DIR="bin"
readonly INCLUDE_DIR="$NATIVE_DIR/include"
readonly LIB_NAME="libnative_engine.so"

export JAVA_INC="${JAVA_HOME:?Error: JAVA_HOME is not set}/include"
export JAVA_INC_OS="$JAVA_INC/$(uname -s | tr '[:upper:]' '[:lower:]')"

rm -rf "$BIN_DIR" "$INCLUDE_DIR"
mkdir -p "$BIN_DIR" "$INCLUDE_DIR"

javac -d "$BIN_DIR" "$SOURCE_DIR/com/processor/NativeEngine.java"

javac -h "$INCLUDE_DIR" \
      -d "$BIN_DIR" \
      "$SOURCE_DIR/com/processor/NativeEngine.java"

gcc -shared -fPIC -O3 \
    -march=native \
    -I"$JAVA_INC" \
    -I"$JAVA_INC_OS" \
    -I"$INCLUDE_DIR" \
    "$NATIVE_DIR/arch_optim.c" \
    -o "$LIB_NAME"

if [ ! -f "$LIB_NAME" ]; then
    exit 1
fi
