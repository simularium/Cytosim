// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/*
 Tests Intel's Streaming SIMD
 FJN, Heidelberg, started July 2013
 
 To compile: c++ -O4 tictoc.cc -mavx test.cc
 To generate assembly: c++ -S test.cc
 */

#include <cstdio>
#include "timer.h"
#include <stdint.h>
#include "vecprint.h"
#include "simd.h"
#include "simd_print.h"


typedef double real;

const size_t SIZ = 1<<14;
real vX[SIZ], vY[SIZ];

void init()
{
    for ( size_t ii=0; ii<SIZ; ++ii )
    {
        vX[ii] = 1/real(SIZ-ii);
        vY[ii] = real(SIZ-ii);
    }
}

void dump(size_t len, const float* vec)
{
    printf(": ");
    for ( size_t n = 0; n < len; ++n )
    {
        if ( n % 4 == 3 )
            printf("%+5.2f  ", vec[n]);
        else
            printf("%+5.2f ", vec[n]);
    }
    printf("\n");
}

void dump(size_t len, const double* vec)
{
    printf(": ");
    for ( size_t n = 0; n < len; ++n )
    {
        if ( n % 4 == 3 )
            printf("%+5.2f  ", vec[n]);
        else
            printf("%+5.2f ", vec[n]);
    }
    printf("\n");
}

real scalar()
{
    real d = 0;
    for ( size_t ii=0; ii<SIZ; ++ii )
        d += vX[ii] * vY[ii];
    return d;
}

real vector2()
{
    vec2 s = setzero2();
    for ( size_t ii=0; ii<SIZ; ii+=2 )
        s = add2(s, mul2( load2(vX+ii), load2(vY+ii) ));
    _mm_empty();
    
    return esum2(s)[0];
}

#ifdef __AVX__

real vector4()
{
    vec4 s = setzero4();
    for ( size_t ii=0; ii<SIZ; ii+=4 )
        s = add4(s, mul4( load4(vX+ii), load4(vY+ii) ));
    _mm_empty();
    
    return esum4(s)[0];
}


real vectorU()
{
    vec4 v0 = setzero4();
    vec4 v1 = setzero4();
    vec4 v2 = setzero4();
    vec4 v3 = setzero4();
    
    for ( size_t ii=0; ii<SIZ; ii+=16 )
    {
        v0 = add4(v0, mul4( load4(vX+ii   ), load4(vY+ii   ) ));
        v1 = add4(v1, mul4( load4(vX+ii+4 ), load4(vY+ii+4 ) ));
        v2 = add4(v2, mul4( load4(vX+ii+8 ), load4(vY+ii+8 ) ));
        v3 = add4(v3, mul4( load4(vX+ii+12), load4(vY+ii+12) ));
    }
    
    vec4 s = add4(add4(v0, v1), add4(v2, v3));
    _mm_empty();
    
    return esum4(s)[0];
}

#endif

void run(real (*func)(), const char name[], const size_t REP)
{
    real a = 0, b = 0, c = 0, d = 0;
    real e = 0, f = 0, g = 0, h = 0;
    fprintf(stderr, "%10s:  ", name);
    tic();
    
    for ( size_t i=0; i<REP; ++i )
    {
        a = (*func)();
        b = (*func)();
        c = (*func)();
        d = (*func)();
        e = (*func)();
        f = (*func)();
        g = (*func)();
        h = (*func)();
    }
    
    real s = ((a + b) + (c + d)) + ((e + f) + (g + h));
    fprintf(stderr, " %16f :  %8.0f ms\n", s, toc());
}


void test_swapSSE()
{
    vec2 a = set2(0, 1);
    vec2 b = set2(2, 3);
    dump(a, "a");
    dump(b, "b");
    
#ifdef __AVX__
    dump(permute2(b,0b00), "permute 0b00");
    dump(permute2(b,0b01), "permute 0b01");
    dump(permute2(b,0b10), "permute 0b10");
    dump(permute2(b,0b11), "permute 0b11");
#endif
    
    dump(shuffle2(a,b,0b00), "0b00");
    dump(shuffle2(a,b,0b01), "0b01");
    dump(shuffle2(a,b,0b10), "0b10");
    dump(shuffle2(a,b,0b11), "0b11");
    dump(unpacklo2(a,b), "unpacklo");
    dump(unpackhi2(a,b), "unpackhi");
    dump(movedup2(a),  "movedup(a)");
    dump(movedup2(b),  "movedup(b)");
}

