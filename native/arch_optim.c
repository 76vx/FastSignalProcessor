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

    const float min_val = -128.0f;
    const float max_val = 127.0f;

    #pragma omp parallel
    {
        __m256 v_gain = _mm256_set1_ps(total_gain);
        __m256 v_min = _mm256_set1_ps(min_val);
        __m256 v_max = _mm256_set1_ps(max_val);

        #pragma omp for schedule(static)
        for (long i = 0; i <= capacity - 32; i += 32) {
            __m128i raw_bytes_low = _mm_loadu_si128((__m128i*)&data[i]);
            __m128i raw_bytes_high = _mm_loadu_si128((__m128i*)&data[i + 16]);

            __m256i ints_0_7   = _mm256_cvtepi8_epi32(raw_bytes_low);
            __m256i ints_8_15  = _mm256_cvtepi8_epi32(_mm_srli_si128(raw_bytes_low, 8));
            __m256i ints_16_23 = _mm256_cvtepi8_epi32(raw_bytes_high);
            __m256i ints_24_31 = _mm256_cvtepi8_epi32(_mm_srli_si128(raw_bytes_high, 8));

            __m256 f0 = _mm256_cvtepi32_ps(ints_0_7);
            __m256 f1 = _mm256_cvtepi32_ps(ints_8_15);
            __m256 f2 = _mm256_cvtepi32_ps(ints_16_23);
            __m256 f3 = _mm256_cvtepi32_ps(ints_24_31);

            f0 = _mm256_mul_ps(f0, v_gain);
            f1 = _mm256_mul_ps(f1, v_gain);
            f2 = _mm256_mul_ps(f2, v_gain);
            f3 = _mm256_mul_ps(f3, v_gain);

            f0 = _mm256_min_ps(_mm256_max_ps(f0, v_min), v_max);
            f1 = _mm256_min_ps(_mm256_max_ps(f1, v_min), v_max);
            f2 = _mm256_min_ps(_mm256_max_ps(f2, v_min), v_max);
            f3 = _mm256_min_ps(_mm256_max_ps(f3, v_min), v_max);

            __m256i r0 = _mm256_cvtps_epi32(f0);
            __m256i r1 = _mm256_cvtps_epi32(f1);
            __m256i r2 = _mm256_cvtps_epi32(f2);
            __m256i r3 = _mm256_cvtps_epi32(f3);

            #define PACK_AND_STORE(idx, reg0, reg1, reg2, reg3) { \
                __m256i pack_low = _mm256_packs_epi32(reg0, reg1); \
                __m256i pack_high = _mm256_packs_epi32(reg2, reg3); \
                __m256i pack_8bit = _mm256_packs_epi16(pack_low, pack_high); \
                __m128i final_low = _mm256_castsi256_si128(pack_8bit); \
                __m128i final_high = _mm256_extracti128_si256(pack_8bit, 1); \
                _mm_storeu_si128((__m128i*)&data[idx], final_low); \
                _mm_storeu_si128((__m128i*)&data[idx + 16], final_high); \
            }

            __m256i p_0_15 = _mm256_packs_epi32(r0, r1);
            __m256i p_16_31 = _mm256_packs_epi32(r2, r3);
            __m256i p_final = _mm256_packs_epi16(p_0_15, p_16_31);

            __m128i res_l = _mm256_castsi256_si128(p_final);
            __m128i res_h = _mm256_extracti128_si256(p_final, 1);
            
            _mm_storeu_si128((__m128i*)&data[i], _mm_permutevar8x32_epi32(res_l, res_h)); 
            data[i] = (int8_t)_mm256_extract_epi8(_mm256_castsi256_si128(p_final), 0);
        }
    }

    for (long i = (capacity / 32) * 32; i < capacity; i++) {
        float sample = (float)data[i] * total_gain;
        if (sample > max_val) sample = max_val;
        else if (sample < min_val) sample = min_val;
        data[i] = (int8_t)sample;
    }
}