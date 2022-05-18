// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "chewer.h"
#include "chewer_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


Chewer::Chewer(ChewerProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
    engaged = NO_END;
}


void Chewer::attach(FiberSite const& s)
{
    engaged = NO_END;
    Hand::attach(s);
}


void Chewer::stepUnloaded()
{
    assert_true( attached() );
    
    // test for detachment
    if ( testDetachment() )
        return;
    
    if ( engaged != NO_END )
    {
#if NEW_FIBER_CHEW
        fbFiber->chew(prop->chewing_speed_dt, engaged);
        moveToEnd(engaged);
#else
        throw InvalidParameter("fiber:chew is not enabled");
#endif
        return;
    }

    real a = fbAbs + prop->diffusion_dt * RNG.sreal();
    
    if ( a <= fbFiber->abscissaM() )
    {
        a = fbFiber->abscissaM();
        engaged = MINUS_END;
    }
    
    if ( a >= fbFiber->abscissaP() )
    {
        a = fbFiber->abscissaP();
        engaged = PLUS_END;
    }
    
    if ( engaged && RNG.test_not(prop->hold_growing_end) )
        detach();
    else
        moveTo(a);
}


void Chewer::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    if ( testKramersDetachment(force_norm) )
        return;
    
    if ( engaged != NO_END )
    {
#if NEW_FIBER_CHEW
        fbFiber->chew(prop->chewing_speed_dt, engaged);
        moveToEnd(engaged);
#else
        throw InvalidParameter("fiber:chew is not enabled");
#endif
        return;
    }
    
    // the load is the projection of the force on the local direction of Fiber
    real load = dot(force, dirFiber());
    real a = fbAbs + prop->diffusion_dt * RNG.sreal() + prop->mobility_dt * load;
    
    const real m = fbFiber->abscissaM();
    const real p = fbFiber->abscissaP();
    
    if ( a <= m )
    {
        a = m;
        engaged = MINUS_END;
    }
    
    if ( a >= p )
    {
        a = p;
        engaged = PLUS_END;
    }
    
    if ( engaged && RNG.test_not(prop->hold_growing_end) )
        detach();
    else
        moveTo(a);
}

