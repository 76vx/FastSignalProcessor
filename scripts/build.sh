#!/bin/bash

set -e

SOURCE_DIR="src"
NATIVE_DIR="native"
BIN_DIR="bin"
INCLUDE_DIR="$NATIVE_DIR/include"
LIB_NAME="libnative_engine.so"

JAVA_HOME="${JAVA_HOME:?Error: JAVA_HOME no definido}"
JAVA_INC="$JAVA_HOME/include"
OS_NAME=$(uname -s | tr '[:upper:]' '[:lower:]')
JAVA_INC_OS="$JAVA_INC/$OS_NAME"

rm -rf "$BIN_DIR" "$INCLUDE_DIR" "$LIB_NAME"
mkdir -p "$BIN_DIR" "$INCLUDE_DIR"

for file in $(find "$SOURCE_DIR" -type f -name "*.java"); do
    echo "Compilando $file"
    javac -d "$BIN_DIR" "$file"
done

for file in $(find "$SOURCE_DIR" -type f -name "*.java"); do
    echo "Generando headers JNI para $file"
    javac -h "$INCLUDE_DIR" -d "$BIN_DIR" "$file"
done

CFLAGS="-shared -fPIC -O3 -march=native"
INCLUDES="-I$JAVA_INC -I$JAVA_INC_OS -I$INCLUDE_DIR"

gcc $CFLAGS $INCLUDES "$NATIVE_DIR/arch_optim.c" -o "$LIB_NAME"

if [[ -f "$LIB_NAME" ]]; then
    echo "Archivo $LIB_NAME generado correctamente"
else
    echo "Error: No se pudo generar $LIB_NAME"
    exit 1
fi
