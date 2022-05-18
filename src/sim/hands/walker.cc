// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "digit.h"
#include "walker.h"
#include "walker_prop.h"
#include "glossary.h"
#include "lattice.h"
#include "simul.h"


Walker::Walker(WalkerProp const* p, HandMonitor* h)
: Digit(p,h), nextStep(0), prop(p)
{
}


void Walker::attach(FiberSite const& s)
{
    Digit::attach(s);
    nextStep = RNG.exponential();
    
#if ( 0 )
    // this allows for step size being a multiple of lattice site
    unsigned n = std::round( prop->step_size / lattice()->unit() );
    stride = std::copysign(n, prop->unloaded_speed);
#else
    // here digit::step_size must be equal to fiber:step_size
    if ( lattice() && lattice()->unit() != prop->step_size  )
        throw InvalidParameter("digit:step_size must be equal to fiber:lattice_unit");
    stride = std::copysign(1, prop->unloaded_speed);
#endif
}


/**
 Currently, the Walker only makes forward steps, but backward steps exist as well.
 \todo simulate occurence of backward steps
 */
void Walker::stepUnloaded()
{
    assert_true( attached() );
    
    nextStep -= prop->walking_rate_dt;
    
    while ( nextStep <= 0 )
    {
        // test detachment due to stepping
        if ( RNG.test(prop->unbinding_chance) )
        {
            detach();
            return;
        }
        
        lati_t s = site() + stride;
        
        if ( outsideMP(s) )
        {
            if ( RNG.test_not(prop->hold_growing_end) )
            {
                detach();
                return;
            }
        }
        else if ( vacant(s) )
            hop(s);
    
        nextStep += RNG.exponential();
    }
    
    testDetachment();
}


/**
 Currently, antagonistic force only reduces the rate of forward stepping.
 However, force is also known to increase the rate of backward steps.
 \todo simulate occurence of backward steps in Walker
 */
void Walker::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    
    // calculate displacement, dependent on the load along the desired direction of displacement
    real R = prop->walking_rate_dt + dot(force, dirFiber()) * prop->var_rate_dt;

    nextStep -= std::max((real)0, R);
    
    while ( nextStep <= 0 )
    {
        // test detachment due to stepping
        if ( RNG.test(prop->unbinding_chance) )
        {
            detach();
            return;
        }

        lati_t s = site() + stride;

        if ( outsideMP(s) )
        {
            if ( RNG.test_not(prop->hold_growing_end) )
            {
                detach();
                return;
            }
        }
        else if ( vacant(s) )
            hop(s);
        
        nextStep += RNG.exponential();
    }
    
    assert_true( nextDetach >= 0 );
    if ( prop->unbinding_force_inv > 0 )
        testKramersDetachment(force_norm);
    else
        testDetachment();
}

