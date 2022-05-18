// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef COUPLE_LONG_H
#define COUPLE_LONG_H

#include "couple.h"

/// A Couple with a non-zero resting length
/**
 The CoupleLong adds a non-zero resting length to Couple,
 using Meca:interSideLink()

 CoupleLong is automatically selected if ( prop:length > 0 )
 @ingroup CoupleGroup
 */
class CoupleLong : public Couple
{
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate `mArm`
    static Torque calcArm(const Interpolation & pt, Vector const& pos, real len);
    
public:
    
    /// constructor
    CoupleLong(CoupleProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~CoupleLong();
    
    /// position on the side of fiber1 used for sideInteractions
    Vector  sidePos() const;
 
    /// force between hands, essentially: stiffness * ( cHand2->posHand() - cHand1->posHand() )
    Vector  force() const;
    
    /// add interactions to a Meca
    void    setInteractions(Meca &) const;
    
};


#endif

