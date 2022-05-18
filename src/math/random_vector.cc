// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "random_vector.h"
#include "random.h"

/**
 Random vectors are generated using the global Random Generator `RNG`
 */


//------------------------------------------------------------------------------
#pragma mark - 1D Vectors

const Vector1 Vector1::randS()        { return Vector1(  RNG.sreal()); }
const Vector1 Vector1::randH()        { return Vector1(  RNG.shalf()); }
const Vector1 Vector1::randS(real n)  { return Vector1(n*RNG.sreal()); }
const Vector1 Vector1::randP()        { return Vector1(  RNG.preal()); }
const Vector1 Vector1::randP(real n)  { return Vector1(n*RNG.preal()); }
const Vector1 Vector1::randU()        { return Vector1(  RNG.sflip()); }
const Vector1 Vector1::randU(real n)  { return Vector1(n*RNG.sflip()); }
void  Vector1::addRand(real n)        { XX += n*RNG.sreal(); }

const Vector1 Vector1::randB()        { return Vector1(   RNG.sreal()); }
const Vector1 Vector1::randB(real n)  { return Vector1( n*RNG.sreal()); }
const Vector1 Vector1::randG(real n)  { return Vector1( n*RNG.gauss());  }

const Vector1 Vector1::randOrthoU(real len) const { return Vector1(0.0); }


//------------------------------------------------------------------------------
#pragma mark - 2D Vectors

const Vector2 Vector2::randS()        { return Vector2(  RNG.sreal(),   RNG.sreal()); }
const Vector2 Vector2::randH()        { return Vector2(  RNG.shalf(),   RNG.shalf()); }
const Vector2 Vector2::randS(real n)  { return Vector2(n*RNG.sreal(), n*RNG.sreal()); }
const Vector2 Vector2::randP()        { return Vector2(  RNG.preal(),   RNG.preal()); }
const Vector2 Vector2::randP(real n)  { return Vector2(n*RNG.preal(), n*RNG.preal()); }
const Vector2 Vector2::randG(real n)  { return Vector2(n*RNG.gauss(), n*RNG.gauss()); }
void  Vector2::addRand(real n)        { XX += n*RNG.sreal(); YY += n*RNG.sreal(); }


const Vector2 Vector2::randU()
{
    real d, x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = x*x + y*y;
    } while ( d > 1.0 );
    if ( d > 0.0 )
        return Vector2(x*x-y*y, 2*x*y) / d;
    else
        return Vector2(1, 0);
}

const Vector2 Vector2::randU(const real n)
{
    real d, x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = x*x + y*y;
    } while ( d > 1.0 );
    if ( d > 0.0 )
        return Vector2(x*x-y*y, 2*x*y) * (n/d);
    else
        return Vector2(n, 0);
}


const Vector2 Vector2::randB()
{
    real x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
    } while ( x*x + y*y > 1.0 );
    return Vector2(x, y);
}


const Vector2 Vector2::randB(const real n)
{
    real x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
    } while ( x*x + y*y > 1.0 );
    return Vector2(x, y) * n;
}


const Vector2 Vector2::randOrthoU(const real len) const
{
    real s = RNG.sflip(len) / sqrt( XX * XX + YY * YY );
    return Vector2(-YY, XX, 0) * s;
}


const Vector2 Vector2::randOrthoB(const real len) const
{
    // this assumes norm(*this) == 1
    real s = RNG.sreal() * len;
    return Vector2(-YY, XX, 0) * s;
}

//------------------------------------------------------------------------------
#pragma mark - 3D Vectors

const Vector3 Vector3::randS()        { return Vector3(RNG.sreal(),     RNG.sreal(),   RNG.sreal()); }
const Vector3 Vector3::randH()        { return Vector3(RNG.shalf(),     RNG.shalf(),   RNG.shalf()); }
const Vector3 Vector3::randS(real n)  { return Vector3(n*RNG.sreal(), n*RNG.sreal(), n*RNG.sreal()); }
const Vector3 Vector3::randP()        { return Vector3(RNG.preal(),     RNG.preal(),   RNG.preal()); }
const Vector3 Vector3::randP(real n)  { return Vector3(n*RNG.preal(), n*RNG.preal(), n*RNG.preal()); }
const Vector3 Vector3::randG(real n)  { return Vector3(n*RNG.gauss(), n*RNG.gauss(), n*RNG.gauss()); }
void  Vector3::addRand(real n)        { XX += n*RNG.sreal(); YY += n*RNG.sreal(); ZZ += n*RNG.sreal(); }


#if ( 0 )

/// hypercube rejection method
const Vector3 Vector3::randU()
{
    real x, y, z, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
        d = x*x + y*y + z*z;
    } while ( d > 1.0  ||  d < 0.01 );
    return Vector3(x, y, z) / sqrt(d);

}

/// hypercube rejection method
const Vector3 Vector3::randU(real n)
{
    real x, y, z, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
        d = x*x + y*y + z*z;
    } while ( d > 1.0  ||  d < 0.01 );
    return Vector3(x, y, z) * (n/sqrt(d));
}

#elif ( 1 )

/**
 Derived from Marsaglia (1972)
 Allen & Tildesley "Computer Simulation of Liquids" Clarendon Pres, Oxford 1987
 http://mathworld.wolfram.com/SpherePointPicking.html
 This uses only 2 random-numbers!
*/
const Vector3 Vector3::randU()
{
    real x, y, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = 1.0 - x*x - y*y;
    } while ( d <= 0 );
    real h = 2 * sqrt(d);
    return Vector3(x*h, y*h, 2.0*d-1.0);
}

