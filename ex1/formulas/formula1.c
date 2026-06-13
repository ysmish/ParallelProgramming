// 330829847 Ido Maimon
// 216764803 Yuli Smishkis

#include <xmmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pmmintrin.h>

#define MAX_STR 255

float formula1(float *x, unsigned int length) { 
    // Initialize 2 SIMD registers : one for addition, one for product
    __m128 v_sum = _mm_setzero_ps(), v_mult = _mm_set1_ps(1.0); 
    unsigned int i, iterations = length - (length % 4);
    __m128 x_pow;
    // iterate and process 4 floats at a time
    for (i = 0; i < iterations; i += 4) {
        __m128 block = _mm_loadu_ps(&x[i]);
        // Sqrt(x)
        v_sum = _mm_add_ps(v_sum, _mm_sqrt_ps(block));
        // Multiply the result by (x² + 1) for each element
        x_pow = _mm_mul_ps(block, block);
        v_mult = _mm_mul_ps(v_mult, _mm_add_ps(x_pow, _mm_set1_ps(1.0f)));
    }
    
    // Process any leftover elements that didn't fit into a full vector
    float sum = 0, mult = 1;
    while(i < length) {
        // Sqrt(x)
        sum = sum + sqrtf(x[i]);
        // Fold the current value into the scalar product as (x² + 1)
        mult = mult * (powf(x[i], 2) + 1.0);
        i++;
    }

    // Combine results
    float sum_to_float[4], mult_to_float[4];
    _mm_storeu_ps(sum_to_float, v_sum);
    _mm_storeu_ps(mult_to_float, v_mult);
    // sum and multiplication
    for (i = 0; i < 4; i++) {
        sum = sum + sum_to_float[i];
        mult = mult * mult_to_float[i];
    }

    // Apply the outer formula: sqrt(1 + cbrt(sum) / product)
    float result = 1.0 + powf(sum, 1.0/3.0) / mult;
    return sqrtf(result);
}
