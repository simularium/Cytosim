// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bundle_prop.h"
#include "property_list.h"
#include "glossary.h"
#include "simul.h"


void BundleProp::clear()
{
    stiffness  = -1;
    overlap    = -1;
    focus      = MINUS_END;
    fiber_rate = 0.0;
    fiber_type = "";
    fiber_spec = "";
}


void BundleProp::read(Glossary& glos)
{
    glos.set(stiffness,    "stiffness");
    glos.set(overlap,      "overlap");
    glos.set(focus, "focus", {{"plus_end", PLUS_END}, {"minus_end", MINUS_END}});
    glos.set(fiber_rate,   "nucleate");
    glos.set(fiber_type,   "nucleate", 1);
    glos.set(fiber_spec,   "nucleate", 2);
}


void BundleProp::complete(Simul const& sim)
{
    if ( fiber_rate < 0 )
        throw InvalidParameter("bundle:nucleation_rate (nucleate) must be >= 0");
    
    if ( fiber_rate > 0 )
    {
        if ( fiber_type.empty() )
            throw InvalidParameter("bundle:fibers (nucleate[1]) must be specified");
        
        // verify that fiber class exists:
        sim.properties.find_or_die("fiber", fiber_type);
    }

    fiber_prob = -std::expm1( -fiber_rate * sim.time_step() );

    if ( overlap < 0 )
        throw InvalidParameter("bundle:overlap must be specified and >= 0");
    
    if ( stiffness < 0 )
        throw InvalidParameter("bundle:stiffness must be specified and >= 0");
}


void BundleProp::write_values(std::ostream& os) const
{
    write_value(os, "stiffness", stiffness);
    write_value(os, "overlap",   overlap);
    write_value(os, "focus",     focus);
    write_value(os, "nucleate",  fiber_rate, fiber_type, "("+fiber_spec+")");
}

