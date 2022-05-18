// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "nucleus_prop.h"
#include "glossary.h"


void NucleusProp::clear()
{
    stiffness = -1;
}


void NucleusProp::read(Glossary& glos)
{
    glos.set(stiffness,       "stiffness");
}


void NucleusProp::complete(Simul const& sim)
{
    if ( stiffness < 0 )
        throw InvalidParameter("nucleus:stiffness must be specified and >= 0");
}


void NucleusProp::write_values(std::ostream& os) const
{
    write_value(os, "stiffness",       stiffness);
}

