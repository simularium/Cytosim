// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "event.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"


void Event::clear()
{
    activity = "";
    rate = 0;
    delay = 0;
    recurrent = true;
    nextTime = 0;
}


void Event::fire_at(real time)
{
    nextTime = time;
    recurrent = false;
}


void Event::reload(real now)
{
    if ( recurrent )
    {
        if ( rate > 0 )
            nextTime = now + RNG.exponential() / rate;
        else
            nextTime = now + delay;
    }
    else
    {
        nextTime = INFINITY;
    }
}


Event::Event(real now, Glossary& opt)
{
    real t = now;
    clear();
    opt.set(activity, "activity") || opt.set(activity, "code");
    if ( opt.set(t, "time") )
    {
        fire_at(t);
    }
    else
    {
        opt.set(rate, "rate") || opt.set(delay, "delay");
        if ( rate < 0 )
            throw InvalidParameter("event:rate must be >= 0");
        if ( delay < 0 )
            throw InvalidParameter("event:delay must be >= 0");
        if ( rate <= 0 && delay <= 0 )
            throw InvalidParameter("event:rate or delay must be > 0");
        reload(now);
    }
}


Event::~Event()
{
    //Cytosim::log("destroying Event %p\n", this);
}


void Event::step(Simul& sim)
{
    if ( sim.time() >= nextTime )
    {
        sim.relax();
        do {
            reload(nextTime);
            sim.evaluate(activity);
        } while ( sim.time() >= nextTime );
        sim.unrelax();
    }
}


void Event::write(Outputter& out) const
{
}


void Event::read(Inputter& in, Simul& sim, ObjectTag tag)
{
}
