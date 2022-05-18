// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_SEGMENT_H
#define FIBER_SEGMENT_H


#include "real.h"
#include "vector.h"
#include "fiber.h"


/// Indicates the segment of a Fiber located between two consecutive vertices
/** 
 FiberSegment is used to refer to the section of a Fiber located between two vertices.
 The i-th FiberSegment covers the section [i, i+1[

 This is used to calculate the distance to this segment,
 the intersection of the segment with a plane.
*/
class FiberSegment
{
private:
    
    /// Fiber to which the segment belongs to
    Fiber const*   fib_;
    
    /// index of segment's first point
    unsigned int   pti_;
    
public:
    
    /// construct without initialization
    FiberSegment() {}
    
    /// constructor
    FiberSegment(Fiber const* f, unsigned p) : fib_(f), pti_(p) {}
    
    /// setter
    void     set(Fiber const* f, unsigned p) { fib_ = f; pti_ = p; }

    /// the Fiber
    Fiber const* fiber()       const { return fib_; }
    
    /// index of segment
    unsigned int point()       const { return pti_; }
    
    /// abscissa at start of segment (i.e. corresponding to point())
    real         abscissa1()   const { return fib_->abscissaPoint(pti_); }
    
    /// abscissa of second point
    real         abscissa2()   const { return fib_->abscissaPoint(pti_+1); }

    /// the length of the segment
    real         len()         const { return fib_->segmentation(); }
        
    /// true if abscissa 'a', counted from point 0 is within the segment
    bool         within(real a) const { return ( 0 <= a ) & ( a <= fib_->segmentation() ); }
    
    /// position of first point
    Vector       pos1()        const { return fib_->posP(pti_); }
    
    /// position of second point
    Vector       pos2()        const { return fib_->posP(pti_+1); }

    /// interpolated position, where c is in [0, 1]
    Vector       pos(real c)   const { return fib_->interpolatePoints(pti_, pti_+1, c); }
    
    /// that is [ pos2() + pos1() ] / 2
    Vector       center()      const { return fib_->interpolatePoints(pti_, pti_+1, 0.5); }

    /// that is pos2() - pos1()
    Vector       diff()        const { return fib_->diffPoints(pti_); }

    /// that is normalize( pos2() - pos1() )
    Vector       dir()         const { return fib_->dirSegment(pti_); }
    
    /// Mecapoint corresponding to first point
    Mecapoint    exact1()      const { return Mecapoint(fib_, pti_); }
    
    /// Mecapoint corresponding to second point
    Mecapoint    exact2()      const { return Mecapoint(fib_, pti_+1); }
    
    /// true if the segment is the first of the Fiber
    bool         isFirst()     const { return ( pti_ == 0 ); }

    /// true if the segment is not the first of the Fiber
    bool         notFirst()    const { return ( pti_ > 0 ); }
    
    /// true if the segment is the last of the fiber
    bool         isLast()      const { return ( pti_+2 == fib_->nbPoints() ); }
    
    /// true if the segment is not the last of the fiber
    bool         notLast()     const { return ( pti_+2 < fib_->nbPoints() ); }

    
    /// calculate the projection of `w` on the line supporting the segment
    real         projectPoint0(Vector const& w, real& dist) const;

    /// calculate the projection of `w` on the line supporting the segment
    real         projectPoint(Vector const& w, real& dist) const;

    /// faster projectionPoint, but incompatible with periodic boundary conditions
    real         projectPointF(const real[], real& dist) const;

    /// calculates the closest distance between two segments
    real         shortestDistance(FiberSegment const&, real& a, real& b) const;

    /// Human friendly ouput
    void         print(std::ostream&) const;

    /// true if segments are adjacent on the same fiber
    friend bool  adjacent(FiberSegment const& a, FiberSegment const& b)
    {
        return ( a.fib_==b.fib_  &&  a.pti_ < 2 + b.pti_  &&  b.pti_ < 2 + a.pti_ );
    }
};

/// print for debug purpose
std::ostream& operator << (std::ostream&, FiberSegment const&);


#endif

