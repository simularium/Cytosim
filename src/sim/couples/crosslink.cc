// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "crosslink.h"
#include "crosslink_prop.h"
#include "exceptions.h"
#include "random.h"
#include "modulo.h"
#include "space.h"
#include "meca.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------
Crosslink::Crosslink(CrosslinkProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
}


Crosslink::~Crosslink()
{
    prop = nullptr;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Simulates:
 - diffusive motion
 - attachment
 .
 */
void Crosslink::stepFF(Simul& sim)
{
    diffuse();
    
    // confinement:
    if ( !prop->confine_space_ptr->inside(cPos) )
        cPos = prop->confine_space_ptr->bounce(cPos);
    
    if ( modulo )
        modulo->fold(cPos);
    
    /*
     To attachment a Couple, we flip a coin to give equal chance to each Hand,
     as if they were sharing the two half of a spherical cap.
     Note that this divides by two the effective binding rate of the Hands.
     */
    if ( RNG.flip() )
        cHand1->stepUnattached(sim, cPos);
    else
        cHand2->stepUnattached(sim, cPos);
}


void Crosslink::setInteractions(Meca & meca) const
{
    assert_true( attached1() && attached2() );
    
    meca.addLink(cHand1->interpolation(), cHand2->interpolation(), prop->stiffness);
}

