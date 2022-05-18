// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University.

#include "random.h"

#include <cstdio>
#include <cstdlib>
#include <climits>
#include <sys/time.h>
#include <cstring>
#include <ctime>


/// static object
Random RNG;


/// the most significant bit in a 32-bits integer
constexpr uint32_t BIT31 = 1U << 31;

//constexpr uint32_t FRAC32 = 0x7FFFFFU;
/// bit mask for exponent in single precision (float)
constexpr uint32_t EXPON32 = 127U << 23;

/// sign bit in double precision (double)
constexpr uint64_t BIT63 = 1ULL << 63;

// constexpr uint64_t EXPON64 = 1023ULL << 52;


/**
 The generator is initialized with a zero state vector,
 and seed() must be called before any random number can be produced.
 */
Random::Random()
{
    if ( sizeof(uint32_t) != 4 )
    {
        fprintf(stderr, "Random can only work if sizeof(uint32_t) == 4\n");
        exit(1);
    }
    
    //fprintf(stderr, "Random with SFMT_N32 = %i\n", SFMT_N32);
    
    // clear state (not necessary):
    memset(integers_, 0, 4*SFMT_N32);
    memset(gaussians_, 0, sizeof(real)*SFMT_N32);
    memset(twister_.state[0].u, 0, 4*SFMT_N32);

    // initialize pointers signalling an empty reserve:
    start_ = integers_;
    end_ = start_;
    next_gaussian_ = gaussians_;
}


Random::~Random()
{
    //printf("Random Number Generator released\n");
}


void Random::seed(const uint32_t s)
{
    sfmt_init_gen_rand(&twister_, s);
    refill();
}

/**
 Get a uint32_t from t and c
 Better than uint32_t(x) in case x is floating point in [0,1]
 Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)
 */
uint32_t hash(long t, int32_t c)
{
    uint32_t h1 = 0;
    unsigned char* p = (unsigned char*) &t;
    for ( size_t i = 0; i < sizeof(t); ++i )
    {
        h1 *= UCHAR_MAX + 2U;
        h1 += p[i];
    }
    uint32_t h2 = 0;
    p = (unsigned char*) &c;
    for ( size_t j = 0; j < sizeof(c); ++j )
    {
        h2 *= UCHAR_MAX + 2U;
        h2 += p[j];
    }
    return h1 ^ h2;
}


uint32_t Random::seed()
{
    uint32_t s = 0;
    // read system source if available, from /dev/urandom which does not block!
    FILE * f = fopen("/dev/urandom", "r");
    if ( f && ! ferror(f) )
    {
        int cnt = 0;
        while ( s == 0 && ++cnt < 32 )
        {
            if ( fread(&s, sizeof(s), 1, f) < sizeof(s) )
            {
                s = 0;
                break;
            }
        }
    }
    // use clock otherwise
    if ( s == 0 )
    {
        struct timeval now;
        gettimeofday(&now, nullptr);
        s = hash(now.tv_sec, now.tv_usec);
    }
    if ( s == 0 )
        s = 1;
    seed(s);
    return s;
}


bool Random::seeded()
{
    uint32_t * buf = twister_.state[0].u;
    for ( size_t n = 0; n < SFMT_N32; ++n )
        if ( buf[n] )
            return true;
    return false;
}

//------------------------------------------------------------------------------
#pragma mark -

float Random::pfloat()
{
    //This assumes IEEE Standard 754 Floating point numbers
    //32 bits: 1 for sign, 8 for exponents, 23 for fraction
    union { uint32_t i; float f; } tmp;
    tmp.i = URAND32();
    uint32_t E = 126;
    while (( tmp.i < BIT31 ) && ( E > 94 ))
    {
        tmp.i = tmp.i << 1; --E;
    }
    tmp.i = (( tmp.i << 1 ) >> 9 ) | ( E << 23 );
    return tmp.f;
}


float Random::sfloat()
{
    //This assumes IEEE Standard 754 Floating point numbers
    //32 bits: 1 for sign, 8 for exponents, 23 for fraction
    union { uint32_t i; float f; } tmp;
    tmp.i = URAND32();
    uint32_t sign = tmp.i & BIT31;
    tmp.i = tmp.i << 1;
    uint32_t E = 126;
    while (( tmp.i < BIT31 ) && ( E > 94 ))
    {
        tmp.i = tmp.i << 1; --E;
    }
    tmp.i = sign | (( tmp.i << 1 ) >> 9 ) | ( E << 23 );
    return tmp.f;
}

/// fast (dirty) random float in [0,1[, requires IEEE Standard 754
float Random::pfloat23()
{
    //by setting random bits for the fraction-bits of a float IEEE 754,
    //we get a random number between 1 and 2. We substract 1.0,
    //but that drops the lower bits, reducing the precision
    union { uint32_t i; float f; } tmp;
    tmp.i = EXPON32 | ( URAND32() >> 9 );
    return tmp.f - 1.0f;
}