//------------------------------------------------------------------------------
#pragma mark -

#ifdef __AVX__

/**
 make dst = { XYZ XYZ XYZ XYZ }
 from src = { XYZ? }
 */
inline void twinedup12(double const* src, double* dst)
{
    vec4 s = load3(src);
    vec4 p = permute2f128(s, s, 0x01);
    vec4 h = shuffle4(s, p, 0b0001);
    vec4 d0 = blend31(s, h);
    vec4 d1 = blend22(h, p);
    vec4 d2 = shuffle4(p, s, 0b0100);
    store4(dst  , d0);
    store4(dst+4, d1);
    store4(dst+8, d2);
}


/**
 make
     dst = { XXX YYY ZZZ TTT }
 from
     src = { XYZT }
 */
inline void repeat12(double const* src, double* dst)
{
    vec4 s = load4(src);
    dump(s, "s");

    vec4 zx = swap2f128(s);
    vec4 xy = unpacklo4(s, s);
    vec4 yz = unpackhi4(s, s);
    
    store4(dst  , blend22(xy, zx));
    store4(dst+4, blend22(yz, xy));
    store4(dst+8, blend22(zx, yz));
}


/**
 make
     dst = { XYZ XYZ XYZ XYZ }
 from
     dX = { XXXX }
     dY = { YYYY }
     dZ = { ZZZZ }
 */
inline void twine12(double const* X, double const* Y, double const* Z, double* dst)
{
    vec4 sx = load4(X);
    vec4 sy = load4(Y);
    vec4 sz = load4(Z);
    dump(sx, "sx");
    dump(sy, "sy");
    dump(sz, "sz");

    vec4 zx = blend4(sx, sz, 0b0101);
    zx = swap2f128(zx);
    vec4 xy = unpacklo4(sx, sy);
    vec4 yz = unpackhi4(sy, sz);
    
    store4(dst  , blend22(xy, zx));
    store4(dst+4, blend22(yz, xy));
    store4(dst+8, blend22(zx, yz));
}

/**
 make
     dX = { XXXX }
     dY = { YYYY }
     dZ = { ZZZZ }
 from src = { XYZ XYZ XYZ XYZ }
 */
inline void untwine12(double const* src, double* X, double* Y, double* Z)
{
    vec4 s0 = load4(src);
    vec4 s1 = load4(src+4);
    vec4 s2 = load4(src+8);
    dump(s0, "s0");
    dump(s1, "s1");
    dump(s2, "s2");

    vec4 zx = blend22(s2, s0);
    zx = swap2f128(zx);
    vec4 xy = blend22(s0, s1);
    vec4 yz = blend22(s1, s2);
    
    store4(X,   blend4(zx, xy, 0b0101));
    store4(Y, shuffle4(xy, yz, 0b0101));
    store4(Z,   blend4(zx, yz, 0b1010));
}


/**
 make
     dst = { X+X+X, Y+Y+Y, Z+Z+Z, ? }
 from
     src = { XYZ XYZ XYZ XYZ }
 */
inline void sumXXX(double const* src, double* dst)
{
    vec4 s0 = load4(src);
    vec4 s1 = load4(src+4);
    vec4 s2 = load4(src+8);
    
    vec4 h = shuffle4(blend31(s1, s0), s2, 0b0101);
    vec4 d3 = twine2f128(s1, s2);
    vec4 d2 = shuffle4(s2, s1, 0b0101);
    vec4 d1 = swap2f128(h);

    vec4 sum = add4(add4(s0, d2), add4(d3, d1));
    store4(dst, sum);
}


/**
 make
     dst = { X+Y+Z, X+Y+Z, X+Y+Z, X+Y+Z }
 from
     src = { XYZ XYZ XYZ XYZ }
 */
inline void sumXYZ(double const* src, double* dst)
{
    vec4 s0 = load4(src);
    vec4 s1 = load4(src+4);
    vec4 s2 = load4(src+8);
    
    vec4 zx = blend22(s2, s0);
    zx = permute2f128(zx, zx, 0x21);
    vec4 xy = blend22(s0, s1);
    vec4 yz = blend22(s1, s2);
    
    vec4 d1 =   blend4(zx, xy, 0b0101);
    vec4 d2 = shuffle4(xy, yz, 0b0101);
    vec4 d3 =   blend4(zx, yz, 0b1010);

    vec4 sum = add4(d2, add4(d3, d1));
    store4(dst, sum);
}


