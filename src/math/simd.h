// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University
// Started on Monday 5 June 2018, which was a very nice day in Strasbourg

#ifndef SIMD_H
#define SIMD_H

//---------------------------------- SSE ---------------------------------------

#if defined(__SSE3__)

#include <immintrin.h>

/// Vector of 2 doubles
typedef __m128d vec2;

constexpr __m128d sgn11 = {-0.0, -0.0};

// Attention: the second value returned by load1() is not set and will be garbage!
inline static vec2 load1(double const* a)           { return _mm_load_sd(a); }
inline static vec2 load1Z(double const* a)          { return _mm_loadl_pd(_mm_setzero_pd(), a); }

// Attention: load2() do not initialize the upper AVX registers
inline static vec2 load2(double const* a)           { return _mm_load_pd(a); }

// unaligned load
inline static vec2 loadu2(double const* a)          { return _mm_loadu_pd(a); }

// load 1 double and duplicate
inline static vec2 loaddup2(double const* a)        { return _mm_loaddup_pd(a); }

inline static vec2 loadhi2(vec2 a, double const* b) { return _mm_loadh_pd(a,b); }
inline static vec2 loadlo2(vec2 a, double const* b) { return _mm_loadl_pd(a,b); }

inline static void store1(double* a, vec2 b)        { _mm_store_sd(a, b); }
inline static void store2(double* a, vec2 b)        { _mm_store_pd(a, b); }
inline static void storedup(double* a, vec2 b)      { _mm_store1_pd(a, b); }
inline static void storelo(double* a, vec2 b)       { _mm_store_sd(a, b); }
inline static void storeu2(double* a, vec2 b)       { _mm_storeu_pd(a, b); }

inline static vec2 movedup2(vec2 a)                 { return _mm_movedup_pd(a); }

inline static vec2 mul1(vec2 a, vec2 b)             { return _mm_mul_sd(a,b); }
inline static vec2 div1(vec2 a, vec2 b)             { return _mm_div_sd(a,b); }
inline static vec2 add1(vec2 a, vec2 b)             { return _mm_add_sd(a,b); }
inline static vec2 sub1(vec2 a, vec2 b)             { return _mm_sub_sd(a,b); }

inline static vec2 mul2(vec2 a, vec2 b)             { return _mm_mul_pd(a,b); }
inline static vec2 div2(vec2 a, vec2 b)             { return _mm_div_pd(a,b); }
inline static vec2 add2(vec2 a, vec2 b)             { return _mm_add_pd(a,b); }
inline static vec2 sub2(vec2 a, vec2 b)             { return _mm_sub_pd(a,b); }
inline static vec2 hadd2(vec2 a, vec2 b)            { return _mm_hadd_pd(a,b); }

inline static vec2 sqrt2(vec2 a)                    { return _mm_sqrt_pd(a); }
inline static vec2 max2(vec2 a, vec2 b)             { return _mm_max_pd(a,b); }
inline static vec2 min2(vec2 a, vec2 b)             { return _mm_min_pd(a,b); }
inline static vec2 and2(vec2 a, vec2 b)             { return _mm_and_pd(a,b); }
inline static vec2 andnot2(vec2 a, vec2 b)          { return _mm_andnot_pd(a,b); }
inline static vec2 abs2(vec2 a)                     { return _mm_andnot_pd(sgn11, a); }
inline static vec2 flipsign2(vec2 a)                { return _mm_xor_pd(a, sgn11); }

inline static vec2 setr2(double a, double b)        { return _mm_setr_pd(a,b); }
inline static vec2 set2(double a, double b)         { return _mm_set_pd(a, b); }
inline static vec2 set2(double a)                   { return _mm_set1_pd(a); }
inline static vec2 setzero2()                       { return _mm_setzero_pd(); }

inline static vec2 unpacklo2(vec2 a, vec2 b)        { return _mm_unpacklo_pd(a,b); }
inline static vec2 unpackhi2(vec2 a, vec2 b)        { return _mm_unpackhi_pd(a,b); }
inline static vec2 swap2(vec2 a)                    { return _mm_shuffle_pd(a, a, 0b01); }

