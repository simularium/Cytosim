// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BRIDGE_H
#define BRIDGE_H

#include "couple.h"
class BridgeProp;

/// A Couple with a different mechanical link
/**
 The Bridge differs from CoupleLong in the nature of the mechanical link that it
 creates betwen two filaments.
 The Bridge uses Meca::interLongLink(), whereas other CoupleLong use Meca::interSideLink().
 
 The "Long link" is a finite resting length Hookean spring, which can freely rotate
 at both of its ends. Hence the angle with respect to the filament is uncontrained,
 unlike the "Side link", in which the spring extends orthogonally to  the direction of the filaments.
 
 Because of this the Bridge does not impose a strict separation between a pair of filaments.
 Longitudinal shear on two filaments connected by 'bridges' will likely affect the distance between them.
 
 \image html meca_links.png
 
 The Bridge should have a non-zero resting length.
 For zero-resting length, use Couple or Crosslink
 @ingroup CoupleGroup
 */
class Bridge : public Couple
{
public:
    
    /// property
    BridgeProp const* prop;
    
    /// constructor
    Bridge(BridgeProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Bridge();
    
    //--------------------------------------------------------------------------
 
    /// force between hands
    Vector  force() const;
    
    /// add interactions to a Meca
    void    setInteractions(Meca &) const;
    
};


#endif

