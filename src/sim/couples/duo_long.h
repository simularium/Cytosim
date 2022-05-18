// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DUO_LONG_H
#define DUO_LONG_H

#include "duo.h"

/// A Duo with a non-zero resting length
/**
 The DuoLong is a couple that can be active or inactive:
 - it is activated instantly inside a given space,
 - is is deactivated spontaneously with the given rate.
 .
 See Duo

 The DuoLong differs from Duo in that is uses a non-zero resting length,
 and creates its interaction using Meca:interSideLink()
 
 DuoLong is automatically selected if ( prop:length > 0 )
 @ingroup CoupleGroup
 */
class DuoLong : public Duo
{    
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate `mArm`
    static Torque calcArm(const Interpolation & pt, Vector const& pos, real len);
    
public:
    
    /// constructor
    DuoLong(DuoProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~DuoLong();
     
    /// position on the side of fiber1 used for sideInteractions
    Vector  sidePos() const;
 
    /// force between hands, essentially: stiffness * ( cHand2->posHand() - cHand1->posHand() )
    Vector  force() const;
    
    /// add interactions to a Meca
    void    setInteractions(Meca &) const;
    
};


#endif

