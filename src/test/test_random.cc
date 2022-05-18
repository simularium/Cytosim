// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "random.h"
#include <cstdio>
#include <cstring>
#include "timer.h"


void print_bits(FILE * f, const void * v, const int size)
{
    for ( int ii=0; ii < size; ++ii )
    {
        char c = ((char*)v)[ii];
        for ( int jj=7; jj >=0; --jj )
            fprintf(f, "%d", ( c >> jj ) & 1 );
        fprintf(f, ".");
    }
    fprintf(f, "\n");
}


void speed_test()
{
    const size_t cnt = 1 << 30;
    tic();
    uint32_t u = 10;
    for (size_t j=0; j<cnt; ++j)
    {
        u = RNG.pint32(1024);
        RNG.pint32(u);
    }
    printf("int %5.2f\n", toc(cnt));
}


void test_int()
{
    for (int j=0; j<8; ++j)
    {
        for (int k=0; k<8; ++k)
            printf(" %12u", RNG.pint32());
        printf("\n");
    }
    printf("\n");
    
    for (int j=0; j<8; ++j)
    {
        for (int k=0; k<8; ++k)
            printf(" %+12i", RNG.sint32());
        printf("\n");
    }
    printf("\n");

    for (int j=0; j<8; ++j)
    {
        for (int k=0; k<32; ++k)
            printf(" %2u", RNG.pint32(100));
        printf("\n");
    }
    printf("\n");

    for (int j=0; j<8; ++j)
    {
        for (int k=0; k<32; ++k)
            printf(" %2u", RNG.pint32_slow(99));
        printf("\n");
    }
    printf("\n");
}


void silly_test()
{
    const uint32_t up = 1 << 30;
    
    const uint32_t cnt = 1 << 24;
    uint32_t hit = 0;
    
    for (uint32_t j=0; j<cnt; ++j)
        hit += ( RNG.pint32() < up );

    printf(" prob( pint() < 1^30 ) = %f\n", hit/(float)cnt);
}


float convertFix(uint32_t x)
{
    //This assumes IEEE Standard 754 Floating point numbers
    //32 bits: 1 for sign, 8 for exponents, 23 for fraction
    const uint32_t FRAC     = 0x7FFFFFU;
    const uint32_t EXPON    = 127 << 23;
    uint32_t result = EXPON | ( x & FRAC );
    return *(float*)&result - 1.0;
}


void testbits()
{
    const int SCALE=2;
    float x;
    for ( int ii=0; ii <= SCALE; ++ii )
    {
        x = ii / float(SCALE);
        printf(" %f :", x);
        print_bits(stdout, &x, 4);
        // x = -ii / float(SCALE);
        // printf("%f :", x);
        // print_bits(stdout, &x, 4);
    }
    
    double y;
    for ( int ii=0; ii <= 20; ++ii )
    {
        y = convertFix( RNG.pint32() );
        printf(" %f :", y);
        print_bits(stdout, &y,8);
    }
}


#define TEST test
void test_test( const real prob, const size_t MAX )
{
    int cnt = 0, a, b, c;
    for ( size_t jj=0; jj < MAX; ++jj )
    {
        a = RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob);
        b = RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob);
        c = RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob);
        cnt += a + b + c;
    }
    printf("prob = %f measured = %f cnt = %i\n", prob, cnt / double(12*MAX), cnt);
}

void test_RNG(const size_t MAX)
{
    for ( size_t jj=0; jj < MAX; ++jj )
    {
        RNG.preal();RNG.preal();RNG.preal();RNG.preal();RNG.preal();
        RNG.preal();RNG.preal();RNG.preal();RNG.preal();RNG.preal();
    }
}


