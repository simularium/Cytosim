// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "kinesin.h"
#include "kinesin_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "lattice.h"
#include "simul.h"


Kinesin::Kinesin(KinesinProp const* p, HandMonitor* h)
: Digit(p,h), prop(p)
{
    ABORT_NOW("the kinesin class is unfinished");
}


void Kinesin::attach(FiberSite const& s)
{
    Digit::attach(s);
    nextStep = RNG.exponential();
}


/**
 \todo simulate occurence of backward steps
 */
void Kinesin::stepUnloaded()
{
    assert_true( attached() );
    
    nextStep -= prop->walking_rate_dt;
    
    while ( nextStep <= 0 )
    {
        assert_true( attached() );
        lati_t s = site() + 1;
        if ( outsideMP(s) )
        {
            //immediately detach at the end of the Fiber:
            detach();
            return;
        }
        if ( vacant(s) )
            hop(s);
        nextStep += RNG.exponential();
    }
    
    testDetachment();
}


/**
 Currently, antagonistic force only reduced the rate of forward stepping.
 However, force is also known to increase the rate of backward steps.
 \todo simulate occurence of backward steps
 */
void Kinesin::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    
    // calculate displacement, dependent on the load along the desired direction of displacement
    real R = prop->walking_rate_dt + dot(force, dirFiber()) * prop->var_rate_dt;

    nextStep -= std::max((real)0, R);

    while ( nextStep <= 0 )
    {
        assert_true( attached() );
        lati_t s = site() + 1;
        if ( outsideMP(s) )
        {
            //immediately detach at the end of the Fiber:
            detach();
            return;
        }
        if ( vacant(s) )
            hop(s);
        nextStep += RNG.exponential();
    }
    
    assert_true( nextDetach >= 0 );
    testKramersDetachment(force_norm);
}

