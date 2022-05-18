// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "actor.h"
#include "actor_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"

//------------------------------------------------------------------------------

Actor::Actor(ActorProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
    throw InvalidParameter("the actor class in unfinished");
}


//------------------------------------------------------------------------------

void Actor::stepUnloaded()
{
    assert_true( attached() );

    // test for detachment
    testDetachment();
}


void Actor::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    if ( testKramersDetachment(force_norm) )
        return;

    // do something:
}

