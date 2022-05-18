// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "property_list.h"
#include "regulator_prop.h"
#include "regulator.h"
#include "simul.h"


Hand * RegulatorProp::newHand(HandMonitor* m) const
{
    return new Regulator(this, m);
}


void RegulatorProp::clear()
{
    HandProp::clear();

    rate = 0;
}


void RegulatorProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(rate,  "rate");
}


void RegulatorProp::complete(Simul const& sim)
{
    HandProp::complete(sim);
    
    if ( rate < 0 )
        throw InvalidParameter("regulator:rate must be >= 0");
    
    rate_dt = rate * sim.time_step();
}


void RegulatorProp::write_values(std::ostream& os) const
{
    HandProp::write_values(os);
    write_value(os, "rate", rate);
}

