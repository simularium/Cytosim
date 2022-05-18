// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SINGLE_H
#define SINGLE_H

#include "dim.h"
#include "vector.h"
#include "movable.h"
#include "object.h"
#include "hand_monitor.h"
#include "mecapoint.h"
#include "single_prop.h"
#include "hand.h"


class Fiber;
class PointDisp;


/// A point-like object containing one Hand.
/**
 A Single contains one pointer to Hand, and consequently
 inherit the 2 possible states: `attached` or `free`.
 
 By default:
 - Free Single are diffusing, and try to bind to nearby Fibers,
 - Attached Singles are moving along the Fiber to which their Hand is attached.
 .
 
 However, two derived classes change this behavior:
 -# a Picket is fixed in position and do not diffuse,
 -# a Wrist is attached to one vertex of a Mecable.
 .
 
 Attached Wrist and Picket exert a force on the Fiber to which the Hand is attached.
 For WristLong and PicketLong, this force can have a non-zero resting length.
 For these class in which the Hand can be under tension, `hasForce()` returns true.

 Wrist and Picket can be distinguished with Single::base():
 - for Single and Picket, this returns zero,
 - for Wrist, this returns the Mecable on which the Wrist is attached.
 .

 @ingroup SingleGroup
 */

class Single : public Object, public HandMonitor
{
private:
    
    /// specialization of HandMonitor
    void      afterAttachment(Hand const*);
    /// specialization of HandMonitor
    void      beforeDetachment(Hand const*);
    /// specialization of HandMonitor
    Vector    otherPosition(Hand const*) const { return posFoot(); }
    /// = identity() of the Object on which a Wrist is attached, or Single::identity()
    ObjectID  nucleatorID()       const { return base()?base()->identity():Object::identity(); }
    /// specialization of HandMonitor
    real      linkRestingLength() const { return prop->length; }
    /// stiffness of the interaction
    real      linkStiffness()     const { return 0; }

protected:
    
    /// the position of the foot
    Vector        sPos;
    
    /// the motor domain
    Hand *        sHand;

public:
    
    /// property
    SingleProp const* prop;

    /// constructor at specified position
    Single(SingleProp const*, Vector const& = Vector(0,0,0));

    /// destructor
    virtual ~Single();
    
    //--------------------------------------------------------------------------
    
    /// associated Hand
    Hand*  hand()                              { return sHand; }
    
    /// sHand->attached()
    bool    attached()                  const  { return sHand->attached(); }
    
    /// sHand->attached()
    int     state()                     const  { return sHand->attached(); }

    /// Fiber to which this is attached
    Fiber*  fiber()                     const  { return sHand->fiber(); }
    
    /// attachment position of Hand along fiber (call is invalid if Hand is not attached)
    real    abscissa()                  const  { return sHand->abscissa(); }
    
    /// position of the Hand (call is invalid if Hand is not attached)
    Vector  posHand()                   const  { return sHand->pos(); }
    
    /// direction of Fiber at attachment point (call is invalid if Hand is not attached)
    Vector  dirFiber()                  const  { return sHand->dirFiber(); }
    
    /// attach Hand at the given site
    void    attach(FiberSite s)                { if ( sHand->attachmentAllowed(s) ) sHand->attach(s); }
    
    /// attach Hand at given Fiber end
    void    attachEnd(Fiber * f, FiberEnd end) { sHand->attachEnd(f, end); }

    /// move Hand at given end
    void    moveToEnd(FiberEnd end)            { sHand->moveToEnd(end); }
    
    /// detach
    void    detach()                           { sHand->detach(); }

    //--------------------------------------------------------------------------
    
    ///return the position in space of the object
    virtual Vector  position() const;
    
    /// Single can be translated only if it is not attached
    virtual int     mobile()              const  { return !sHand->attached(); }
    
    /// translate object's position by the given vector
    virtual void    translate(Vector const& x)   { sPos += x; }
    
    /// move object to specified position
    virtual void    setPosition(Vector const& x) { sPos = x; }

    /// bring object to centered image using periodic boundary conditions
    virtual void    foldPosition(Modulo const*);
    
    /// set the position randomly inside prop->confine_space
    void            randomizePosition();

    //--------------------------------------------------------------------------
    
    /// the position of the anchoring point
    virtual Vector  posFoot()     const { return sPos; }
    
    /// position on the side of fiber used for sideInteractions
    virtual Vector  sidePos()     const { return sHand->pos(); }
    
    /// the Mecable to which this is anchored, or zero
    virtual Mecable const* base() const { return nullptr; }
    
    /// true if Single creates an interaction
    virtual bool    hasForce()    const { return false; }

    /// force = stiffness * ( position_anchor - position_hand ), or zero for a diffusible Single
    virtual Vector  force()       const { return Vector(0,0,0); }

    /// Monte-Carlo step if the Hand is not attached
    virtual void    stepF(Simul&);
    
    /// Monte-Carlo step if the Hand is attached
    virtual void    stepA();
    
    /// add interactions to a Meca
    virtual void    setInteractions(Meca &) const;
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    Single*         next()   const  { return static_cast<Single*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Single*         prev()   const  { return static_cast<Single*>(nPrev); }

    //--------------------------------------------------------------------------

    /// a unique character identifying the class
    static const ObjectTag TAG = 's';
    
    /// return unique character identifying the class
    ObjectTag        tag() const { return TAG; }
    
    /// return associated Property
    Property const*  property() const { return prop; }
    
    /// return PointDisp of associated Hand
    PointDisp const* disp() const { return sHand->prop->disp; }
    
    /// return Property::confine_space_ptr
    Space const* confineSpace() const { return prop->confine_space_ptr; }

    /// read from file
    virtual void    read(Inputter&, Simul&, ObjectTag);
    
    /// write to file
    virtual void    write(Outputter&) const;
    
};


#endif
