// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 Some basic mathematical functions
 Francois Nedelec, 
*/

#ifndef SMATH_H
#define SMATH_H

#include "real.h"
#include <cmath>
#include <sstream>
#include <stdint.h>
#include <iomanip>


#ifndef M_PI
/// Ratio of a circle's circumference to its diameter
constexpr real M_PI=3.14159265358979323846264338327950288;
#endif

#ifndef M_E
constexpr real M_E=2.7182818284590452354;
#endif


/// simple mathematical functions, mostly templated
namespace sMath
{
    // limit `x` inside [`a` , `b` ]:
    template <typename T>
    inline const T& crop(T& x, const T& a, const T& b)
    {
        if ( x < a )
            return a;
        if ( x > b )
            return b;
        return x;
    }

    /// minimum of three arguments
    template <typename T>
    inline const T& min(const T& a, const T& b, const T& c)
    {
        return std::min(a, std::min(b, c));
    }
    
    /// maximum of three arguments
    template <typename T>
    inline const T& max(const T& a, const T& b, const T& c)
    {
        return std::max(a, std::max(b, c));
    }
    
    /// minimum of four arguments
    template <typename T>
    inline const T& min(const T& a, const T& b, const T& c, const T& d)
    {
        return std::min(std::min(a,b), std::min(c,d));
    }
    
    /// maximum of four arguments
    template <typename T>
    inline const T& max(const T& a, const T& b, const T& c, const T& d)
    {
        return std::max(std::max(a,b), std::max(c,d));
    }
    
    /// return index of the arguments that is the smallest
    template <typename T>
    inline int arg_min(const T& a, const T& b, const T& c)
    {
        if ( a > b )
            return 1 + ( b > c );
        else
            return ( a > c ) * 2;
    }
    
    /// return index of the arguments that is the largest
    template <typename T>
    inline int arg_max(const T& a, const T& b, const T& c)
    {
        if ( a < b )
            return 1 + ( b < c );
        else
            return ( a < c ) * 2;
    }

    /// return index of the arguments that is the smallest
    template <typename T>
    inline int arg_min(const T& a, const T& b, const T& c, const T& d)
    {
        if ( a > b )
        {
            // consider ( b, c, d )
            if ( b > c )
                return 2 + ( c > d );
            else
                return 1 + ( b > d ) * 2;
        }
        else
        {
            // consider ( a, c, d )
            if ( a > c )
                return 2 + ( c > d );
            else
                return ( a > d ) * 3;
        }
    }
    
    /// return index of the arguments that is the largest
    template <typename T>
    inline int arg_max(const T& a, const T& b, const T& c, const T& d)
    {
        if ( a < b )
        {
            // consider ( b, c, d )
            if ( b < c )
                return ( c < d ) ? 3 : 2;
            else
                return ( b < d ) ? 3 : 1;
        }
        else
        {
            // consider ( a, c, d )
            if ( a < c )
                return ( c < d ) ? 3 : 2;
            else
                return ( a < d ) ? 3 : 0;
        }
    }


    /**
     Set vectors 'x' and 'y' to make an orthonormal basis (x, y, z)
     
     Building an Orthonormal Basis, Revisited
     Tom Duff et al. Journal of Computer Graphics Techniques Vol. 6 N.1, 2017
     */
    template < typename FLOAT >
    inline void orthonormal(const FLOAT z[3], FLOAT x[3], FLOAT y[3])
    {
        const FLOAT s = std::copysign((FLOAT)1.0, z[2]);
#if ( 1 )
        /// optimized version by Marc B. Reynolds
        const FLOAT a = z[1] / ( z[2] + s );
        const FLOAT b = z[1] * a;
        const FLOAT c = z[0] * a;
        x[0] = -z[2] - b;
        x[1] = c;
        x[2] = z[0];
        y[0] = s * c;
        y[1] = s * b - 1.0f;
        y[2] = s * z[1];
#else
        const FLOAT a = -1.0f / ( a[2] + s );
        const FLOAT b = a[0] * a[1] * a;
        x[0] = 1.0 + s * z[0] * z[0] * a;
        x[1] = s * b;
        x[2] = -s * z[0];
        y[0] = b;
        y[1] = s + z[1] * z[1] * a;
        y[2] = -z[1];
#endif
    }
    
    template < typename FLOAT >
    inline void orthonormal(const FLOAT z[3], FLOAT x[3], FLOAT y[3], FLOAT scale)
    {
        const FLOAT s = std::copysign((FLOAT)1.0, z[2]);
        /// optimized version by Marc B. Reynolds
        const FLOAT a = z[1] / ( z[2] + s );
        const FLOAT b = z[1] * a;
        const FLOAT c = z[0] * a;
        float ss = s * scale;
        x[0] = scale * ( -z[2] - b );
        x[1] = scale * c;
        x[2] = scale * z[0];
        y[0] = ss * c;
        y[1] = ss * b - scale;
        y[2] = ss * z[1];
    }
    
