// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef REGULATOR_PROP_H
#define REGULATOR_PROP_H

#include "hand_prop.h"


/// Additional Property for Regulator
/**
 @ingroup Properties
 */
class RegulatorProp : public HandProp
{
    
    friend class Regulator;
    
public:
    
    /**
     @defgroup RegulatorPar Parameters of Regulator
     @ingroup Parameters
     Inherits @ref HandPar.
     @{
     */
    
    /// rate for something the regulator does
    /**
     This parameter is not used
     */
    real    rate;
    
    /// @}
    
    /// derived variable
    real    rate_dt;
    
public:
    
    /// constructor
    RegulatorProp(const std::string& n) : HandProp(n)  { clear(); }
    
    /// destructor
    ~RegulatorProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor*) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new RegulatorProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

