// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CHAIN_H
#define CHAIN_H

#include "sim.h"
#include "vector.h"
#include "common.h"
#include "mecable.h"
#include "interpolation.h"

class Mecapoint;
class Glossary;

/// Mecable with linear geometry
/**
 This class describes a thin flexible filament that is longitudinally incompressible.
 The curvilinear length of the filament can be changed by growP(), growM(), cutP() and cutM().
 
 \par Number of points:
 
 The best number of points to describe a Chain is automatically calculated:
 It is the integer `number_of_points` that minimizes:
 
    fabs( length() / number_of_points - FiberProp::segmentation )
 
 where segmentation is a parameter of the fiber class.
 All the segments in a fiber all have the same length

    Chain::segmentation() = length() / ( number_of_points - 1 )

 Note that Chain::segmentation() is not always equal to FiberProp::segmentation.
 If the fibers have various length, their segmentation() will be different,
 even though they all share the same value of FiberProp::segmentation.

 See related functions: length(), nbPoints() and segmentation().
 
 \par Longitudinal incompressibility:
 
 Successive vertices are kept at a constant distance via constrained dynamics:

    norm( posP(N+1)-posP(N) ) == Chain::segmentation()
 
 \par Origin:
 
 An abscissa is a curvilinear distance taken along the Fiber,
 and the Chain provides an origin to make this independent of the vertices. 
 Thus even if the fiber lengthen from its ends, a position described by an abscissa will
 stay associated with the same local lattice site.
 
 Functions are provided in Chain to convert abscissa measured from different references,
 and to obtain positions of the fiber for a given abcissa.

 \par Derived classes:
 
 The class FiberSite keeps track of its position using an abscissa from the origin,
 and all Hand objects are built from this class.
 The class Fiber keeps track of the FiberSite that are attached to itself.
 
*/
class Chain : public Mecable
{
    /// the ideal number of points for ratio = length / segmentation
    static unsigned bestNumberOfPoints(real ratio);

    /// calculate length of given string of points
    static real contourLength(const real* pts, unsigned n_pts);
    
private:
        
    /// actual section length: distance between consecutive points
    real         fnCut;
    
    /// target segmentation length (equal to parameter 'fiber:segmentation')
    real         fnSegmentation;
    
    /// abscissa of the minus-end (equal to zero initially)
    real         fnAbscissaM;
    
    /// abscissa of the plus-end
    real         fnAbscissaP;

#if FIBER_HAS_NORMAL
    /// vector orthogonal to backbone at the origin, used for display only
    mutable Vector3 fnNormal;
#endif
protected:
    
    /// time at birth
    real         fnBirthTime;

    /// flag to update
    bool         needUpdate;
    
    /// callback to signal that update is needed, to be called after a change in length
    void         postUpdate() { needUpdate = true; }
    
    /// called if a Fiber tip has elongated or shortened
    virtual void updateFiber() {}

    /// restore the distance between two points
    static void  reshape_two(const real*, real*, real cut);

    /// oldest method to restore the distance between successive vertices
    static void  reshape_global(unsigned, const real*, real*, real cut);

    /// iterative method to restore the distance between successive vertices
    static int   reshape_calculate(unsigned, real cut, Vector const*, real*, size_t);

    /// apply the forces movements needed to the distance between two points
    static void  reshape_apply(unsigned, const real*, real*, const real*);

    /// iterative method to restore the distance between successive vertices
    static int   reshape_local(unsigned, const real*, real*, real cut, real* tmp, size_t);

    /// change segmentation
    void         setSegmentation(real c) { fnCut = std::max(c, REAL_EPSILON); }
    
public:
    
    /// Constructor
    Chain();
    
    /// Destructor
    ~Chain() {}
    
    /// Number of segments = nbPoints() - 1
    unsigned     nbSegments()  const { return nPoints - 1; }
    
    /// Index of the last segment = nbPoints() - 2
    unsigned     lastSegment() const { return nPoints - 2; }

    //---------------------

    /// set position of MINUS_END and direction (length and Nb of points are not modified)
    /** dir does not need to be normalized */
    void         setStraight(Vector const& pos, Vector const& dir);
    
