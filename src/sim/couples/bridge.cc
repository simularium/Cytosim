// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bridge.h"
#include "bridge_prop.h"
#include "exceptions.h"
#include "modulo.h"
#include "meca.h"
#include "random.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------

Bridge::Bridge(BridgeProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
}


Bridge::~Bridge()
{
    prop = nullptr;
}

//------------------------------------------------------------------------------

/**
 Calculates the force for the interLongLink()
 */
Vector Bridge::force() const
{
    Vector d = cHand2->pos() - cHand1->pos();
    
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    real dn = d.norm();
    
    return ( prop->stiffness * ( 1 - prop->length / dn ) ) * d;
}


/**
 This uses interLongLink().
 */
void Bridge::setInteractions(Meca & meca) const
{
    meca.addLongLink(cHand1->interpolation(), cHand2->interpolation(), prop->length, prop->stiffness);
}
