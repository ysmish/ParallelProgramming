// 330829847 Ido Maimon
// 216764803 Yuli Smishkis

#include <xmmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pmmintrin.h>

#define MAX_STR 255

float formula1(float *x, unsigned int length) { 
    // Initialize SIMD registers for sum and multiplication
    __m128 v_sum = _mm_setzero_ps(), v_mult = _mm_set1_ps(1.0); 
    unsigned int i, iterations = length - (length % 4);
    __m128 x_pow;
    // Process 4 floats at a time
    for (i = 0; i < iterations; i += 4) {
        __m128 block = _mm_loadu_ps(&x[i]);
        // Sum of sqrt(x)
        v_sum = _mm_add_ps(v_sum, _mm_sqrt_ps(block));
        // Multiplication of x^2 + 1
        x_pow = _mm_mul_ps(block, block);
        v_mult = _mm_mul_ps(v_mult, _mm_add_ps(x_pow, _mm_set1_ps(1.0f)));
    }
    
    // Handle remaining floats
    float sum = 0, mult = 1;
    while(i < length) {
        // Sum of sqrt(x)
        sum = sum + sqrtf(x[i]);
        // Multiplication of x^2 + 1
        mult = mult * (powf(x[i], 2) + 1.0);
        i++;
    }

    // Combine results from SIMD registers
    float sum_to_float[4], mult_to_float[4];
    _mm_storeu_ps(sum_to_float, v_sum);
    _mm_storeu_ps(mult_to_float, v_mult);
    // Final sum and multiplication
    for (i = 0; i < 4; i++) {
        sum = sum + sum_to_float[i];
        mult = mult * mult_to_float[i];
    }

    // Final calculation
    float result = 1.0 + powf(sum, 1.0/3.0) / mult;
    return sqrtf(result);
}
