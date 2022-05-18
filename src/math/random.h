// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

#ifndef REAL_H
#  include "real.h"
#endif

#define SFMT_MEXP 19937

#include "SFMT.h"

/// the maximum value of a signed 32-bit integer is 2^31-1
#define TWO_POWER_MINUS_31 0x1p-31
/// the maximum value of a unsigned 32-bit integer is 2^32-1
#define TWO_POWER_MINUS_32 0x1p-32
/// the maximum value of a unsigned 64-bit integer is 2^64-1
#define TWO_POWER_MINUS_64 0x1p-64


/// Random Number Generator
/**
 The generation of random bits is done with Mersenne Twister from U. of Hiroshima
 
 http://en.wikipedia.org/wiki/Mersenne_twister
 http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 
 This class provides a convenient interface, and functions to generate floating
 point values following various distributions (Gaussian, Exponential, Poisson),
 and other facilities.
 
 F. Nedelec, reclassified on 13.09.2018
*/
class alignas(64) Random
{
    /// reserve of random integers
    uint32_t integers_[SFMT_N32];

    /// reserve of standard normally-distributed numbers ~ N(0,1)
    real gaussians_[SFMT_N32];

    /// Mersenne Twister Generator
    sfmt_t twister_;

    /// pointer to integer reserve, at unused lower end
    uint32_t const* start_;

    /// pointer to integer reserve, at unused upper end + 1
    uint32_t const* end_;
    
    /// pointer to access the next value in `gaussians_[]`
    real * next_gaussian_;
    
protected:
    
    /// replenish state vector
    void refill()
    {
        memcpy(integers_, twister_.state, 4*SFMT_N32);
        start_ = integers_;
        end_ = start_ + SFMT_N32;
        sfmt_gen_rand_all(&twister_);
    }

    /// extract next 32 random bits
    uint32_t URAND32()
    {
        if ( end_ <= start_ )
            refill();
        --end_;
        return *end_;
    }

    /// extract next 32 random bits as signed integer
    int32_t RAND32()
    {
        if ( end_ <= start_ )
            refill();
        --end_;
        return *reinterpret_cast<int32_t const*>(end_);
    }
    
    /// extract next 64 random bits
    uint64_t URAND64()
    {
        // consume data from the 'start_' to preserve alignment
        if ( end_ <= 1 + start_ )
            refill();
        uint64_t const* p = reinterpret_cast<uint64_t const*>(start_);
        start_ += 2;
        return *p;
    }
    
    /// extract next 64 random bits as signed integer
    int64_t RAND64()
    {
        // consume data from the 'start_' to preserve alignment
        if ( end_ <= 1 + start_ )
            refill();
        int64_t const* p = reinterpret_cast<int64_t const*>(start_);
        start_ += 2;
        return *p;
    }
    
    /// a non-negative 'real' using 31 random bits, in [0, 1[
    real ZERO2ONE()
    {
        // the cast gives a number with (x <= 2^31-1)
        return std::fabs(static_cast<real>(RAND32())) * TWO_POWER_MINUS_31;
    }

    /// a signed 'real' using 31 random bits + sign bit (-2^31+1 < x <= 2^31-1)
    real BIGREAL()
    {
        return static_cast<real>(RAND32());
    }

public:
    
    /// Constructor sets the state vector to zero
    Random();
    
    /// destructor
    ~Random();

    /// true if state vector is not entirely zero
    bool     seeded();

    /// seed with given 32 bit integer
    void     seed(const uint32_t s);
    
    /// seed by reading /dev/random and if this fails using the clock
    uint32_t seed();

    /// signed integer in [-2^31+1, 2^31-1];
    int32_t  sint32() { return RAND32(); }

    /// unsigned integer in [0, 2^32-1]
    uint32_t pint32() { return URAND32(); }
    
    /// unsigned integer in [0, 2^64-1]
    int64_t  sint64() { return RAND64(); }

    /// unsigned integer in [0, 2^64-1]
    uint64_t pint64() { return URAND64(); }

    /// unsigned integer in [0,n-1] for n < 2^32
    uint32_t pint32(const uint32_t& n) { return uint32_t(ZERO2ONE()*n); }

    /// unsigned integer in [0,n-1] for n < 2^64
    uint64_t pint64(const uint64_t& n) { return uint64_t(ZERO2ONE()*n); }
 
    /// integer in [0,n] for n < 2^32, (slow) bitwise algorithm
    uint32_t pint32_slow(uint32_t n);
    
    /// a random unsigned integer with exactly `b` bit equal to `1`
    uint32_t distributed_bits(unsigned b);

    /// integer in [0 N], with probabilities given in ratio[] of size N, with sum(ratio)>0
    uint32_t pint32_ratio(uint32_t n, const uint32_t ratio[]);

    /// integer k of probability distribution p(k,E) = exp(-E) * pow(E,k) / factorial(k)
    uint32_t poisson(real E);
    
    /// integer k of probability distribution p(k,E) = EL * pow(E,k) / factorial(k)
    uint32_t poissonE(real EL);
    
    /// integer k of probability distribution p(k,E) = exp(-E) * pow(E,k) / factorial(k)
    uint32_t poisson_knuth(real E);

    /// number of successive unsuccessful trials, when success has probability p (result >= 0)
    uint32_t geometric(real p);

