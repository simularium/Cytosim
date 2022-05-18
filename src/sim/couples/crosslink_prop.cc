// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "crosslink_prop.h"
#include "crosslink_long.h"
#include "crosslink.h"
#include "simul.h"


/**
 This returns a new Crosslink if ( prop::length <= 0 ),
 or a CrosslinkLong if ( prop::length > 0 ).
 */
Couple * CrosslinkProp::newCouple(Glossary*) const
{
    //std::clog << "CrosslinkProp::newCouple" << std::endl;
    if ( length > 0 )
        return new CrosslinkLong(this);
    else
        return new Crosslink(this);
}


void CrosslinkProp::clear()
{
    CoupleProp::clear();
}


void CrosslinkProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
}


void CrosslinkProp::complete(Simul const& sim)
{
    CoupleProp::complete(sim);
    
    // check for unsupported features:
    if ( sim.ready() && trans_activated )
        throw InvalidParameter("couple:activity=crosslink and trans_activated are incompatible");
}


void CrosslinkProp::write_values(std::ostream& os) const
{
    CoupleProp::write_values(os);
}