void test_twine()
{
    printf("------ test_twine\n");
    double dst[12] = { 0 };
    const double src[12] = { 1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 3.1, 3.2, 3.3, 4.1, 4.2, 4.3 };
    double X[4] = { 1 }, Y[4] = { 2 }, Z[4] = { 3 };
    untwine12(src, X, Y, Z);
    dump(4, X);
    dump(4, Y);
    dump(4, Z);
    twine12(X, Y, Z, dst);
    printf("twine12  "); dump(12, dst);
    repeat12(X, dst);
    printf("repeat12 "); dump(12, dst);
    twinedup12(src, dst);
    printf("twinedup "); dump(12, dst);
    sumXYZ(src, dst);
    printf("sumXYZ "); dump(4, dst);
    sumXXX(src, dst);
    printf("sumXXX "); dump(4, dst);
}


/**
 Change the data stride from 3 to 4, specifically:
 make dst = { XYZ? XYZ? XYZ? XYZ? }
 from src = { XYZ XYZ XYZ XYZ }
 */
inline void destride3x4(double const* src, double* dst)
{
    vec4 s0 = load4(src);
    vec4 s1 = load4(src+4);
    vec4 s2 = load4(src+8);
    store4(dst, s0);
    
    vec4 d1 = twine2f128(s0, s1);
    d1 = shuffle4(d1, s1, 0b0101);
    store4(dst+4 , d1);
    
    vec4 d2 = blend22(s2, s1);
    d2 = swap2f128(d2);
    store4(dst+8 , d2);
    
    vec4 d3 = swap2f128(s2);
    d3 = shuffle4(s2, d3, 0b0101);
    store4(dst+12, d3);
}


void test_stride()
{
    printf("------ test_stride\n");
    const size_t CNT = 1024;
    double a[CNT*12];
    double b[CNT*16];
    
    for ( size_t n = 0; n < 12*CNT; ++n )
        a[n] = 1 + n % 12;
    
#pragma ivdep
    for ( size_t n = 0; n < CNT; ++n )
        destride3x4(a+12*n, b+16*n);
    
    for ( size_t n = 0; n < 4*CNT; ++n )
        b[4*n+3] = 0;
    
    dump(12, a);
    dump(16, b);
}

//------------------------------------------------------------------------------
#pragma mark -

/* transforms packed symmetric storage
 src = { a b c d e f }
 into { a b c 0 }, { b d e 0 }, { c e f 0 }
 */
void unpack_matrix()
{
    double src[] = { 1, 2, 3, 4, 5, 6 };
    
    vec2 zz = setzero2();
    vec2 ab = load2(src);
    vec2 cd = load2(src+2);
    vec2 ef = load2(src+4);
    
    vec2 m0h = blend11(cd, zz),  m0l = ab;
    vec2 m1h = blend11(ef, zz),  m1l = unpackhi2(ab, cd);
    vec2 m2h = gethilo2(ef, zz), m2l = unpacklo2(cd, ef);
    
    dump(cat4(m0h, m0l), "m0 ");
    dump(cat4(m1h, m1l), "m1 ");
    dump(cat4(m2h, m2l), "m2 ");
}


//------------------------------------------------------------------------------
#pragma mark -

void test_cat()
{
    printf("------ test_cat\n");
    vec2 y{1.0, 2.0};
    vec2 x{3.0, 4.0};
    x = setr2(1.0, 2.0);
    y = setr2(3.0, 4.0);
    dump(x, "x");
    dump(y, "y");

    dump(cat4(x, y), "cat4(x, y)");
    dump(cat4(y, x), "cat4(y, x)");
}

