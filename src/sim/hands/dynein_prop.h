// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef DYNEIN_PROP_H
#define DYNEIN_PROP_H

#include "digit_prop.h"


/// Additional Property for Dynein
/**
 @ingroup Properties
*/
class DyneinProp : public DigitProp
{
    friend class Dynein;
    
public:
    
    /**
     @defgroup DyneinPar Parameters of Dynein
     @ingroup Parameters
     Inherits @ref DigitPar.
     @{
     */

    /// force at which stepping rate becomes zero
    /**
     Yeast Dynein's stall force and detachment force were measured in:
     Force-Induced Bidirectional Stepping of Cytoplasmic Dynein
     Gennerich et al. Cell 2007
     http://dx.doi.org/10.1016/j.cell.2007.10.016
     
     In short:
     - Figure 1: stall_force = 7 pN
     - Figure 5: unbinding_force = 5.25 pN
     .
     */
    real    stall_force;
    
    /// speed if force=0 ( unloaded_speed = rate * step_size )
    /**
     A positive value specifies a plus-end directed motor.
     A negative value specifies a minus-end directed motor.
     
     Yeast Dynein's displacement speed were measured in:
     Force-Induced Bidirectional Stepping of Cytoplasmic Dynein
     Gennerich et al. Cell 2007
     http://dx.doi.org/10.1016/j.cell.2007.10.016
     
     unloaded_speed = -0.06 um/s
     */
    real    unloaded_speed;
    
    /// @}
    
private:
    
    real    var_rate_dt;
    real    walking_rate_dt;
    
public:

    /// constructor
    DyneinProp(const std::string& n) : DigitProp(n)  { clear(); }
    
    /// destructor
    ~DyneinProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor*) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new DyneinProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