    /// set position of 'ref', direction and length of Fiber
    void         setStraight(Vector const& pos, Vector const& dir, real len);

    /// translate Fiber to place 'ref' at the position where the CENTER is located
    void         placeEnd(FiberEnd ref);
    
    /// set shape with `np` points from the given array of size DIM*n_pts
    void         setShape(const real pts[], unsigned n_pts, unsigned np);

    /// set shape as a random walk with given parameters
    void         setEquilibrated(real length, real persistence_length);

    /// change the current segmentation to force `length()==len` (normally not needed)
    void         imposeLength(real len) { setSegmentation(len/nbSegments()); fnAbscissaP = fnAbscissaM + len; }
    
    /// return updated `normal` that is orthogonal to `d` (used for fake 3D display)
    Vector3      adjustedNormal(Vector3 const& d) const;
    
    //---------------------

    /// returns simulation time at which Fiber was created
    real         birthTime() const { return fnBirthTime; }

    /// set birth time
    void         birthTime(real t) { fnBirthTime = t; }
    
    /// returns current age of the fiber
    real         age() const;

    //---------------------
    
    /// return Mecapoint representation of given end
    Mecapoint     exactEnd(FiberEnd) const;

    /// interpolation representing MINUS_END
    Interpolation interpolateEndM() const { return Interpolation(this, 0, 1, 0); }

    /// interpolation representing PLUS_END
    Interpolation interpolateEndP() const { return Interpolation(this, nPoints-2, nPoints-1, 1); }

    /// interpolation representing a given end
    Interpolation interpolateEnd(FiberEnd) const;

    /// interpolation representing the mid-point between the two ends
    Interpolation interpolateCenter() const;

    /// interpolation of the site specified by its distance from the ORIGIN
    Interpolation interpolate(real ab) const { return interpolateM(ab-fnAbscissaM); }
    
    /// interpolation of the site specified from the MINUS_END
    Interpolation interpolateM(real ab) const;
    
    /// interpolation of a site specified by its distance from a FiberEnd
    Interpolation interpolate(real ab, FiberEnd ref) const;
    
    //---------------------
    
    /// length of the Fiber, estimated from the segmentation and number of segments
    real         length1()               const { return nPoints * fnCut - fnCut; }
    
    /// length of the Fiber, estimated from the difference of abscissa at the ends
    real         length()                const { return fnAbscissaP - fnAbscissaM; }
    
    /// the sum of the distance between vertices (used for debugging)
    real         trueLength()            const { return contourLength(pPos, nPoints); }
    
    /// true if ( abscissaM() <= a ) AND ( a <= abscissaP() )
    bool         betweenMP(const real a) const { return abscissaM() <= a + REAL_EPSILON && a <= abscissaP() + REAL_EPSILON; }
    
    /// true if ( a < abscissaM() ) OR ( abscissaP() < a )
    bool         outsideMP(const real a) const { return a < abscissaM() || abscissaP() < a; }

    /// true if abscissa is smaller than abscissa of PLUS_END
    bool         belowP(const real a)    const { return a <= abscissaP(); }
    
    /// true if abscissa is greater than abscissa of MINUS_END
    bool         aboveM(const real a)    const { return abscissaM() <= a; }
    
    /// calculate the domain in which ab is located (near a FiberEnd, or central)
    FiberEnd     whichEndDomain(real a, real lambda) const;
    
    /// return P where segment [ P, P+1 [ contains point at distance `a` from the MINUS_END
    /** returns 0 if ( a < 0 ) and last point index if ( a > lastSegment() ) */
    unsigned     clampedIndexM(const real a) const { return std::min((unsigned)(std::max(a,(real)0)/fnCut), lastSegment()); }

    //---------------------
    
    /// displace the ORIGIN of abscissa at distance `a` from the MINUS_END
    void         setOrigin(real a) { fnAbscissaM = -a; fnAbscissaP = fnCut * real(nbSegments()) - a; }

    /// signed distance from ORIGIN to MINUS_END (abscissa of MINUS_END)
    real         abscissaM() const { return fnAbscissaM; }
    