    /// number of sucesses among n trials of probability p
    uint32_t binomial(int n, real p);
    
    
    /// returns true with probability (p), and false with probability (1-p)
    bool test(real p)     { return ( ZERO2ONE() <  p ); }
    
    /// returns true with probability (1-p), and false with probability (p)
    bool test_not(real p) { return ( ZERO2ONE() >= p ); }
    
    /// 0  or  1  with equal chance
    int  flip()           { return URAND32() & 1U; }
    
    /// returns -1  or  1 with equal chance
    real flipsign()       { return std::copysign(1, RAND32()); }
    
    /// returns 1 with probability P and -1 with probability 1-P
    real flipsign(real p) { return 2*(int)test(p) - 1; }

    /// True with probability 1/8
    bool flip_8th()       { return URAND32() < 1<<29; }
    
    /// fast (dirty) random float in [0,1[, requires IEEE Standard 754
    float pfloat23();
    
    /// random float in [0,1[, requires IEEE Standard 754 
    float pfloat();
    
    /// random float in ]-1,1[, requires IEEE Standard 754
    float sfloat();
    
    /// slow random double in [0,1[, using two uint32_t to set all the fraction bits, requires IEEE Standard 754
    double pdouble();
    
    /// slow random double in ]-1,1[, using two uint32_t to set all the fraction bits, requires IEEE Standard 754
    double sdouble();
    
    /// positive real number in [0,1[, zero included
    real preal()           { return ZERO2ONE(); }
    
    /// positive real number in [0,n[ = n * preal() : deprecated, use preal() * n
    real preal(real n)     { return n * ( ZERO2ONE() ); }
    
    /// signed real number in ]-1,1[, boundaries excluded
    real sreal()           { return BIGREAL() * TWO_POWER_MINUS_31; }
    
    /// signed real number in ]-1/2, 1/2[, boundaries excluded
    real shalf()           { return BIGREAL() * TWO_POWER_MINUS_32; }
    
    /// returns -1.0 or 1.0 with equal chance
    real sflip()           { return sign_real(BIGREAL()); }
    
    /// returns -a or a with equal chance
    real sflip(real a)     { return std::copysign(a, BIGREAL()); }

    /// non-zero real number in ]0,1]
    real preal_exc()       { return 1 - ZERO2ONE(); }
    
    /// non-zero real number in ]0,n]
    real preal_exc(real n) { return preal_exc() * n; }
    
    /// real number uniformly distributed in [a,b[
    real real_uniform(real a, real b) { return a + preal() * ( b - a ); }
    
    
    
    /// refill array `gaussians_[]` with normal law N(0,1), and reset next_gaussian_
    void refill_gaussians();
    
    /// set two independent random numbers, both following a normal law N(0,v*v)
    void gauss_set(real&, real&, real v);

    /// random Gaussian number, following a normal law N(0,1)
    real gauss()
    {
        while ( next_gaussian_ <= gaussians_ )
            refill_gaussians();
        --next_gaussian_;
        return *next_gaussian_;
    }

    /// fill array `vec` with independent random numbers following normal law N(0,1).
    void gauss_set(real vec[], size_t n);
    
    /// fill array `vec` with independent random numbers following normal law N(0,v*v).
    void gauss_set(real vec[], size_t n, real v);

    /// signed real number, following a normal law N(0,1), slower algorithm
    void gauss_boxmuller(real &, real&);
    
    /// random in [0, inf[, with P(x) = exp(-x), mean = 1.0, variance = 1.0
    real exponential() { return -std::log(1 - ZERO2ONE());  }
    
    /// exponentially distributed positive real, with P(x) = exp(-x/E) / E,  parameter E is 1/Rate
    real exponential(const real E) { return -E * std::log(1 - ZERO2ONE());  }

    /// fair choice among two given values
    template<typename T>
    T choice(const T& x, const T& y)
    {
        if ( RAND32() > 0 )
            return x;
        else
            return y;
    }
    
    /// fair choice within an array of `size` values
    template<typename T>
    T choice(const T val[], uint32_t size)
    {
        return val[ pint32(size) ];
    }
    
    /// uniform shuffling of array `T[]`.
    /** Algorithm from knuth's The Art of Programming, Vol 2 chp. 3.4.2 */
    template <typename T> 
    void shuffle(T val[], uint32_t size)
    {
        uint32_t jj = size, kk;
        while ( jj > 1 )
        {
            kk = pint32(jj);
            --jj;
            T tmp   = val[jj];
            val[jj] = val[kk];
            val[kk] = tmp;
        }
    }
};


/// declaring a global Random Number Generator
extern Random RNG;

/**
 Linear congruential random number generator
 The coefficients are found in Numerical Recipes 3rd Ed. Chapter 7
 See also http://en.wikipedia.org/wiki/Linear_congruential_generator
 
 The low-order bits should never be relied on for any degree of randomness whatsoever. 
 To use the topmost bit:

     if ( z & 0x80000000U )
 
 */

inline static uint32_t lcrng1(uint32_t z) { return z * 2024337845U + 797082193U; }

inline static uint32_t lcrng2(uint32_t z) { return z * 279470273U + 4294967291U; }

inline static uint32_t lcrng3(uint32_t z) { return z * 1372383749U + 1289706101U; }

inline static uint32_t lcrng4(uint32_t z) { return z * 1103515245U + 12345U; }

#endif  //RANDOM_H
