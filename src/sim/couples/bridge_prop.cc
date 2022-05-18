// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "bridge_prop.h"
#include "bridge.h"


Couple * BridgeProp::newCouple(Glossary*) const
{
    //std::clog << "BridgeProp::newCouple" << std::endl;
    return new Bridge(this);
}


void BridgeProp::clear()
{
    CoupleProp::clear();
}


void BridgeProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
}


void BridgeProp::complete(Simul const& sim)
{
    CoupleProp::complete(sim);
    
#if ( DIM > 2 )
    //@todo: the obligation for length > 0 can be removed, once interSideLink3D() is implemented
    if ( length <= 0 )
        throw InvalidParameter("bridge:length should be defined and > 0");
#endif
}


void BridgeProp::write_values(std::ostream& os) const
{
    CoupleProp::write_values(os);
}

