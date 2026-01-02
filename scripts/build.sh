#!/bin/bash

# Configuración de rutas, recueda ajustarla segun el sistema operativo que este usando
# creo que si entiendes algo de bash es logico lo que hice
export JAVA_INC="$JAVA_HOME/include"
export JAVA_INC_OS="$JAVA_HOME/include/linux" # Cambiar a /win32 para Windows

echo "[1/3] Compiling Java source..."
javac -d bin src/com/processor/NativeEngine.java

echo "[2/3] Generating JNI Headers..."
javac -h native/include src/com/processor/NativeEngine.java

echo "[3/3] Compiling Native C Engine..."
gcc -shared -fPIC -O3 \
    -I"$JAVA_INC" \
    -I"$JAVA_INC_OS" \
    native/arch_optim.c \
    -o libnative_engine.so

echo "Build Complete. Run with: java -Djava.library.path=. -cp bin com.processor.NativeEngine"


# un script sencillo, pero eficaz hablando para el funcionamiento del proyecto