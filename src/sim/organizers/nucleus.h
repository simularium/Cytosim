// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NUCLEUS_H
#define NUCLEUS_H

#include "nucleus_prop.h"
#include "organizer.h"
#include "sphere.h"
#include "fiber.h"

//------------------------------------------------------------------------------
/// Organizer built around a Sphere
/**
A Nucleus attaches Fibers to a Sphere:\n
 - organized(0) is the Sphere
 - organized(n) for n>0 is a Fiber attached to the sphere
 - prop->stiffness is the stiffness of the link.
 .

 @ingroup OrganizerGroup
 */
class Nucleus : public Organizer
{
public:
    
    /// Properties for the Nucleus
    NucleusProp const* prop;
        
    //------------------- construction and destruction -------------------------
    /// constructor
    Nucleus(NucleusProp const* p) : prop(p) { }

    /// create a Nucleus and requested associated Objects
    ObjectList    build(Glossary&, Simul&);
    
    //------------------- simulation -------------------------------------------    

    /// monte-carlo simulation step
    void          step();
    
    ///add interactions for this object to a Meca
    void          setInteractions(Meca &) const;

    //------------------- querying the nucleus ---------------------------------    
    
    ///position of center of gravity (returns the center of the sphere)
    Vector        position() const { return sphere()->position(); }
    
    ///the Sphere on which the nucleus is built
    Sphere *      sphere()   const { return static_cast<Sphere*>(organized(0)); }
    
    /// i-th fiber attached to the nucleus
    Fiber *       fiber(int i) const { return static_cast<Fiber*>(organized(i+1)); }
    
    
    
    /// retrieve link between Sphere and ends of Fiber
    bool          getLink(size_t, Vector&, Vector&) const;
    
    /// display parameters 
    PointDisp const* disp() const { if ( sphere() ) return sphere()->prop->disp; return nullptr; }
    
    //------------------------------ read/write --------------------------------
    
    /// a unique character identifying the class
    static const ObjectTag TAG = 'n';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }

};


#endif