const Vector3 Vector3::randU(const real n)
{
    real x, y, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = 1.0 - x*x - y*y;
    } while ( d <= 0 );
    real h = ( n + n ) * sqrt(d);
    return Vector3(x*h, y*h, n*(2.0*d-1.0));
}

#else


/**
 From Cook (1957)
 http://mathworld.wolfram.com/SpherePointPicking.html
 This uses 4 random-numbers, but avoids the square-root
 */
const Vector3 Vector3::randU()
{
    real x, y, z, t, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
        t = RNG.sreal();
        d = x*x + y*y + z*z + t*t;
    } while ( d > 1.0 );
    return Vector3(2*(y*t+x*z), 2*(z*t-x*y), x*x+t*t-y*y-z*z) / d;
}

const Vector3 Vector3::randU(const real n)
{
    real x, y, z, t, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
        t = RNG.sreal();
        d = x*x + y*y + z*z + t*t;
    } while ( d > 1.0 );
    return Vector3(2*(y*t+x*z), 2*(z*t-x*y), x*x+t*t-y*y-z*z) * (n/d);
}

#endif


const Vector3 Vector3::randB()
{
    real x, y, z;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
    } while ( x*x + y*y + z*z > 1.0 );
    return Vector3(x, y, z);
}


const Vector3 Vector3::randB(const real n)
{
    real x, y, z;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
    } while ( x*x + y*y + z*z > 1.0 );
    return Vector3(x, y, z) * n;
}


#if ( 1 )

const Vector3 Vector3::randOrthoU(const real len) const
{
    real n = normSqr();
    if ( n > REAL_EPSILON )
    {
        const Vector2 V = Vector2::randU();
        Vector3 x, y, z = *this / sqrt(n);
        z.orthonormal(x, y);
        return x * ( len * V.XX ) + y * ( len * V.YY );
    }
    return randU(len);
}

#else

/**
 This method is less efficient
 */
const Vector3 Vector3::randOrthoU(const real len) const
{
    const Vector2 V = Vector2::randU();
    Vector3 b = orthogonal(1);
    Vector3 c = normalize(cross(*this, b));
    return b * ( len * V.XX ) + c * ( len * V.YY );
}

#endif

const Vector3 Vector3::randOrthoB(const real len) const
{
    //this assumes norm(*this) == 1
    const Vector2 V = Vector2::randB();
    Vector3 x, y;
    orthonormal(x, y);
    return x * ( len * V.XX ) + y * ( len * V.YY );
}

//------------------------------------------------------------------------------
#pragma mark - 4D Vectors

const Vector4 Vector4::randS()        { return Vector4(RNG.sreal(),     RNG.sreal(),   RNG.sreal()); }
const Vector4 Vector4::randH()        { return Vector4(RNG.shalf(),     RNG.shalf(),   RNG.shalf()); }
const Vector4 Vector4::randS(real n)  { return Vector4(n*RNG.sreal(), n*RNG.sreal(), n*RNG.sreal()); }
const Vector4 Vector4::randP()        { return Vector4(RNG.preal(),     RNG.preal(),   RNG.preal()); }
const Vector4 Vector4::randP(real n)  { return Vector4(n*RNG.preal(), n*RNG.preal(), n*RNG.preal()); }
const Vector4 Vector4::randG(real n)  { return Vector4(n*RNG.gauss(), n*RNG.gauss(), n*RNG.gauss()); }
void  Vector4::addRand(real n)        { XX += n*RNG.sreal(); YY += n*RNG.sreal(); ZZ += n*RNG.sreal(); }


//------------------------------------------------------------------------------
#pragma mark - Functions to distribute multiple points


/**
 Generate a random distribution of points on the unit disc,
 with the distance between two points never below `sep`.
 @return number of points stored in 'pts[]'
 */
size_t tossPointsDisc(std::vector<Vector2>& pts, real sep, size_t limit_trials)
{
    const real ss = sep * sep;
    size_t ouf = 0;
    size_t n = 0;
    
    for ( Vector2& vec : pts )
    {
    toss:
        if ( ++ouf > limit_trials )
            break;
        
        const Vector2 V = Vector2::randB();
        
        for ( size_t i = 0; i < n; ++i )
            if ( distanceSqr(V, pts[i]) < ss )
                goto toss;
        
        vec = V;
        ouf = 0;
        ++n;
    }
    return n;
}


/**
 Generate a random distribution of points on a cap of solid-angle `cap` on the unit circle,
 with the distance between two points never below `sep`.
 @return number of points stored in 'pts[]'
 */
size_t tossPointsCap(std::vector<Vector3>& pts, real cap, real sep, size_t limit_trials)
{
    const real ss = sep * sep;
    size_t ouf = 0;
    size_t n = 0;
    
    for ( Vector3& vec : pts )
    {
    toss:
        if ( ++ouf > limit_trials )
            break;
        
        real a = M_PI * RNG.sreal();
        real u = 1.0 - cap * RNG.preal();
        real v = sqrt( 1.0 - u * u );
        Vector3 pos(u, v*cos(a), v*sin(a));
        
        for ( size_t i = 0; i < n; ++i )
            if ( distanceSqr(pos, pts[i]) < ss )
                goto toss;
        
        vec = pos;
        ouf = 0;
        ++n;
    }
    return n;
}