/// combine and swap to return { low = a[1], high = b[0] }
inline static vec2 gethilo2(vec2 a, vec2 b)         { return _mm_shuffle_pd(a, b, 0b01); }

#define shuffle2(a,b,k)   _mm_shuffle_pd(a,b,k)
#define cmp2(a,b,k)       _mm_cmp_pd(a,b,k)

/// returns the sum of the elements, broadcasted
inline static vec2 esum2(vec2 v)
{
    return add2(v, swap2(v));
}

/// returns the dot product of two vectors, broadcasted
inline static vec2 dot2(vec2 a, vec2 b)
{
    vec2 p = mul2(a, b);
    return add2(p, swap2(p));
}

/// square of vector norm, broadcasted
inline static vec2 normsqr2(vec2 vec)
{
    vec2 p = mul2(vec, vec);
    return add2(p, swap2(p));
}

/// normalize vector
inline static vec2 normalize2(vec2 vec)
{
    vec2 p = mul2(vec, vec);
    vec2 s = add2(p, swap2(p));
    return div2(vec, sqrt2(s));
}

/// normalize vector to 'n'
inline static vec2 normalize2(vec2 vec, double n)
{
    vec2 p = mul2(vec, vec);
    vec2 s = add2(p, swap2(p));
    return mul2(vec, div2(set2(n), sqrt2(s)));
}

#endif

#if defined(__SSE4_1__)

#define blend2(a,b,k) _mm_blend_pd(a,b,k)

/// blend to return { low = a[0], high = b[1] }
inline static vec2 blend11(vec2 a, vec2 b) { return _mm_blend_pd(a, b, 0b10); }

inline static vec2 sign_select2(vec2 val, vec2 neg, vec2  pos)
{
    return _mm_blendv_pd(pos, neg, val);
}

#endif

//---------------------------------- AVX ---------------------------------------

#if defined(__AVX__)

/// Vector of 4 doubles
typedef __m256d vec4;

#define set64x(a,b,c,d)     _mm256_setr_epi64x(a,b,c,d)

constexpr __m256d sgn1111 = {-0.0, -0.0, -0.0, -0.0};


inline static vec4 setr4(double a, double b, double c, double d) { return _mm256_setr_pd(a,b,c,d); }
inline static vec4 set4(double a, double b, double c, double d)  { return _mm256_set_pd(a,b,c,d); }

inline static vec4 set4(double a)               { return _mm256_set1_pd(a); }
inline static vec4 setzero4()                   { return _mm256_setzero_pd(); }

inline static vec4 cast4(vec2 a)                { return _mm256_castpd128_pd256(a); }
inline static vec2 cast2(vec4 a)                { return _mm256_castpd256_pd128(a); }

inline static vec4 load4(double const* a)       { return _mm256_load_pd(a); }
inline static vec4 loadu4(double const* a)      { return _mm256_loadu_pd(a); }

/// unaligned load 2 values, and zeros out the upper two
inline static vec4 load2Z(double const* a)      { return _mm256_blend_pd(_mm256_castpd128_pd256(_mm_load_pd(a)), _mm256_setzero_pd(), 0b1100); }

/// unaligned load 4 values, and zeros out the upper one
inline static vec4 load3Z(double const* a)      { return _mm256_blend_pd(_mm256_loadu_pd(a), _mm256_setzero_pd(), 0b1000); }

/// unaligned load 2 values, allowing for undefined value in upper positions
inline static vec4 load2crap(double const* a)   { return _mm256_castpd128_pd256(_mm_load_pd(a)); }

inline static void store1(double* a, vec4 b)    { _mm_store_sd(a, cast2(b)); }
inline static void store2(double* a, vec4 b)    { _mm_store_pd(a, cast2(b)); }
inline static void storeu2(double* a, vec4 b)   { _mm_storeu_pd(a, cast2(b)); }
inline static void store4(double* a, vec4 b)    { _mm256_store_pd(a,b); }
inline static void storeu4(double* a, vec4 b)   { _mm256_storeu_pd(a,b); }
inline static void store4(double* a, __m128 b)  { _mm256_store_pd(a, _mm256_cvtps_pd(b)); }

