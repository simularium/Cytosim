// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPHERE_H
#define SPHERE_H

#include "dim.h"
#include "array.h"
#include "object.h"
#include "mecable.h"
#include "sphere_prop.h"

class Meca;
class Wrist;

/// Spherical object with a viscous surface
/** 
 A Mecable representing a spherical object using:
 - a radius,
 - the position of the center (point index 0),
 - fixed points on the surface to keep track of the orientation,
 - mobile points on the surface.
 .

 A set of 'fixed' points provide a reference frame for the sphere:
 nbRefPoints = 2 in 2D and 4 in 3D.

 The sphere can move as a solid body by rotation and translation.
 In addition, the surface-points can move on the surface. This motion includes
 diffusion and force-induced drag and is characterized by a mobility scalar. 
 Finally, a mobile point can also carry a Single.

 This class was started by Dietrich Foethke in 2005 to simulate the nucleus of S.pombe.
 Related classes are Bead and Solid.
*/
class Sphere : public Mecable
{
public:
    
    /// number of reference points, including center: 1, 2, 4 for DIM = 1, 2 and 3
    static constexpr unsigned nbRefPoints = DIM+(DIM==3);

private:
    
    /// radius
    real             spRadius;
    
    /// drag coefficients for translation and rotation
    real             spDrag, spDragRot;
        
    //--------------------------------------------------------------------------
    
    /// used for projecting forces
    real *           sRad;

public:
    
    /// Property
    SphereProp const* prop;
       
    //------------------- construction and destruction -------------------------
    
    /// create but do not initialize
    Sphere(SphereProp const*);

    /// constructor
    Sphere(SphereProp const*, real radius);
    
    /// Copy constructor
    Sphere(const Sphere&);
    
    /// Assignement operator
    Sphere& operator =(const Sphere&);

    /// destructor
    virtual    ~Sphere();
    
    //-------------------------------- info ------------------------------------
    
    /// calculate mobility with piston effect
    void        setDragCoefficientPiston();
    
    /// calculate mobility with piston effect
    void        setDragCoefficientStokes();
    
    /// calculate mobility
    void        setDragCoefficient();
    
    /// total drag-coefficient of object (force = drag * speed)
    real        dragCoefficient() const { return spDrag; }

    /// allocate memory
    size_t      allocateMecable(size_t);
    
    /// free allocated memory
    void        release();

    /// calculate mobility and diffusion constant
    void        prepareMecable();

    /// returns position of center of gravity (the center of the sphere)
    Vector      position()        const { return posP(0); }
    
    /// radius of the sphere
    real        radius()          const { return spRadius; }

    /// change radius
    void        resize(real);
    
    /// add the interactions due to confinement
    void        setInteractions(Meca &) const;
    
    //------------------- technical functions and mathematics ------------------
        
    /// add contribution of Brownian forces
    real        addBrownianForces(real const* rnd, real sc, real* rhs) const;

    /// bring all surface points at distance spRadius from center, by moving them radially
    void        reshape();
    
    
    /// move the reference points such as to restore a orthogonal reference
    void        orthogonalize(unsigned i);

    /// set position
    void        getPoints(real const* x) { Mecable::getPoints(x); reshape(); }

    /// normalize point and add center
    unsigned    addSurfacePoint(Vector const&);
    
    /// number of points on the surface
    unsigned    nbSurfacePoints() const { return nPoints - nbRefPoints; }
    
    /// initialize according to options given in Glossary
    ObjectList  build(Glossary&, Simul&);

    //------------------- methods for the projection ---------------------------
    
    /// prepare for constrained projection
    void        makeProjection();
    
    /// calculate speed of points in Y, for the forces given in X, scaled by sc
    void        projectForces(const real* X, real* Y) const;
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    Sphere * next() const { return static_cast<Sphere*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Sphere * prev() const { return static_cast<Sphere*>(nPrev); }
    
    //------------------------------ read/write --------------------------------
    
    /// a unique character identifying the class
    static const ObjectTag TAG = 'o';
    
    /// return unique character identifying the class
    ObjectTag    tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }
    
    /// convert pointer to Solid* if the conversion seems valid; returns 0 otherwise
    static Sphere* toSphere(Object * obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Sphere*>(obj);
        return nullptr;
    }

    /// write to file
    void        write(Outputter&) const;
    
    /// read from file
    void        read(Inputter&, Simul&, ObjectTag);
    
    /// Human friendly ouput
    void        print(std::ostream&) const;
};


/// output operator:
std::ostream& operator << (std::ostream& os, Sphere const&);

#endif
