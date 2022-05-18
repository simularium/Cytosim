// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECTOR4_H
#define VECTOR4_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

#ifdef __AVX__
#  define VECTOR4_USES_AVX REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define VECTOR4_USES_AVX 0
#endif


/// Vector4 is a vector with 4 `real` components.
/**
 Note: We assume that the coordinates XX, YY, ZZ and TT are adjacent in memory,
 allowing easy conversion operators to and from C-array.
 Although this is not guaranteed by the C-standard, this is usually the case.
 */
#if VECTOR4_USES_AVX
class alignas(32) Vector4
#else
class Vector4
#endif
{
    
public:
    
    /// dimensionality is 4
    static unsigned dimensionality() { return 4; }
    
    /// coordinates are public
    union {
        struct {
            real XX;
            real YY;
            real ZZ;
            real TT;
        };
#if VECTOR4_USES_AVX
        vec4 vec;
#endif
    };
    
    /// by default, coordinates are not initialized
    Vector4() {}
    
    /// construct from 3 values
    Vector4(real x, real y, real z) : XX(x), YY(y), ZZ(z), TT(0.0) {}

    /// construct from 4 values
    Vector4(real x, real y, real z, real t) : XX(x), YY(y), ZZ(z), TT(t) {}
    
    /// construct from address
    Vector4(const real v[]) : XX(v[0]), YY(v[1]), ZZ(v[2]), TT(v[3]) {}
    
#ifdef __AVX__
    /// construct from SIMD vector
    Vector4(vec4 const& v) { XX = v[0]; YY = v[1]; ZZ = v[2]; TT = v[3]; }
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
        assert_true(i<4);
        return (&XX)[i];
    }
    
    /// modifiable access to individual coordinates
    template< typename T >
    real& operator[](T i)
    {
        assert_true(i<4);
        return (&XX)[i];
    }
