// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "tracker_prop.h"
#include "tracker.h"


Hand * TrackerProp::newHand(HandMonitor* m) const
{
    return new Tracker(this, m);
}


void TrackerProp::clear()
{
    HandProp::clear();

    track_end             = NO_END;
    bind_only_growing_end = false;
}


void TrackerProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(track_end,  "track_end", {{"off",       NO_END},
#ifdef BACKWARD_COMPATIBILITY
                                       {"none",      NO_END},
#endif
                                       {"plus_end",  PLUS_END},
                                       {"minus_end", MINUS_END},
                                       {"both_ends", BOTH_ENDS}});

    glos.set(bind_only_growing_end, "bind_only_growing_end");
}


void TrackerProp::complete(Simul const& sim)
{
    HandProp::complete(sim);
}


void TrackerProp::write_values(std::ostream& os) const
{
    HandProp::write_values(os);
    write_value(os, "track_end",             track_end);
    write_value(os, "bind_only_growing_end", bind_only_growing_end);
}

