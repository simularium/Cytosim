// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "property_list.h"
#include "nucleator_prop.h"
#include "nucleator.h"
#include "simul.h"

//------------------------------------------------------------------------------
Hand * NucleatorProp::newHand(HandMonitor* m) const
{
    return new Nucleator(this, m);
}

//------------------------------------------------------------------------------
void NucleatorProp::clear()
{
    HandProp::clear();

    rate         = 0;
    fiber_type   = "";
    fiber_spec   = "";
    specificity  = NUCLEATE_ORIENTATED;
    track_end    = NO_END;
    hold_end     = MINUS_END;
    addictive    = false;
    nucleation_angle = 0;
}

//------------------------------------------------------------------------------
void NucleatorProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(rate,       "rate");
    glos.set(fiber_type, "fibers");
    glos.set(fiber_spec, "fibers", 1);
    
    glos.set(rate,       "nucleate", 0);
    glos.set(fiber_type, "nucleate", 1);
    glos.set(fiber_spec, "nucleate", 2);

    glos.set(nucleation_angle, "nucleation_angle");

#ifdef BACKWARD_COMPATIBILITY
    glos.set(fiber_spec, "nucleation_spec");
    glos.set(fiber_spec, "spec");
#endif
    
    glos.set(addictive,  "addictive");
    
    glos.set(hold_end,   "hold_end", {{"off",       NO_END},
#ifdef BACKWARD_COMPATIBILITY
                                      {"none",      NO_END},
#endif
                                      {"minus_end", MINUS_END},
                                      {"plus_end",  PLUS_END}});

    glos.set(track_end,  "track_end", {{"off",       NO_END},
#ifdef BACKWARD_COMPATIBILITY
                                       {"none",      NO_END},
#endif
                                       {"minus_end", MINUS_END},
                                       {"plus_end",  PLUS_END}});

#ifdef BACKWARD_COMPATIBILITY
    glos.set(specificity, "specificity", {{"off",          NUCLEATE_ORIENTATED},
                                          {"none",         NUCLEATE_ORIENTATED},
                                          {"parallel",     NUCLEATE_PARALLEL},
                                          {"antiparallel", NUCLEATE_ANTIPARALLEL},
                                          {"parallel_if",  NUCLEATE_PARALLEL_IF}});
    if ( specificity == NUCLEATE_PARALLEL || specificity == NUCLEATE_PARALLEL_IF )
        throw InvalidParameter("Please use nucleation_angle = 0 instead of specificity == parallel");
    if ( specificity == NUCLEATE_ANTIPARALLEL )
        throw InvalidParameter("Please use nucleation_angle = 3.1415 instead of specificity == antiparallel");
#endif

}

//------------------------------------------------------------------------------
void NucleatorProp::complete(Simul const& sim)
{
    HandProp::complete(sim);

    if ( fiber_type.empty() )
        throw InvalidParameter("hand:nucleate[1] (=fiber_type) must be specified if activity=nucleate");

    sim.properties.find_or_die("fiber", fiber_type);
    
    if ( rate < 0 )
        throw InvalidParameter("hand:nucleate (=rate) must be positive");

    if ( track_end && track_end != hold_end )
        throw InvalidParameter("if set, hand:track_end should be equal to hold_end");
        
    rate_dt = rate * sim.time_step();
}


//------------------------------------------------------------------------------

void NucleatorProp::write_values(std::ostream& os) const
{
    HandProp::write_values(os);
    write_value(os, "nucleate",  rate, fiber_type, "("+fiber_spec+")");
    write_value(os, "nucleation_angle", nucleation_angle);
    write_value(os, "hold_end",  hold_end);
    write_value(os, "track_end", track_end);
    write_value(os, "addictive", addictive);
}

