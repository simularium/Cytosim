// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MYOSIN_H
#define MYOSIN_H

#include "digit.h"
class MyosinProp;


/// A model of the myosin motor with discrete stepping
/**
 THIS CLASS IS A STUB and should not be used!
 
 Myosin is derived from Digit, and it makes discrete jumps along the fiber.
 
 Stepping is stochastic.
 
 See Examples and the @ref MyosinPar.
 @ingroup HandGroup
 
 @todo implement Myosin
*/
class Myosin : public Digit
{
private:
    
    /// disabled default constructor
    Myosin();
    
    /// Gillespie countdown timer for stepping
    real   nextStep;

public:
    
    /// Property
    MyosinProp const* prop;
    
    /// constructor
    Myosin(MyosinProp const*, HandMonitor*);
    
    /// destructor
    ~Myosin() {}

    /// attach and update variables
    void   attach(FiberSite const&);

    /// simulate when `this` is attached but not under load
    void   stepUnloaded();
    
    /// simulate when `this` is attached and under load
    void   stepLoaded(Vector const& force, real force_norm);
    
};

#endif

