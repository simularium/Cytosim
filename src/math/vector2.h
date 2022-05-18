// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECTOR2_H
#define VECTOR2_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

#ifdef __SSE3__
#  define VECTOR2_USES_SSE REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define VECTOR2_USES_SSE 0
#endif


/// Vector2 is a vector with 2 `real` components.
/**
 Note: We assume that the coordinates XX and YY are adjacent in memory,
 allowing easy conversion operators to and from C-array.
 Although this is not guaranteed by the C-standard, this is usually the case.
 */
class Vector2
{
    
public:
    
    /// dimensionality is 2
    static unsigned dimensionality() { return 2; }
    
    /// coordinates are public
    union {
        struct {
            real XX;
            real YY;
        };
#if VECTOR2_USES_SSE
        vec2 vec;
#endif
    };
    
    /// by default, coordinates are not initialized
    Vector2() {}
    
    /// construct from 3 values
    Vector2(real x, real y, real) : XX(x), YY(y) {}
    
    /// construct from 2 values
    Vector2(real x, real y) : XX(x), YY(y) {}
    
    /// construct from address
    Vector2(const real v[]) : XX(v[0]), YY(v[1]) {}

#if VECTOR2_USES_SSE
    /// construct from SIMD vector
    Vector2(vec2 const& v) { vec = v; }
#elif defined(__SSE3__)
    /// construct from SIMD vector
    Vector2(vec2 const& v) { XX = v[0]; YY = v[1]; }
#endif
    
    
    /// address of coordinate array
    real * data()                { return &XX; }
    
    /// constant address of coordinate array
    real const* data()     const { return &XX; }
    
#if ( 1 )
    /// implicit conversion to a modifiable real pointer
    operator real*()             { return &XX; }
    
    /// implicit conversion to a constant real pointer
    operator const real*() const { return &XX; }
#else
    /// value of a coordinate
    template< typename T >
    real operator[](T i) const
    {
        assert_true(i<2);
        return (&XX)[i];
    }
    
    /// modifiable access to individual coordinates
    template< typename T >
    real& operator[](T i)
    {
        assert_true(i<2);
        return (&XX)[i];
    }
#endif
    
    /// return x-component
    real x() const { return XX; }
    /// return y-component
    real y() const { return YY; }
    /// return z-component
    real z() const { return 0; }

    /// copy coordinates from array of size d
    void load(const real v[], const int& d)
    {
        XX = ( d > 0 ) ? v[0] : 0;
        YY = ( d > 1 ) ? v[1] : 0;
    }
    
    /// replace coordinates by the ones provided
    void load(const float b[])
    {
        XX = b[0];
        YY = b[1];
    }
    
    /// replace coordinates by the ones provided
    void load(const double b[])
    {
        XX = b[0];
        YY = b[1];
    }
    
    /// copy coordinates to given array
    void store(float b[]) const
    {
        b[0] = (float)XX;
        b[1] = (float)YY;
    }
    
    /// copy coordinates to given array
    void store(double b[]) const
    {
#if VECTOR2_USES_SSE
        store2(b, vec);
#else
        b[0] = (double)XX;
        b[1] = (double)YY;
#endif
    }
    
    /// add content to given address
    void add_to(real b[]) const
    {
#if VECTOR2_USES_SSE
        store2(b, add2(vec, load2(b)));
#else
        b[0] += XX;
        b[1] += YY;
#endif
    }
    
    /// add content scaled by `alpha` to given address
    void add_to(real alpha, real b[]) const
    {
#if VECTOR2_USES_SSE
        store2(b, fmadd2(set2(alpha), vec, load2(b)));
#else
        b[0] += alpha * XX;
        b[1] += alpha * YY;
#endif
    }
    
    /// add content `n` times to array `b` of size `ldd*n`
    void add_to(real b[], int n, int ldd) const
    {
        for ( int i = 0; i < n; ++i )
        {
            b[ldd*i  ] += XX;
            b[ldd*i+1] += YY;
        }
    }
    
