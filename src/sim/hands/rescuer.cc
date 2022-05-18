// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "rescuer.h"
#include "rescuer_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


Rescuer::Rescuer(RescuerProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}


/**
 Warning:
 This will only work if the time step is small such that only one Hand is
 affected at any time step by the shrinkage of the Fiber.
 Otherwise, the order in which the Hand are considered is random,
 and a distal Hand might be detached, even if the Fiber is rescued prior to this.
 
 This condition should be satisfied for Microtubule systems, since:
 - Shrinking speed ~ -0.1 um/second
 - time_step = 0.010 seconds
 -> shrinkage = 1 nm / time_step
 which would work for the density of 13 binding sites / 8 nm of microtubule.
 */
void Rescuer::handleDisassemblyM()
{
    assert_true( attached() );
    
    if ( RNG.test(prop->rescue_prob) )
    {
        Fiber * fib = fiber();
        assert_true( fbAbs < fbFiber->abscissaM() );
        // induce rescue:
        fib->setDynamicStateM(STATE_GREEN);
        // increase MT length to cover position of Hand
        fib->growM(fiber()->abscissaM()-fbAbs);
        // update all Hands:
        fib->updateHands();
    }
    else
        detach();
}

/**
 Warning:
 This will only work if the time step is small such that only one Hand is
 affected at any time step by the shrinkage of the Fiber.
 Otherwise, the order in which the Hand are considered is random,
 and a distal Hand might be detached, even if the Fiber is rescued prior to this.
 
 This condition should be satisfied for Microtubule systems, since:
 - Shrinking speed ~ -0.1 um/second
 - time_step = 0.010 seconds
 -> shrinkage = 1 nm / time_step
 which would work for the density of 13 binding sites / 8 nm of microtubule.
 */
void Rescuer::handleDisassemblyP()
{
    assert_true( attached() );
    
    if ( RNG.test(prop->rescue_prob) )
    {
        Fiber * fib = fiber();
        assert_true( fbAbs > fbFiber->abscissaP() );
        // induce rescue:
        fib->setDynamicStateP(STATE_GREEN);
        // increase MT length to cover position of Hand
        fib->growP(fbAbs-fiber()->abscissaP());
        // update all Hands:
        fib->updateHands();
    }
    else
        detach();
}


void Rescuer::stepUnloaded()
{
    assert_true( attached() );
    
    testDetachment();
}


void Rescuer::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    testKramersDetachment(force_norm);
}

