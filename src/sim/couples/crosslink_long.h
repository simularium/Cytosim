// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CROSSLINK_LONG_H
#define CROSSLINK_LONG_H

#include "crosslink.h"

/// A Crosslink with a non-zero resting length
/**
 The CrosslinkLong adds a non-zero resting length to Crosslink,
 using Meca:interSideLink()
 
 CrosslinkLong is automatically selected if ( prop:length > 0 )
 @ingroup CoupleGroup
 */
class CrosslinkLong : public Crosslink
{
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate `mArm`
    static Torque calcArm(const Interpolation & pt, Vector const& pos, real len);
    
public:
    
    /// constructor
    CrosslinkLong(CrosslinkProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~CrosslinkLong();
    
    /// position on the side of fiber1 used for sideInteractions
    Vector  sidePos() const;
 
    /// force between hands, essentially: stiffness * ( cHand2->posHand() - cHand1->posHand() )
    Vector  force() const;
    
    /// add interactions to a Meca
    void    setInteractions(Meca &) const;
    
};


#endif

