// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_segment.h"
#include "space.h"
#include "fiber.h"
#include "simul.h"
#include "modulo.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------
//---------------- DISTANCE FROM A POINT TO A SECTION OF FIBER -----------------
//------------------------------------------------------------------------------

/**
 W is projected on the line that supports this FiberSegment
 The function calculates:
 - abs <- the signed distance from pos1() to the projection of W
 - dis <- the distance between W and its projection
 .
 
 It is assumed here that len() returns the distance between the two points of the FiberSegment
 Attention: `dis` is not set if ( abs < 0 ) or ( abs > len() )
 */
real FiberSegment::projectPoint0(Vector const& w, real& dis) const
{
    assert_true( fib_ );
    
    Vector dx = dir();
    Vector aw = w - pos1();
    
    if ( modulo )
        modulo->fold(aw);
    
    // project with the scalar product:
    real abs = dot(aw, dx);
    
    // calculate distance to projection that fall inside the segment
    if ( 0 <= abs  &&  abs <= len() )
    {
#if ( DIM == 1 )
        dis = 0;
#else
        dis = aw.normSqr() - abs * abs;
#endif
    }
    return abs;
}


/**
 W is projected on the line that supports this FiberSegment
 The function calculates:
 - abs <- the signed distance from pos1() to the projection of W
 - dis <- the distance between W and its projection
 .
 
 It is assumed here that len() returns the distance between the two points of the FiberSegment
 Attention: `dis` may NOT be set if ( abs < 0 ) or ( abs > len() )
 */
real FiberSegment::projectPoint(Vector const& w, real& dis) const
{
    assert_true( fib_ );
    
    Vector dx = dir();
    Vector aw = w - pos1();
    
    if ( modulo )
        modulo->fold(aw);
    
    // project with the scalar product:
    real abs = dot(aw, dx);
    
    // test boundaries of filament:
    if ( abs < 0 )
    {
        if ( isFirst() )
            dis = aw.normSqr();
    }
    else if ( abs > len() )
    {
        if ( isLast() )
            dis = (aw-dx*len()).normSqr();
    }
    else
    {
#if ( DIM == 1 )
        dis = 0;
#else
        dis = aw.normSqr() - abs * abs;
#endif
    }
    return abs;
}


/**
 This may be faster than projectPoint(), but it does not work with periodic boundaries
 */
real FiberSegment::projectPointF(const real w[], real& dis) const
{
    assert_true( fib_ );
    assert_true( !modulo );
    
    const real * p = fib_->addrPoint(pti_);
    
    real dX = p[DIM  ] - p[0];
    real aX = w[0]     - p[0];
#if ( DIM > 1 )
    real dY = p[DIM+1] - p[1];
    real aY = w[1]     - p[1];
#endif
#if ( DIM > 2 )
    real dZ = p[DIM+2] - p[2];
    real aZ = w[2]     - p[2];
#endif
    
    const real ls = len();
    
    // project with the scalar product:
#if ( DIM == 1 )
    real abs = ( dX * aX ) / ls;
#elif ( DIM == 2 )
    real abs = ( dX * aX + dY * aY ) / ls;
#elif ( DIM == 3 )
    real abs = ( dX * aX + dY * aY + dZ * aZ ) / ls;
#endif
    
    // test boundaries of segment:
    if ( abs < 0 )
    {
        if ( isFirst() )
            dis = distanceSqr(w, pos1());
    }
    else if ( abs > ls )
    {
        if ( isLast() )
            dis = distanceSqr(w, pos2());
    }
    else
    {
#if   ( DIM == 1 )
        dis = 0;
#elif ( DIM == 2 )
        dis = aX * aX + aY * aY - abs * abs;
#elif ( DIM == 3 )
        dis = aX * aX + aY * aY + aZ * aZ - abs * abs;
#endif
        
#if ( 0 )
        // verify that the results are identical to projectPoint()
        real d = dis;
        real a = projectPoint(Vector(w), d);
        assert_small(a-abs);
        assert_small(d-dis);
#endif
    }
    return abs;
}


//------------------------------------------------------------------------------
//---------------- DISTANCE TO ANOTHER SECTION OF A FIBER ----------------------
//------------------------------------------------------------------------------