void test_load()
{
    printf("------ test_load\n");
    double mem[4] = { 1, 2, 3, 4 };
    vec4 x{1.0, 2.0, 3.0, 4.0};
    dump(x, "set ");
    
    vec4 y = load4(mem);
    dump(y, "load");
    
    // Attention: the upper 128-bits are in undefined state after a cast
    dump(cast4(load1(mem)), "cast4(load1)");
    dump(cast4(load2(mem)), "cast4(load2)");
    dump(load2Z(mem), "load2Z");

    dump(load3(mem), "load3");
    dump(cat4(load1(mem+2), load2(mem)), "cat4(load1, load2)");

    vec4 t = blend31(load4(mem), setzero4());
    dump(t, "blend(load4, zero)");


    vec2 u = load2(mem);
    vec4 a = broadcast2(mem);
    vec4 n = permute4(a, 0b1100);

    dump(u, "src");
    dump(n, "permute4(broadcast2)");
    dump(interleave4(u), "interleave4");
    dump(cat4(u,u), "cat4");
    dump(_mm256_set_m128d(u,u),    "set_m128d");
    dump(insertf128(cast4(u),u,1), "insertf128");
    dump(permute4(cat4(u,u), 0b1100), "permute4(cat4, 0b1100)");
    
    vec4 p{-1.0, -2.0, -3.0, -4.0};
    _mm_storeu_pd(mem, _mm256_castpd256_pd128(p));
    _mm_store_sd(mem+2,_mm256_extractf128_pd(p,1));
    dump(load4(mem), "load(store3)");
}


void test_broadcast()
{
    printf("------ test_broadcast\n");
    double mem[4] = { 1, 2, 3, 4 };
    
    // using 4 loads
    dump(broadcast1(mem  ), "mem[0]");
    dump(broadcast1(mem+1), "mem[1]");
    dump(broadcast1(mem+2), "mem[2]");
    dump(broadcast1(mem+3), "mem[3]");
    
    // using 2 loads
    vec4 xy = broadcast2(mem);
    vec4 zt = broadcast2(mem+2);
    dump(duplo4(xy), " x");
    dump(duphi4(xy), " y");
    dump(duplo4(zt), " z");
    dump(duphi4(zt), " t");
    
    // using 1 load
    vec4 xyzt = load4(mem);
    xy = permute2f128(xyzt, xyzt, 0x00);
    zt = permute2f128(xyzt, xyzt, 0x11);
    dump(duplo4(xy), "X ");
    dump(duphi4(xy), "T ");
    dump(duplo4(zt), "Z ");
    dump(duphi4(zt), "T ");

    // using 2 permutes and 4 blends
    xyzt = load4(mem);
    vec4 u0 = unpacklo4(xyzt, xyzt);
    vec4 u1 = unpackhi4(xyzt, xyzt);
    vec4 v0 = swap2f128(u0);
    vec4 v1 = swap2f128(u1);
    dump(blend22(u0, v0), " x");
    dump(blend22(u1, v1), " y");
    dump(blend22(v0, u0), " z");
    dump(blend22(v1, u1), " t");

    // using 1 permute and 4 blends
    xyzt = load4(mem);
    vec4 ztxy = swap2f128(xyzt);
    u0 = unpacklo4(xyzt, xyzt);
    u1 = unpackhi4(xyzt, xyzt);
    v0 = unpacklo4(ztxy, ztxy);
    v1 = unpackhi4(ztxy, ztxy);
    //dump(u0, "u0");
    //dump(u1, "u1");
    //dump(v0, "v0");
    //dump(v1, "v1");
    dump(blend22(u0, v0), " x");
    dump(blend22(u1, v1), " y");
    dump(blend22(v0, u0), " z");
    dump(blend22(v1, u1), " t");

    // using 1 permute and 2 blends
    xyzt = load4(mem);
    ztxy = swap2f128(xyzt);
    xy = blend22(xyzt, ztxy);
    zt = blend22(ztxy, xyzt);
    dump(duplo4(xy), " x");
    dump(duphi4(xy), " y");
    dump(duplo4(zt), " z");
    dump(duphi4(zt), " t");
}

__m256i make_mask2(long i)
{
    switch( i )
    {
        case  0: return _mm256_setr_epi64x( 0, 0, 0, 0);
        case  1: return _mm256_setr_epi64x(-1, 0, 0, 0);
        case  2: return _mm256_setr_epi64x(-1,-1, 0, 0);
        case  3: return _mm256_setr_epi64x(-1,-1,-1, 0);
        default: return _mm256_setr_epi64x(-1,-1,-1,-1);
    }
}

__m256i make_mask(long i)
{
    vec4 v{0.5, 1.5, 2.5, 3.5};
    return _mm256_castpd_si256(cmp4(v, set4(i), _CMP_LT_OQ));
}

