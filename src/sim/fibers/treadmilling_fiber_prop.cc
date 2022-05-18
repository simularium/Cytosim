// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include <cmath>
#include "treadmilling_fiber_prop.h"
#include "treadmilling_fiber.h"
#include "property_list.h"
#include "exceptions.h"
#include "glossary.h"
#include "simul.h"


Fiber* TreadmillingFiberProp::newFiber() const
{
    return new TreadmillingFiber(this);
}


void TreadmillingFiberProp::clear()
{
    FiberProp::clear();
    
    for ( int i = 0; i < 2; ++i )
    {
        growing_force[i]   = INFINITY;
        growing_speed[i]   = 0;
        shrinking_speed[i] = 0;
    }
}


void TreadmillingFiberProp::read(Glossary& glos)
{
    FiberProp::read(glos);
    
    glos.set(growing_speed,   2, "growing_speed");
    glos.set(growing_force,   2, "growing_force");
    glos.set(shrinking_speed, 2, "shrinking_speed");
}


void TreadmillingFiberProp::complete(Simul const& sim)
{
    FiberProp::complete(sim);

    for ( int i = 0; i < 2; ++i )
    {
        if ( growing_force[i] <= 0 )
            throw InvalidParameter("fiber:growing_force should be > 0");
        if ( growing_speed[i] < 0 )
            throw InvalidParameter("fiber:growing_speed should be >= 0");
        
        growing_speed_dt[i]   = growing_speed[i] * sim.time_step();
        
        if ( shrinking_speed[i] > 0 )
            throw InvalidParameter("fiber:shrinking_speed should be <= 0");
        shrinking_speed_dt[i] = shrinking_speed[i] * sim.time_step();
    }
}


void TreadmillingFiberProp::write_values(std::ostream& os) const
{
    FiberProp::write_values(os);
    write_value(os, "growing_force",   growing_force, 2);
    write_value(os, "growing_speed",   growing_speed, 2);
    write_value(os, "shrinking_speed", shrinking_speed, 2);
}

