#!/bin/bash

set -euo pipefail

SRC_DIR="src"
NATIVE_DIR="native"
BIN_DIR="bin"
INCLUDE_DIR="$NATIVE_DIR/include"
SO_NAME="libnative_engine.so"

JAVA_HOME="${JAVA_HOME:?JAVA_HOME no definido}"
JAVA_INC="$JAVA_HOME/include"
OS_DIR=$(uname -s | tr '[:upper:]' '[:lower:]')
JAVA_OS_INC="$JAVA_INC/$OS_DIR"

mkdir -p "$BIN_DIR" "$INCLUDE_DIR"
rm -f "$SO_NAME"

for src in $(find "$SRC_DIR" -type f -name "*.java"); do
    javac -d "$BIN_DIR" "$src"
done

for src in $(find "$SRC_DIR" -type f -name "*.java"); do
    javac -h "$INCLUDE_DIR" -d "$BIN_DIR" "$src"
done

gcc -shared -fPIC -O3 -march=native \
    -I"$JAVA_INC" -I"$JAVA_OS_INC" -I"$INCLUDE_DIR" \
    "$NATIVE_DIR/arch_optim.c" \
    -o "$SO_NAME"

[[ -f "$SO_NAME" ]] || { echo "Error compilando librería"; exit 1; }

echo "Proceso finalizado correctamente"
