// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef WRIST_H
#define WRIST_H

#include "single.h"
#include "interpolation4.h"

/// a Single anchored to a Mecable.
/**
 The Wrist is anchored to a Solid, on a position that is interpolated from the
 Solid's vertices. See class Interpolation4

 @ingroup SingleGroup
 */
class Wrist : public Single
{
protected:
    
    Interpolation4 anchor;
    
public:
     
    /// Construct object anchored at one Mecapoint
    Wrist(SingleProp const*, Mecable const*, unsigned point);
    
    /// Construct object anchored between two Mecapoint
    Wrist(SingleProp const*, Mecable const*, unsigned, unsigned, real);
   
    /// Constructor object interpolated over a triad of Mecapoint
    Wrist(SingleProp const*, Mecable const*, unsigned ref, Vector pos);

    /// destructor
    ~Wrist();
    
    //--------------------------------------------------------------------------
    
    /// return the position in space of the object
    Vector  position() const { return posFoot(); }
    
    /// Wrist accepts translation
    int     mobile() const { return 1; }
    
    /// translate object's position by the given vector
    void    translate(Vector const&) { }
    
    /// bring object to centered image using periodic boundary conditions
    void    foldPosition(Modulo const*) { }
    
    /// stiffness of the interaction
    real    linkStiffness() const { return prop->stiffness; }

    //--------------------------------------------------------------------------
    
    /// Object to which this is anchored
    Mecable const* base() const { return anchor.base(); }

    /// the position of the anchoring point
    Vector  posFoot() const { return anchor.pos(); }
    
    
    /// true if Single creates a link
    bool    hasForce() const { return true; }

    /// force = stiffness * ( posFoot() - posHand() )
    Vector  force() const;
    
    
    /// Monte-Carlo step for a free Single
    void    stepF(Simul&);
    
    /// Monte-Carlo step for a bound Single
    void    stepA();

    /// add interactions to a Meca
    void    setInteractions(Meca &) const;

    //--------------------------------------------------------------------------
    
    /// the Wrist uses a specific TAG to distinguish itself from the Single
    static const ObjectTag TAG = 'w';
    
    /// return unique character identifying the class
    ObjectTag    tag() const { return TAG; }
    
    /// read from file
    void    read(Inputter&, Simul&, ObjectTag);
    
    /// write to file
    void    write(Outputter&) const;
    
};


#endif
