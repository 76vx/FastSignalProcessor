#!/bin/bash

set -e

SRC_PATH="src"
NATIVE_PATH="native"
BIN_PATH="bin"
JNI_INCLUDE="$NATIVE_PATH/include"
OUTPUT_LIB="libnative_engine.so"

JAVA_PATH="${JAVA_HOME:?JAVA_HOME indefinido}"
JAVA_GENERAL_INCLUDE="$JAVA_PATH/include"
JAVA_OS_INCLUDE="$JAVA_GENERAL_INCLUDE/$(uname -s | tr '[:upper:]' '[:lower:]')"

rm -rf "$BIN_PATH" "$JNI_INCLUDE"
mkdir -p "$BIN_PATH" "$JNI_INCLUDE"

JAVA_FILES=$(find "$SRC_PATH" -type f -name "*.java")
for file in $JAVA_FILES; do
    javac -d "$BIN_PATH" "$file"
done

for file in $JAVA_FILES; do
    javac -h "$JNI_INCLUDE" -d "$BIN_PATH" "$file"
done

gcc -shared -fPIC -O3 -march=native \
    -I"$JAVA_GENERAL_INCLUDE" \
    -I"$JAVA_OS_INCLUDE" \
    -I"$JNI_INCLUDE" \
    "$NATIVE_PATH/arch_optim.c" \
    -o "$OUTPUT_LIB"

if [[ ! -f "$OUTPUT_LIB" ]]; then
    echo "Error: librería no compilada"
    exit 1
fi

echo "Librería compilada con éxito"