void test_store()
{
    printf("------ test_store\n");
    double mem[4] = { 0, 0, 0, 0 };
    vec4 x{1.0, 2.0, 3.0, 4.0};
    dump(x, "value");
    for ( int i = 0; i < 5; ++i )
    {
        __m256i msk = make_mask(i);
        maskstore4(mem, msk, x);
        dump(load4(mem), "store");
    }
}


void test_swap1()
{
    printf("------ test_swap1\n");
    vec4 a{ 1, 2, 3, 4};
    vec4 b{-1,-2,-3,-4};
    dump(a, "a = ");
    dump(b, "b = ");
    
    dump(permute4(a,0x05), "permute(a,0x05)");
    dump(permute4(b,0x05), "permute(b,0x05)");
    
    dump(permute2f128(a,b,0x20), "permute2f128(a,b,0x20)");
    dump(permute2f128(a,b,0x31), "permute2f128(a,b,0x31)");
    dump(permute2f128(a,b,0x01), "permute2f128(a,b,0x01)");

    vec4 x = permute2f128(a, a, 0x21);
    dump(shuffle4(a, x, 0b0101), "rotate >");
    dump(shuffle4(x, a, 0b0101), "rotate <");
    
#if 0 //def __AVX2__
    dump(rotater4(a), "rotate >");
    dump(rotatel4(a), "rotate <");
#endif
    
    vec4 u = shuffle4(a, x, 0b0101);
    dump(blend22(u, x), "rotate 3");
}


void test_swap2()
{
    printf("------ test_swap2\n");
    vec4 a{ 1,  2,  3,  4};
    vec4 b{-1, -2, -3, -4};
    dump(a, "a");
    dump(b, "b");
    
    dump(permute2f128(a,a,0x08), "permute2f128(a,a,0x08)");
    dump(permute2f128(a,a,0x21), "permute2f128(a,a,0x21)");
    dump(permute2f128(a,a,0x81), "permute2f128(a,a,0x81)");
    
    dump(permute2f128(a,a,0x28), "permute2f128(a,a,0x28)");
    dump(permute2f128(a,a,0x81), "permute2f128(a,a,0x81)");
}


/*
 How to transform 'xyz' into 'xxx', 'yyy' and 'zzz'
 */
void test_swap4()
{
    printf("------ test_swap4\n");
    vec4 s{1, 2, 3, 4};

    {
        vec4 z = unpacklo4(s, s);
        vec4 u = unpackhi4(s, s);
        
        dump(s, "src");
        dump(z, "lo ");
        dump(u, "hi ");
        
        dump(permute2f128(z, z, 0x00), "permute2f128(z,z) 0x00");
        dump(permute2f128(u, u, 0x00), "permute2f128(u,u) 0x00");
        dump(permute2f128(z, z, 0x11), "permute2f128(z,z) 0x11");
        dump(permute2f128(u, u, 0x11), "permute2f128(u,u) 0x11");
    }
    {
        vec4 p = permute2f128(s, s, 0x01);
        vec4 l = blend22(s, p);
        vec4 u = blend22(p, s);
        vec4 x0 = unpacklo4(l,l);
        vec4 x1 = unpackhi4(l,l);
        vec4 x2 = unpacklo4(u,u);
        vec4 x3 = unpackhi4(u,u);

        dump(s, "src");
        dump(x0, "xxxx");
        dump(x1, "yyyy");
        dump(x2, "zzzz");
        dump(x3, "tttt");
    }
    {
        vec4 p = permute2f128(s, s, 0x01);
        vec4 l = blend22(s, p);
        vec4 u = blend22(p, s);
        vec4 z = unpacklo4(u,u);
        
        dump(s, "xyzt");
        dump(l, "xyxy");
        dump(z, "zzzz");
    }
}

void test_hadd()
{
    printf("------ test_hadd\n");
    vec4 a{1, -1, 2, -2};
    vec4 b{3, -3, 4, -4};
    dump(a, "a");
    dump(b, "b");

    vec4 p = permute2f128(a, b, 0x20);
    vec4 q = permute2f128(a, b, 0x31);
    dump(p, "p ");
    dump(q, "q ");

    vec4 z = unpacklo4(p, q);
    vec4 u = unpackhi4(p, q);

    dump(z, "z ");
    dump(u, "u ");
}


