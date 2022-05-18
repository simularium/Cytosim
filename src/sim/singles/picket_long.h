// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef PICKET_LONG_H
#define PICKET_LONG_H

#include "picket.h"


/// a Picket with a non-zero resting length.
/**
 This single is fixed at its foot position in absolute space.
 It has a non-zero resting length.

 @ingroup SingleGroup
 */
class PicketLong : public Picket
{
    
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to recalculate `mArm`
    static Torque calcArm(const Interpolation & pt, Vector const& pos, real len);
    
public:

    /// constructor
    PicketLong(SingleProp const*, Vector const& = Vector(0,0,0));

    /// destructor
    ~PicketLong();
        
    /// position on the side of fiber used for sideInteractions
    Vector  sidePos() const;
    
    /// force = stiffness * ( posFoot() - posHand() )
    Vector  force() const;
    
    /// add interactions to a Meca
    void    setInteractions(Meca &) const;
    
};


#endif
