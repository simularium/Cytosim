// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef HAND_MONITOR
#define HAND_MONITOR

#include "real.h"
#include "vector.h"
#include "inventoried.h"
#include "object.h"

class Hand;
class Simul;
class FiberSite;


/// base class to monitor and control Hand's actions
/**
 The HandMonitor defines an interface that is implemented in both Single and Couple.
 It has two functions:
 1- It allows to inform Single and Couple if their Hand bind or unbind.
 2- It is a mechanism for a Hand to access data from the Single or Couple
  to which it belongs.
 .
 */
class HandMonitor
{

public:
    
    /// Returning `false` prevents the attachment (this is called before every attempt)
    virtual bool allowAttachment(FiberSite const&) { return true; }
    
    /// called after attachement
    virtual void afterAttachment(Hand const*) {}
    
    /// called before detachment
    virtual void beforeDetachment(Hand const*) {}
    
    
    /// return the Hand that is not the argument, in a Couple
    virtual Hand * otherHand(Hand const*) const { return nullptr; }

    /// return the position of the Hand that is not the argument, for a Couple
    /** If the hand is not part of a Couple, this returns Vector(0,0,0) */
    virtual Vector otherPosition(Hand const*) const { return Vector(0,0,0); }

    /// return the direction of the Fiber for the Hand that is not the argument, in a Couple
    /** If the hand is not part of a Couple, this returns a random unit vector */
    virtual Vector otherDirection(Hand const*) const { return Vector::randU(); }

    /// resting length of the interaction
    virtual real   linkRestingLength() const { return 0; }
    
    /// stiffness of the interaction
    virtual real   linkStiffness() const { return 0; }

    /// identity() of containing object
    virtual ObjectID nucleatorID() const { return 0; }

};


#endif
