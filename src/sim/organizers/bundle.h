// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BUNDLE_H
#define BUNDLE_H

#include "object.h"
#include "organizer.h"
#include "bundle_prop.h"
#include "fiber.h"

//------------------------------------------------------------------------------

///a set of Fiber held together using an fixed overlap
/**
 The Fibers are attached to form a bundle with anti-parallel overlaps near the MINUS_END.
 The connections form a ring, such that each Fiber is attached to the preceding,
 and to the following Fiber in an anti-parallel manner.
 - object(n) are Fiber.
 - The stiffness of the interaction is 'prop->stiffness'.
 - The amount of overlap is 'prop->overlap'.
 .
 
 @ingroup OrganizerGroup
 */
class Bundle : public Organizer
{
    
private:
    
    /// add interactions to a Meca
    void       linkAntiparallel(Meca &, Fiber *, Fiber *) const;
    
    /// add interactions to a Meca
    void       linkParallel(Meca &, Fiber *, Fiber *) const;

public:

    /// Property
    BundleProp const* prop;
    
    /// constructor
    Bundle(BundleProp const* p) : prop(p) {}
    
    /// destructor  
    virtual   ~Bundle();
    
    /// construct all the dependent Objects of the Organizer
    ObjectList build(Glossary&, Simul&);

    /// perform one Monte-Carlo step
    void       step();
    
    /// add interactions to a Meca
    void       setInteractions(Meca &) const;
    
    /// return the center of gravity from all MT central ends
    Vector     position() const;
    
    /// display parameters
    PointDisp const* disp() const { return nullptr; }

    //------------------------------ read/write --------------------------------

    /// a unique character identifying the class in 
    static const ObjectTag TAG = 'u';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }
    
 };


#endif

