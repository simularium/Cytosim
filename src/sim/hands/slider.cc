// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "slider.h"
#include "slider_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


Slider::Slider(SliderProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}


void Slider::stepUnloaded()
{
    assert_true( attached() );
    
    // spontaneous detachment:
    if ( testDetachment() )
        return;
    
    /// diffusion?
}


void Slider::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    
    real a = fbAbs + dot(force, dirFiber()) * prop->mobility_dt;
    
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

    assert_true( nextDetach >= 0 );
    if ( testKramersDetachment(force_norm) )
        return;

    // movement can lead to detachment, so we do it last:
    moveTo(a);
}