inline static __m256i makemask(long i)
{
    constexpr __m256d ramp{0.5, 1.5, 2.5, 3.5};
    return _mm256_castpd_si256(_mm256_cmp_pd(ramp, _mm256_set1_pd((double)i), _CMP_LT_OQ));
}

inline static vec4 maskload4(double const* a, __m256i k)     { return _mm256_maskload_pd(a,k); }
inline static void maskstore4(double* a, __m256i k, vec4 b)  { _mm256_maskstore_pd(a,k,b); }

/// load 1 double into all 4 positions
inline static vec4 broadcast1(double const* a)  { return _mm256_broadcast_sd(a); }
/// load 2 doubles and duplicate: X, Y, X, Y
inline static vec4 broadcast2(double const* a)  { return _mm256_broadcast_pd((__m128d const*)a); }

inline static vec2 getlo(vec4 a)                { return _mm256_castpd256_pd128(a); }
inline static vec2 gethi(vec4 a)                { return _mm256_extractf128_pd(a,1); }

inline static vec4 mul4(vec4 a, vec4 b)         { return _mm256_mul_pd(a,b); }
inline static vec4 div4(vec4 a, vec4 b)         { return _mm256_div_pd(a,b); }
inline static vec4 add4(vec4 a, vec4 b)         { return _mm256_add_pd(a,b); }
inline static vec4 sub4(vec4 a, vec4 b)         { return _mm256_sub_pd(a,b); }
inline static vec4 hadd4(vec4 a, vec4 b)        { return _mm256_hadd_pd(a,b); }

inline static vec4 sqrt4(vec4 a)                { return _mm256_sqrt_pd(a); }
inline static vec4 max4(vec4 a, vec4 b)         { return _mm256_max_pd(a,b); }
inline static vec4 min4(vec4 a, vec4 b)         { return _mm256_min_pd(a,b); }
inline static vec4 and4(vec4 a, vec4 b)         { return _mm256_and_pd(a,b); }
inline static vec4 andnot4(vec4 a, vec4 b)      { return _mm256_andnot_pd(a,b); }
inline static vec4 abs4(vec4 a)                 { return _mm256_andnot_pd(sgn1111, a); }
inline static vec4 flipsign4(vec4 a)            { return _mm256_xor_pd(a, sgn1111); }

inline static vec4 unpacklo4(vec4 a, vec4 b)    { return _mm256_unpacklo_pd(a,b); }
inline static vec4 unpackhi4(vec4 a, vec4 b)    { return _mm256_unpackhi_pd(a,b); }

inline static vec4 duplo4(vec4 a)               { return _mm256_movedup_pd(a); } //_mm256_unpacklo_pd(a,a)
inline static vec4 duphi4(vec4 a)               { return _mm256_permute_pd(a,15); } //_mm256_unpackhi_pd(a,a)

// copy a[0] into all elements of dst.
inline static vec4 broadcastd(vec4 a)         { return _mm256_movedup_pd(_mm256_permute2f128_pd(a, a, 0x00)); }

/* Unused functions:
 inline static vec4 loadu22(double const* a, double const* b) { return _mm256_loadu2_m128d(a,b); }
 inline static void store22(double* a, double* b, vec4 c) { return _mm256_storeu2_m128d(a,b,c); }
 */

// swap the two 128 bit lanes
inline static vec4 swap2f128(vec4 a)          { return _mm256_permute2f128_pd(a, a, 0x01); }

// return { A0, A0 } from a = { A0, A1 }
inline static vec4 duplo2f128(vec4 a)         { return _mm256_permute2f128_pd(a, a, 0x00); }

// return { A1, A1 } from a = { A0, A1 }
inline static vec4 duphi2f128(vec4 a)         { return _mm256_permute2f128_pd(a, a, 0x11); }

// return { A1, B0 } from a = { A0, A1 } and b = { B0, B1 }
inline static vec4 twine2f128(vec4 a, vec4 b) { return _mm256_permute2f128_pd(a, b, 0x21); }

