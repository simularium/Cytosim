// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef TRACKER_PROP_H
#define TRACKER_PROP_H

#include "common.h"
#include "hand_prop.h"

/// additional Property for Tracker
/**
 @ingroup Properties
 */
class TrackerProp : public HandProp
{
    friend class Tracker;
        
public:
    
    /**
     @defgroup TrackerPar Parameters of Tracker
     @ingroup Parameters
     Inherits @ref HandPar.
     @{
     */
    
    /// if set, always follow specified end `[minus-end, plus_end, nearest_end]`
    /**
     The hand will stay always positionned at the given fiber end, 
     even if this end is growing or shrinking. Possible values are:
        - plus_end
        - minus_end
        - nearest_end
        .
     */
    FiberEnd   track_end;
    
    /// if true, bind only to growing fiber ends
    bool       bind_only_growing_end;

    /// @}

public:
        
    /// constructor
    TrackerProp(const std::string& n) : HandProp(n)  { clear(); }
   
    /// destructor
    ~TrackerProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor*) const;
    
    /// set default values
    void clear();
        
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new TrackerProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
   
};

#endif

