#!/bin/bash

set -e
set -o pipefail

readonly SOURCE_DIR="src"
readonly NATIVE_DIR="native"
readonly BIN_DIR="bin"
readonly INCLUDE_DIR="$NATIVE_DIR/include"
readonly LIB_NAME="libnative_engine.so"

export JAVA_HOME="${JAVA_HOME:?Error: JAVA_HOME is not set}"
export JAVA_INC="$JAVA_HOME/include"
export OS_NAME=$(uname -s | tr '[:upper:]' '[:lower:]')
export JAVA_INC_OS="$JAVA_INC/$OS_NAME"

if [ -d "$BIN_DIR" ]; then rm -rf "$BIN_DIR"; fi
if [ -d "$INCLUDE_DIR" ]; then rm -rf "$INCLUDE_DIR"; fi
mkdir -p "$BIN_DIR"
mkdir -p "$INCLUDE_DIR"

find "$SOURCE_DIR" -name "*.java" > /tmp/java_sources.txt

while IFS= read -r file; do
    javac -d "$BIN_DIR" "$file"
done < /tmp/java_sources.txt

while IFS= read -r file; do
    javac -h "$INCLUDE_DIR" -d "$BIN_DIR" "$file"
done < /tmp/java_sources.txt

gcc -shared -fPIC -O3 -march=native \
    -I"$JAVA_INC" \
    -I"$JAVA_INC_OS" \
    -I"$INCLUDE_DIR" \
    "$NATIVE_DIR/arch_optim.c" \
    -o "$LIB_NAME"

if [ ! -f "$LIB_NAME" ]; then
    echo "Error: compilación fallida"
    exit 1
fi

echo "Compilación completada exitosamente"
