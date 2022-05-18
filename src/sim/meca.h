// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MECA_H
#define MECA_H

#include "dim.h"
#include "array.h"
#include "vector.h"
#include "matrix.h"
//#include "matsparse.h"
#include "matsparsesym1.h"
#include "matsparsesymblk.h"
#include "allocator.h"


class Mecable;
class Mecapoint;
class Interpolation;
class SimulProp;
class Modulo;
class Simul;

/// MatrixBlock is an alias to a matrix class of size DIM * DIM
class Matrix11;
class Matrix22;
class Matrix33;

#if ( DIM == 1 )
typedef Matrix11 MatrixBlock;
#elif ( DIM == 2 )
typedef Matrix22 MatrixBlock;
#else
typedef Matrix33 MatrixBlock;
#endif

/**
 Option to allow the user to see Links made by Meca in 'play'.
 This option affects display speed since it requires two calls to setInteractions()
 This option is normally OFF. Only supported with Xcode compilation.
 */
#define DRAW_MECA_LINKS 0


/// A class to calculate the motion of objects in Cytosim
/**
Meca solves the motion of objects defined by points (i.e. Mecable),
using an equation that includes terms for each interaction between Objects,
and also forces that are internal to an object, for instance bending elasticity
for Fibers, and external forces such as confinements.
The equation is formulated using linear-algebra:
 
    d vPTS/dt = mobility * mP * ( Force + mdiffP * vPTS )
 
 with
 
    Force = vBAS + ( mB + mC + mR ) * vPTS
 
 The equation is solved for a small increment of time `time_step`, in the presence
 of Brownian motion, and at low Reynolds number, ie. a regime in which inertial
 forces that are proportional to mass are negligible.
 
 The equation contains `DIM * nbPoints()` degrees of freedom, where `nbPoints()`
 is the total number of points in the system. It contains vectors and matrices.
 The different  terms of the equation are:
 
 - Vector vPTS containing all the Mecable coordinates (x, y, z):
   Fiber, Sphere, Solid and other Mecable. 
 
 - Vector vBAS is of same size as vPTS, and includes the constant part obtained by
   linearization of the forces. It includes for instance the positions of Single,
   calibrated random forces simulating Brownian motion, and also offsets for periodic
   boundary conditions.
 
 - Matrix mB is the isotropic part obtained after linearization of the forces.
   It operates similarly and independently on the different dimension X, Y and Z.
   mB is square of size nbPoints(), symmetric and sparse.
 
 - Matrix mC is the non-isotropic part obtained after linearization of the forces.
   mC is square of size DIM*nbPoints(), symmetric and sparse.
 .
 
 Typically, mB and mC will inherit the stiffness coefficients of the interactions, 
 while vBAS will get forces (stiffness * position). They are set by the member functions
 addLink(), addLongLink(), addSideLink(), addSlidingLink(), etc.

 - mR add the bending elasticity for Mecafil, or other internal forces.
   mR is symmetric of size DIM*nbPoints(), diagonal by blocks, each block corresponding to a Fiber.
 
 - mP applies the projection due to constrained dynamics.
   For Mecafil, this maintains the distance between neighboring points (longitudinal incompressibility). 
   mP is symmetric of size DIM*nbPoints(), diagonal by blocks, each block corresponding to a Fiber.
   mP is not actually calculated as a matrix:
   its application on each block is done by Mecable::projectForces()
 
 - mdiffP is a term coming from the derivative of the projection P.
   It can provide better numerical stability in some situations where the filament are stretched.
   You can however define ADD_PROJECTION_DIFF = 0 in meca.cc to remove mdiffP.
 .
 
 
 Note: All Links have no effect if the given Mecapoint or Interpolation have a 
 point in common, because the matrix elements would not be calcuated correctly 
 in that case. Generally, such interactions are anyway not desirable. It would 
 correspond for example to a link between two point of the same segment, without 
 effect since the segment is straight, or between two successive segments on the
 same Fiber, which at best would fold it in a non-physical way.

 */

class Meca
{
public:

    /// enables graphical display of all interactions
    bool            drawLinks;

private:
    
    /// flag to indicate that result is available
    int             ready_;
    
    /// local copy of the SimulProp::time_step
    real            time_step;
    
