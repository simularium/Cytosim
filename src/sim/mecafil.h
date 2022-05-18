// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef MECAFIL_H
#define MECAFIL_H

#include "chain.h"
#include "fiber_prop.h"  // needed for NEW_FIBER_LOOP

class Matrix;

/// incompressible Filament with bending elasticity
/**
 Implements the methods of a Mecable for the Chain:
 
 -# projectForces() includes longitudinal incompressibility,
 which means keeping successive points equidistants:
 norm( point(p+1) - point(p) ) = segmentation()
 
 -# addRigidity() implements bending elasticity.
 .
*/
class Mecafil : public Chain
{
private:
    
    /// Lagrange multipliers associated with longitudinal imcompressibility
    real   *    rfLag;
    
    /// normalized differences of successive vertices (size is DIM*nbSegments)
    real   *    rfDiff;
    
    /// work array allocated to hold DIM*nbPoints() coordinates
    real   *    rfLLG;
    
    /// work array allocated to hold DIM*nbPoints() coordinates
    real   *    rfVTP;
    
    /// J*J', a nbSegments^2 matrix. We store the diagonal and one off-diagonal
    real   *    mtJJt, * mtJJtU;
    
    /// vector for the projection correction of size nbSegments
    real   *    mtJJtiJforce;
    
    /// true if all elements of mtJJtiJforce[] are null
    bool        useProjectionDiff;
    
protected:
    
    /// mobility of the points (all points have the same drag coefficient)
    real        rfPointMobility;
    
    /// rigidity scaling factor used in addRigidity()
    real        rfRigidity;
    
#if NEW_FIBER_LOOP
    /// link filament into a loop
    bool        rfRigidityLoop;
#endif
    
    /// calculate the normalized difference of successive vertices in rfDiff[]
    void        storeDirections();

private:
    
    /// reset the memory pointers for the projection
    void        buildProjection();
    
    /// allocate memory for the projection
    void        allocateProjection(size_t);
    
    /// free the memory for the projection
    void        destroyProjection();

public:
    
    /// Constructor
    Mecafil();
    
    /// copy constructor
    Mecafil(Mecafil const&);
    
    /// copy assignment
    Mecafil& operator=(Mecafil const&);
    
    /// Destructor
    virtual    ~Mecafil();
    
    /// allocate memory
    size_t      allocateMecable(size_t);
    
    /// free allocated memory
    void        release();

    /// compute Lagrange multipliers associated with length constraints, given the force
    void        computeTensions(const real* force);
    
    /// debug output
    void        printTensions(FILE *, char = ' ') const;
    
    /// replace current forces by the ones provided as argument, and compute tensions
    void        getForces(const real* ptr);

    /// longitudinal force dipole between vertices `p` and `p+1`
    /**
     Tensions are calculated as the Lagrange multipliers associated with the
     distance between neigboring vertices, i.e. the fiber segment's lengths.
     This tension is:
     - positive when the segment is being pulled
     - negative when the segment is under compression
     .
     It is given in units of force (pico-Newton, if all quantitites use our units).
     */
    real        tension(unsigned p) const { assert_true(p+1<nPoints); return rfLag[p]; }
    
    /// total drag-coefficient of object (force = drag * speed)
    real        dragCoefficient() const { return nPoints / rfPointMobility; }
    
    /// drag coefficient of one point
    real        leftoverMobility() const { return rfPointMobility; }
    
    //--------------------- Projection  / Dynamics
    
    /// prepare for projection
    void        makeProjection();

    /// prepare the correction to the projection
    void        makeProjectionDiff(const real* );
    
    /// add the contribution from the projection correction
    void        addProjectionDiff(const real*, real*) const;
    
    /// true if addProjectionDiff() does something
    bool        hasProjectionDiff() const { return useProjectionDiff; }

    /// add displacements due to the Brownian motion to rhs[]
    real        addBrownianForces(real const* rnd, real alpha, real* rhs) const;

    /// calculate the speeds from the forces, including projection
    void        projectForces(const real* X, real* Y) const;
    
    /// print projection matrix
    void        printProjection(std::ostream&) const;

    //--------------------- Rigidity

    /// add the rigidity force corresponding to configuration X into vector Y
    void        addRigidity(const real* X, real* Y) const;
    
    /// add rigidity terms to upper side of matrix
    void        addRigidityUpper(real*, unsigned) const;

};


#endif
