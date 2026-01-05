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

    float total_gain = 1.0f;
    for (int i = 0; i < iterations; i++) {
        total_gain *= gain;
    }

    const float min_val = -128.0f;
    const float max_val = 127.0f;

    int chunk_size = 32;
    long simd_end = (capacity / chunk_size) * chunk_size;

    #pragma omp parallel
    {
        __m256 v_gain = _mm256_set1_ps(total_gain);
        __m256 v_min = _mm256_set1_ps(min_val);
        __m256 v_max = _mm256_set1_ps(max_val);

        #pragma omp for schedule(static)
        for (long i = 0; i < simd_end; i += chunk_size) {
            __m128i low_bytes  = _mm_loadu_si128((__m128i*)&data[i]);
            __m128i high_bytes = _mm_loadu_si128((__m128i*)&data[i + 16]);

            __m256i int0 = _mm256_cvtepi8_epi32(low_bytes);
            __m256i int1 = _mm256_cvtepi8_epi32(_mm_srli_si128(low_bytes, 8));
            __m256i int2 = _mm256_cvtepi8_epi32(high_bytes);
            __m256i int3 = _mm256_cvtepi8_epi32(_mm_srli_si128(high_bytes, 8));

            __m256 f0 = _mm256_mul_ps(_mm256_cvtepi32_ps(int0), v_gain);
            __m256 f1 = _mm256_mul_ps(_mm256_cvtepi32_ps(int1), v_gain);
            __m256 f2 = _mm256_mul_ps(_mm256_cvtepi32_ps(int2), v_gain);
            __m256 f3 = _mm256_mul_ps(_mm256_cvtepi32_ps(int3), v_gain);

            f0 = _mm256_min_ps(_mm256_max_ps(f0, v_min), v_max);
            f1 = _mm256_min_ps(_mm256_max_ps(f1, v_min), v_max);
            f2 = _mm256_min_ps(_mm256_max_ps(f2, v_min), v_max);
            f3 = _mm256_min_ps(_mm256_max_ps(f3, v_min), v_max);

            __m256i r0 = _mm256_cvtps_epi32(f0);
            __m256i r1 = _mm256_cvtps_epi32(f1);
            __m256i r2 = _mm256_cvtps_epi32(f2);
            __m256i r3 = _mm256_cvtps_epi32(f3);

            __m256i pack01 = _mm256_packs_epi32(r0, r1);
            __m256i pack23 = _mm256_packs_epi32(r2, r3);
            __m256i pack_final = _mm256_packs_epi16(pack01, pack23);

            __m128i low128  = _mm256_castsi256_si128(pack_final);
            __m128i high128 = _mm256_extracti128_si256(pack_final, 1);

            _mm_storeu_si128((__m128i*)&data[i], low128);
            _mm_storeu_si128((__m128i*)&data[i + 16], high128);
        }
    }

    for (long i = simd_end; i < capacity; i++) {
        float tmp = (float)data[i] * total_gain;
        tmp = tmp < min_val ? min_val : (tmp > max_val ? max_val : tmp);
        data[i] = (int8_t)tmp;
    }
}
