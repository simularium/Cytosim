// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "actor_prop.h"
#include "actor.h"

//------------------------------------------------------------------------------
Hand * ActorProp::newHand(HandMonitor* m) const
{
    return new Actor(this, m);
}

//------------------------------------------------------------------------------
void ActorProp::clear()
{
    HandProp::clear();
    rate = 0;
}

//------------------------------------------------------------------------------
void ActorProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(rate,  "rate");
}

//------------------------------------------------------------------------------
void ActorProp::complete(Simul const& sim)
{
    HandProp::complete(sim);
}

//------------------------------------------------------------------------------

void ActorProp::write_values(std::ostream& os) const
{
    HandProp::write_values(os);
    write_value(os, "rate", rate);
}