#define insertf128(a,b,k)   _mm256_insertf128_pd(a,b,k)
#define permute4(a,k)       _mm256_permute_pd(a,k)
#define permute2(a,k)       _mm_permute_pd(a,k)       // same as shuffle2(a,a,k)
#define permute2f128(a,b,k) _mm256_permute2f128_pd(a,b,k)
#define shuffle4(a,b,k)     _mm256_shuffle_pd(a,b,k)
#define blend4(a,b,k)       _mm256_blend_pd(a,b,k)
#define cmp4(a,b,k)         _mm256_cmp_pd(a,b,k)

inline static vec4 blend31(vec4 a, vec4 b) { return _mm256_blend_pd(a,b,0b1000); }
inline static vec4 blend22(vec4 a, vec4 b) { return _mm256_blend_pd(a,b,0b1100); }
inline static vec4 blend13(vec4 a, vec4 b) { return _mm256_blend_pd(a,b,0b1110); }

/// concatenate two vec2 into a vec4
inline static vec4 cat4(vec2 h, vec2 l) { return _mm256_set_m128d(h, l); }
inline static vec4 cat4(vec2 h, vec4 l) { return _mm256_set_m128d(h, cast2(l)); }

inline static vec4 sign_select4(vec4 val, vec4 neg, vec4 pos)
{
    return _mm256_blendv_pd(pos, neg, val);
}

#if 0
  inline static vec4  load3(double const* a)    { return blend4(load2Z(a)), broadcast1(a+2), 0b0100); }
  inline static void store3(double* a, vec4 b)  { storeu2(a, getlo(b)); store1(a+2, gethi(b)); }
#else
  //inline static vec4  load3(double const* a)  { return _mm256_loadu_pd(a); }
  constexpr __m256i msk1110 = {-1, -1, -1, 0};  // -1 = all bits to 1
  inline static vec4  load3(double const* a)    { return _mm256_maskload_pd(a, msk1110); }
  inline static void store3(double* a, vec4 b)  { _mm256_maskstore_pd(a, msk1110, b); }
#endif


/// returns the sum of the elements, broadcasted
inline static vec4 esum4(vec4 v)
{
    vec4 s = add4(v, swap2f128(v));
    return add4(s, permute4(s, 0b0101));
}

/// returns the dot product of two vectors, broadcasted
inline static vec4 dot4(vec4 a, vec4 b)
{
    vec4 m = mul4(a, b);
    vec4 s = add4(m, swap2f128(m));
    return add4(s, permute4(s, 0b0101));
}

/// square of vector norm, broadcasted
inline static vec4 normsqr4(vec4 vec)
{
    vec4 m = mul4(vec, vec);
    vec4 s = add4(m, swap2f128(m));
    return add4(s, permute4(s, 0b0101));
}

/// normalize vector
inline static vec4 normalize4(vec4 vec)
{
    vec4 m = mul4(vec, vec);
    vec4 s = add4(m, swap2f128(m));
    m = add4(s, permute4(s, 0b0101));
    return div4(vec, sqrt4(m));
}

/// normalize vector to 'n'
inline static vec4 normalize4(vec4 vec, double n)
{
    vec4 m = mul4(vec, vec);
    vec4 s = add4(m, swap2f128(m));
    m = add4(s, permute4(s, 0b0101));
    return mul4(vec, div4(set4(n), sqrt4(m)));
}

#endif

//---------------------------------- AVX2 --------------------------------------

#if defined(__AVX2__)

#define permute4x64(a,k)    _mm256_permute4x64_pd(a,k)
#define rotater4(a)         _mm256_castsi256_pd(_m256_alignr_epi8(a, a, 1));
#define rotatel4(a)         _mm256_castsi256_pd(_m256_alignr_epi8(a, a, 7));

// load a memory address = [ X Y ] into [ X X Y Y ]:
inline static vec4 interleave4(vec2 a) { return _mm256_permute4x64_pd(cast4(a), 0x50); }
inline static vec4 interleave4(vec4 a) { return _mm256_permute4x64_pd(a, 0x50); }

// copy a[0] into all elements of dst.
inline static vec4 broadcastd(vec2 a)  { return _mm256_broadcastsd_pd(a); }