    /// abscissa of center, midway between MINUS_END and PLUS_END
    real         abscissaC() const { return 0.5 * (fnAbscissaM + fnAbscissaP); }

    /// signed distance from ORIGIN to PLUS_END (abscissa of PLUS_END)
    real         abscissaP() const { return fnAbscissaP; }

    /// signed distance from ORIGIN to vertex specified with index (or intermediate position)
    real         abscissaPoint(const real n) const { return fnAbscissaM + fnCut * n; }

    /// signed distance from the ORIGIN to the specified FiberEnd
    real         abscissaEnd(FiberEnd end) const;
    
    /// converts distance from the specified FiberEnd, to abscissa from the ORIGIN
    real         abscissaFrom(real dis, FiberEnd ref) const;
    
    /// return abscissa specified with `dis, ref, mod`
    real         someAbscissa(real dis, FiberEnd ref, int mod, real alpha) const;

    //---------------------

#if ( DIM == 1 )
    /// position at distance `ab` from the MINUS_END
    Vector       posM(real ab) const { return Vector(pPos[0]+std::copysign(ab, pPos[1]-pPos[0])); }
#else
    /// position at distance `ab` from the MINUS_END
    Vector       posM(real ab) const;
#endif
    /// position of a point specified by abscissa from the ORIGIN
    Vector       pos(real ab) const { return posM(ab-fnAbscissaM); }

    /// position of a point specified by abscissa `ab` from reference `ref`
    Vector       posFrom(real ab, FiberEnd ref) const { return pos(abscissaFrom(ab, ref)); }

    /// position of the point taken mid-way along the curve
    Vector       posMiddle() const { return posM(0.5*length()); }
    
    /// position of a FiberEnd
    Vector       posEnd(FiberEnd end) const;
    
    /// position of MINUS_END
    Vector       posEndM() const { return Vector(pPos); }

    /// position of PLUS_END
    Vector       posEndP() const { return Vector(pPos+DIM*(nPoints-1)); }
    
    /// external force acting on MINUS_END
    Vector       netForceEndM() const { return netForce(0); }
    
    /// external force acting on PLUS_END
    Vector       netForceEndP() const { return netForce(nPoints-1); }

    //---------------------
    
    /// vector between two consecutive points `p` and `p+1` (alias to diffPoints())
    Vector       diffP(unsigned p) const { return diffPoints(p); }

#if ( 1 )
    /// normalized tangent vector to the fiber within segment [p, p+1]
    /** We divide by fnCut, which should be the distance between points */
    Vector       dirSegment(unsigned p)  const { return diffPoints(p) / fnCut; }
#else
    /// normalized tangent vector to the fiber within segment [p, p+1]
    /** Normalizing the difference between points is slow due to sqrt() */
    Vector       dirSegment(unsigned p)  const { return normalize(diffPoints(p)); }
#endif
#if ( DIM == 1 )
    /// direction at distance `ab` from the MINUS_END
    Vector       dirM(real ab) const { return Vector(std::copysign(1.0, pPos[1]-pPos[0])); }
#else
    /// direction at distance `ab` from the MINUS_END
    Vector       dirM(real ab) const;
#endif
    /// normalized tangent vector to the fiber at given abscissa from the origin
    Vector       dir(real ab) const { return dirM(ab-fnAbscissaM); }

    /// normalized tangent vector to the fiber at given abscissa from given reference
    Vector       dir(real ab, FiberEnd ref) const { return dirM(abscissaFrom(ab, ref)); }
    
    /// normalized tangent vector to the fiber at given end
    Vector       dirEnd(FiberEnd end) const;
    
    /// normalized tangent vector to the fiber at MINUS_END
    Vector       dirEndM() const { return dirSegment(0); }
    
    /// normalized tangent vector to the fiber at PLUS_END
    Vector       dirEndP() const { return dirSegment(lastSegment()); }

    /// force on the MINUS_END projected on the direction of elongation
    real         projectedForceEndM() const;

    /// force on the PLUS_END projected on the direction of elongation
    real         projectedForceEndP() const;

    /// dot-product (force at the end of the Fiber).(direction of Fiber growth)
    real         projectedForceEnd(FiberEnd end) const;
    
