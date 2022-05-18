// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "fork_prop.h"
#include "fork.h"


Couple * ForkProp::newCouple(Glossary*) const
{
    //std::clog << "ForkProp::newHand" << std::endl;
    return new Fork(this);
}


void ForkProp::clear()
{
    CoupleProp::clear();
    angle   = 0;
    cosinus = 1;
    sinus   = 0;
    angular_stiffness = 0;
    flip    = true;
}


void ForkProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
    
    // compact syntax
    glos.set(angular_stiffness, "torque");
    glos.set(angle,             "torque", 1);

    // alternative syntax:
    glos.set(angle,             "angle");
    glos.set(angular_stiffness, "angular_stiffness");
    
    glos.set(flip, "flip");
}


void ForkProp::complete(Simul const& sim)
{
    CoupleProp::complete(sim);
    
    cosinus = cos(angle);
    sinus   = sin(angle);
    
#if ( 0 )
    if ( angle < 0 || sinus < 0 )
        throw InvalidParameter("The equilibrium angle should be defined in [0, pi]");
#endif

    if ( angular_stiffness < 0 )
        throw InvalidParameter("The angular stiffness, fork:torque[0] should be set and >= 0");
}


void ForkProp::write_values(std::ostream& os) const
{
    CoupleProp::write_values(os);
    write_value(os, "torque", angular_stiffness, angle);
    write_value(os, "flip", flip);
}