//------------------------------------------------------------------------------
#pragma mark -

void test_mat()
{
    printf("------ test_mat\n");
    // matrix:
    vec4 m012{0, 1, 2, -1};
    vec4 m345{3, 4, 5, -1};
    vec4 m678{6, 7, 8, -1};
    dump(m012, "m012");
    dump(m345, "m345");
    dump(m678, "m678");
    
    // symmetrized matrix:
    vec4 z = shuffle4(m012, m345, 0b0011);
    vec4 u = permute2f128(m678, z, 0x03);
    dump(z, "z");
    dump(u, "u");
    
    vec4 m145 = blend22(z, m345);
    vec4 m258 = blend22(u, m678);
    dump(m145, "m145");
    dump(m258, "m258");
    
    // transposed matrix:
    vec4 m036 = blend31(u, shuffle4(m012, m345, 0b1000));
    vec4 m147 = blend4(m145, permute4(u, 0b0101), 0b0100);
    
    dump(m036, "m036");
    dump(m147, "m147");
}


void test_transpose2()
{
    printf("------ test_transpose2\n");
    vec4 m{1, 2, 3, 4};
    vec4 t = blend4(m, permute4(permute2f128(m,m,0x01),0b1100), 0b0110);
    dump(m, "m");
    dump(t, "t");
#ifdef __AVX2__
    vec4 s = permute4x64(m, 0b11011000);
    dump(s, "s");
    dump(permute4x64(m, 0x88), "permute4x64(m, 0x88)");
    dump(permute4x64(m, 0xD8), "permute4x64(m, 0xD8)");
    dump(permute4x64(m, 0xDD), "permute4x64(m, 0xDD)");
    dump(permute4x64(m, 0x50), "permute4x64(m, 0x50)");
#endif
}


void test_transpose3()
{
    printf("------ test_transpose3\n");
    // matrix:
    vec4 m012{1, 2, 3, 0};
    vec4 m345{4, 5, 6, 0};
    vec4 m678{7, 8, 9, 0};

    dump(m012, "m012");
    dump(m345, "m345");
    dump(m678, "m678");
    
    // symmetrized matrix:
    vec4 z = shuffle4(m012, m345, 0b0011);
    vec4 u = permute2f128(m678, z, 0x03);
    vec4 t = shuffle4(m012, m345, 0b1000);
    dump(z, "z");
    dump(u, "u");
    dump(t, "t");
    dump(shuffle4(u, m345, 0b1110), "tmp");
    // transposed matrix:
    vec4 m036 = blend4(u, t, 0b1011);
    vec4 m147 = blend22(z, shuffle4(u, m345, 0b1100));
    vec4 m258 = blend22(u, m678);
    
    dump(m036, "m036");
    dump(m147, "m147");
    dump(m258, "m258");
}

void test_transpose4()
{
    printf("------ test_transpose4\n");
    // matrix:
    vec4 m0{ 0,  1,  2,  3};
    vec4 m1{ 4,  5,  6,  7};
    vec4 m2{ 8,  9, 10, 11};
    vec4 m3{12, 13, 14, 15};

    dump(m0, "m0");
    dump(m1, "m1");
    dump(m2, "m2");
    dump(m3, "m3");
    printf("\n");

    // transpose all 2x2 subblocks:
    vec4 u0 = unpacklo4(m0, m1);
    vec4 u1 = unpackhi4(m0, m1);
    vec4 u2 = unpacklo4(m2, m3);
    vec4 u3 = unpackhi4(m2, m3);

    dump(u0, "u0");
    dump(u1, "u1");
    dump(u2, "u2");
    dump(u3, "u3");
    printf("\n");
    
    // using 4 permutes
    vec4 t0 = permute2f128(u0, u2, 0x20);
    vec4 t1 = permute2f128(u1, u3, 0x20);
    vec4 t2 = permute2f128(u0, u2, 0x31);
    vec4 t3 = permute2f128(u1, u3, 0x31);

    // transposed matrix:
    dump(t0, "t0");
    dump(t1, "t1");
    dump(t2, "t2");
    dump(t3, "t3");
    printf("\n");

    // using 2 permutes and 4 blend
    vec4 x02 = permute2f128(u0, u2, 0x21);
    vec4 x13 = permute2f128(u1, u3, 0x21);
    dump(x02, "x02");
    dump(x13, "x13");
    t0 = blend22(u0, x02);
    t1 = blend22(u1, x13);
    t2 = blend22(x02, u2);
    t3 = blend22(x13, u3);

    // transposed matrix:
    dump(t0, "t0");
    dump(t1, "t1");
    dump(t2, "t2");
    dump(t3, "t3");
}