    /// subtract to given address
    void sub_to(real b[]) const
    {
#if VECTOR2_USES_SSE
        store2(b, sub2(load2(b), vec));
#else
        b[0] -= XX;
        b[1] -= YY;
#endif
    }
    
    /// subtract content scaled by `alpha` to given address
    void sub_to(real alpha, real b[]) const
    {
#if VECTOR2_USES_SSE
        store2(b, fnmadd2(set2(alpha), vec, load2(b)));
#else
        b[0] -= alpha * XX;
        b[1] -= alpha * YY;
#endif
    }
    
    /// set coordinates to zero
    void reset()
    {
#if VECTOR2_USES_SSE
        vec = setzero2();
#else
        XX = 0;
        YY = 0;
#endif
    }
    
    /// change coordinates
    void set(const real x, const real y)
    {
#if VECTOR2_USES_SSE
        vec = setr2(x, y);
#else
        XX = x;
        YY = y;
#endif
    }
    
    /// change coordinates (last argument is discarded)
    void set(const real x, const real y, const real)
    {
#if VECTOR2_USES_SSE
        vec = setr2(x, y);
#else
        XX = x;
        YY = y;
#endif
    }
    
    /// change signs of all coordinates
    void oppose()
    {
#if VECTOR2_USES_SSE
        vec = _mm_xor_pd(vec, set2(-0.0));
#else
        XX = -XX;
        YY = -YY;
#endif
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX + YY*YY;
    }
    
    
    /// the standard norm = sqrt(x^2+y^2)
    real norm() const
    {
        return sqrt(XX*XX + YY*YY);
    }
    
    /// the standard norm = sqrt(x^2+y^2)
    friend real norm(Vector2 const& V)
    {
        return V.norm();
    }
    
    /// the inversed magnitude = 1.0 / sqrt(x^2+y^2)
    real inv_norm() const
    {
        return 1.0 / sqrt(XX*XX + YY*YY);
    }

    /// the 2D norm = sqrt(x^2+y^2)
    real normXY() const
    {
        return sqrt(XX*XX + YY*YY);
    }
    
    /// the 2D norm = |y| since Z = 0
    real normYZ() const
    {
        return fabs(YY);
    }
    
    /// the 2D norm = y^2 since Z = 0
    real normYZSqr() const
    {
        return YY*YY;
    }
    
    /// square of the distance between two points, equivalent to (a-b).normSqr()
    friend real distanceSqr(Vector2 const& a, Vector2 const& b)
    {
        real x = a.XX - b.XX;
        real y = a.YY - b.YY;
        return x*x + y*y;
    }

    /// distance between two points, equivalent to (a-b).norm()
    friend real distance(Vector2 const& a, Vector2 const& b)
    {
        return sqrt(distanceSqr(a, b));
    }
    
    /// absolute values: (|x|, |y|)
    Vector2 abs() const
    {
        return Vector2(fabs(XX), fabs(YY));
    }

    /// the infinite norm = max(|x|, |y|)
    real norm_inf() const
    {
        return std::max(fabs(XX), fabs(YY));
    }
    
    /// true if no component is NaN
    bool valid() const
    {
        return ( XX == XX ) && ( YY == YY );
    }
    
    /// true if some component is not zero
    bool is_not_zero() const
    {
        return ( XX || YY );
    }

    /// scale to unit norm
    void normalize()
    {
#if VECTOR2_USES_SSE
        vec = normalize2(vec);
#else
        real s = norm();
        XX /= s;
        YY /= s;
#endif
    }

    /// scale to obtain norm `n`
    void normalize(const real n)
    {
#if VECTOR2_USES_SSE
        vec = normalize2(vec, n);
#else
        real s = n / norm();
        XX *= s;
        YY *= s;
#endif
    }
    