    /// average direction
    Vector       avgDirection() const { return normalize(posEndP()-posEndM()); }
    
    //--------------------- Segmentation / discrete representation

    /// set desired segmentation
    void         targetSegmentation(real c) { assert_true(c>0); fnSegmentation = c; }
    
    /// return desired segmentation (this is not the length of the segments)
    real         targetSegmentation() const { return fnSegmentation; }
    
    /// the current segment length (distance between successive vertices)
    real         segmentation() const { return fnCut; }
    
    /// returns third power of segmentation()
    real         segmentationCube() const { return fnCut*fnCut*fnCut; }
    
    /// reinterpolate vertices and adjust fiber to have `ns` segments
    void         resegment(unsigned ns);
    
    /// automatically select the number of points if needed, and resegment the fiber
    void         adjustSegmentation();
    
    /// change the target segmentation, and adjust number of points if needed
    void         adjustSegmentation(real);

    /// change all vertices to given array of coordinates
    void         getPoints(real const*);
    
    /// restore the distance between successive vertices
    void         reshape() { getPoints(pPos); }

    /// invert polarity (swap PLUS end MINUS ends in place)
    void         flipChainPolarity();
    
    //--------------------- Info
    
    /// calculate the minimum and maximum segment length
    void         segmentationMinMax(real const* ptr, real&, real&) const;
    
    /// calculate the minimum and maximum segment length
    void         segmentationMinMax(real& n, real& x) const { segmentationMinMax(pPos, n, x); }

    /// calculate average and variance of the segment length
    void         segmentationVariance(real&, real&) const;

    /// curvature calculated at joint `p`, where `0 < p < nbPoints()-1`
    real         curvature(unsigned p) const;
    
    /// normalized energy associated with bending
    real         bendingEnergy0() const;

    /// the cosine of the maximum segment angle: indicate the errors due to curvature
    real         minCosinus() const;
    
    /// number of joints at which ( cosine(angle) < threshold )
    unsigned     nbKinks(real threshold = 0) const;
    
    /// calculate intersection between segment `s` and the plane defined by <em> n.pos + a = 0 </em>
    real         planarIntersect(unsigned s, Vector const& n, const real a) const;

    //--------------------- Growing/Shrinking
    
    /// merge two fibers by attaching given Chain at the PLUS_END of `this`
    void         join(Chain const*);

    /// increase/decrease length of Fiber by `delta`, at the MINUS_END
    void         growM(real delta);
    
    /// add a segment of length segmentation() at the MINUS_END
    void         addSegmentM();
    
    /// remove a portion of length `delta` including the MINUS_END
    void         cutM(real delta);
    
    /// increase/decrease length of Fiber by `delta`, at the PLUS_END
    void         growP(real delta);
    
    /// add a segment of length segmentation() at the PLUS_END
    void         addSegmentP();
    
    /// remove a portion of length `delta` including the PLUS_END
    void         cutP(real delta);
    
    /// grow at specified end (PLUS_END or MINUS_END)
    void         grow(FiberEnd end, real delta);
    
    /// shorten or lengthen Fiber without changing the position of `ref`
    void         adjustLength(real len, FiberEnd ref);

    /// Discard vertices in [ 0, P-1 ] and keep [ P, end ]
    virtual void truncateM(unsigned p);

    /// Keep vertices [ 0, P ] and discard the others
    virtual void truncateP(unsigned p);

    //---------------------
    
    /// print info such as length and segmentation
    void         briefdoc(std::ostream&, real, real, real, real) const;

    /// print info such as length and segmentation
    void         document(std::ostream&, real, real, real, real) const;
    
    /// print info such as length and segmentation
    void         document(real const* ptr, std::ostream&) const;
    
    /// return string with info such as length and segmentation
    std::string  document(real const* ptr) const;

    /// check length and segmentation and write if suspicious
    int          checkLength(real const* ptr, std::ostream&, real len) const;

    /// write to Outputter
    void         write(Outputter&) const;
    
    /// read from Inputter
    void         read(Inputter&, Simul&, ObjectTag);
    
};


#endif
