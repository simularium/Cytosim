// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#ifndef MECABLE_H
#define MECABLE_H

#include "dim.h"
#include "object.h"
#include "matrix.h"
#include "buddy.h"
#include "sim.h"

class Meca;
class MatrixSparseSymmetric1;


/// Can be simulated using a Meca.
/**
 A Mecable is an Object made of points that can can be simulated in a Meca.
 
 Mecable defines an interface that is implemented in Bead, Fiber, Sphere and Solid.
 It derives from Movable, and thus can be translated or rotated.
 Mecable is also a Buddy, and can thus be part of an Organizer.
 
 This is one of the fundamental class of Cytosim.
 Merged PointSet on 20/01/2018
 */
class Mecable : public Object, public Buddy
{
protected:
    
    /// array of size DIM*pAllocated contains DIM*nPoints coordinates
    /**
     The coordinates are organized as follows:
     X1,       X2,       etc. for DIM==1
     X1 Y1,    X2 Y2,    etc. for DIM==2
     X1 Y1 Z1, X2 Y2 Z2, etc. for DIM==3
     */
    real *      pPos;

    /// Array containing force-coordinates which is allocated in Meca
    real const* pForce;
    
    /// Number of points in pForce[]
    size_t      pForceMax;
    
    /// Number of points in the Mecable
    unsigned    nPoints;

private:

    /// Currently allocated size of arrays pPos[]
    size_t      pAllocated;

    /// Matrix block used for preconditionning in Meca::solve()
    real *      pBlock;
    
    /// Pivot indices for LAPACK
    int *       pPivot;
    
    /// Allocated size of pBlock[]
    size_t      pBlockAlc;
    
    /// Current size of pBlock[]
    unsigned    pBlockSize;
    
    /// Flag that pBlock[] is used for preconditionning
    int         pBlockUse;
    
    /// Index that Object coordinates occupy in the matrices and vectors of Meca
    index_t     pIndex;

    /// Clear pointers
    void        clearMecable();
    
public:

    /// The constructor resets the pointers to memory
    Mecable()            { clearMecable(); }
    
    /// Destructor should release memory
    virtual ~Mecable()   { release(); }

    /// Copy constructor
    Mecable(Mecable const&);
    
    /// Copy assignment
    Mecable& operator=(Mecable const&);
    
    //--------------------------------------------------------------------------
    
    /// Set the number of points of the object
    void            setNbPoints(const unsigned n) { allocateMecable(n); nPoints = n; }
    
    /// Returns number of points
    unsigned        nbPoints()     const { return nPoints; }
    
    /// Index of the last point = nbPoints() - 1
    unsigned        lastPoint()    const { return nPoints - 1; }
    
    /// size currently allocated
    size_t          allocated()    const { return pAllocated; }

    //--------------------------------------------------------------------------
    
    /// Position of point 'p' of the object
    /** this is identical to posPoint(), it exists for historical reasons*/
    const Vector    posP(unsigned p)      const { return Vector(pPos+DIM*p); }
    
    /// Position of vertex number 'p' (indices starting at zero)
    Vector          posPoint(unsigned p)  const { assert_true(pPos && p<nPoints); return Vector(pPos+DIM*p); }
    
    /// Address of coordinate array
    const real*     data()                const { return pPos; }
    
    /// Address of point `p`
    const real*     addrPoint(unsigned p) const { return pPos + DIM*p; }
    
    /// Set position of point `i` to `x`
    void            setPoint(unsigned i, Vector const& x) { assert_true(i<nPoints); x.store(pPos+DIM*i); }
    
    /// Shift point at index `i` by `x`
    void            movePoint(unsigned i, Vector const& x) { assert_true(i<nPoints); x.add_to(pPos+DIM*i); }

    /// copy current vertex coordinates to given array
    void            putPoints(real*) const;
    
    /// replace current coordinates by values from the given array
    virtual void    getPoints(real const*);
    
    /// Add a point and expand the object, returning the array index that was used
    unsigned        addPoint(Vector const& w);
    
    /// Remove `nbp` points starting from index `inx`
    void            removePoints(unsigned inx, unsigned nbp);
    
    /// Remove all points
    void            clearPoints()  { nPoints = 0; }
    
    /// Shift `nbp` points starting from index `inx`
    void            shiftPoints(unsigned inx, unsigned nbp);
    
    /// Remove all points with indices [ 0, p-1 ], keep [ p, nbPoints() ]
    virtual void    truncateM(unsigned int p);
    
    /// Keep points [ 0, p ], remove other points
    virtual void    truncateP(unsigned int p);
    