/* return transposed matrix
make dst = { XXXX YYYY ZZZZ TTTT }
from src = { XYZT XYZT XYZT XYZT }
*/
void transpose16(double const* src, double* dst)
{
    vec4 u0 = loadu4(src);
    vec4 u1 = loadu4(src+4);
    vec4 v2 = loadu4(src+8);
    vec4 v3 = loadu4(src+12);
    vec4 v0 = unpacklo4(u0, u1);
    vec4 v1 = unpackhi4(u0, u1);
    u0 = unpacklo4(v2, v3);
    u1 = unpackhi4(v2, v3);
    v2 = twine2f128(v0, u0);
    v3 = twine2f128(v1, u1);
    storeu4(dst   , blend22(v0, v2));
    storeu4(dst+4 , blend22(v1, v3));
    storeu4(dst+8 , blend22(v2, u0));
    storeu4(dst+12, blend22(v3, u1));
}

void test_transpose16()
{
    printf("------ test_transpose16\n");
    {
        double dst[16] = { 0 };
        //const double src[16] = { 1.1, 1.2, 1.3, 1.4, 2.1, 2.2, 2.3, 2.4, 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3, 4.4 };
        const double src[16] = { 1.1, 2.1, 3.1, 4.1, 1.2, 2.2, 3.2, 4.2, 1.3, 2.3, 3.3, 4.3, 1.4, 2.4, 3.4, 4.4 };
        transpose16(src, dst);
        printf("double    "); dump(16, src);
        printf("transpose "); dump(16, dst);
    }
}


void test_swap7()
{
    printf("------ test_swap7\n");
    vec4 s{1, 2, 3, 4};
    dump(s, "source");

    dump(permute4(s, 0b1010), "permute 0b1010");
    dump(permute4(s, 0b0101), "permute 0b0101");

    dump(shuffle4(s, s, 0b1100), "shuffle 0b1100");
    dump(shuffle4(s, s, 0b0011), "shuffle 0b0011");
    
    dump(permute4(s, 0b1100), "permute 0b1100");
    dump(permute4(s, 0b0011), "permute 0b0011");

    dump(permute4(s, 0b0000), "permute 0b0000");
    dump(permute4(s, 0b1111), "permute 0b1111");

    dump(unpacklo4(s, s), "unpacklo");
    dump(unpackhi4(s, s), "unpackhi");

#ifdef __AVX2__
    dump(permute4x64(s, 0xDD), "permute4x64 0xDD");
    dump(permute4x64(s, 0x88), "permute4x64 0x88");
    dump(permute4x64(s, 0xD8), "permute4x64 0xD8");
    dump(permute4x64(s, 0xC9), "permute4x64 0xC9");
    dump(permute4x64(s, 0xD2), "permute4x64 0xD2"); // Z X Y T
    dump(permute4x64(s, 0xC9), "permute4x64 0xC9"); // Y Z X T
#endif
}

#endif


int main(int argc, char * argv[])
{
    int i = 0;
    if ( argc > 1 )
        i = std::max(0, atoi(argv[1]));

    switch ( i )
    {
        case 0:
            init();
            run(scalar,  "scalar ", 1<<16);
            run(vector2, "vector2", 1<<16);
#ifdef __AVX__
            run(vector4, "vector4", 1<<16);
            run(vectorU, "vectorU", 1<<16);
#endif
            break;
#ifdef __AVX__
        case 1:
            unpack_matrix();
            test_twine();
            test_stride();
            break;
        case 2:
            test_cat();
            test_load();
            test_broadcast();
            test_store();
            break;
        case 3:
            //test_swap1();
            //test_swap2();
            test_swap4();
            test_hadd();
            break;
        case 4:
            test_transpose2();
            test_transpose3();
            test_transpose4();
            test_transpose16();
            break;
        case 5:
            test_swapSSE();
            test_swap7();
            break;
#endif
    }
}

