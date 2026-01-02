#include <jni.h> //recuerda usarla, yo no la use y me saltaron varios errores, no seas estupido como yo
#include <stdlib.h>

/*
 * implementacion optimizada de procesamiento de señal.
 * se utiliza Critical Array access para evitar copias de memoria innecesarias. es decir solo use lo que tenga que usar 
 * y no desperdicie nada, si nno hacer esto es porq eres un estupido y tu logica de programacion
 * es una mierda, esto es una regla basica que aprendes cuando empiezas a desarrollar con C
 */
JNIEXPORT void JNICALL Java_com_processor_NativeEngine_applyGainFilter(
    JNIEnv *env, 
    jobject obj, 
    jbyteArray data, 
    jfloat gain, 
    jint iterations) 
{
    // Obtenemos acceso directo al heap de la JVM, algo sencillo la verdad
    jbyte *buffer = (*env)->GetPrimitiveArrayCritical(env, data, NULL);
    jsize len = (*env)->GetArrayLength(env, data);

    if (buffer == NULL) return;

    // Procesamiento intensivo, usado el for para realizar dichos procesamientos
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < len; i++) {
            // Aplicamos transformación de ganancia y normalización de bits
            float sample = (float)buffer[i] * gain;
            
            // Hard clipping simple
            if (sample > 127.0f) sample = 127.0f;
            if (sample < -128.0f) sample = -128.0f;
            
            buffer[i] = (jbyte)sample; //esta seccion de aqui me mato, esto lo desconocia hasta el dia de hoy
        }
    }

    // Liberar y confirmar cambios en la memoria de Java, lo pone en la documentacion, recomiendo leerla
    (*env)->ReleasePrimitiveArrayCritical(env, data, buffer, 0);
}