    /// Set all coordinates to zero (nicer for debug/testing)
    void            resetPoints();
    
    /// Add random noise uniformly to all coordinate (used for testing purposes)
    void            addNoise(real amount);
    
    /// calculate first and second momentum of point coordinates
    void            calculateMomentum(Vector&, Vector&, bool sub);
    
    //--------------------------------------------------------------------------
    // Some functions are defined here to enable inlining, which may be faster
    
    
    /// Difference of two points = src[P+1] - src[P]
    static inline Vector diffPoints(const real* src, const unsigned P)
    {
        const real * p = src + DIM*P;
        const real * q = src + DIM*P + DIM;
#if ( DIM == 1 )
        return Vector(q[0]-p[0]);
#elif ( DIM == 2 )
        return Vector(q[0]-p[0], q[1]-p[1]);
#else
        return Vector(q[0]-p[0], q[1]-p[1], q[2]-p[2]);
#endif
    }
    
    /// Difference of two points = src[Q] - src[P]
    static inline Vector diffPoints(const real* src, const unsigned P, const unsigned Q)
    {
        const real * p = src + DIM*P;
        const real * q = src + DIM*Q;
#if ( DIM == 1 )
        return Vector(q[0]-p[0]);
#elif ( DIM == 2 )
        return Vector(q[0]-p[0], q[1]-p[1]);
#else
        return Vector(q[0]-p[0], q[1]-p[1], q[2]-p[2]);
#endif
    }
    
    /// Difference of two consecutive points: (P+1) - (P)
    Vector diffPoints(const unsigned P) const
    {
        assert_true( P+1 < nPoints );
        return diffPoints(pPos, P);
    }
    
    /// Difference of two points = Q - P = vector PQ
    Vector diffPoints(const unsigned P, const unsigned Q) const
    {
        assert_true( P < nPoints );
        assert_true( Q < nPoints );
        return diffPoints(pPos, P, Q);
    }
    
    /// Calculate intermediate position = P + a ( Q - P )
    Vector interpolatePoints(const unsigned P, const unsigned Q, const real a) const
    {
        assert_true( P < nPoints );
        assert_true( Q < nPoints );
#if ( DIM == 1 )
        return Vector(pPos[P]+a*(pPos[Q]-pPos[P]));
#elif ( DIM == 2 )
#if REAL_IS_DOUBLE && ( defined __SSE3__ )
        vec2 p = load2(pPos+2*P);
        vec2 q = load2(pPos+2*Q);
        return Vector(fmadd2(set2(a), sub2(q, p), p));
#else
        const real * p = pPos + DIM*P;
        const real * q = pPos + DIM*Q;
        return Vector(p[0]+a*(q[0]-p[0]), p[1]+a*(q[1]-p[1]));
#endif
        //return Vector(pPos+2*P) + a * ( Vector(pPos+2*Q) - Vector(pPos+2*P) );
#else
        const real * p = pPos + DIM*P;
        const real * q = pPos + DIM*Q;
        return Vector(p[0]+a*(q[0]-p[0]), p[1]+a*(q[1]-p[1]), p[2]+a*(q[2]-p[2]));
        //return Vector(pPos+DIM*P) + a * ( Vector(pPos+DIM*Q) - Vector(pPos+DIM*P) );
#endif
    }
    
    /// interpolate 'rank' points starting from 'ref'
    Vector interpolatePoints(size_t ref, real const coef[], size_t rank) const;
    
    //--------------------------------------------------------------------------
    
    /// Allocate memory to store given number of vertices
    virtual size_t  allocateMecable(size_t);
    
    /// free allocated memory
    void            release();
    
    /// prepare the Mecable to solve the mechanics in Meca::solve()
    /**
     This should prepare necessary variables to solve the system:
     - set rigidity coefficients, for addRigidity() to work properly
     - set drag mobility, for projectForces() to work,
     - set matrix/variables necessary for constrained dynamics
     .
     */
    virtual void    prepareMecable() = 0;

    /// Calculate the mobility coefficient
    virtual void    setDragCoefficient() = 0;
    
    /// The total drag coefficient of the object ( force = drag * speed )
    virtual real    dragCoefficient() const = 0;

    /// Add Brownian noise terms to a force vector (alpha = kT / time_step)
    virtual real    addBrownianForces(real const* rnd, real alpha, real* rhs) const { return INFINITY; }
    
    /// add the interactions (for example due to confinements)
    virtual void    setInteractions(Meca &) const {}
    
    //--------------------------------------------------------------------------
    
    /// Store the index where coordinates are located in Meca
    void            matIndex(index_t inx) { pIndex = inx; }
    
