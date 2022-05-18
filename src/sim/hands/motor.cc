// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "motor.h"
#include "motor_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


Motor::Motor(MotorProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}


void Motor::stepUnloaded()
{
    assert_true( attached() );
    real a = fbAbs + prop->set_speed_dt;

    if ( a < fbFiber->abscissaM() )
    {
        if ( RNG.test_not(prop->hold_growing_end) )
        {
            detach();
            return;
        }
        a = fbFiber->abscissaM();
    }
    
    if ( a > fbFiber->abscissaP() )
    {
        if ( RNG.test_not(prop->hold_growing_end) )
        {
            detach();
            return;
        }
        a = fbFiber->abscissaP();
    }

    if ( !testDetachment() )
        moveTo(a);
}


void Motor::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    
    // the load is the projection of the force on the local direction of Fiber
    real load = dot(force, dirFiber());
    
    // calculate load-dependent displacement:
    real dab = prop->set_speed_dt + load * prop->var_speed_dt;

    // possibly limit the range of the speed:
    if ( prop->limit_speed )
    {
        dab = std::max(dab, prop->min_dab);
        dab = std::min(dab, prop->max_dab);
    }
    
    real a = fbAbs + dab;
    
    if ( a < fbFiber->abscissaM() )
    {
        if ( RNG.test_not(prop->hold_growing_end) )
        {
            detach();
            return;
        }
        a = fbFiber->abscissaM();
    }
    
    if ( a > fbFiber->abscissaP() )
    {
        if ( RNG.test_not(prop->hold_growing_end) )
        {
            detach();
            return;
        }
        a = fbFiber->abscissaP();
    }
    
    if ( testKramersDetachment(force_norm) )
        return;

    //std::cerr << this << " > " << fbAbs << "  " << dab << "\n";
    moveTo(a);
}