    /// list of Mecable containing points to simulate
    Array<Mecable*> objs;
    
    /// total number of points in the system
    index_t         nbPts;
    
    /// size of the currently allocated memory
    size_t          allocated_;

    //--------------------------------------------------------------------------
    // Vectors of size DIM * nbPoints()
    
    real*  vPTS;         ///< coordinates of Mecable points
    real*  vSOL;         ///< coordinates after the dynamics has been solved
    real*  vBAS;         ///< part of the force that is independent of positions
    real*  vRND;         ///< vector of Gaussian random numbers
    real*  vRHS;         ///< right hand side of the dynamic system
    real*  vFOR;         ///< the calculated forces, with Brownian components
    real*  vTMP;         ///< intermediate of calculus
    real*  vMEM;         ///< another temporary array
    
    //--------------------------------------------------------------------------

    /// working memory allocator for BCGS and GMRES used in solve()
    LinearSolvers::Allocator allocator;
    
    /// secondary memory allocator for GMRES
    LinearSolvers::Allocator temporary;
    
    /// Matrices used for GMRES
    LinearSolvers::Matrix mH, mV;
    
    /// true if the matrix mC is non-zero
    bool   useMatrixC;

public:

    /// isotropic symmetric part of the dynamic
    /** 
     This is a symmetric square matrix of size `nbPoints()`
     It contains terms which have identical coefficients on the X, Y, Z subspaces
    */
    MatrixSparseSymmetric1  mB;

    
    /// non-isotropic symmetric part of the dynamic
    /** 
     This is a symmetric square matrix of size `DIM*nbPoints()`
     It contains terms which are different in the X, Y, Z subspaces,
     arising from interactions which link coordinates from different subspaces.
    */
    MatrixSparseSymmetricBlock  mC;
    
    /// base for force
    real*   base()             { return vBAS; }

    /// base for force
    real&   base(index_t i) { return vBAS[i]; }
    
    /// position of point stored at vPTS[i]
    Vector  position1(const index_t inx) const
    {
        return Vector(vPTS+inx);
    }
    
    /// position interpolated from two points in vPTS[]
    Vector  position2(const index_t inx[2], const real coef[2]) const
    {
        Vector P0(vPTS+inx[0]);
        Vector P1(vPTS+inx[1]);
        return coef[0] * P0 + coef[1] * P1;
    }

    /// position interpolated from three points in vPTS[]
    Vector  position3(const index_t inx[3], const real coef[3]) const
    {
        Vector P0(vPTS+inx[0]);
        Vector P1(vPTS+inx[1]);
        Vector P2(vPTS+inx[2]);
        return ( coef[0] * P0 + coef[1] * P1 ) + coef[2] * P2;
   }

    /// position interpolated from four points in vPTS[]
    Vector  position4(const index_t inx[4], const real coef[4]) const
    {
        Vector P0(vPTS+inx[0]);
        Vector P1(vPTS+inx[1]);
        Vector P2(vPTS+inx[2]);
        Vector P3(vPTS+inx[3]);
        return ( coef[0] * P0 + coef[1] * P1 ) + ( coef[2] * P2 + coef[3] * P3 );
    }
    
    /// position interpolated from five points in vPTS[]
    Vector  position5(const index_t inx[5], const real coef[5]) const
    {
        Vector P0(vPTS+inx[0]);
        Vector P1(vPTS+inx[1]);
        Vector P2(vPTS+inx[2]);
        Vector P3(vPTS+inx[3]);
        Vector P4(vPTS+inx[4]);
        return ( coef[0] * P0 + coef[1] * P1 ) + ( coef[2] * P2 + coef[3] * P3 ) + coef[4] * P4;
    }
    
    /// position interpolated from six points in vPTS[]
    Vector  position6(const index_t inx[6], const real coef[6]) const
    {
        Vector P0(vPTS+inx[0]);
        Vector P1(vPTS+inx[1]);
        Vector P2(vPTS+inx[2]);
        Vector P3(vPTS+inx[3]);
        Vector P4(vPTS+inx[4]);
        Vector P5(vPTS+inx[5]);
        return ( coef[0] * P0 + coef[1] * P1 ) + ( coef[2] * P2 + coef[3] * P3 ) + ( coef[4] * P4 + coef[5] * P5 );
    }