void test_float()
{
    for (int j=0; j<8; ++j)
    {
        for (int k=0; k<8; ++k)
            printf(" %10f", RNG.sreal());
        printf("\n");
    }

    printf("\n");
    
    for (int j=0; j<8; ++j)
    {
        for (int k=0; k<8; ++k)
            printf(" %10f", RNG.preal());
        printf("\n");
    }

    printf("\n");

    printf("pfloat:     ");
    float x;
    for ( int kk=0; kk < 10; ++kk )
    {
        x = RNG.pfloat();
        printf(" %+f", x);
    }
    printf("\n");
    printf("sfloat:     ");
    for ( int kk=0; kk < 10; ++kk )
    {
        x = RNG.sfloat();
        printf(" %+f", x);
    }
    printf("\n");
    
    double d;
    printf("pdouble:    ");
    for ( int kk=0; kk < 10; ++kk )
    {
        d = RNG.pdouble();
        printf(" %+f", d);
    }
    printf("\n");
    printf("sdouble:    ");
    for ( int kk=0; kk < 10; ++kk )
    {
        d = RNG.sdouble();
        printf(" %+f", d);
    }
    printf("\n");
    printf("sflip:      ");
    for ( int kk=0; kk < 10; ++kk )
    {
        d = RNG.sflip();
        printf(" %+f", d);
    }
    printf("\n");
}

//==========================================================================

void test_uniform()
{
    size_t cnt = 1<<28;
    real avg = 0;
    real var = 0;
    for ( size_t i = 0; i < cnt; ++i )
    {
        real x = RNG.sreal();
        real y = RNG.sreal();
        real z = RNG.sreal();
        real t = RNG.sreal();
        avg += x + y + z + t;
        var += x*x + y*y + z*z + t*t;
    }
    cnt *= 4;
    avg /= (real)cnt;
    var = var/(real)cnt - avg * avg;
    printf("UNIFORM      avg = %.12e   var = %.12e\n", avg, var);
}


void test_gauss()
{
    printf("Gauss\n");
    size_t cnt = 0;
    real avg = 0;
    real var = 0;
    const size_t n_max = 1<<6;
    real vec[n_max] = { 0 };
    for ( size_t i = 0; i < 10000000; ++i )
    {
        size_t n = RNG.pint32(n_max);
        RNG.gauss_set(vec, n);
        cnt += n;
        for ( size_t u = 0; u < n; ++u )
        {
            avg += vec[u];
            var += vec[u] * vec[u];
        }
    }
    avg /= (real)cnt;
    var = var/(real)cnt - avg * avg;
    printf("GAUSS      avg = %.12e   var = %.12e\n", avg, var);

}


void test_prob()
{
    size_t avg = 0;
    size_t cnt = 1 << 28;
    for ( size_t i = 0; i < cnt; ++i )
        avg += RNG.flip_8th();

    printf("8th      prob = %.6f\n", avg/(double)cnt);
}


void test_exponential()
{
    size_t cnt = 1 << 29;
    real avg = 0;
    real var = 0;
    for ( size_t i = 0; i < cnt; ++i )
    {
        real x = RNG.exponential();
        real y = RNG.exponential();
        real z = RNG.exponential();
        real t = RNG.exponential();
        avg += x + y + z + t;
        var += x*x + y*y + z*z + t*t;
    }
    cnt *= 4;
    avg /= (real)cnt;
    var = var/(real)cnt - avg * avg;
    printf("EXPONENTIAL  avg = %.12e   var = %.12e\n", avg, var);
}


void test_poisson(size_t sup)
{
    for ( size_t n = 0; n < sup; ++n )
    {
        int x = (int)(RNG.gauss() * sqrt(n) + n);
        printf("%10lu %9i %9i %9i\n", n, RNG.poisson_knuth(n), RNG.poisson(n), x);
    }
}


//==========================================================================
//test 3 methods to generate a random event time, when the rate varies in time
// F. Nedelec, Oct 2005

//this is our standard method: 64s CPU
int method1(const int maxTime, const real rate[])
{
    for ( int ii=0; ii<maxTime; ++ii )
    {
        if (RNG.test(rate[ii])) return ii;
    }
    return maxTime;
}

//this is 'exact' and very slow: 370s CPU (an exponential at each step!)
int method2(const int maxTime, const real rate[])
{
    for ( int ii=0; ii<maxTime; ++ii )
    {
        if ( RNG.preal() < -std::expm1(-rate[ii]) )
            return ii;
    }
    return maxTime;
}

