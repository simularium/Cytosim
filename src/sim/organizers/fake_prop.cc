// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "property_list.h"
#include "fake_prop.h"
#include "aster_prop.h"
#include "glossary.h"


void FakeProp::clear()
{
    stiffness  = -1;
}


void FakeProp::read(Glossary& glos)
{
    glos.set(stiffness,   "stiffness");
}


void FakeProp::complete(Simul const& sim)
{
    if ( stiffness < 0 )
        throw InvalidParameter("fake:stiffness must be specified and >= 0");
}


void FakeProp::write_values(std::ostream& os) const
{
    write_value(os, "stiffness", stiffness);
}

