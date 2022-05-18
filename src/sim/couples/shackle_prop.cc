// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "shackle_prop.h"
#include "shackle.h"
#include "shackle_long.h"


Couple * ShackleProp::newCouple(Glossary*) const
{
    //std::clog << "ShackleProp::newCouple" << std::endl;
    if ( length > 0 )
        return new ShackleLong(this);
    else
        return new Shackle(this);
}


void ShackleProp::clear()
{
    CoupleProp::clear();
}


void ShackleProp::read(Glossary& glos)
{
    CoupleProp::read(glos);

    //glos.set(variable,  "variable");
}


void ShackleProp::complete(Simul const& sim)
{
    CoupleProp::complete(sim);
}


void ShackleProp::write_values(std::ostream& os) const
{
    CoupleProp::write_values(os);
    //write_value(os, "variable", variable);
}

