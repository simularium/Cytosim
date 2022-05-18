// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BEAD_H
#define BEAD_H

#include "dim.h"
#include "array.h"
#include "object.h"
#include "mecable.h"
#include "bead_prop.h"

class Meca;
class Single;
class SingleProp;

/// A single point with a radius
/**
 The Bead is the simplest Mecable.
 It represents a spherical object using: 
 - a position vector,
 - a radius.
 .
 The orientational degrees of freedom are neglected.
 Translation follows Stokes's law.
 A Single can be attached in the center of the bead.
 
 For more elaborate models, see Sphere and Solid.
*/
class Bead : public Mecable
{
private:
    
    /// radius
    real        paRadius;

    /// the total drag coefficient for translation
    real        paDrag;
    
public:
    
    /// Property
    BeadProp const* prop;
    
    /// create following specifications
    Bead(BeadProp const*, Vector pos, real rad);

    /// destructor
    virtual ~Bead();
    
    //--------------------------------------------------------------------------
    
    /// return the position in space of the object
    Vector      pos()                  const { return Vector(pPos); }

    /// return the position in space of the object
    Vector      position()             const { return Vector(pPos); }
    
    /// move the object position ( position += given vector )
    void        translate(Vector const& x)   { x.add_to(pPos); }
    
    /// set the object position ( position = given vector )
    void        setPosition(Vector const& x) { x.store(pPos); }

    //--------------------------------------------------------------------------
        
    /// the radius of the Bead
    real        radius()               const { return paRadius; }
    
    /// the volume of the bead
    real        radiusSqr()            const { return paRadius * paRadius; }
    
    /// set the radius of the Bead
    void        resize(real R)               { assert_true(R>0); paRadius = R; }
    
    /// the volume of the bead
    real        volume() const;
    
    //--------------------------------------------------------------------------
    
    /// sets the mobility
    void        setDragCoefficient();
    
    /// the total drag-coefficient of object (force = drag * speed)
    real        dragCoefficient()      const { return paDrag; }

    /// sets the mobility
    void        prepareMecable() { setDragCoefficient(); }
    
    /// calculates the speed of points in Y, for the forces given in X
    void        projectForces(const real* X, real* Y) const;
    
    /// add contribution of Brownian forces
    real        addBrownianForces(real const* rnd, real sc, real* rhs) const;

    /// add the interactions due to confinement
    void        setInteractions(Meca &) const;
    
    //--------------------------------------------------------------------------

    /// a static_cast<> of Node::next()
    Bead *      next()  const { return static_cast<Bead*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Bead *      prev()  const { return static_cast<Bead*>(nPrev); }
    
    //--------------------------------------------------------------------------

    /// a unique character identifying the class
    static const ObjectTag TAG = 'b';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }
    
    /// convert pointer to Fiber* if the conversion seems valid; returns 0 otherwise
    static Bead* toBead(Object * obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Bead*>(obj);
        return nullptr;
    }
    
    /// convert pointer to Fiber* if the conversion seems valid; returns 0 otherwise
    static Bead const* toBead(Object const* obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Bead const*>(obj);
        return nullptr;
    }

    //--------------------------------------------------------------------------

    /// read from file
    void        read(Inputter&, Simul&, ObjectTag);
    
    /// write to file
    void        write(Outputter&) const;

};

#endif
