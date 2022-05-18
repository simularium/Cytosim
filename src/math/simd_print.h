// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University

#ifndef SIMD_PRINT_H
#define SIMD_PRINT_H

// Functions to print SIMD vectors

#include <cstdio>

//---------------------------------- SSE ---------------------------------------

#ifdef __SSE3__

/// print SIMD vector of 2 doubles
inline static void dump(__m128d v, char const* s)
{
    printf("%16s d( %5.2f %5.2f )\n", s, v[1], v[0]);
}

/// print two SIMD vector
inline static void dump(__m128d v, vec2 w, char const* s)
{
    printf("%16s d( %5.2f %5.2f )( %5.2f %5.2f )\n", s, v[1], v[0], w[1], w[0]);
}

/// print SIMD vector of 4 floats
inline static void dump(__m128 v, char const* s)
{
    printf("%16s f( %5.2f %5.2f %5.2f %5.2f )\n", s, v[3], v[2], v[1], v[0]);
}

#endif

//------------------------------- INTEGERS -------------------------------------

#ifdef __SSE3__

inline static void dump16(__m128i v, char const* s)
{
    uint16_t a = _mm_extract_epi16(v, 0);
    uint16_t b = _mm_extract_epi16(v, 1);
    uint16_t c = _mm_extract_epi16(v, 2);
    uint16_t d = _mm_extract_epi16(v, 3);
    uint16_t e = _mm_extract_epi16(v, 4);
    uint16_t f = _mm_extract_epi16(v, 5);
    uint16_t g = _mm_extract_epi16(v, 6);
    uint16_t h = _mm_extract_epi16(v, 7);
    printf("%16s int16( %3i %3i %3i %3i %3i %3i %3i %3i )\n", s, h, g, f, e, d, c, b, a);
}

inline static void dump32(__m128i v, char const* s)
{
    uint32_t a = _mm_extract_epi32(v, 0);
    uint32_t b = _mm_extract_epi32(v, 1);
    uint32_t c = _mm_extract_epi32(v, 2);
    uint32_t d = _mm_extract_epi32(v, 3);
    printf("%16s int32( %5i %5i %5i %5i )\n", s, d, c, b, a);
}

#endif

//---------------------------------- AVX ---------------------------------------

#ifdef __AVX__

/// print SIMD vector of 4 doubles
inline static void dump(__m256d v, char const* s)
{
    printf("%16s d( %5.2f %5.2f %5.2f %5.2f )\n", s, v[3], v[2], v[1], v[0]);
}

/// print two SIMD vector of 4 doubles
inline static void dump(__m256d v, __m256d w, char const* s)
{
    printf("%16s d( %5.2f %5.2f %5.2f %5.2f )( %5.2f %5.2f %5.2f %5.2f )\n",
           s, v[3], v[2], v[1], v[0], w[3], w[2], w[1], w[0]);
}

/// print SIMD vector of 8 floats
inline static void dump(__m256 v, char const* s)
{
    printf("%16s f( %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f )\n", s,
           v[7], v[6], v[5], v[4], v[3], v[2], v[1], v[0]);
}

#endif

#endif