//------------------------------------------------------------------------------
#pragma mark -

double Random::pdouble()
{
    //This assumes IEEE Standard 754 Floating point numbers
    //64 bits: 1 for sign, 11 for exponents, 52 for Fraction
    uint64_t w = URAND64();
    uint64_t E = 1022;
    while ( w < BIT63  &&  E > 959 )
    {
        w = w << 1; --E;
    }
    union { uint64_t i; double f; } tmp;
    tmp.i = ((w >> 11) & 0x000FFFFFFFFFFFFFULL) | ( E << 52 );
    return tmp.f;
}


double Random::sdouble()
{
    //This assumes IEEE Standard 754 Floating point numbers
    //64 bits: 1 for sign, 11 for exponents, 52 for Fraction
    uint64_t w = URAND64();
    uint64_t sign = w & BIT63;
    w = w << 1;
    uint64_t E = 1022;
    while ( w < BIT63  &&  E > 959 )
    {
        w = w << 1; --E;
    }
    union { uint64_t i; double f; } tmp;
    tmp.i = sign | ((w >> 11) & 0x000FFFFFFFFFFFFFULL) | ( E << 52 );
    return tmp.f;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Set two signed real number, following a normal law N(0,v*v)
 using Marsaglia polar method (George Marsaglia, 1964)
 */
void Random::gauss_set(real & a, real & b, real v)
{
    real x, y, w;
    do {
        x = sreal();
        y = sreal();
        w = x * x + y * y;
    } while ( w >= 1.0 || w == 0 );
    /*
     formula below are only valid if ( w > 0 ),
     which may be false only with a minuscule probability
     */
    w = v * std::sqrt( -2 * std::log(w) / w );
    a = w * x;
    b = w * y;
}


/**
 Fill array `vec[]` with Gaussian values ~ N(0,1).
 the size of `vec` should be a multiple of 2, and sufficient to hold `end-src` values
 For each 4 input values, this produces ~PI values.
 @Return address past the last value stored in `dst` = dst + nb_of_values_set
 */
real * gauss_fill(real dst[], size_t cnt, const int32_t src[])
{
    int32_t const*const end = src + cnt;
    while ( src < end )
    {
        real x = src[0] * TWO_POWER_MINUS_31;
        real y = src[1] * TWO_POWER_MINUS_31;
        real w = x * x + y * y;
        if (( w <= 1 ) & ( 0 < w ))
        {
            w = sqrt( -2 * log(w) / w );
            dst[0] = w * x;
            dst[1] = w * y;
            dst += 2;
        }
        src += 2;
    }
    return dst;
}

/**
 Fill array `gaussians_` with approximately 500 Gaussian values ~ N(0,1).
 Set `next_gaussian` past the last position containing a valid number.
 The number of gaussian values set by this function is random,
 and it may even be zero.
 */
void Random::refill_gaussians()
{
    next_gaussian_ = gauss_fill(gaussians_, SFMT_N32, (int32_t*)twister_.state);
    sfmt_gen_rand_all(&twister_);
    //printf("refill_gaussians %lu\n", next_gaussian_ - gaussians_);
}


#if ( 0 )

/**
 Fill `n` Gaussian values ~ N(0,1) in array `vec[]`.
 */
void Random::gauss_set(real vec[], size_t cnt, real v = 1.0)
{
    unsigned u = cnt % 8;
    unsigned w = u % 2;
    
    if ( w )
        vec[0] = v * gauss();
    
    for ( ; w < u; w += 2 )
        gauss_set(vec[w], vec[w+1], v);
    
    for ( ; u < cnt; u += 8 )
    {
        gauss_set(vec[u  ], vec[u+1], v);
        gauss_set(vec[u+2], vec[u+3], v);
        gauss_set(vec[u+4], vec[u+5], v);
        gauss_set(vec[u+6], vec[u+7], v);
    }
}

#else

/**
 Fill `n` Gaussian values ~ N(0,1) in array `vec[]`.
 */
void Random::gauss_set(real vec[], size_t cnt)
{
    size_t n = (size_t)( next_gaussian_ - gaussians_ );
    // check if `vec` would consume all the buffer:
    while ( n <= cnt )
    {
        // use all values in buffer:
        copy_real(n, gaussians_, vec);
        vec += n;
        cnt -= n;
        refill_gaussians();
        n = (size_t)( next_gaussian_ - gaussians_ );
    };
    
    // use `cnt` values from buffer:
    next_gaussian_ -= cnt;
    copy_real(cnt, next_gaussian_, vec);
}

#endif


/**
 This is the Box & Muller method.
 A note on the generation of random normal deviates
 Box & Muller, 1958
 */
void Random::gauss_boxmuller(real& x, real& y)
{
    real ang = real(RAND32()) * ( TWO_POWER_MINUS_31 * M_PI );
    real nrm = std::sqrt( -2 * std::log( preal_exc() ));
    x = nrm * std::cos(ang);
    y = nrm * std::sin(ang);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 integer in [0,n] for n < 2^32
 */
uint32_t Random::pint32_slow(const uint32_t n)
{
    // Find which bits are used in n
    uint32_t used = n | ( n >> 1 );
    used |= (used >> 2);
    used |= (used >> 4);
    used |= (used >> 8);
    used |= (used >> 16);
    
    // Draw numbers until one is found in [0,n]
    uint32_t i;
    do
        i = URAND32() & used;  // toss unused bits to shorten search
    while ( i > n );
    return i;
}


/**
 returns a random integer with exactly `b` bits equal to `1`,
 but randomly positionned.
 */
uint32_t Random::distributed_bits(unsigned b)
{
    uint32_t n = 0;
    if ( b < 16 )
    {
        while ( b > 0 )
        {
            uint32_t x = 1 << ( URAND32() & 31 );
            if (!( n & x ))
            {
                n += x;
                --b;
            }
        }
    }
    else
    {
        n = ~0U;
        while ( b < 32 )
        {
            uint32_t x = 1 << ( URAND32() & 31 );
            if ( n & x )
            {
                n -= x;
                ++b;
            }
        }
    }
    return n;
}


/**
 returns an integer in [0 n], with the ratios given in the array of ints
 */
uint32_t Random::pint32_ratio(const uint32_t n, const uint32_t ratio[])
{
    uint32_t ii, sum = 0;
    for ( ii = 0; ii < n; ++ii )
        sum += ratio[ii];
    // `sum==0` may be caused by wrong arguments; might be safer to throw an exception
    if ( sum == 0 )
        return 0; //throw InvalidParameter("invalid argument to Random::pint32_ratio");
    sum = (int) std::floor( preal() * sum );
    ii = 0;
    while ( sum >= ratio[ii] )
        sum -= ratio[ii++];
    return ii;
}

//------------------------------------------------------------------------------
#pragma mark -
/**
 Return Poisson distributed integer, with expectation=E  variance=E
 http://en.wikipedia.org/wiki/Poisson_distribution
 
 This routine is slow for large values of E.
 If E > 256, this returns a Gaussian distribution of parameter (E, E),
 which is a good approximation of the Poisson distribution
 
 Knuth D.E. The art of computer programming, Vol II: Seminumerical algorithms.
 
 This method fails for E > 700, in double precision
 */
uint32_t Random::poisson_knuth(const real E)
{
    if ( E > 256 )
        return static_cast<uint32_t>( gauss() * std::sqrt(E) + E );
    if ( E < 0 )
        return 0;
    real L = std::exp(-E);
    real p = preal();
    uint32_t k = 0;
    while ( p > L )
    {
        ++k;
        p *= preal();
    }
    return k;
}


/**
 Return Poisson distributed integer, with expectation=E  variance=E
 http://en.wikipedia.org/wiki/Poisson_distribution
 
 This routine is slow for large values of E.
 If E > 512, this returs a Gaussian distribution of parameter (E, E),
 which is a good approximation of the Poisson distribution.
 
 This method fails for E > 700, in double precision
 */
uint32_t Random::poisson(const real E)
{
    if ( E > 256 )
        return static_cast<uint32_t>( gauss() * std::sqrt(E) + E );
    if ( E < 0 )
        return 0;
    real p = std::exp(-E);
    real s = p;
    uint32_t k = 0;
    real u = preal();
    while ( u > s )
    {
        ++k;
        p *= E / k;
        s += p;
    }
    return k;
}


/**
 This is equivalent to calling poisson(exp(-E))
 The argument is EL = exp(-E)
 expectation=E  variance=E (see wikipedia, Poisson Distribution)
 */
uint32_t Random::poissonE(const real EL)
{
    real p = preal();
    uint32_t k = 0;
    while ( p > EL )
    {
        ++k;
        p *= preal();
    }
    return k;
}


uint32_t Random::geometric(const real P)
{
    if ( P < 0 )
        return 0;
    const uint32_t pi = (uint32_t)( P * 0x1p32 );
    
    uint32_t s = 0;
    while ( URAND32() > pi )
        ++s;
    return s;
}


uint32_t Random::binomial(const int N, const real P)
{
    if ( P < 0 )
        return 0;
    const uint32_t pi = (uint32_t)( P * 0x1p32 );
    
    uint32_t s = 0;
    for ( int x = 0; x < N; ++x )
        if ( URAND32() < pi )
            ++s;
    return s;
}

