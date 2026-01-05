#!/bin/bash

set -e

SRC="src"
NATIVE="native"
BIN="bin"
INCLUDE="$NATIVE/include"
LIBRARY="libnative_engine.so"

JAVA_HOME="${JAVA_HOME:?JAVA_HOME no configurado}"
JAVA_INCLUDE="$JAVA_HOME/include"
OS_SPEC=$(uname -s | tr '[:upper:]' '[:lower:]')
JAVA_INCLUDE_OS="$JAVA_INCLUDE/$OS_SPEC"

[ -d "$BIN" ] && rm -rf "$BIN"
[ -d "$INCLUDE" ] && rm -rf "$INCLUDE"
mkdir -p "$BIN" "$INCLUDE"

files=($(find "$SRC" -name "*.java"))
for f in "${files[@]}"; do
    javac -d "$BIN" "$f"
done

for f in "${files[@]}"; do
    javac -h "$INCLUDE" -d "$BIN" "$f"
done

INCLUDE_FLAGS="-I$JAVA_INCLUDE -I$JAVA_INCLUDE_OS -I$INCLUDE"
gcc -shared -fPIC -O3 -march=native $INCLUDE_FLAGS "$NATIVE/arch_optim.c" -o "$LIBRARY"

if [[ ! -f "$LIBRARY" ]]; then
    echo "Compilación fallida"
    exit 1
fi

echo "Todo compilado correctamente"
