// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CROSSLINK_PROP_H
#define CROSSLINK_PROP_H

#include "couple_prop.h"


/// Additional Property for Crosslink and CroslinkLong
/**
 @ingroup Properties
*/
class CrosslinkProp : public CoupleProp
{
    
    friend class Crosslink;
    friend class CrosslinkLong;
    
public:
    
    /**
     @defgroup CrosslinkPar Parameters of Crosslink
     @ingroup Parameters
     Inherits @ref CouplePar.
     @{
     */
    
    
    /// @}

public:
    
    /// constructor
    CrosslinkProp(const std::string& n) : CoupleProp(n)  { clear(); }
    
    /// destructor
    ~CrosslinkProp() { }
    
    /// return a Crosslink or a CrosslinkLong with this property
    Couple * newCouple(Glossary*) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new CrosslinkProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