    /// add block 'T' to mC at position (i, j)
    void add_block(index_t i, index_t j, MatrixBlock const& T);
    
    /// subtract block 'T' to mC at position (i, j)
    void sub_block(index_t i, index_t j, MatrixBlock const& T);
 
    /// add block 'alpha*T' to mC at position (i, j)
    void add_block(index_t i, index_t j, real alpha, MatrixBlock const& T);

private:
    
    /// allocate memory
    void allocate(size_t);
    
    /// release memory
    void release();
    
    /// prepare matrices for 'solve'
    void prepareMatrices();

    /// calculate the linear part of forces:  Y <- B + ( mB + mC ) * X
    void calculateForces(const real* X, const real* B, real* Y) const;
    
    /// add forces due to bending elasticity
    void addAllRigidity(const real* X, real* Y) const;

    /// compute the matrix diagonal block corresponding to a Mecable
    void getBlock(real* res, const Mecable*) const;
    
    /// DEBUG: extract the matrix diagonal block corresponding to a Mecable using 'multiply()'
    void extractBlock(real* res, const Mecable*) const;
    
    /// DEBUG: compare `blk` with block extracted using extractBlockSlow()
    void verifyBlock(const Mecable*, const real*);
    
    /// DEBUG: test if `blk` is inverse of block extracted using extractBlockSlow()
    void checkBlock(const Mecable*, const real*);
    
    /// compute the preconditionner block corresponding to given Mecable
    void computePreconditionner(Mecable*);
    
    /// compute all blocks of the preconditionner (method=1)
    void computePreconditionner();

public:
    
    /// constructor
    Meca();
    
    /// destructor
    ~Meca() { release(); }
    
    /// Add a Mecable to the list of objects to be simulated
    void     addMecable(Mecable* p) { objs.push_back(p); }
    
    /// Number of Mecable
    size_t   nbMecables() const { return objs.size(); }
    
    /// Number of points in the Mecable that has the most number of points
    unsigned largestMecable() const;

    /// true if system does not contain any object
    bool     empty() const { return nbPts == 0; }
    
    /// number of points in the system
    size_t nb_points() const { return nbPts; }
    
    /// Implementation of LinearOperator::size()
    size_t dimension() const { return DIM * nbPts; }
    
    /// calculate Y <- M*X, where M is the matrix associated with the system
    void multiply(const real* X, real* Y) const;

    /// apply preconditionner: Y <- P*X (note that X maybe equal to Y)
    void precondition(const real* X, real* Y) const;
    
    //--------------------------- FORCE ELEMENTS -------------------------------

    /// Add a constant force on Mecapoint
    void addForce(Mecapoint const&, Vector const& force);
    
    /// Add a constant force on Interpolated point
    void addForce(Interpolation const&, Vector const& force);
    
    /// Add a torque to the segment indicated by Interpolation
    void addTorque(Interpolation const&, Torque const& torque);
    
    /// Add a torque to constrain the segment to be oriented in direction `dir`
    void addTorqueClamp(Interpolation const&, Vector const& dir, real weight);
    
    /// Add an explicit torque to constrain two segments to be parallel
    void addTorque(Interpolation const&, Interpolation const&, real weight);

    /// Add an explicit torque to constrain two segments to an angle defined by (sinus, cosinus)
    void addTorque(Interpolation const&, Interpolation const&, real cosinus, real sinus, real weight);
    
    /// this only works in 2D
    void addTorquePoliti(Interpolation const&, Interpolation const&, real cosinus, real sinus, real weight);

    /// Force of stiffness `weight` from fixed position `g`
    void addPointClamp(Mecapoint const&, Vector, real weight);
    
    /// Force of stiffness `weight` from fixed position `g`
    void addPointClamp(Interpolation const&, Vector, real weight);
    
    /// Force of stiffness `weight` and sphere of radius `rad` and center `cen`
    void addSphereClamp(Vector const& pos, Mecapoint const&, Vector const& cen, real rad, real weight);
    
    /// Force of stiffness `weight` with cylinder of axis Z and radius `len`
    void addCylinderClampZ(Mecapoint const&, real len, real weight);
    
