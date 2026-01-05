#!/bin/bash

set -e
set -u

SRC_DIR="src"
NATIVE_DIR="native"
BIN_DIR="bin"
INCLUDE_DIR="$NATIVE_DIR/include"
LIB_SO="libnative_engine.so"

JAVA_HOME="${JAVA_HOME:?JAVA_HOME no definido}"
JAVA_INC="$JAVA_HOME/include"
JAVA_INC_OS="$JAVA_INC/$(uname -s | tr '[:upper:]' '[:lower:]')"

rm -rf "$BIN_DIR" "$INCLUDE_DIR"
mkdir -p "$BIN_DIR"
mkdir -p "$INCLUDE_DIR"

SRC_FILES=$(find "$SRC_DIR" -name "*.java")

for f in $SRC_FILES; do
    javac -d "$BIN_DIR" "$f"
done

for f in $SRC_FILES; do
    javac -h "$INCLUDE_DIR" -d "$BIN_DIR" "$f"
done

gcc_flags="-shared -fPIC -O3 -march=native"
include_flags="-I$JAVA_INC -I$JAVA_INC_OS -I$INCLUDE_DIR"

gcc $gcc_flags $include_flags "$NATIVE_DIR/arch_optim.c" -o "$LIB_SO"

if [ ! -f "$LIB_SO" ]; then
    echo "Compilación fallida"
    exit 1
fi

echo "Compilación terminada correctamente"
