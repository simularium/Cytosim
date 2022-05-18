// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "tracker.h"
#include "tracker_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


Tracker::Tracker(TrackerProp const* p, HandMonitor* h)
: Hand(p, h), prop(p)
{
}


bool Tracker::attachmentAllowed(FiberSite& sit) const
{
    if ( !Hand::attachmentAllowed(sit) )
        return false;

    // check if fiber end in growing:
    if ( prop->bind_only_growing_end && !sit.fiber()->isGrowing(sit.nearestEnd()) )
        return false;
    
    return true;
}

//------------------------------------------------------------------------------
#pragma mark -


void Tracker::stepUnloaded()
{
    assert_true( attached() );
    
    // detachment
    if ( testDetachment() )
        return;

    
    switch ( prop->track_end )
    {
        case NO_END:
            break;
            
        case PLUS_END:
            relocateP();
            break;
            
        case MINUS_END:
            relocateM();
            break;
            
        case BOTH_ENDS:
            moveToEnd(nearestEnd());
            break;
            
        default:
            throw InvalidParameter("invalid value of tracker:track_end");
    }
}


void Tracker::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    
    assert_true( nextDetach >= 0 );
    if ( testKramersDetachment(force_norm) )
        return;

    switch ( prop->track_end )
    {
        case NO_END:
            break;
            
        case PLUS_END:
            relocateP();
            break;
            
        case MINUS_END:
            relocateM();
            break;
            
        case BOTH_ENDS:
            moveToEnd(nearestEnd());
            break;
            
        default:
            throw InvalidParameter("invalid value of tracker:track_end");
    }
}


