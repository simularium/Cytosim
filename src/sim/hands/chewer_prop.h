// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CHEWER_PROP_H
#define CHEWER_PROP_H

#include "hand_prop.h"


/// Additional Property for Chewer
/**
 @ingroup Properties
 */
class ChewerProp : public HandProp
{
    friend class Chewer;
    
public:
    
    /**
     @defgroup ChewerPar Parameters of Chewer
     @ingroup Parameters
     Inherits @ref HandPar.
     @{
     */
    
    /// length of Fiber eaten per unit time, once the engaged with an end.
    real    chewing_speed;
    
    /// unidimensional diffusion coefficient while bound to the Fiber
    real    diffusion;
    
    /// @}
    
private:
    
    real chewing_speed_dt;

    real diffusion_dt, mobility_dt;
    
public:
    
    /// constructor
    ChewerProp(const std::string& n) : HandProp(n)  { clear(); }
    
    /// destructor
    ~ChewerProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor*) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(Simul const&);
    
    /// perform additional tests for the validity of parameters, given the elasticity
    void checkStiffness(real stiff, real len, real mul, real kT) const;

    /// return a carbon copy of object
    Property* clone() const { return new ChewerProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

