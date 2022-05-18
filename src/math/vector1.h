// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECTOR1_H
#define VECTOR1_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

/// Vector1 is a vector with 1 `real` component.
class Vector1
{
    
public:
    
    /// dimensionality is 1
    static unsigned dimensionality() { return 1; }
    
    /// coordinates are public
    real XX;
    
    
    /// by default, coordinates are not initialized
    Vector1() {}
    
    /// construct from 3 values
    Vector1(real x, real, real) : XX(x) {}
    
    /// construct from 1 value
    explicit Vector1(real x) : XX(x) {}

    /// construct from address
    Vector1(const real v[]) : XX(v[0]) {}    
    
    
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
        return ( d > 0 ) ? 0 : XX;
    }
    
    /// modifiable access to individual coordinates
    template< typename T >
    real& operator[](T i)
    {
        assert_true(i==0);
        return XX;
    }
#endif
    
    /// return x-component
    real x() const { return XX; }
    /// return y-component
    real y() const { return 0; }
    /// return z-component
    real z() const { return 0; }

    /// copy at most one coordinate from array of size d
    void load(const real v[], int d)
    {
        XX = ( d > 0 ) ? v[0] : 0;
    }
    
    /// replace coordinates by the ones provided
    void load(const float b[])
    {
        XX = b[0];
    }
    
    /// replace coordinates by the ones provided
    void load(const double b[])
    {
        XX = b[0];
    }
    
    /// copy coordinates to given array
    void store(float b[]) const
    {
        b[0] = (float)XX;
    }
    
    /// copy coordinates to given array
    void store(double b[]) const
    {
        b[0] = (double)XX;
    }
    
    /// add content to given address
    void add_to(real b[]) const
    {
        b[0] += XX;
    }
    
    /// add content scaled by `alpha` to given address
    void add_to(real alpha, real b[]) const
    {
        b[0] += alpha * XX;
    }
    
    /// add content `n` times to array `b` of size `ldd*n`
    void add_to(real b[], int n, int ldd) const
    {
        for ( int i = 0; i < n; ++i )
            b[ldd*i] += XX;
    }
    
    /// subtract content to given address
    void sub_to(real b[]) const
    {
        b[0] -= XX;
    }
    
    /// subtract content scaled by `alpha` to given address
    void sub_to(real alpha, real b[]) const
    {
        b[0] -= alpha * XX;
    }
    
    /// set coordinates to zero
    void reset()
    {
        XX = 0;
    }
    
    /// change coordinates
    void set(const real x)
    {
        XX = x;
    }
    
    /// change coordinates (last 2 arguments are discarded)
    void set(const real x, const real, const real)
    {
        XX = x;
    }
    
    /// change signs of all coordinates
    void oppose()
    {
        XX = -XX;
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX;
    }
    
    
    /// the standard norm = sqrt(x^2)
    real norm() const
    {
        return fabs(XX);
    }
    
    /// the standard norm = sqrt(x^2)
    friend real norm(Vector1 const& V)
    {
        return V.norm();
    }
    
    /// the inversed magnitude = 1.0 / abs(x)
    real inv_norm() const
    {
        return 1.0 / fabs(XX);
    }
    
    /// the 2D norm = sqrt(x^2+y^2)
    real normXY() const
    {
        return fabs(XX);
    }
    
    /// the 2D norm = 0 since Y = Z = 0
    real normYZ() const
    {
        return 0;
    }
    
    /// the 2D norm = 0 since Y = Z = 0
    real normYZSqr() const
    {
        return 0;
    }

    /// square of the distance between two points, equivalent to (a-b).normSqr()
    friend real distanceSqr(Vector1 const& a, Vector1 const& b)
    {
        real x = a.XX - b.XX;
        return x*x;
    }
    
    /// distance between two points, equivalent to (a-b).norm()
    friend real distance(Vector1 const& a, Vector1 const& b)
    {
        return fabs(a.XX-b.XX);
    }
 
    /// absolute values: (|x|)
    Vector1 abs() const
    {
        return Vector1(fabs(XX));
    }

    /// the infinite norm = |x|
    real norm_inf() const
    {
        return fabs(XX);
    }
    
    /// true if no component is NaN
    bool valid() const
    {
        return ( XX == XX );
    }
    
    /// true if component is not zero
    bool is_not_zero() const
    {
        return XX != 0.0;
    }

    /// scale to unit norm
    void normalize()
    {
        XX = std::copysign(1.0, XX);
    }

    /// scale to obtain norm `n`
    void normalize(const real n)
    {
        XX = std::copysign(n, XX);
    }
    
    /// returns the colinear vector of norm `n` (default 1.0)
    const Vector1 normalized(const real n = 1.0) const
    {
        return Vector1(std::copysign(n, XX));
    }
    
    /// returns vector parallel to argument of unit norm
    friend const Vector1 normalize(Vector1 const& V)
    {
        return Vector1(std::copysign(1.0, V.XX));
    }
    
    /// returns a perpendicular vector, of comparable but unspecified norm
    const Vector1 orthogonal() const
    {
        ABORT_NOW("Vector::orthogonal() is meaningless in 1D");
        return Vector1(0.0);
    }
    
    /// returns a perpendicular vector, of norm `n`
    const Vector1 orthogonal(const real) const
    {
        ABORT_NOW("Vector::orthogonal() is meaningless in 1D");
        return Vector1(0.0);
    }
    
    /// returns a vector perpendicular to *this, close to `d` and of norm = `n`
    const Vector1 orthogonal(Vector1 const&, const real n) const
    {
        ABORT_NOW("Vector::orthogonal() is meaningless in 1D");
        return Vector1(n);
    }
    
    /// convert from cartesian to spherical coordinates ( r, theta, phi )
    const Vector1 spherical() const { return Vector1(XX); }
    
    /// convert from spherical to cartesian coordinates ( x, y, z )
    const Vector1 cartesian() const { return Vector1(XX); }
    
    //------------------------------------------------------------------
    
    /// linear interpolation, returning a + x * b
    friend const Vector1 interpolate(const Vector1& a, real x, const Vector1& b)
    {
        return Vector1(a.XX+x*b.XX);
    }
    
    /// addition of two vectors
    friend const Vector1 operator +(Vector1 const& a, Vector1 const& b)
    {
        return Vector1(a.XX+b.XX);
    }
    
    /// subtraction of two vectors
    friend const Vector1 operator -(Vector1 const& a, Vector1 const& b)
    {
        return Vector1(a.XX-b.XX);
    }
    
    /// unary + operator does nothing
    friend const Vector1 operator +(Vector1 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector1 operator -(Vector1 const& b)
    {
        return Vector1(-b.XX);
    }
    
    /// returns the element-by-element product
    const Vector1 e_mul(const Vector1& b) const
    {
        return Vector1(XX*b.XX);
    }
    
    /// returns the element-by-element division
    const Vector1 e_div(const Vector1& b) const
    {
        return Vector1(XX/b.XX);
    }
    
    /// returns a vector with each element squared
    const Vector1 e_squared() const
    {
        return Vector1(XX*XX);
    }
    
    /// returns sum of all coordinates
    real e_sum() const
    {
        return XX;
    }
    
    /// returns X
    real e_min() const
    {
        return XX;
    }
    
    /// returns X
    real e_max() const
    {
        return XX;
    }
    
    /// returns the element-by-element minimum
    const Vector1 e_min(Vector1 const& v) const
    {
        return Vector1(std::min(XX, v.XX));
    }
    
    /// returns the element-by-element maximum
    const Vector1 e_max(Vector1 const& v) const
    {
        return Vector1(std::max(XX, v.XX));
    }
    
    
    /**
     In dimension 1, the vector product is not really useful,
     but it is defined for completeness with the other class Vector2, Vector3.
     */
    
    /// the cross product of two vectors is a Z-Vector
    friend real cross(Vector1 const&, Vector1 const&)
    {
        return 0;
    }
    
    /// cross product of a vector with a Z-Vector
    friend const Vector1 cross(Vector1 const&, const real)
    {
        return Vector1(0.0);
    }
    
    /// cross product of a Z-vector with a Vector
    friend const Vector1 cross(const real, Vector1 const&)
    {
        return Vector1(0.0);
    }
    
    /// scalar product of two vectors
    friend real dot(Vector1 const& a, Vector1 const& b)
    {
        return a.XX * b.XX;
    }
    
    /// multiplication by scalar s
    friend const Vector1 operator *(Vector1 const& a, const real s)
    {
        return Vector1(s*a.XX);
    }
    
    /// mutiplication by scalar s
    friend const Vector1 operator *(const real s, Vector1 const& a)
    {
        return Vector1(s*a.XX);
    }
    
    /// division by scalar s
    friend const Vector1 operator /(Vector1 const& a, const real s)
    {
        return Vector1(a.XX/s);
    }
    
    /// addition of another vector b
    void operator +=(Vector1 const& b)
    {
        XX += b.XX;
    }
    
    /// subtraction of another vector b
    void operator -=(Vector1 const& b)
    {
        XX -= b.XX;
    }
    
    /// multiplication by a scalar
    void operator *=(const real s)
    {
        XX *= s;
    }
    
    /// division by a scalar
    void operator /=(const real s)
    {
        XX /= s;
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector1 const& a, Vector1 const& b)
    {
        return ( a.XX==b.XX );
    }
    
    /// non-equality test
    friend bool operator !=(Vector1 const& a, Vector1 const& b)
    {
        return ( a.XX!=b.XX );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string toString() const
    {
        std::ostringstream oss;
        oss << XX;
        return oss.str();
    }
    
    /// conversion to a string with given precision
    std::string toString(int w, int p) const
    {
        std::ostringstream oss;
        oss.precision(p);
        oss << std::setw(w) << XX;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f", XX);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f )", XX);
    }
    
    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f\n", XX);
    }
    
    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    
    /// a vector orthogonal to *this, with `norm == n`, chosen randomly and uniformly
    const Vector1 randOrthoU(real n) const;

    /// Vector with random independent coordinates in [0,+1]
    static const Vector1 randP();
    
    /// Vector with random independent coordinates in [0,+n]
    static const Vector1 randP(real n);
    
    /// Vector with random independent coordinates in [-1,+1]
    static const Vector1 randS();
    
    /// Vector with random independent coordinates in [-1/2,+1/2]
    static const Vector1 randH();
    
    /// Vector with random independent coordinates in [-n,+n]
    static const Vector1 randS(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector1 randU();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector1 randU(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector1 randB();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector1 randB(real n);
    
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector1 randG(real n);
    
};


//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream& operator >> (std::istream&, Vector1&);

/// stream output operator
std::ostream& operator << (std::ostream&, Vector1 const&);


#endif

