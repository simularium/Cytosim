// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DYNEIN_H
#define DYNEIN_H

#include "digit.h"
class DyneinProp;


/// A model of the dynein motor with discrete stepping
/**
 THIS CLASS IS A STUB and should not be used!
 
 Dynein is derived from Digit, and it makes discrete jumps along the fiber.
 
 Stepping is stochastic.
 
 See Examples and the @ref DyneinPar.
 @ingroup HandGroup
 
 @todo implement Dynein
*/
class Dynein : public Digit
{
private:
    
    /// disabled default constructor
    Dynein();
    
    /// Gillespie countdown timer for stepping
    real   nextStep;

public:
    
    /// Property
    DyneinProp const* prop;
    
    /// constructor
    Dynein(DyneinProp const*, HandMonitor*);
    
    /// destructor
    ~Dynein() {}

    /// attach and update variables
    void   attach(FiberSite const&);

    /// simulate when `this` is attached but not under load
    void   stepUnloaded();
    
    /// simulate when `this` is attached and under load
    void   stepLoaded(Vector const& force, real force_norm);
    
};

#endif

