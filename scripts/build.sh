#!/bin/bash

set -e

SRC="src"
NATIVE="native"
BIN="bin"
JNI_HDR="$NATIVE/include"
LIB_NAME="libnative_engine.so"

JAVA_HOME="${JAVA_HOME:?JAVA_HOME no configurado}"
JAVA_INCLUDE="$JAVA_HOME/include"
OS_INCLUDE="$JAVA_INCLUDE/$(uname -s | tr '[:upper:]' '[:lower:]')"

rm -rf "$BIN" "$JNI_HDR"
mkdir -p "$BIN" "$JNI_HDR"

for java_file in $(find "$SRC" -name "*.java"); do
    javac -d "$BIN" "$java_file"
done

for java_file in $(find "$SRC" -name "*.java"); do
    javac -h "$JNI_HDR" -d "$BIN" "$java_file"
done

gcc -shared -fPIC -O3 -march=native \
    -I"$JAVA_INCLUDE" \
    -I"$OS_INCLUDE" \
    -I"$JNI_HDR" \
    "$NATIVE/arch_optim.c" \
    -o "$LIB_NAME"

if [[ ! -f "$LIB_NAME" ]]; then
    echo "Error: librería no creada"
    exit 1
fi

echo "Proceso completo exitosamente"
