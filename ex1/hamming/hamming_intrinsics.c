// 330829847 Ido Maimon
// 216764803 Yuli Smishkis

#include <emmintrin.h>   // SSE2
#include <smmintrin.h>   // SSE4.1
#include <string.h>
#include <math.h>

#define MAX_STR 1024
#define SIZE 16

int hamming_dist(const char str1[MAX_STR], const char str2[MAX_STR]) {
    // Set up counters and minimum
    int d = 0, len1 = strlen(str1), len2 = strlen(str2), min = len1;
    if (len1 > len2) min = len2;

    int i, iterations = min - SIZE;
    //Iterate and process 16 bytes each time
    for (i = 0; i <= iterations; i += SIZE) {
        // Pull a 128-bit chunk from each string into SSE registers
        __m128i vector1 = _mm_loadu_si128((__m128i*)&str1[i]);
        __m128i vector2 = _mm_loadu_si128((__m128i*)&str2[i]);
        // Compare bytewise, then extract and invert the equality mask
        int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(vector1, vector2));
        mask = ~mask & 0b1111111111111111;
        d = d + __builtin_popcount(mask);
    }

    // Deal with the tail that doesn't fill a full 16-byte vector
    int remaining = min - i;
    char remaining_str1[SIZE] = {0}, remaining_str2[SIZE] = {0};

    // Pad the leftover bytes 
    if (remaining > 0) {
        // Transfer the tail bytes into temporary aligned arrays
        memcpy(remaining_str1, &str1[i], remaining);
        memcpy(remaining_str2, &str2[i], remaining);
        // Run the same SIMD comparison on the padded buffers
        __m128i vector1 = _mm_loadu_si128((__m128i*)remaining_str1);
        __m128i vector2 = _mm_loadu_si128((__m128i*)remaining_str2);
        // Mask off only the bits that correspond to actual remaining bytes
        int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(vector1, vector2));
        mask = ~mask & ((1 << remaining) - 1);
        d = d + __builtin_popcount(mask);
    }

    // Deal with extra characters in the longer string
    if (len1 > len2) d = d + len1 - len2;
    else if (len2 > len1) d = d + len2 - len1;

    return d;
}
