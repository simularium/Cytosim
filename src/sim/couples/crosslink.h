// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CROSSLINK_H
#define CROSSLINK_H

#include "couple.h"
class CrosslinkProp;

/// A specialized kind of Couple
/**
 The Crosslink is a simpler kind of Couple, which does not support `trans_activated`
 
 It has a zero resting length, and uses Meca:addLink()
 
 CrosslinkLong has a non-zero resting length, and is selected automatically 
 @ingroup CoupleGroup
 */
class Crosslink : public Couple
{
public:
    
    /// property
    CrosslinkProp const* prop;
    
    /// constructor
    Crosslink(CrosslinkProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual      ~Crosslink();
    
    /// simulation step for a free Couple: diffusion
    virtual void  stepFF(Simul&);
    
    /// add interactions to a Meca
    void          setInteractions(Meca &) const;

};


#endif

