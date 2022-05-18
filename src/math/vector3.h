// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECTOR3_H
#define VECTOR3_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

class Vector1;
class Vector2;

#ifdef __AVX__
#  define VECTOR3_USES_AVX REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define VECTOR3_USES_AVX 0
#endif


/// Vector3 is a vector with 3 `real` components.
/**
 Note: We assume that the coordinates XX, YY and ZZ are adjacent in memory,
 allowing easy conversion operators to and from C-array.
 Although this is not guaranteed by the C-standard, this is usually the case.
 */
#if VECTOR3_USES_AVX
class alignas(32) Vector3
#else
class Vector3
#endif
{
    
public:
    
    /// dimensionality is 3
    static unsigned dimensionality() { return 3; }
    
    /// coordinates are public
    union {
        struct {
            real XX;
            real YY;
            real ZZ;
#if VECTOR3_USES_AVX
            real TT;
        };
        vec4 vec;
#else
        };
#endif
    };

#if VECTOR3_USES_AVX
    /// by default, coordinates are not initialized
    Vector3() { TT = 0; }
    
    /// construct from 3 values
    Vector3(real x, real y, real z) : XX(x), YY(y), ZZ(z), TT(0) {}
    
    /// construct from address
    Vector3(const real v[]) : XX(v[0]), YY(v[1]), ZZ(v[2]), TT(0) {}
#else
    /// by default, coordinates are not initialized
    Vector3() { }

    /// construct from 3 values
    Vector3(real x, real y, real z) : XX(x), YY(y), ZZ(z) {}

    /// construct from address
    Vector3(const real v[]) : XX(v[0]), YY(v[1]), ZZ(v[2]) {}
#endif

#if VECTOR3_USES_AVX
    /// construct from SIMD vector
    Vector3(vec4 const& v) { vec = v; }
    /// conversion to SIMD vector
    operator vec4 () const { return vec; }
#elif defined(__AVX__) && REAL_IS_DOUBLE
    /// conversion to SIMD vector
    operator vec4 () const { return load3(&XX); }
    /// construct from SIMD vector
    Vector3(vec4 const& v) { XX = v[0]; YY = v[1]; ZZ = v[2]; }
