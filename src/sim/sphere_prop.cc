// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "sphere_prop.h"
#include "glossary.h"
#include "sphere.h"
#include "space.h"
#include "sim.h"

#include "sphere.h"
#include "property_list.h"
#include "simul_prop.h"
#include "space_prop.h"
#include "simul.h"


void SphereProp::clear()
{
    point_mobility    = -1;
    viscosity         = -1;
    piston_effect     = false;
    steric            = 0;
    steric_range      = 0;
    
    confine           = CONFINE_OFF;
    confine_stiffness = 0;
    confine_space     = "first";
    confine_space_ptr = nullptr;
    
    display           = "";
    display_fresh     = false;
}


void SphereProp::read(Glossary& glos)
{
    glos.set(point_mobility,  "point_mobility");
    glos.set(piston_effect,   "piston_effect");
    glos.set(viscosity,       "viscosity");
    
    glos.set(steric,          "steric");
    glos.set(steric_range,    "steric", 1);
 
    glos.set(confine, "confine", {{"off",        CONFINE_OFF},
                                  {"on",         CONFINE_ON},
                                  {"inside",     CONFINE_INSIDE},
                                  {"none",       CONFINE_OFF},
                                  {"surface",    CONFINE_ON},
                                  {"all_inside", CONFINE_ALL_INSIDE}});
    
    glos.set(confine_stiffness, "confine", 1);
    glos.set(confine_space,     "confine", 2);

    glos.set(confine_stiffness, "confine_stiffness");
    glos.set(confine_space,     "confine_space");

#ifdef BACKWARD_COMPATIBILITY
    if ( confine_space == "current" )
        confine_space = "last";

    glos.set(confine, "confined",{{"none",    CONFINE_OFF},
                                  {"inside",  CONFINE_INSIDE},
                                  {"surface", CONFINE_ON}});
    glos.set(confine_stiffness, "confined", 1);
#endif
    
    if ( glos.set(display, "display") )
        display_fresh = true;
}


void SphereProp::complete(Simul const& sim)
{
    if ( viscosity < 0 )
        viscosity = sim.prop->viscosity;
        
    if ( viscosity < 0 )
        throw InvalidParameter("sphere:viscosity or simul:viscosity should be defined");
    
    confine_space_ptr = sim.findSpace(confine_space);

    if ( confine_space_ptr )
        confine_space = confine_space_ptr->name();

    if ( sim.ready() && confine != CONFINE_OFF )
    {
        if ( !confine_space_ptr )
            throw InvalidParameter(name()+":confine_space `"+confine_space+"' was not found");

        if ( confine_stiffness < 0 )
            throw InvalidParameter(name()+":confine_stiffness must be specified and >= 0");
    }
    
    if ( point_mobility < 0 )
        throw InvalidParameter("sphere:point_mobility must be specified and >= 0");
}


void SphereProp::write_values(std::ostream& os) const
{
    write_value(os, "viscosity",      viscosity);
    write_value(os, "point_mobility", point_mobility);
    write_value(os, "piston_effect",  piston_effect);
    write_value(os, "steric",         steric, steric_range);
    write_value(os, "confine",        confine, confine_stiffness, confine_space);
    write_value(os, "display",        "("+display+")");
}

