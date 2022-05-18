// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "myosin.h"
#include "myosin_prop.h"
#include "exceptions.h"
#include "glossary.h"
#include "simul.h"


Hand * MyosinProp::newHand(HandMonitor* m) const
{
    return new Myosin(this, m);
}


void MyosinProp::clear()
{
    DigitProp::clear();

    stall_force     = 0;
    unloaded_speed  = 0;
    walking_rate_dt = 0;
    var_rate_dt     = 0;
}


void MyosinProp::read(Glossary& glos)
{
    DigitProp::read(glos);
    
    glos.set(stall_force,    "stall_force")    || glos.set(stall_force,    "force");
    glos.set(unloaded_speed, "unloaded_speed") || glos.set(unloaded_speed, "speed");
#ifdef BACKWARD_COMPATIBILITY
    glos.set(unloaded_speed, "max_speed");
#endif
}


void MyosinProp::complete(Simul const& sim)
{
    DigitProp::complete(sim);
   
    if ( sim.ready() && stall_force <= 0 )
        throw InvalidParameter("myosin:stall_force must be > 0");
    
    if ( unloaded_speed < 0 )
        throw InvalidParameter("myosin:unloaded_speed must be >= 0");

    walking_rate_dt = sim.time_step() * fabs(unloaded_speed) / step_size;
    var_rate_dt     = std::copysign(walking_rate_dt/stall_force, unloaded_speed);
}


void MyosinProp::write_values(std::ostream& os) const
{
    DigitProp::write_values(os);
    write_value(os, "stall_force",    stall_force);
    write_value(os, "unloaded_speed", unloaded_speed);
}

