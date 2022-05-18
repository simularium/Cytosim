// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "cutter.h"
#include "cutter_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"

//------------------------------------------------------------------------------

Cutter::Cutter(CutterProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
    gspTime = RNG.exponential();
}


void Cutter::cut()
{
    assert_true( attached() );

    /**
     Cutting the fiber can invalidate the FiberGrid used for attachment,
     and this becomes a problem if the Cutter is part of a Couple,
     because calls for attachments and actions are intermingled.
     
     This is why the call sever() here only register the position of the cut,
     that will be performed later.
     */
    fiber()->sever(abscissa(), prop->new_end_state[0], prop->new_end_state[1]);
    detach();
}

//------------------------------------------------------------------------------

void Cutter::stepUnloaded()
{
    assert_true( attached() );
    
    // test for detachment
    if ( testDetachment() )
        return;

    gspTime -= prop->cutting_rate_dt;
    
    if ( gspTime < 0 )
    {
        gspTime = RNG.exponential();
        cut();
    }
}


void Cutter::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    if ( testKramersDetachment(force_norm) )
        return;
    
    gspTime -= prop->cutting_rate_dt;
    
    if ( gspTime < 0 )
    {
        gspTime = RNG.exponential();
        cut();
    }
}

