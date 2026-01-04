#include <jni.h>
#include <stdlib.h>
#include <omp.h>

JNIEXPORT void JNICALL Java_com_processor_NativeEngine_applyGainFilter(
    JNIEnv *env, 
    jobject obj, 
    jbyteArray data, 
    jfloat gain, 
    jint iterations) 
{
    jsize len = (*env)->GetArrayLength(env, data);
    jbyte *buffer = (*env)->GetPrimitiveArrayCritical(env, data, NULL);

    if (buffer == NULL) return;

    if (iterations > 0) {
        #pragma omp parallel
        {
            #pragma omp for schedule(static)
            for (int i = 0; i < len; i++) {
                float sample = (float)buffer[i];
                
                for (int iter = 0; iter < iterations; iter++) {
                    sample *= gain;
                    
                    if (sample > 127.0f) {
                        sample = 127.0f;
                        break; 
                    }
                    if (sample < -128.0f) {
                        sample = -128.0f;
                        break;
                    }
                }
                buffer[i] = (jbyte)sample;
            }
        }
    }

    (*env)->ReleasePrimitiveArrayCritical(env, data, buffer, 0);
}