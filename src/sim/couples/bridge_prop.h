// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BRIDGE_PROP_H
#define BRIDGE_PROP_H

#include "couple_prop.h"


/// Additional Property for Bridge
/**
 @ingroup Properties
*/
class BridgeProp : public CoupleProp
{
    
    friend class Bridge;
    
public:
    
    /**
     @defgroup BridgePar Parameters of Bridge
     @ingroup Parameters
     Inherits @ref CouplePar.
     @{
     */
    
    /// @}
    

public:
    
    /// constructor
    BridgeProp(const std::string& n) : CoupleProp(n)  { clear(); }
    
    /// destructor
    ~BridgeProp() { }
    
    /// return a Hand with this property
    Couple * newCouple(Glossary*) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new BridgeProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

