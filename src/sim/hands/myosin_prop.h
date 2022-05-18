// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MYOSIN_PROP_H
#define MYOSIN_PROP_H

#include "digit_prop.h"


/// Additional Property for Myosin
/**
 @ingroup Properties
*/
class MyosinProp : public DigitProp
{
    friend class Myosin;
    
public:
    
    /**
     @defgroup MyosinPar Parameters of Myosin
     @ingroup Parameters
     Inherits @ref DigitPar.
     @{
     */

    /// force at which stepping rate becomes zero
    real    stall_force;
    
    /// speed if force=0 ( unloaded_speed = rate * step_size )
    /**
     A positive value specifies a plus-end directed motor.
     A negative value specifies a minus-end directed motor.
     */
    real    unloaded_speed;
    
    /// @}
    
private:
    
    real    var_rate_dt;
    real    walking_rate_dt;
    
public:

    /// constructor
    MyosinProp(const std::string& n) : DigitProp(n)  { clear(); }
    
    /// destructor
    ~MyosinProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor*) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new MyosinProp(*this); }
    
    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