/**
 Evaluate the minimal distance between two segments.
 
 This finds the positions P1, P2 connecting the two supporting lines in the shortest
 way possible. P1 belongs to *this segment, and P2 to that segment. These points 
 are characterized by real abscissa (abs1, abs2) for which values {0, segment_length}
 match the edges of the segments.
 
 The function then evaluates if the points P1, P2 are inside or outside their 
 respective segments.
 
 @return `0` if `P1` or `P2` are outside their respective segment.
 If P1 is outside, then `abs2` is not evaluated.
 
 @return `1` if P1 and P2 are both inside their segments.
 In this case, `dis` is set to be the square of the distance (P1, P2)

 Hence `dis`, `abs1` and `abs2` are only set valid if the return value is `1`.
 If the segments are parallel, the mid-point of the overlapping section is returned.
 */

real FiberSegment::shortestDistance(FiberSegment const& seg, real& abs1, real& abs2) const
{
    Vector off = seg.pos1() - pos1();
    Vector d11 = dir();
    Vector d22 = seg.dir();
    
    if ( modulo )
        modulo->fold(off);
    
    real C = dot(d11, d22);  // cosinus of angle
    
    if ( 1-abs_real(C) > 128 * REAL_EPSILON )
    {
        real iS = 1 / (( 1 - C ) * ( 1 + C ));    // 1.0 / sinus^2
        // This deals with the general case of non-parallel lines
#if ( DIM > 2 )
        // direction N of the shortest path is orthogonal to both lines:
        // distance between lines = dot(off, N) / N.norm()
        real D = dot(off, cross(d11, d22));
#endif
        real d1off = dot(d11, off);
        real d2off = dot(d22, off);
        
        abs1 = ( d1off - C * d2off ) * iS;
        abs2 = ( C * d1off - d2off ) * iS;
#if 0
        // check that identified line path is orthogonal to both segments:
        Vector p1 = pos1() + abs1 * d11;
        Vector p2 = seg.pos1() + abs2 * d22;
        real n1 = dot(d11, p2-p1);
        real n2 = dot(d22, p2-p1);
        printf("shortestDistance %+9.6f %+9.6f :", n1, n2);
        // check different formula for distance betwen lines:
        real res0 = ( off + abs2 * d22 - abs1 * d11 ).normSqr();
        real res1 = ( off - d11 * abs1 ).normSqr() - abs2 * abs2;
        real res2 = ( off + d22 * abs2 ).normSqr() - abs1 * abs1;
        real res3 = ( D * D ) * iS;  // 1.0 / N.normSqr() == iS
        printf("%6.4f  %6.4f  %6.4f  %6.4f\n", sqrt(res0), sqrt(res1), sqrt(res2), sqrt(res3));
#endif
#if ( DIM > 2 )
        //printf("dis %03u:%02lu", seg.fiber()->identity(), seg.point());
        //printf(" %03u:%02lu  %6.4f\n", fiber()->identity(), point(), fabs(D)*sqrt(iS));
        return ( D * D ) * iS;
#else
        return 0;
#endif
    }
    else
    {
        const real len1 = len();
        const real len2 = seg.len();
        /*
         This deals with the case where the two segments are almost parallel:
         S ~= 0; C ~= +/- 1
         m1 = projection of seg.pos1() on this segment
         p1 = projection of seg.pos2()
         */
        const real d1off = dot(d11, off);
        
        real m1 = d1off;
        real p1 = d1off + C * len2;
        
        // clamp inside segment and use mid-point
        abs1 = 0.5 * ( std::min(len1, std::max(m1, p1)) + std::max((real)0, std::min(m1, p1)));
        
        real m2 = -dot(d22, off);
        real p2 = m2 + C * len1;
        
        // clamp inside segment and use mid-point
        abs2 = 0.5 * ( std::min(len2, std::max(m2, p2)) + std::max((real)0, std::min(m2, p2)));
        
        // return distance between lines
        return off.normSqr() - d1off * d1off;
    }
}


void FiberSegment::print(std::ostream& os) const
{
    if ( fiber() )
        //os << "(" << fiber()->reference() << " seg " << point() << ":" << point()+1 << ")";
        os << "(f" << fiber()->identity() << " " << point() << ":" << point()+1 << ")";
    else
        os << "(null)";
}


std::ostream& operator << (std::ostream& os, FiberSegment const& obj)
{
    obj.print(os);
    return os;
}