    /// Index in mB of the first point. the index in the vectors is DIM*matIndex()
    /** X1 is stored at DIM*matIndex(), Y1 at DIM*matIndex()+1, Z1 at DIM*matIndex()+2
     then X2, Y2, Z2...
     */
    index_t         matIndex()           const { return pIndex; }
    
    /// Allocates pBlock[] to hold a `N x N` full matrix, where N = DIM * nbPoints()
    void            allocateBlock();
    
    /// True if preconditionner block is 'in use'
    int             useBlock()           const { return pBlockUse; }
    
    /// Change preconditionning flag
    void            useBlock(int b)            { pBlockUse = b; }
    
    /// Returns current size of block allocated for preconditionning
    unsigned        blockSize()          const { return pBlockSize; }
    
    /// Returns address of memory allocated for preconditionning
    real *          block()              const { return pBlock; }
    
    /// Returns address of memory allocated for preconditionning (pivot)
    int *           pivot()              const { return pPivot; }

    //--------------------------------------------------------------------------
    
    /// returns the force on point `p` calculated at the previous Meca::solve()
    Vector          netForce(const unsigned p) const;
    
    /// replace current forces by the ones provided as argument
    virtual void    getForces(const real* ptr) { pForce = ptr; pForceMax = nPoints; }
    
    //--------------------------------------------------------------------------
    
    /// Add rigidity terms Y <- Y + Rigidity * X
    /**
        Rigidity can be any force acting internally to the objects
     for example, the bending rigidity of Fibers.
     This version is used to calculate the Matrix * Vector in Meca.
     */
    virtual void    addRigidity(const real* X, real* Y) const {}

    /// Fill upper diagonal of `mat` with matrix elements
    /**
     The function should add terms to the upper part of matrix `mat`.
     The array `mat` should be square of size `DIM*nbPoints()`.
     This version is used to build the preconditionner in Meca.
     It should be consistent with addRigidity(), adding exactly the same terms.
     */
    virtual void    addRigidityUpper(real * mat, unsigned ldd) const {}

    /// Calculate speeds for given forces: Y <- forces(X)
    /**
     The function calculates the 'legal' forces with constraints applied.
     It may or may not scale by the object's mobility coefficient, and one may
     derive the speeds in conjunction with `leftoverMobility()`:
     
         speed = leftoverMobility() * projectForces(forces)
     
     Note that:
     - The input `X` and output `Y` must be vectors of size `DIM * nPoints`
     - `X` and `Y` may point to the same address
     
     The default implementation ( Y <- 0 ) makes the object immobile
     */
    virtual void    projectForces(const real* X, real* Y) const { zero_real(DIM*nPoints, Y); }
    
    /// Return drag coefficient that was not applied by projectForces()
    virtual real    leftoverMobility() const { return 1.0; }

    //--------------------------------------------------------------------------

    /// set the terms obtained from the linearization of the Projection operator, from the given forces
    /** This is enabled by a keyword ADD_PROJECTION_DIFF in meca.cc */
    virtual void    makeProjectionDiff(const real* force) {}
    
    /// add terms from projection correction terms: Y <- Y + P' * X;
    /** This is enabled by a keyword ADD_PROJECTION_DIFF in meca.cc */
    virtual void    addProjectionDiff(const real* X, real* Y) const {}
    
    /// true if addProjectionDiff() does something
    virtual bool    hasProjectionDiff() const { return false; }
    
    //--------------------------------------------------------------------------
    //           Position-related functions derived from Movable
    //--------------------------------------------------------------------------
    
    /// Position of center of gravity
    virtual Vector  position() const;
    
    /// Mecable accepts translation and rotation
    virtual int     mobile() const { return 3; }
    
    /// Translate object (moves all the points by the same vector)
    virtual void    translate(Vector const&);
    
    /// Rotate object by given rotation
    virtual void    rotate(Rotation const&);
    
    /// bring object to centered image using periodic boundary conditions
    virtual void    foldPosition(Modulo const*);
    
    /// true if all points are inside Space
    bool            allInside(Space const*) const;
    
    //--------------------------------------------------------------------------
    
    /// Write to file
    void            write(Outputter&) const;
    
    /// Read from file
    void            read(Inputter&, Simul&, ObjectTag);
    
    /// Human friendly ouput
    void            print(std::ostream&, real const*) const;
    
    /// return index encoded in `str`
    static unsigned point_index(std::string const& str, unsigned max);

};


/// output operator:
std::ostream& operator << (std::ostream& os, Mecable const&);

#endif