//this is exact, and the fastest method: 10s CPU!
int method3(const int maxTime, const real rate[])
{
    real T = -log( RNG.preal() );
    for ( int ii=0; ii<maxTime; ++ii )
    {
        T -= rate[ii];
        if ( T < 0 ) return ii;
    }
    return maxTime;
}


int testGillespie(const int method)
{
    //test new idea for gillespie with changing rate (Oct 2005)
    const int maxTime = 200;
    real rate[maxTime];
    for ( int ii=0; ii<maxTime; ++ii )
        rate[ii] = ( ii % 10 ) / 30.0;
    
    int bins[3][maxTime+1];
    for ( int ii=0; ii<=maxTime; ++ii )
    {
        bins[0][ii] = 0;
        bins[1][ii] = 0;
        bins[2][ii] = 0;
    }
    
    const int nbSamples = 1000000;
    const int subSamples = 10;
    int result;
    switch( method )
    {
        case 0:
            for ( int ii=0; ii<nbSamples; ++ii )
            {
                bins[0][ method1(maxTime, rate) ]++;
                bins[1][ method2(maxTime, rate) ]++;
                bins[2][ method3(maxTime, rate) ]++;
            }
            break;
            
        case 1:
            printf("method 1:");
            for ( int ii=0; ii<nbSamples; ++ii )
                for ( int jj=0; jj<subSamples; ++jj )
                    result = method1(maxTime, rate);
            return result;
            
        case 2:
            printf("method 2:");
            for ( int ii=0; ii<nbSamples; ++ii )
                for ( int jj=0; jj<subSamples; ++jj )
                    result = method2(maxTime, rate);
            return result;
            
        case 3:
            printf("method 3:");
            for ( int ii=0; ii<nbSamples; ++ii )
                for ( int jj=0; jj<subSamples; ++jj )
                    result = method3(maxTime, rate);
            return result;
    }
    
    FILE* file = fopen("test.out", "w");
    for ( int ii=0; ii<=maxTime; ++ii )
        fprintf(file, "%4i   %6i %6i %6i\n", ii, bins[0][ii], bins[1][ii], bins[2][ii]);
    fclose(file);
    return 0;
}


//==========================================================================


/**
 Fill array `vec[]` with Gaussian values ~ N(0,1).
 the size of `vec` should be a multiple of 2, and sufficient to hold `end-src` values
 @Return the number of values that were stored in `vec`
 */
real * gauss_fill_0(real dst[], const int32_t src[], int32_t const*const end)
{
    while ( src < end )
    {
        real x = src[0] * TWO_POWER_MINUS_31;
        real y = src[1] * TWO_POWER_MINUS_31;
        real w = x * x + y * y;
        if ( w <= 1 && 0 < w )
        {
            w = sqrt( -2 * log(w) / w );
            *dst++ = w * x;
            *dst++ = w * y;
        }
        src += 2;
    }
    return dst;
}

#if defined(__INTEL_COMPILER) && defined(__AVX__)

#include "simd.h"

template < typename T >
void print(T const* vec, T const*const end)
{
#if ( 1 )
    for ( T const* f = vec; f < end; ++f )
    {
        for ( int i = 0; i < 8 && f < end; ++i )
            printf(" %10.6f", *f++);
            printf("\n");
    }
#else
    for ( T const* f = vec; f < end; ++f )
        printf(" %10.6f\n", *f);
#endif
}


// pack array by removing 'nan' values
template < typename T >
T * remove_nans(T * s, T * e)
{
    while ( s < e )
    {
        --e;
        // find the next `nan` going upward:
        while ( *s == *s )
        {
            if ( ++s > e )
                return s;
        }
        // skip `nan` values going downward:
        while ( *e != *e )
        {
            if ( --e <= s )
                return s;
        }
        // copy number over:
        *s++ = *e;
    }
    return s;
}