    /// Force of stiffness `weight` with cylinder of axis X and radius `len`
    void addCylinderClampX(Mecapoint const&, real len, real weight);
    
#if ( DIM == 2 )
    /// Force of stiffness `weight` and resting length `len`, on the side of first segment
    void addSidePointClamp2D(Interpolation const&, Vector const&, real arm, real weight);
#elif ( DIM >= 3 )
    /// Force of stiffness `weight` and resting length `len`, on the side of first segment
    void addSidePointClamp3D(Interpolation const&, Vector const&, Vector const& arm, real weight);
#endif
    /// Force of stiffness `weight` with fixed position `g`, on the side of the segment
    void addSidePointClamp(Interpolation const&, Vector const&, real len, real weight);
    
    /// Force of stiffness `weight` with a line defined by `g` and its tangent `dir`
    void addLineClamp(Mecapoint const&, Vector const& g, Vector const& dir, real weight);
    
    /// Force of stiffness `weight` with a line defined by `g` and its tangent `dir`
    void addLineClamp(Interpolation const&, Vector const& g, Vector const& dir, real weight);
    
    /// Force of stiffness `weight` with a plane defined by `g` and its normal `dir`
    void addPlaneClamp(Mecapoint const&, Vector const& g, Vector const& dir, real weight);
    
    /// Force of stiffness `weight` with a plane defined by `g` and its normal `dir`
    void addPlaneClamp(Interpolation const&, Vector const& g, Vector const& dir, real weight);

    //------------ ZERO-RESTING LENGTH ELEMENTS LINKING POINTS -----------------
    
    /// linear force of stiffness `weight` between two vertices
    void addLink(Mecapoint const&, Mecapoint const&, real weight);
    
    /// linear force of stiffness `weight` (use the other one)
    void addLink(Interpolation const&, Mecapoint const&, real weight);
    
    /// linear force of stiffness `weight` between a vertex and a interpolated point
    void addLink(Mecapoint const&, Interpolation const&, real weight);
    
    /// linear force of stiffness `weight` between two interpolated points
    void addLink(Interpolation const&, Interpolation const&, real weight);
    
    
    /// linear force of stiffness `weight` between vertex and interpolated point
    void addLink2(Mecapoint const&, const unsigned[], const real[], real weight);
    
    /// linear force of stiffness `weight` between vertex and interpolated point
    void addLink3(Mecapoint const&, const unsigned[], const real[], real weight);

    /// linear force of stiffness `weight` between vertex and interpolated point
    void addLink4(Mecapoint const&, const unsigned[], const real[], real weight);
    
    
    /// linear force of stiffness `weight` between Interpolation and vertex
    void addLink1(Interpolation const&, index_t, real weight);

    /// linear force of stiffness `weight` between Interpolation and interpolated point
    void addLink2(Interpolation const&, const index_t[], const real[], real weight);
    
    /// linear force of stiffness `weight` between Interpolation and interpolated point
    void addLink3(Interpolation const&, const index_t[], const real[], real weight);

    /// linear force of stiffness `weight` between Interpolation and interpolated point
    void addLink4(Interpolation const&, const index_t[], const real[], real weight);

    //----------------------- ELEMENTS LINKING POINTS --------------------------

    /// Force of stiffness `weight` and resting length `len`
    void addLongLink(Mecapoint const&, Mecapoint const&, real len, real weight);
    
    /// Force of stiffness `weight` and resting length `len`
    void addLongLink(Mecapoint const&, Interpolation const&, real len, real weight);
    
    /// Force of stiffness `weight` and resting length `len`
    void addLongLink(Interpolation const&, Interpolation const&, real len, real weight);

#if ( DIM == 2 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment
    void addSideLink2D(Interpolation const&, Mecapoint const&, real arm, real weight);
#elif ( DIM >= 3 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment
    void addSideLink3D(Interpolation const&, Mecapoint const&, Vector const& arm, real weight);
    
