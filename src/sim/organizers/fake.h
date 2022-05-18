// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FAKE_H
#define FAKE_H

#include "object.h"
#include "organizer.h"
#include "fake_prop.h"
#include "mecapoint.h"
#include "solid.h"

//------------------------------------------------------------------------------

///a set of two asters held together by a Solid 
/**
 This object cannot handle the destruction of the Asters
 
 The Fake should just link two Solid, without reference to the Asters
 
 @ingroup OrganizerGroup
 */
class Fake : public Organizer
{

private:

    /// Property
    FakeProp const* prop;
    
    /// Display parameters
    PointDisp const* disp_ptr;
    
    /// connections
    std::vector<Mecapoint> asterPoints, solidPoints;

public:
    
    /// constructor
    Fake(FakeProp const* p) : prop(p), disp_ptr(nullptr) { }
 
    /// construct all the dependent Objects of the Organizer
    ObjectList build(Glossary&, Simul&);

    /// perform one Monte-Carlo step
    void       step();
    
    /// add interactions to a Meca
    void       setInteractions(Meca &) const;
    
    /// return pointer to central Solid
    Solid *    solid() const { return static_cast<Solid*>(organized(0)); }

    //------------------------------ read/write --------------------------------
    
    /// retrieve link between Solid and Aster's core
    bool       getLink(size_t, Vector&, Vector&) const;
    
    /// return PointDisp of Solid
    PointDisp const* disp() const { return disp_ptr; }

    /// a unique character identifying the class
    static const ObjectTag TAG = 'k';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }

 };


#endif