    /// returns the colinear vector of norm `n` (default 1.0)
    const Vector2 normalized(const real n = 1.0) const
    {
#if VECTOR2_USES_SSE
        return Vector2(normalize2(vec, n));
#else
        real s = n / norm();
        return Vector2(s*XX, s*YY);
#endif
    }
    
    /// returns vector parallel to argument of unit norm
    friend const Vector2 normalize(Vector2 const& V)
    {
#if VECTOR2_USES_SSE
        return Vector2(normalize2(V.vec));
#else
        const real s = V.norm();
        return Vector2(V.XX/s, V.YY/s);
#endif
    }

    //------------------------------------------------------------------
    
    /// returns a perpendicular vector, of same norm
    const Vector2 orthogonal() const
    {
        return Vector2(-YY, XX);
    }
    
    /// returns a perpendicular vector, of norm `n`
    const Vector2 orthogonal(const real n) const
    {
        real s = n / sqrt( XX*XX + YY*YY );
        return Vector2(-s*YY, s*XX);
    }
    
    /// returns a vector perpendicular to *this, close to `d` and of norm = `n`
    const Vector2 orthogonal(Vector2 const& d, const real n) const
    {
        real s = dot(*this, d) / normSqr();
        return ( d - s * (*this) ).normalized(n);
    }
    
    /// convert from cartesian to spherical coordinates ( r, theta, phi )
    const Vector2 spherical() const
    {
        return Vector2(sqrt(XX*XX+YY*YY), atan2(YY, XX));
    }
    
    /// convert from spherical to cartesian coordinates ( x, y, z )
    const Vector2 cartesian() const
    {
        return Vector2(XX*cos(YY), XX*sin(YY));
    }
    
    //------------------------------------------------------------------
    
    /// linear interpolation, returning a + x * b
    friend const Vector2 interpolate(const Vector2& a, real x, const Vector2& b)
    {
        return Vector2(a.XX+x*b.XX, a.YY+x*b.YY);
    }
    
    /// addition of two vectors
    friend const Vector2 operator +(Vector2 const& a, Vector2 const& b)
    {
        return Vector2(a.XX+b.XX, a.YY+b.YY);
    }
    
    /// subtraction of two vectors
    friend const Vector2 operator -(Vector2 const& a, Vector2 const& b)
    {
        return Vector2(a.XX-b.XX, a.YY-b.YY);
    }
    
