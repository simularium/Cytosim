// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "property_list.h"
#include "cutter_prop.h"
#include "cutter.h"
#include "simul.h"


Hand * CutterProp::newHand(HandMonitor* m) const
{
    return new Cutter(this, m);
}


void CutterProp::clear()
{
    HandProp::clear();

    cutting_rate = 0;
    new_end_state[0] = STATE_WHITE;
    new_end_state[1] = STATE_WHITE;
}


void CutterProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(cutting_rate,  "cutting_rate");
    
    // possible dynamic states of the ends
    Glossary::dict_type<state_t> keys{{"white",     STATE_WHITE},
                                      {"green",     STATE_GREEN},
                                      {"yellow",    STATE_YELLOW},
                                      {"orange",    STATE_ORANGE},
                                      {"red",       STATE_RED},
                                      {"static",    STATE_WHITE},
                                      {"growing",   STATE_GREEN},
                                      {"shrinking", STATE_RED},
                                      {"delete",    STATE_BLACK}};
    
    glos.set(new_end_state[0], "new_end_state", keys);
    glos.set(new_end_state[1], "new_end_state", 1, keys);
}


void CutterProp::complete(Simul const& sim)
{
    HandProp::complete(sim);
    
    if ( cutting_rate < 0 )
        throw InvalidParameter("cutter:cutting_rate must be >= 0");

    cutting_rate_dt = cutting_rate * sim.time_step();
    
    if ( new_end_state[0] == STATE_BLACK )
        throw InvalidParameter("cutter:new_end_state[0] is invalid");
}


void CutterProp::write_values(std::ostream& os) const
{
    HandProp::write_values(os);
    write_value(os, "cutting_rate",  cutting_rate);
    write_value(os, "new_end_state", new_end_state, 2);
}

