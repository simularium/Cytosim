// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "solid_prop.h"
#include "glossary.h"
#include "property_list.h"
#include "simul_prop.h"
#include "simul.h"
#include "sim.h"


void SolidProp::clear()
{
    drag              = -1;
    viscosity         = -1;
    steric            = 0;
    steric_range      = 0;
    
    confine           = CONFINE_OFF;
    confine_stiffness = 0;
    confine_space     = "first";
    confine_space_ptr = nullptr;
    
#if NEW_RADIAL_FLOW
    flow_time[0] = 0;
    flow_time[1] = 0;
    flow_center.reset();
#endif
#if NEW_SOLID_CLAMP
    clamp_stiff  = 0;
    clamp_pos.reset();
#endif

    display           = "";
    display_fresh     = false;
}


void SolidProp::read(Glossary& glos)
{
    glos.set(drag,           "drag");
    glos.set(viscosity,      "viscosity");
    
    glos.set(steric,         "steric");
    glos.set(steric_range,   "steric", 1);
    
    glos.set(confine,        "confine", {{"off",        CONFINE_OFF},
                                         {"on",         CONFINE_ON},
                                         {"inside",     CONFINE_INSIDE},
                                         {"outside",    CONFINE_OUTSIDE},
                                         {"point",      CONFINE_POINT},
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
    
#if NEW_RADIAL_FLOW
    glos.set(flow_time,  2, "flow_time");
    glos.set(flow_center,   "flow_center");
    if ( flow_time[0] > flow_time[1] )
        throw InvalidParameter("flow_time[0] should be lower than flow_time[1]");
#endif
#if NEW_SOLID_CLAMP
    glos.set(clamp_pos,    "clamp");
    glos.set(clamp_stiff,  "clamp", 1);
    if ( clamp_stiff < 0 )
        throw InvalidParameter("clamp[0] (stiffness) should be >= 0");
#endif

    if ( glos.set(display, "display") )
        display_fresh = true;
}


void SolidProp::complete(Simul const& sim)
{
    if ( viscosity < 0 )
        viscosity = sim.prop->viscosity;
    
    if ( viscosity < 0 )
        throw InvalidParameter("bead:viscosity or simul:viscosity should be defined");
    
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
}


void SolidProp::write_values(std::ostream& os) const
{
    if ( drag > 0 )  write_value(os, "drag", drag);
    write_value(os, "viscosity", viscosity);
    write_value(os, "steric",    steric, steric_range);
    write_value(os, "confine",   confine, confine_stiffness, confine_space);
#if NEW_RADIAL_FLOW
    write_value(os, "flow_center", flow_center);
    write_value(os, "flow_time",   flow_time, 2);
#endif
#if NEW_SOLID_CLAMP
    write_value(os, "clamp", clamp_pos, clamp_stiff);
#endif
    write_value(os, "display",   "("+display+")");
}