    /// Force of stiffness `weight`, at distance `arm` on the side of segment supporting the first argument
    void addSideLinkS(Interpolation const&, Mecapoint const&, Vector const& arm, real len, real weight);
#endif
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment
    void addSideLink(Interpolation const&, Mecapoint const&, real arm, real weight);

    
#if ( DIM == 2 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment
    void addSideLink2D(Interpolation const&, Interpolation const&, real arm, real weight);
#elif ( DIM >= 3 )
    /// Force of stiffness `weight`, at distance `arm` on the side of segment supporting first argument
    void addSideLinkS(Interpolation const&, Interpolation const&, Vector const& arm, real len, real weight);
#endif
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment
    void addSideLink(Interpolation const&, Interpolation const&, real arm, real weight);

#if ( DIM == 2 )
    /// Force of stiffness `weight` and resting length `arm`, on the sides of both fibers
    void addSideSideLink2D(Interpolation const&, Interpolation const&, real arm, real weight, real side1, real side2);
#endif
    /// Force of stiffness `weight` and resting length `arm`, on the sides of both fibers
    void addSideSideLink(Interpolation const&, Interpolation const&, real arm, real weight);

    /// Force of stiffness `weight` and perpendicular to first segment
    void addSlidingLink(Interpolation const&, Mecapoint const&, real weight);
    
    /// Force of stiffness `weight` and perpendicular to first segment
    void addSlidingLink(Interpolation const&, Interpolation const&, real weight);

    
#if ( DIM == 2 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLink2D(Interpolation const&, Mecapoint const&, real arm, real weight);

    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLinkS(Interpolation const&, Mecapoint const&, real arm, real weight);
#elif ( DIM >= 3 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLinkS(Interpolation const&, Mecapoint const&, Vector const& arm, real len, real weight);
#endif
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLink(Interpolation const&, Mecapoint const&, real arm, real weight);
    
    
#if ( DIM == 2 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLink2D(Interpolation const&, Interpolation const&, real arm, real weight);
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLinkS(Interpolation const&, Interpolation const&, real arm, real weight);
#elif ( DIM >= 3 )
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLinkS(Interpolation const&, Interpolation const&, Vector const& arm, real len, real weight);
#endif
    
    /// Force of stiffness `weight`, at distance `arm` on the side of first segment and perpendicular to this segment
    void addSideSlidingLink(Interpolation const&, Interpolation const&, real len, real weight);
    
    /// Create a 3-way link with given weights on each branch
    void addTriLink(Interpolation const& pt1, real w1, Interpolation const& pt2, real w2, Interpolation const& pt3, real w3);
    
    
    /// Allocate the memory necessary to solve(). This must be called after the last add()
    void prepare(Simul const*);
    
    /// Calculate motion of all Mecables in the system
    void solve(SimulProp const*, int precondition);
    
    /// transfer newly calculated point coordinates back to Mecables
    void apply();

    /// calculate Forces on Mecables and Lagrange multipliers for Fiber, without thermal motion
    void computeForces();
    
    
    //Count number of non-zero entries in the entire system
    size_t nbNonZeros(real threshold) const;

    /// Extract the complete dynamic matrix in column-major format in a C-array
    void getMatrix(unsigned, real * matrix) const;
    
    /// Save complete matrix in Matrix Market format
    void saveMatrix(FILE *, real threshold) const;
    
    /// Save right-hand-side vector
    void saveRHS(FILE *) const;
    
    /// Output vectors and matrices, in a format that can be imported in MATLAB
    void saveSystem(const char dirname[]) const;

    /// Save complete matrix in binary format
    void dumpMatrix(FILE *) const;
    
    /// Save elasticity matrix in binary format
    void dumpElasticity(FILE *) const;
    
    /// Save mobility/projection matrix in binary format
    void dumpMobility(FILE *) const;
    
    /// Save preconditionner in binary format
    void dumpPreconditionner(FILE *) const;
    
    /// Save drag coefficients associated with each degree of freedom in binary format
    void dumpDrag(FILE *) const;
    
    /// Save the object ID associated with each degree of freedom
    void dumpObjectID(FILE *) const;
    
    /// Output vectors and matrices, in a format that can be imported in MATLAB
    void dump() const;
 
    /// Output vectors and matrices, in a format that can be imported in MATLAB
    void dump(const char dirname[]) const;

    /// Output vectors and matrices in various files (for debugging)
    void dumpSparse();
    
};

#endif