    /// square of a number
    template <typename T> 
    inline T square(const T& a)
    {
        return a * a;
    }
    
    /// cube of a number
    template <typename T>
    inline T cube(const T& a)
    {
        return a * a * a;
    }
    
    /// power of `a` by positive integer exponent `n`
    /** This should be equivalent to std::pow(a, n) */
    template <typename T>
    inline T power_int(const T& a, unsigned n)
    {
        T x = a;
        T y = 1;
        while ( n )
        {
            if ( n & 1 )
                y = y * x;
            x = x * x;
            n = n >> 1;
        }
        return y;
    }
    
    
    ///power of `a` by integer exponent `n`
    template <typename T>
    inline T power(const T& a, const int n)
    {
        if ( n < 0 )
            return power_int(1.0/a, -n);
        return power_int(a, n);
    }
    
    
    ///power of `a` by integer exponent `n`
    template <typename T>
    T nextPowerOf2(T k)
    {
        if ( k & (k-1) )
        {
            do
                k &= k-1;
            while ( k & (k-1) );
            
            k <<= 1;
        }
        return k;
    }
    
    
    ///square of distance between two vectors in dimension `dim`
    template <int dim, typename T>
    inline T distanceSqr(const T a[], const T b[])
    {
        T x = a[0] - b[0];
        T n = x * x;
        for( int i = 1; i < dim; ++i )
        {
            x = a[i] - b[i];
            n += x * x;
        }
        return n;
    }
 
    ///usual distance between two vectors of dimension `dim`
    template <int dim, typename T>
    inline T distance(const T a[], const T b[])
    {
        T x = a[0] - b[0];
        T n = x * x;
        for( int i = 1; i < dim; ++i )
        {
            x = a[i] - b[i];
            n += x * x;
        }
        return std::sqrt(n);
    }
    
    //------------------------------------------------------------------------------

    /// return the usual base-10 representation of a number
    /** Note that with C++11, you can use std::to_string */
    template <typename T> 
    std::string repr(T const& x)
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    
    template <typename T>
    std::string repr(T const& x, unsigned width, unsigned precision)
    {
        std::ostringstream oss;
        oss.precision(precision);
        oss << std::setw(width) << std::fixed << x;
        return oss.str();
    }
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    /// used for periodic boundary conditions:
    inline void fold(real& x, const real p)
    {
        while ( x >  p ) x -= p+p;
        while ( x < -p ) x += p+p;
    }

#ifdef WIN32
    
    //this is needed under windows:
    inline real remainder( const real a, const real b )
    {
        int p = (int)floor( 0.5 + a / b );
        if ( p )
            return a - p * b;
        else
            return a;
    }
    
    inline real round(real x)
    {
        if ( x < 0 )
            return -floor(0.5-x);
        else
            return  floor(0.5+x);
    }

#endif
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    ///extract a 10-decimal digit form a number:
    /** 1st digit is really the first one, as index here do not start at zero */
    template <typename T> 
    inline int digit(T x, const int p)
    {
        for ( int q=1; q < p; ++q )
            x /= 10;
        return x % 10;
    }
    
    ///copy bytes
    inline void copyBytes( void * dest, const void * src, const unsigned cnt)
    {
        for ( unsigned ii=0; ii < cnt; ++ii )
            ((char*)dest)[ii] = ((char*)src)[ii];
    }
    

    //------------------------------------------------------------------------------
    
    /// return smallest power of 2 that is greater or equal to `x`
    inline unsigned next_power(unsigned x)
    {
        if ( x > 0 )
            return 0;
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return x+1;
    }
    
    /// return smallest power of 2 that is greater or equal to `x`
    inline size_t next_power(size_t x)
    {
        if ( x == 0 )
            return 0;
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        return x+1;
    }

    /// number of '1' bits in a 32-bits integer (Charlie Gordon & Don Clugston)
    /** Should use Intel SIMD instruction POPCNT */
    inline unsigned int count_bits(uint32_t v)
    {
        v = v - ((v >> 1) & 0x55555555);
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
        return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
    }
    
    
    /// number of '1' bits, from: http://graphics.stanford.edu/~seander/bithacks.html
    /** Works up to 128 bits */
    template <typename T>
    unsigned int count_bits2(T v)
    {
        v = v - ((v >> 1) & (T)~(T)0/3);
        v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);
        v = (v + (v >> 4)) & (T)~(T)0/255*15;
        return (T)(v * ((T)~(T)0/255)) >> (sizeof(v) - 1) * 8;
    }
   
}


#endif //#ifdef SMATH_H