    /// unary + operator does nothing
    friend const Vector2 operator +(Vector2 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector2 operator -(Vector2 const& b)
    {
        return Vector2(-b.XX, -b.YY);
    }
    
    /// returns the element-by-element product
    const Vector2 e_mul(Vector2 const& b) const
    {
        return Vector2(XX*b.XX, YY*b.YY);
    }
    
    /// returns the element-by-element division
    const Vector2 e_div(Vector2 const& b) const
    {
        return Vector2(XX/b.XX, YY/b.YY);
    }
    
    /// returns a vector with each element squared
    const Vector2 e_squared() const
    {
#if VECTOR2_USES_SSE
        return Vector2(mul2(vec, vec));
#else
        return Vector2(XX*XX, YY*YY);
#endif
    }
    
    /// returns sum of all coordinates
    real e_sum() const
    {
        return XX + YY;
    }
    
    /// returns min(x, y)
    real e_min() const
    {
        return std::min(XX, YY);
    }
    
    /// returns max(x, y)
    real e_max() const
    {
        return std::max(XX, YY);
    }
    
    /// returns the element-by-element minimum
    const Vector2 e_min(Vector2 const& v) const
    {
        return Vector2(std::min(XX, v.XX), std::min(YY, v.YY));
    }
    
    /// returns the element-by-element maximum
    const Vector2 e_max(Vector2 const& v) const
    {
        return Vector2(std::max(XX, v.XX), std::max(YY, v.YY));
    }
    
    /**
     In dimension 2, we define a cross-product operator which returns a real,
     which in this case represents a Vector aligned with the Z axis.
     We also define the cross-product with a scalar, also corresponding to a
     Vector aligned with Z. This is a fair contraction of the 3D vector product.
     */
    
    /// the cross product of two vectors is a Z-Vector
    friend real cross(Vector2 const& a, Vector2 const& b)
    {
        return a.XX * b.YY - a.YY * b.XX;
    }
    
    /// cross product of a vector with a Z-Vector
    friend const Vector2 cross(Vector2 const& a, const real b)
    {
        return Vector2(a.YY*b, -a.XX*b);
    }
    
    /// cross product of a Z-vector with a Vector
    friend const Vector2 cross(const real a, Vector2 const& b)
    {
        return Vector2(-a*b.YY, a*b.XX);
    }
    
    /// scalar product of two vectors
    friend real dot(Vector2 const& a, Vector2 const& b)
    {
        return a.XX * b.XX + a.YY * b.YY;
    }
    
    /// multiplication by scalar s
    friend const Vector2 operator *(Vector2 const& a, const real s)
    {
        return Vector2(s*a.XX, s*a.YY);
    }
    
    /// mutiplication by scalar s
    friend const Vector2 operator *(const real s, Vector2 const& a)
    {
        return Vector2(s*a.XX, s*a.YY);
    }
    
    /// division by scalar s
    friend const Vector2 operator /(Vector2 const& a, const real s)
    {
        return Vector2(a.XX/s, a.YY/s);
    }
    
    /// addition of another vector b
    void operator +=(Vector2 const& b)
    {
        XX += b.XX;
        YY += b.YY;
    }
    
    /// subtraction of another vector b
    void operator -=(Vector2 const& b)
    {
        XX -= b.XX;
        YY -= b.YY;
    }
    
    /// multiplication by a scalar
    void operator *=(const real s)
    {
        XX *= s;
        YY *= s;
    }
    
    /// division by a scalar
    void operator /=(const real s)
    {
        XX /= s;
        YY /= s;
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector2 const& a, Vector2 const& b)
    {
        return ( a.XX==b.XX  &&  a.YY==b.YY );
    }
    
    /// non-equality test
    friend bool operator !=(Vector2 const& a, Vector2 const& b)
    {
        return ( a.XX!=b.XX  ||  a.YY!=b.YY );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string toString() const
    {
        std::ostringstream oss;
        oss << XX << " " << YY;
        return oss.str();
    }
    
    /// conversion to a string with given precision
    std::string toString(int w, int p) const
    {
        std::ostringstream oss;
        oss.precision(p);
        oss << std::setw(w) << XX << " ";
        oss << std::setw(w) << YY;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f", XX, YY);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f %+9.3f )", XX, YY);
    }
    
    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f\n", XX, YY);
    }
    
    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    
    /// a vector orthogonal to *this, with `norm == n`, chosen randomly and uniformly
    const Vector2 randOrthoU(real n) const;
    
    /// a vector orthogonal to *this, with `norm <= n`, assuming `norm(*this)==1`
    const Vector2 randOrthoB(real n) const;
    
    
    /// Vector with random independent coordinates in [0,+1]
    static const Vector2 randP();
    
    /// Vector with random independent coordinates in [0,+n]
    static const Vector2 randP(real n);
    
    /// Vector with random independent coordinates in [-1,+1]
    static const Vector2 randS();
    
    /// Vector with random independent coordinates in [-1/2,+1/2]
    static const Vector2 randH();
    
    /// Vector with random independent coordinates in [-n,+n]
    static const Vector2 randS(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector2 randU();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector2 randU(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector2 randB();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector2 randB(real n);
    
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector2 randG(real n);
    
};


//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream& operator >> (std::istream&, Vector2&);

/// stream output operator
std::ostream& operator << (std::ostream&, Vector2 const&);


#endif