#endif
    
    /// return x-component
    real x() const { return XX; }
    /// return y-component
    real y() const { return YY; }
    /// return z-component
    real z() const { return ZZ; }

    /// copy coordinates from array of size d
    void load(const real v[], const int& d)
    {
        XX = ( d > 0 ) ? v[0] : 0;
        YY = ( d > 1 ) ? v[1] : 0;
        ZZ = ( d > 2 ) ? v[2] : 0;
        TT = ( d > 3 ) ? v[3] : 0;
    }
    
    /// replace coordinates by the ones provided
    void load(const float b[])
    {
        XX = b[0];
        YY = b[1];
        ZZ = b[2];
        TT = b[3];
    }
    
    /// replace coordinates by the ones provided
    void load(const double b[])
    {
        XX = b[0];
        YY = b[1];
        ZZ = b[2];
        TT = b[3];
    }
    
    /// copy coordinates to given array
    void store(float b[]) const
    {
        b[0] = (float)XX;
        b[1] = (float)YY;
        b[2] = (float)ZZ;
        b[3] = (float)TT;
    }
    
    /// copy coordinates to given array
    void store(double b[]) const
    {
        b[0] = (double)XX;
        b[1] = (double)YY;
        b[2] = (double)ZZ;
        b[3] = (double)TT;
    }
    
    /// add content to given address
    void add_to(real b[]) const
    {
        b[0] += XX;
        b[1] += YY;
        b[2] += ZZ;
        b[3] += TT;
    }
    
    /// add content scaled by `alpha` to given address
    void add_to(real alpha, real b[]) const
    {
        b[0] += alpha * XX;
        b[1] += alpha * YY;
        b[2] += alpha * ZZ;
        b[3] += alpha * TT;
    }
    
    /// add content `n` times to array `b` of size `ldd*n`
    void add_to(real b[], int n, int ldd) const
    {
        for ( int i = 0; i < n; ++i )
        {
            b[ldd*i  ] += XX;
            b[ldd*i+1] += YY;
            b[ldd*i+2] += ZZ;
            b[ldd*i+3] += TT;
        }
    }
    
    /// subtract to given address
    void sub_to(real b[]) const
    {
        b[0] -= XX;
        b[1] -= YY;
        b[2] -= ZZ;
        b[3] -= TT;
    }
    
    /// subtract content scaled by `alpha` to given address
    void sub_to(real alpha, real b[]) const
    {
        b[0] -= alpha * XX;
        b[1] -= alpha * YY;
        b[2] -= alpha * ZZ;
        b[3] -= alpha * TT;
    }
    
    /// set coordinates to zero
    void reset()
    {
#if VECTOR4_USES_AVX
        vec = setzero4();
#else
        XX = 0;
        YY = 0;
        ZZ = 0;
        TT = 0;
#endif
    }
    
    /// change coordinates
    void set(const real x, const real y, const real z)
    {
        XX = x;
        YY = y;
        ZZ = z;
        TT = 0;
    }

    /// change coordinates
    void set(const real x, const real y, const real z, const real t)
    {
        XX = x;
        YY = y;
        ZZ = z;
        TT = t;
    }
    
    /// change signs of all coordinates
    void oppose()
    {
        XX = -XX;
        YY = -YY;
        ZZ = -ZZ;
        TT = -TT;
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX + YY*YY + ZZ*ZZ + TT*TT;
    }
    
    /// the standard norm = sqrt(x^2+y^2+z^2+t^2)
    real norm() const
    {
        return sqrt(XX*XX + YY*YY + ZZ*ZZ + TT*TT);
    }
    
    /// the standard norm = sqrt(x^2+y^2+z^2+t^2)
    friend real norm(Vector4 const& V)
    {
        return V.norm();
    }
    
    /// the inversed magnitude = 1.0 / sqrt(x^2+y^2+z^2+t^2)
    real inv_norm() const
    {
        return 1.0 / sqrt(XX*XX + YY*YY + ZZ*ZZ + TT*TT);
    }

    /// the 2D norm = sqrt(x^2+y^2)
    real normXY() const
    {
        return sqrt(XX*XX + YY*YY);
    }
    
    /// the 2D norm = sqrt(x^2+z^2)
    real normXZ() const
    {
        return sqrt(XX*XX + ZZ*ZZ);
    }
    
    /// the 2D norm = sqrt(y^2+z^2)
    real normYZ() const
    {
        return sqrt(YY*YY + ZZ*ZZ);
    }
    
    /// the 2D norm = sqrt(y^2+z^2)
    real normYZSqr() const
    {
        return YY*YY + ZZ*ZZ;
    }
    
    /// square of the distance between two points, equivalent to (a-b).normSqr()
    friend real distanceSqr(Vector4 const& a, Vector4 const& b)
    {
        real x = a.XX - b.XX;
        real y = a.YY - b.YY;
        real z = a.ZZ - b.ZZ;
        real t = a.TT - b.TT;
        return x*x + y*y + z*z + t*t;
    }

    /// distance between two points, equivalent to (a-b).norm()
    friend real distance(Vector4 const& a, Vector4 const& b)
    {
        return sqrt(distanceSqr(a, b));
    }
    
    /// absolute values: (|x|, |y|, |z|, |t|)
    Vector4 abs() const
    {
        return Vector4(fabs(XX), fabs(YY), fabs(ZZ), fabs(TT));
    }

    /// the infinite norm = max(|x|, |y|, |z|)
    real norm_inf() const
    {
        return std::max(std::max(fabs(XX), fabs(YY)), std::max(fabs(ZZ), fabs(TT)));
    }
    
    /// true if no component is NaN
    bool valid() const
    {
        return ( XX == XX ) && ( YY == YY ) && ( ZZ == ZZ ) && ( TT == TT );
    }
    
    /// true if some component is not zero
    bool is_not_zero() const
    {
        return ( XX || YY || ZZ || TT );
    }

    /// scale to unit norm
    void normalize()
    {
#if VECTOR4_USES_AVX
        vec = normalize4(vec);
#else
        real s = norm();
        XX /= s;
        YY /= s;
        ZZ /= s;
        TT /= s;
#endif
    }
    
    /// scale to obtain norm `n`
    void normalize(const real n)
    {
#if VECTOR4_USES_AVX
        vec = normalize4(vec, n);
#else
        real s = n / norm();
        XX *= s;
        YY *= s;
        ZZ *= s;
        TT *= s;
#endif
    }

    /// returns the colinear vector of norm `n` (default 1.0)
    const Vector4 normalized(const real n = 1.0) const
    {
#if VECTOR4_USES_AVX
        return Vector4(normalize4(vec, n));
#else
        real s = n / norm();
        return Vector4(s*XX, s*YY, s*ZZ, s*TT);
#endif
    }
    
    /// returns vector parallel to argument of unit norm
    friend const Vector4 normalize(Vector4 const& V)
    {
#if VECTOR4_USES_AVX
        return Vector4(normalize4(V.vec));
#else
        const real s = V.norm();
        return Vector4(V.XX/s, V.YY/s, V.ZZ/s, V.TT/s);
#endif
    }

    //------------------------------------------------------------------
    
    /// linear interpolation, returning a + x * b
    friend const Vector4 interpolate(const Vector4& a, real x, const Vector4& b)
    {
        return Vector4(a.XX+x*b.XX, a.YY+x*b.YY, a.ZZ+x*b.ZZ, a.TT+x*b.TT);
    }
    
    /// addition of two vectors
    friend const Vector4 operator +(Vector4 const& a, Vector4 const& b)
    {
        return Vector4(a.XX+b.XX, a.YY+b.YY, a.ZZ+b.ZZ, a.TT+b.TT);
    }
    
    /// subtraction of two vectors
    friend const Vector4 operator -(Vector4 const& a, Vector4 const& b)
    {
        return Vector4(a.XX-b.XX, a.YY-b.YY, a.ZZ-b.ZZ, a.TT-b.TT);
    }
    
    /// unary + operator does nothing
    friend const Vector4 operator +(Vector4 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector4 operator -(Vector4 const& b)
    {
        return Vector4(-b.XX, -b.YY, -b.ZZ, -b.TT);
    }
    
    /// returns the element-by-element product
    const Vector4 e_mul(Vector4 const& b) const
    {
        return Vector4(XX*b.XX, YY*b.YY, ZZ*b.ZZ, TT*b.TT);
    }

    /// returns the element-by-element division
    const Vector4 e_div(Vector4 const& b) const
    {
        return Vector4(XX/b.XX, YY/b.YY, ZZ/b.ZZ, TT/b.TT);
    }
    
    /// returns a vector with each element squared
    const Vector4 e_squared() const
    {
#if VECTOR4_USES_AVX
        return Vector4(mul4(vec, vec));
#else
        return Vector4(XX*XX, YY*YY, ZZ*ZZ, TT*TT);
#endif
    }
    
    /// returns sum of all coordinates
    real e_sum() const
    {
        return XX + YY + ZZ + TT;
    }
    
    /// returns min(x, y, z, t)
    real e_min() const
    {
        return std::min(std::min(XX, YY), std::min(ZZ, TT));
    }
    
    /// returns max(x, y, z, t)
    real e_max() const
    {
        return std::max(std::max(XX, YY), std::max(ZZ, TT));
    }
    
    /// returns the element-by-element minimum
    const Vector4 e_min(Vector4 const& v) const
    {
        return Vector4(std::min(XX, v.XX), std::min(YY, v.YY), std::min(ZZ, v.ZZ), std::min(TT, v.TT));
    }
    
    /// returns the element-by-element maximum
    const Vector4 e_max(Vector4 const& v) const
    {
        return Vector4(std::max(XX, v.XX), std::max(YY, v.YY), std::max(ZZ, v.ZZ), std::max(TT, v.TT));
    }
    
    /// scalar product of two vectors
    friend real dot(Vector4 const& a, Vector4 const& b)
    {
        return a.XX * b.XX + a.YY * b.YY + a.ZZ * b.ZZ + a.TT * b.TT;
    }
    
    /// 3D cross product of two vector (TT components are ignored)
    friend const Vector4 cross(Vector4 const& a, Vector4 const& b)
    {
        return Vector4(a.YY * b.ZZ - a.ZZ * b.YY,
                       a.ZZ * b.XX - a.XX * b.ZZ,
                       a.XX * b.YY - a.YY * b.XX);
    }

    /// multiplication by scalar s
    friend const Vector4 operator *(Vector4 const& a, const real s)
    {
        return Vector4(s*a.XX, s*a.YY, s*a.ZZ, s*a.TT);
    }
    
    /// mutiplication by scalar s
    friend const Vector4 operator *(const real s, Vector4 const& a)
    {
        return Vector4(s*a.XX, s*a.YY, s*a.ZZ, s*a.TT);
    }
    
    /// division by scalar s
    friend const Vector4 operator /(Vector4 const& a, const real s)
    {
        return Vector4(a.XX/s, a.YY/s, a.ZZ/s, a.TT/s);
    }
    
    /// addition of another vector b
    void operator +=(Vector4 const& b)
    {
        XX += b.XX;
        YY += b.YY;
        ZZ += b.ZZ;
        TT += b.TT;
    }
    
    /// subtraction of another vector b
    void operator -=(Vector4 const& b)
    {
        XX -= b.XX;
        YY -= b.YY;
        ZZ -= b.ZZ;
        TT -= b.TT;
    }
    
    /// multiplication by a scalar
    void operator *=(const real s)
    {
        XX *= s;
        YY *= s;
        ZZ *= s;
        TT *= s;
    }
    
    /// division by a scalar
    void operator /=(const real s)
    {
        XX /= s;
        YY /= s;
        ZZ /= s;
        TT /= s;
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector4 const& a, Vector4 const& b)
    {
        return ( a.XX==b.XX  &&  a.YY==b.YY  &&  a.ZZ==b.ZZ  &&  a.TT==b.TT );
    }
    
    /// non-equality test
    friend bool operator !=(Vector4 const& a, Vector4 const& b)
    {
        return ( a.XX!=b.XX  ||  a.YY!=b.YY  ||  a.ZZ!=b.ZZ  ||  a.TT!=b.TT );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string toString() const
    {
        std::ostringstream oss;
        oss << XX << " " << YY << " " << ZZ << " " << TT;
        return oss.str();
    }
    
    /// conversion to ASCII string with given precision
    std::string toString(int w, int p) const
    {
        std::ostringstream oss;
        oss.precision(p);
        oss << std::setw(w) << XX << " ";
        oss << std::setw(w) << YY << " ";
        oss << std::setw(w) << ZZ << " ";
        oss << std::setw(w) << TT;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f %+9.3f %+9.3f", XX, YY, ZZ, TT);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f %+9.3f %+9.3f %+9.3f )", XX, YY, ZZ, TT);
    }
    
    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f %+9.3f %+9.3f\n", XX, YY, ZZ, TT);
    }
    
    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    
    /// Vector with random independent coordinates in [0,+1]
    static const Vector4 randP();
    
    /// Vector with random independent coordinates in [0,+n]
    static const Vector4 randP(real n);
    
    /// Vector with random independent coordinates in [-1,+1]
    static const Vector4 randS();
    
    /// Vector with random independent coordinates in [-1/2,+1/2]
    static const Vector4 randH();
    
    /// Vector with random independent coordinates in [-n,+n]
    static const Vector4 randS(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector4 randU();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector4 randU(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector4 randB();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector4 randB(real n);
  
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector4 randG(real n);
    
};


//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream& operator >> (std::istream&, Vector4&);

/// stream output operator
std::ostream& operator << (std::ostream&, Vector4 const&);


#endif