/**
 Calculates Gaussian-distributed, single precision random number,
 using SIMD AVX instructions
 Array `dst` should be able to hold as many 32-bit numbers as `src`.
 if 'real==float', for 256 bits of input, this produces ~64*PI bits of numbers.
 if 'real==double', this produces more output bits than input!

 The function used to calculate logarithm on SIMD data is part of the
 Intel SVML library, and is provided by the Intel compiler.

 F. Nedelec 02.01.2017
 */
real * gauss_fill(real dst[], const __m256i src[], __m256i* src_end)
{
    const vec8f fac = set8f(TWO_POWER_MINUS_31);
    const vec8f two = set8f(-2.0);
    
    real * d = dst;
    while ( src < src_end )
    {
        vec8f x = mulf(fac, cvt8i(load8si(src++)));
        vec8f y = mulf(fac, cvt8i(load8si(src++)));
        vec8f n = addf(mulf(x,x), mulf(y,y));
        /*
         The function used to calculate logarithm on SIMD data is part of the
         Intel SVML library, and is provided by the Intel compiler.
         */
        //w = sqrt( -2 * log(w) / n );
        n = rsqrtf(divf(n, mulf(two, _mm256_log_ps(n))));
        // the 16 single-precision values are converted to double-precision:
#if REAL_IS_DOUBLE
        x = mulf(n, x);
        y = mulf(n, y);
        store4(d   , cvt4f(getlof(x)));
        store4(d+4 , cvt4f(getlof(y)));
        store4(d+8 , cvt4f(gethif(x)));
        store4(d+12, cvt4f(gethif(y)));
#else
        store8f(d  , mulf(n, x));
        store8f(d+8, mulf(n, y));
#endif
        d += 16;
    }
    _mm_empty();
    return remove_nans(dst, d);
}

#endif

void test_gaussian(int cnt)
{
    printf("test_gaussian --- %lu bytes real --- %s\n", sizeof(real), __VERSION__);
    sfmt_t sfmt;
    sfmt_init_gen_rand(&sfmt, time(nullptr));
    int32_t * buf = (int32_t*)sfmt.state;

    if ( 1 )
    {
        tic();
        for ( int i = 0; i < cnt; ++i )
            sfmt_gen_rand_all(&sfmt);
        printf("refill %5.2f\n", toc(cnt));
        //print(vec, end);
    }
    if ( 1 )
    {
        real *end, vec[SFMT_N32] = { 0 };
        tic();
        for ( int i = 0; i < cnt; ++i )
        {
            end = gauss_fill_0(vec, buf, buf+SFMT_N32);
            sfmt_gen_rand_all(&sfmt);
        }
        printf("gauss0 %5.2f\n", toc(cnt));
        //print(vec, end);
    }
#if defined(__INTEL_COMPILER) && defined(__AVX__)
    __m256i * mem = (__m256i*)buf;
    if ( 1 )
    {
        real *end, vec[SFMT_N32] = { 0 };
        tic();
        for ( int i = 0; i < cnt; ++i )
        {
            end = gauss_fill(vec, mem, mem+SFMT_N256);
            sfmt_gen_rand_all(&sfmt);
        }
        printf("gauss avx %5.2f\n", toc(cnt));
        //print(vec, end);
    }
#endif
}


//==========================================================================
int main(int argc, char* argv[])
{
    int mode = 4;
    RNG.seed();

    if ( argc > 1 )
        mode = atoi(argv[1]);
    real rate = 1;
    if ( argc > 2 )
        rate = strtod(argv[2], 0);

    switch ( mode )
    {
        case 0:
            test_poisson(1024);
            test_prob();
            break;
            
        case 1:
            test_exponential();
            test_uniform();
            test_gauss();
            break;
    
        case 2:
            testGillespie(rate);
            break;

        case 3:
            for ( int kk=0; kk < 11; ++kk )
                test_test(rate*kk, 5000000);
            break;
            
        case 4:
            printf("sizeof(uint32_t) = %lu\n", sizeof(uint32_t));
            test_int();
            test_float();
            break;
            
        case 5:
            speed_test();
            break;
            
        case 6:
            silly_test();
            break;
            
        case 7:
            test_gaussian(1<<18);
            break;
    }
    
    printf("done\n");
    return EXIT_SUCCESS;
}

