// 330829847 Ido Maimon
// 216764803 Yuli Smishkis

#include <emmintrin.h>   // SSE2
#include <smmintrin.h>   // SSE4.1
#include <string.h>
#include <math.h>

#define MAX_STR 1024
#define SIZE 16

int hamming_dist(const char str1[MAX_STR], const char str2[MAX_STR]) {
    // Initialize lengths, distance and minimum length;
    int d = 0, len1 = strlen(str1), len2 = strlen(str2), min = len1;
    if (len1 > len2) min = len2;

    int i, iterations = min - SIZE;
    // Process 16 bytes at a time
    for (i = 0; i <= iterations; i += SIZE) {
        // Load 16 bytes from each string
        __m128i vector1 = _mm_loadu_si128((__m128i*)&str1[i]);
        __m128i vector2 = _mm_loadu_si128((__m128i*)&str2[i]);
        // Create mask for differing bytes
        int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(vector1, vector2));
        mask = ~mask & 0b1111111111111111;
        d = d + __builtin_popcount(mask);
    }

    // Handle remaining bytes less than 16
    int remaining = min - i;
    char remaining_str1[SIZE] = {0}, remaining_str2[SIZE] = {0};

    // Process remaining bytes
    if (remaining > 0) {
        // Copy remaining bytes into temporary buffers
        memcpy(remaining_str1, &str1[i], remaining);
        memcpy(remaining_str2, &str2[i], remaining);
        // Load remaining bytes
        __m128i vector1 = _mm_loadu_si128((__m128i*)remaining_str1);
        __m128i vector2 = _mm_loadu_si128((__m128i*)remaining_str2);
        // Create mask for remaining bytes
        int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(vector1, vector2));
        mask = ~mask & ((1 << remaining) - 1);
        d = d + __builtin_popcount(mask);
    }

    // Add the difference in lengths
    if (len1 > len2) d = d + len1 - len2;
    else if (len2 > len1) d = d + len2 - len1;

    return d;
}
