// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SOLID_H
#define SOLID_H

#include "dim.h"
#include "array.h"
#include "object.h"
#include "mecable.h"
#include "matrix33.h"
#include "solid_prop.h"

class Meca;

/// Undeformable set of points
/**
 This is a Mecable behaving like a undeformable cloud of points.
 Each point can have its own radius and together they define the viscous drag
 of the Solid in the medium.
 
 \par Geometry:
 
 The ensemble can rotate and translate like a rigid body under external forces,
 but the relative configuration of the points in space is fixed: 
 the distance between any two points is constant.  

 A snapshot of the current geometry is saved in soShape[] by fixShape().
 This configuration is reapplied to the current points by reshape(). 
 reshape() however find the best isometric transformation of soShape[] 
 into the current configuration to maintain the current position and the
 current orientation of the object.
 
 \par Viscous Drag:
 
 The distance between the points, and their radii define a total drag
 coefficient according to Stokes' law applied to individual spheres.
 Points that have a radius = 0 do not induce viscous drag.
 The hydrodynamic interactions between the beads in the ensemble,
 and more advanced hydrodynamic effects are neglected.
 The drag coefficent for translation is simply the sum of Stokes' law,
 for all points that have a radius > 0.
 The rotational drag coefficient involves the second momentum of the configuration.
 
 \par Related classes:
 
 Solid is an extension of Bead. 
 A Solid with only one point is equivalent to a Bead, but slower to simulate.
*/
class Solid : public Mecable
{
private:
    
#if ( DIM > 2 )
    /// matrix containing the reduced momentum of inertia for 3D
    Matrix33       soMomentum;
#endif

    /// the mean of the the points weighted by their drag coefficients
    Vector         soCenter;
    
    /// the dimensions used in Stokes' law to calculate overall mobility
    real     *     soRadius;
    
    /// array to store the reference shape of the solid, as coordinates
    real     *     soShape;
    
    /// the number of points when fixShape() was last called, used for verifications.
    unsigned int   soShapeSize;
    
    /// a counter used in reshape()
    unsigned int   soReshapeTimer;
    
    /// the reduced total (all points summed) drag coefficient for translation
    real           soDrag;
    
    /// the reduced total drag coefficient for rotation
    real           soDragRot;

    /// second momentum of the reference shape
    real           soShapeSqr;
    
    /// reset private variables
    void           reset();
    
public:
    
    /// Property
    SolidProp const* prop;
    
    /// allocate memory
    size_t      allocateMecable(size_t);
    
    /// free allocated memory
    void        release();

    /// initialize according to options given in Glossary
    ObjectList  build(Glossary&, Simul&);
    
    /// constructor
    Solid(SolidProp const*);
    
    /// Copy constructor
    Solid(const Solid&);
    
    /// Assignement operator
    Solid& operator =(const Solid&);

    /// destructor
    virtual    ~Solid();
    
    //------------------------------- Mecable ----------------------------------
    
    /// sets the mobility
    void        setDragCoefficient();

    /// total translation drag-coefficient (force = drag * speed)
    real        dragCoefficient() const;

    /// prepare for Meca
    void        prepareMecable();

    /// prepare for constrained projection
    void        makeProjection();
    
    /// calculates the speed of points in Y, for the forces given in X
    void        projectForces(const real* X, real* Y) const;
    
    /// add contribution of Brownian forces
    real        addBrownianForces(real const* rnd, real sc, real* rhs) const;
    
    /// monte-carlo step
    void        step();
    
    //------------------------------- Shaping ----------------------------------

    /// set the reference shape as a copy of the current one
    void        fixShape();
    
    /// scale the reference shape
    void        scaleShape(const real[DIM]);
    
    /// scale current shape to match the reference set in fixShape()
    void        rescale();
    
    /// restore the reference shape in the place and orientation of the current one
    void        reshape();
    
    /// change coordinate values
    void        getPoints(real const*);

    /// add a new point with a sphere (extends Mecable::addPoint)
    unsigned    addSphere(Vector const&, real radius);
    
    /// change radius of the sphere around point `i`
    void        radius(unsigned i, real radius);

    /// add DIM points separated by `len`, to make a coordinate system around the last point
    unsigned    addTriad(real len);

    //--------------------------------------------------------------------------

    /// add the interactions due to confinement
    void        setInteractions(Meca &) const;
    
    /// radius of the sphere around point `i`
    real        radius(const unsigned i) const { return soRadius[i]; }
    
    /// mean of all spheres weighted with their drag coefficients (or equivalently radius)
    Vector      centroid() const;
    
    /// Position of center of gravity
    Vector      position() const { return centroid(); }

#if NEW_SOLID_CLAMP
    Vector      clampForce() const { return prop->clamp_stiff * ( prop->clamp_pos - posPoint(0) ); }
#endif
    
    //--------------------------------------------------------------------------

    /// a static_cast<> of Node::next()
    Solid *     next()  const { return static_cast<Solid*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Solid *     prev()  const { return static_cast<Solid*>(nPrev); }
    
    //--------------------------------------------------------------------------

    /// a unique character identifying the class
    static const ObjectTag TAG = 'd';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }
    
    /// convert pointer to Solid* if the conversion seems valid; returns 0 otherwise
    static Solid* toSolid(Object * obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Solid*>(obj);
        return nullptr;
    }

    //--------------------------------------------------------------------------

    /// read from file
    void        read(Inputter&, Simul&, ObjectTag);
    
    /// write to file
    void        write(Outputter&) const;

    /// Human friendly ouput
    void        print(std::ostream&, bool write_shape = false) const;
};

/// output operator:
std::ostream& operator << (std::ostream& os, Solid const&);

#endif
