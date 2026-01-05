#include <jni.h>
#include <immintrin.h>
#include <omp.h>
#include <stdint.h>

JNIEXPORT void JNICALL Java_com_processor_NativeEngine_applyGainFilterDirect(
    JNIEnv *env,
    jobject obj,
    jobject byteBuffer,
    jfloat gain,
    jint iterations)
{
    int8_t *data = (int8_t *)(*env)->GetDirectBufferAddress(env, byteBuffer);
    jlong capacity = (*env)->GetDirectBufferCapacity(env, byteBuffer);

    if (!data || capacity <= 0 || iterations <= 0) return;

    float total_gain = gain;
    for (int i = 1; i < iterations; i++) {
        total_gain *= gain;
    }

    const float minVal = -128.0f;
    const float maxVal = 127.0f;
    long vectorizedEnd = (capacity / 32) * 32;

    #pragma omp parallel
    {
        __m256 vGain = _mm256_set1_ps(total_gain);
        __m256 vMin  = _mm256_set1_ps(minVal);
        __m256 vMax  = _mm256_set1_ps(maxVal);

        #pragma omp for schedule(static)
        for (long i = 0; i < vectorizedEnd; i += 32) {
            __m128i bytesLo = _mm_loadu_si128((__m128i*)&data[i]);
            __m128i bytesHi = _mm_loadu_si128((__m128i*)&data[i + 16]);

            __m256 f0 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(bytesLo));
            __m256 f1 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(bytesLo, 8)));
            __m256 f2 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(bytesHi));
            __m256 f3 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(bytesHi, 8)));

            f0 = _mm256_min_ps(_mm256_max_ps(f0, vMin), vMax);
            f1 = _mm256_min_ps(_mm256_max_ps(f1, vMin), vMax);
            f2 = _mm256_min_ps(_mm256_max_ps(f2, vMin), vMax);
            f3 = _mm256_min_ps(_mm256_max_ps(f3, vMin), vMax);

            __m256i r0 = _mm256_cvtps_epi32(f0);
            __m256i r1 = _mm256_cvtps_epi32(f1);
            __m256i r2 = _mm256_cvtps_epi32(f2);
            __m256i r3 = _mm256_cvtps_epi32(f3);

            __m256i packedLow  = _mm256_packs_epi32(r0, r1);
            __m256i packedHigh = _mm256_packs_epi32(r2, r3);
            __m256i packedAll  = _mm256_packs_epi16(packedLow, packedHigh);

            __m128i low  = _mm256_castsi256_si128(packedAll);
            __m128i high = _mm256_extracti128_si256(packedAll, 1);

            _mm_storeu_si128((__m128i*)&data[i], low);
            _mm_storeu_si128((__m128i*)&data[i + 16], high);
        }
    }

    for (long i = vectorizedEnd; i < capacity; i++) {
        float val = (float)data[i] * total_gain;
        val = val < minVal ? minVal : (val > maxVal ? maxVal : val);
        data[i] = (int8_t)val;
    }
}