#endif
    
    /// copy 2 coordinates from Vector2
    Vector3(const Vector2&);
    
    /// copy 1 coordinate from Vector1
    Vector3(const Vector1&);
    
    
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
        assert_true(i<3);
        return (&XX)[i];
    }
    
    /// modifiable access to individual coordinates
    template< typename T >
    real& operator[](T i)
    {
        assert_true(i<3);
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
    }
    
    /// replace coordinates by the ones provided
    void load(const float b[])
    {
        XX = b[0];
        YY = b[1];
        ZZ = b[2];
    }
    
    /// replace coordinates by the ones provided
    void load(const double b[])
    {
#if VECTOR3_USES_AVX
        vec = load3(b);
#else
        XX = b[0];
        YY = b[1];
        ZZ = b[2];
#endif
    }
    
    /// copy coordinates to given array
    void store(float b[]) const
    {
        b[0] = (float)XX;
        b[1] = (float)YY;
        b[2] = (float)ZZ;
    }
    
    /// copy coordinates to given array
    void store(double b[]) const
    {
#if VECTOR3_USES_AVX
        store3(b, vec);
#else
        b[0] = (double)XX;
        b[1] = (double)YY;
        b[2] = (double)ZZ;
#endif
    }
    
    /// add content to given address
    void add_to(real b[]) const
    {
#if VECTOR3_USES_AVX
        store3(b, add4(vec, load3(b)));
#else
        b[0] += XX;
        b[1] += YY;
        b[2] += ZZ;
#endif
    }
    
    /// add content scaled by `alpha` to given address
    void add_to(real alpha, real b[]) const
    {
#if VECTOR3_USES_AVX
        store3(b, add4(mul4(set4(alpha), vec), load3(b)));
#else
        b[0] += alpha * XX;
        b[1] += alpha * YY;
        b[2] += alpha * ZZ;
#endif
    }
    
    /// add content `n` times to array `b` of size `ldd*n`
    void add_to(real b[], int n, int ldd) const
    {
        for ( int i = 0; i < n; ++i )
        {
            b[ldd*i  ] += XX;
            b[ldd*i+1] += YY;
            b[ldd*i+2] += ZZ;
        }
    }
    
    /// subtract to given address
    void sub_to(real b[]) const
    {
#if VECTOR3_USES_AVX
        store3(b, sub4(load3(b), vec));
#else
        b[0] -= XX;
        b[1] -= YY;
        b[2] -= ZZ;
#endif
    }
    
    /// subtract content scaled by `alpha` to given address
    void sub_to(real alpha, real b[]) const
    {
#if VECTOR3_USES_AVX
        store3(b, sub4(load3(b), mul4(set4(alpha), vec)));
#else
        b[0] -= alpha * XX;
        b[1] -= alpha * YY;
        b[2] -= alpha * ZZ;
#endif
    }
    
    /// set coordinates to zero
    void reset()
    {
#if VECTOR3_USES_AVX
        vec = setzero4();
#else
        XX = 0;
        YY = 0;
        ZZ = 0;
#endif
    }
    
    /// change coordinates
    void set(const real x, const real y, const real z)
    {
        XX = x;
        YY = y;
        ZZ = z;
#if VECTOR3_USES_AVX
        TT = 0;
#endif
    }
    
    /// change signs of all coordinates
    void oppose()
    {
#if VECTOR3_USES_AVX
        vec = _mm256_xor_pd(vec, set4(-0.0));
#else
        XX = -XX;
        YY = -YY;
        ZZ = -ZZ;
#endif
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX + YY*YY + ZZ*ZZ;
    }
    
    /// the standard norm = sqrt(x^2+y^2+z^2)
    real norm() const
    {
        return sqrt(XX*XX + YY*YY + ZZ*ZZ);
    }
    
    /// the standard norm = sqrt(x^2+y^2+z^2)
    friend real norm(Vector3 const& V)
    {
        return V.norm();
    }
    
    /// the inversed magnitude = 1.0 / sqrt(x^2+y^2+z^2)
    real inv_norm() const
    {
        return 1.0 / sqrt(XX*XX + YY*YY + ZZ*ZZ);
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

    /// the 2D norm = y^2+z^2
    real normYZSqr() const
    {
        return YY*YY + ZZ*ZZ;
    }

    /// square of the distance between two points, equivalent to (a-b).normSqr()
    friend real distanceSqr(Vector3 const& a, Vector3 const& b)
    {
        real x = a.XX - b.XX;
        real y = a.YY - b.YY;
        real z = a.ZZ - b.ZZ;
        return x*x + y*y + z*z;
    }

    /// distance between two points, equivalent to (a-b).norm()
    friend real distance(Vector3 const& a, Vector3 const& b)
    {
        return sqrt(distanceSqr(a,b));
    }

    /// absolute values: (|x|, |y|, |z|)
    Vector3 abs() const
    {
        return Vector3(fabs(XX), fabs(YY), fabs(ZZ));
    }

    /// the infinite norm = max(|x|, |y|, |z|)
    real norm_inf() const
    {
        return std::max(std::max(fabs(XX), fabs(YY)), fabs(ZZ));
    }
    
    /// true if no component is NaN
    bool valid() const
    {
        return ( XX == XX ) && ( YY == YY ) && ( ZZ == ZZ );
    }
    
    /// true if some component is not zero
    bool is_not_zero() const
    {
        return ( XX || YY || ZZ );
    }

    /// scale to unit norm
    void normalize()
    {
#if VECTOR3_USES_AVX
        vec = normalize4(vec);
#else
        real s = norm();
        XX /= s;
        YY /= s;
        ZZ /= s;
#endif
    }

    /// scale to obtain norm `n`
    void normalize(const real n)
    {
#if VECTOR3_USES_AVX
        vec = normalize4(vec, n);
#else
        real s = n / norm();
        XX *= s;
        YY *= s;
        ZZ *= s;
#endif
    }

    /// returns vector parallel to argument of unit norm
    friend const Vector3 normalize(Vector3 const& V)
    {
#if VECTOR3_USES_AVX
        return Vector3(normalize4(V.vec));
#else
        const real s = V.norm();
        return Vector3(V.XX/s, V.YY/s, V.ZZ/s);
#endif
    }

    /// returns the colinear vector of norm `n` (default 1.0)
    const Vector3 normalized(const real n = 1.0) const
    {
#if VECTOR3_USES_AVX
        return Vector3(normalize4(vec, n));
#else
        real s = n / norm();
        return Vector3(s*XX, s*YY, s*ZZ);
#endif
    }

    //------------------------------------------------------------------
    
    /// returns a perpendicular vector, of comparable but unspecified norm
    const Vector3 orthogonal() const
    {
        if ( fabs(XX) < fabs(YY) )
        {
            if ( fabs(XX) < fabs(ZZ) )
                return Vector3(0.0, -ZZ,  YY); //XX is the smallest
            else
                return Vector3( YY, -XX, 0.0); //ZZ is the smallest
        }
        else
        {
            if ( fabs(YY) < fabs(ZZ) )
                return Vector3(-ZZ, 0.0,  XX); //YY is the smallest
            else
                return Vector3( YY, -XX, 0.0); //ZZ is the smallest
        }
    }
    
    /// returns a perpendicular vector, of norm `n`
    const Vector3 orthogonal(const real n) const
    {
        if ( fabs(XX) < fabs(YY) )
        {
            if ( fabs(XX) < fabs(ZZ) )
            {
                // XX is the smallest component
                real s = n / sqrt(YY*YY+ZZ*ZZ);
                return Vector3(0.0, -s*ZZ, s*YY);
            }
            else
            {
                // ZZ is the smallest component
                real s = n / sqrt(XX*XX+YY*YY);
                return Vector3(s*YY, -s*XX, 0.0);
            }
        }
        else
        {
            if ( fabs(YY) < fabs(ZZ) )
            {
                // YY is the smallest component
                real s = n / sqrt(XX*XX+ZZ*ZZ);
                return Vector3(-s*ZZ, 0.0, s*XX);
            }
            else
            {
                // ZZ is the smallest component
                real s = n / sqrt(XX*XX+YY*YY);
                return Vector3(s*YY, -s*XX, 0.0);
            }
        }
    }
    
    /// returns a vector perpendicular to *this, close to `d` and of norm = `n`
    /**
     This removes the component of `n` parallel to *this,
     and will fail if `d` is parallel to *this
     */
    const Vector3 orthogonal(Vector3 const& d, const real n) const
    {
        real s = dot(*this, d) / normSqr();
        return ( d - s * (*this) ).normalized(n);
    }
    
    /**
     Set vectors 'E' and 'F' to build an orthonormal basis (this, E, F),
     assuming that 'norm(*this) == 1'
     
     From `Building an Orthonormal Basis, Revisited`,
     Tom Duff et al. Journal of Computer Graphics Techniques Vol. 6 N.1, 2017
     */
    void orthonormal(Vector3& E, Vector3& F) const
    {
        assert_small(normSqr() - 1.0);
#if 0
        if ( fabs(normSqr() - 1.0) > 0.01 )
        {
            // this should not happen...
            E = orthogonal(1);
            F = cross(*this, E).normalized();
            std::clog << "rescued orthonormal(" << toString() << ")\n";
        }
#endif
        
        real s = std::copysign(real(1.0), ZZ);
#if ( 1 )
        /// optimized version by Marc B. Reynolds
        const real a = YY / ( ZZ + s );
        const real b = YY * a;
        const real c = XX * a;
        // below normSqr(ex) = normSqr(this) + a*a*(normSqr(this)-s*s)
        E.set(-ZZ - b, c, XX);
        F.set(s * c, s * b - 1.0, s * YY);
        //if you do not mind an inverted basis, use ey.set(c, b-s, YY);
#else
        /// original code from Duff et al.
        const real a = -1.0 / ( ZZ + s );
        const real b = XX * YY * a;
        // below normSqr(ex) = 1 + x*x*a*a*(normSqr(this)-s*s)
        E.set(1.0 + s * XX * XX * a, s * b, -s * XX);
        F.set(b, s + YY * YY * a, -YY);
#endif
    }
    
    
    /// return unit vector obtained by rotating `n` around `*this`, by angle defined by cosinus and sinus
    /**
     The result is a Vector orthogonal to *this
     */
    const Vector3 rotateOrtho(Vector3 const& vec, real c, real s)
    {
        //Set two orthogonal vector to 'd' to set an orientated basis
        Vector3 ex, ey;
        orthonormal(ex, ey);
        // compute coordinates of n in reference frame (x, y):
        real x = dot(vec, ex);
        real y = dot(vec, ey);
        // normalization factor:
        real n = sqrt( x * x + y * y );
        // rotate coefficients:
        real a = ( c * x - s * y ) / n;
        real b = ( s * x + c * y ) / n;
        return a * ex + b * ey;
    }
    
    /// convert from cartesian to spherical coordinates ( r, theta, phi )
    const Vector3 spherical() const
    {
        return Vector3(sqrt(XX*XX+YY*YY+ZZ*ZZ),
                       atan2(YY, XX),
                       atan2(sqrt(XX*XX+YY*YY), ZZ));
    }
    
    /// convert from spherical to cartesian coordinates ( x, y, z )
    const Vector3 cartesian() const
    {
        return Vector3(XX*cos(YY)*sin(ZZ),
                       XX*sin(YY)*sin(ZZ),
                       XX*cos(ZZ));
    }
    
    //------------------------------------------------------------------
    
    /// linear interpolation, returning a + x * b
    friend const Vector3 interpolate(const Vector3& a, real x, const Vector3& b)
    {
        return Vector3(a.XX+x*b.XX, a.YY+x*b.YY, a.ZZ+x*b.ZZ);
    }
    
    /// addition of two vectors
    friend const Vector3 operator +(Vector3 const& a, Vector3 const& b)
    {
#if VECTOR3_USES_AVX
        return Vector3(add4(a.vec, b.vec));
#else
        return Vector3(a.XX+b.XX, a.YY+b.YY, a.ZZ+b.ZZ);
#endif
    }
    
    /// subtraction of two vectors
    friend const Vector3 operator -(Vector3 const& a, Vector3 const& b)
    {
#if VECTOR3_USES_AVX
        return Vector3(sub4(a.vec, b.vec));
#else
        return Vector3(a.XX-b.XX, a.YY-b.YY, a.ZZ-b.ZZ);
#endif
    }
    
    /// unary + operator does nothing
    friend const Vector3 operator +(Vector3 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector3 operator -(Vector3 const& b)
    {
#if VECTOR3_USES_AVX
        return Vector3(_mm256_xor_pd(b.vec, set4(-0.0)));
#else
        return Vector3(-b.XX, -b.YY, -b.ZZ);
#endif
    }
    
    /// returns the element-by-element product
    const Vector3 e_mul(Vector3 const& b) const
    {
#if VECTOR3_USES_AVX
        return Vector3(mul4(vec, b.vec));
#else
        return Vector3(XX*b.XX, YY*b.YY, ZZ*b.ZZ);
#endif
    }

    /// returns the element-by-element division
    const Vector3 e_div(Vector3 const& b) const
    {
#if VECTOR3_USES_AVX
        return Vector3(div4(vec, b.vec));
#else
        return Vector3(XX/b.XX, YY/b.YY, ZZ/b.ZZ);
#endif
    }
    
    /// returns a vector with each element squared
    const Vector3 e_squared() const
    {
#if VECTOR3_USES_AVX
        return Vector3(mul4(vec, vec));
#else
        return Vector3(XX*XX, YY*YY, ZZ*ZZ);
#endif
    }
    
    /// returns sum of all coordinates
    real e_sum() const
    {
        return XX + YY + ZZ;
    }
    
    /// returns min(x, y, z)
    real e_min() const
    {
        return std::min(std::min(XX, YY), ZZ);
    }
    
    /// returns max(x, y, z)
    real e_max() const
    {
        return std::max(std::max(XX, YY), ZZ);
    }
    
    /// returns the element-by-element minimum
    const Vector3 e_min(Vector3 const& v) const
    {
        return Vector3(std::min(XX, v.XX), std::min(YY, v.YY), std::min(ZZ, v.ZZ));
    }
    
    /// returns the element-by-element maximum
    const Vector3 e_max(Vector3 const& v) const
    {
        return Vector3(std::max(XX, v.XX), std::max(YY, v.YY), std::max(ZZ, v.ZZ));
    }
    

    /// cross product of two vectors
    friend const Vector3 cross(Vector3 const& a, Vector3 const& b)
    {
#if VECTOR3_USES_AVX && defined __AVX2__
        return Vector3(cross4(a.vec, b.vec));
#else
        return Vector3(a.YY * b.ZZ - a.ZZ * b.YY,
                       a.ZZ * b.XX - a.XX * b.ZZ,
                       a.XX * b.YY - a.YY * b.XX);
#endif
    }

    /// scalar product of two vectors
    friend real dot(Vector3 const& a, Vector3 const& b)
    {
        return a.XX * b.XX + a.YY * b.YY + a.ZZ * b.ZZ;
    }
    
    /// multiplication by scalar s
    friend const Vector3 operator *(Vector3 const& a, const real s)
    {
#if VECTOR3_USES_AVX
        return Vector3(mul4(a.vec, set4(s)));
#else
        return Vector3(s*a.XX, s*a.YY, s*a.ZZ);
#endif
    }
    
    /// mutiplication by scalar s
    friend const Vector3 operator *(const real s, Vector3 const& a)
    {
#if VECTOR3_USES_AVX
        return Vector3(mul4(set4(s), a.vec));
#else
        return Vector3(s*a.XX, s*a.YY, s*a.ZZ);
#endif
    }
    
    /// division by scalar s
    friend const Vector3 operator /(Vector3 const& a, const real s)
    {
#if VECTOR3_USES_AVX
        return Vector3(div4(a.vec, set4(s)));
#else
        return Vector3(a.XX/s, a.YY/s, a.ZZ/s);
#endif
    }
    
    /// addition of another vector b
    void operator +=(Vector3 const& b)
    {
#if VECTOR3_USES_AVX
        vec = add4(vec, b.vec);
#else
        XX += b.XX;
        YY += b.YY;
        ZZ += b.ZZ;
#endif
    }
    
    /// subtraction of another vector b
    void operator -=(Vector3 const& b)
    {
#if VECTOR3_USES_AVX
        vec = sub4(vec, b.vec);
#else
        XX -= b.XX;
        YY -= b.YY;
        ZZ -= b.ZZ;
#endif
    }
    
    /// multiplication by a scalar
    void operator *=(const real s)
    {
#if VECTOR3_USES_AVX
        vec = mul4(vec, set4(s));
#else
        XX *= s;
        YY *= s;
        ZZ *= s;
#endif
    }
    
    /// division by a scalar
    void operator /=(const real s)
    {
#if VECTOR3_USES_AVX
        vec = div4(vec, set4(s));
#else
        XX /= s;
        YY /= s;
        ZZ /= s;
#endif
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector3 const& a, Vector3 const& b)
    {
        return ( a.XX==b.XX  &&  a.YY==b.YY  &&  a.ZZ==b.ZZ );
    }
    
    /// non-equality test
    friend bool operator !=(Vector3 const& a, Vector3 const& b)
    {
        return ( a.XX!=b.XX  ||  a.YY!=b.YY  ||  a.ZZ!=b.ZZ );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string toString() const
    {
        std::ostringstream oss;
        oss << XX << " " << YY << " " << ZZ;
        return oss.str();
    }
    
    /// conversion to ASCII string with given precision
    std::string toString(int w, int p) const
    {
        std::ostringstream oss;
        oss.precision(p);
        oss << std::setw(w) << XX << " ";
        oss << std::setw(w) << YY << " ";
        oss << std::setw(w) << ZZ;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f %+9.3f", XX, YY, ZZ);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f %+9.3f %+9.3f )", XX, YY, ZZ);
    }
    
    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f %+9.3f\n", XX, YY, ZZ);
    }
    
    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    
    /// a vector orthogonal to *this, with `norm == n`, chosen randomly and uniformly
    const Vector3 randOrthoU(real n) const;
    
    /// a vector orthogonal to *this, with `norm <= n`, assuming `norm(*this)==1`
    const Vector3 randOrthoB(real n) const;
    
    
    /// Vector with random independent coordinates in [0,+1]
    static const Vector3 randP();
    
    /// Vector with random independent coordinates in [0,+n]
    static const Vector3 randP(real n);
    
    /// Vector with random independent coordinates in [-1,+1]
    static const Vector3 randS();
    
    /// Vector with random independent coordinates in [-1/2,+1/2]
    static const Vector3 randH();
    
    /// Vector with random independent coordinates in [-n,+n]
    static const Vector3 randS(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector3 randU();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector3 randU(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector3 randB();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector3 randB(real n);
    
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector3 randG(real n);
    
};

//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream& operator >> (std::istream&, Vector3&);

/// stream output operator
std::ostream& operator << (std::ostream&, Vector3 const&);


#endif