/// cross product of two 3D vectors ( X Y Z T )
inline static vec4 cross4(vec4 a, vec4 b)
{
    vec4 a1 = permute4x64(a, 0xC9); // Y Z X T
    vec4 b1 = permute4x64(b, 0xC9); // Y Z X T
#if defined(__FMA__)
    return permute4x64(_mm256_fmsub_pd(a, b1, mul4(a1,b)), 0xC9);
#else
    return permute4x64(sub4(mul4(a,b1), mul4(a1,b)), 0xC9);
#endif
}

/*
 Using `stream_load` may or may not improve performance
 It is a aligned load intruction that bypasses the cache.
 It would be advantageous if the cache is too small to hold the whole `data',
 since in this case data caching would simply load value that are not used anymore
 */
inline static vec4 streamload4(double const* a) { return (vec4)_mm256_stream_load_si256((__m256i const*)a); }

/// streamload is a load that bypass the cache
//inline static vec4 streamload4(double const* a) { return _mm256_loadu_pd(a); }

#elif defined(__AVX__)

inline static vec4 streamload4(double const* a) { return _mm256_loadu_pd(a); }

inline static vec4 interleave4(vec2 a) { return permute4(permute2f128(cast4(a), cast4(a), 0x00), 0b1100); }
inline static vec4 interleave4(vec4 a) { return permute4(permute2f128(a, a, 0x00), 0b1100); }

#endif

//----------------------------------- FMA --------------------------------------

#if defined(__FMA__)
inline static vec2 fmadd1(vec2 a, vec2 b, vec2 c)  { return _mm_fmadd_sd(a,b,c); }  // a * b + c
inline static vec2 fmsub1(vec2 a, vec2 b, vec2 c)  { return _mm_fmsub_sd(a,b,c); }  // a * b - c
inline static vec2 fnmadd1(vec2 a, vec2 b, vec2 c) { return _mm_fnmadd_sd(a,b,c); } // c - a * b

inline static vec2 fmadd2(vec2 a, vec2 b, vec2 c)  { return _mm_fmadd_pd(a,b,c); }
inline static vec2 fmsub2(vec2 a, vec2 b, vec2 c)  { return _mm_fmsub_pd(a,b,c); }
inline static vec2 fnmadd2(vec2 a, vec2 b, vec2 c) { return _mm_fnmadd_pd(a,b,c); }

inline static vec4 fmadd4(vec4 a, vec4 b, vec4 c)  { return _mm256_fmadd_pd(a,b,c); }
inline static vec4 fmsub4(vec4 a, vec4 b, vec4 c)  { return _mm256_fmsub_pd(a,b,c); }
inline static vec4 fnmadd4(vec4 a, vec4 b, vec4 c) { return _mm256_fnmadd_pd(a,b,c); }
#else
// erzatz functions
#  if defined(__SSE3__)
inline static vec2 fmadd1(vec2 a, vec2 b, vec2 c)  { return _mm_add_sd(_mm_mul_sd(a,b), c); }
inline static vec2 fmsub1(vec2 a, vec2 b, vec2 c)  { return _mm_sub_sd(_mm_mul_sd(a,b), c); }
inline static vec2 fnmadd1(vec2 a, vec2 b, vec2 c) { return _mm_sub_sd(c, _mm_mul_sd(a,b)); }

inline static vec2 fmadd2(vec2 a, vec2 b, vec2 c)  { return _mm_add_pd(_mm_mul_pd(a,b), c); }
inline static vec2 fmsub2(vec2 a, vec2 b, vec2 c)  { return _mm_sub_pd(_mm_mul_pd(a,b), c); }
inline static vec2 fnmadd2(vec2 a, vec2 b, vec2 c) { return _mm_sub_pd(c, _mm_mul_pd(a,b)); }
#  endif
#  if defined(__AVX__)
inline static vec4 fmadd4(vec4 a, vec4 b, vec4 c)  { return _mm256_add_pd(_mm256_mul_pd(a,b), c); }
inline static vec4 fmsub4(vec4 a, vec4 b, vec4 c)  { return _mm256_sub_pd(_mm256_mul_pd(a,b), c); }
inline static vec4 fnmadd4(vec4 a, vec4 b, vec4 c) { return _mm256_sub_pd(c, _mm256_mul_pd(a,b)); }
#  endif
#endif

#endif // SIMD_H
