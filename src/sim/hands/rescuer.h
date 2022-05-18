// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef RESCUER_H
#define RESCUER_H

#include "hand.h"
class RescuerProp;

/// A Hand that may rescue a shrinking fiber.
/**
 The Rescuer is a Hand, and as such can bind and unbind from fibers.
 
 A bound rescuer has the ability to switch a shrinking fiber to a growing state.
 This may occur if the shrinking end of the fiber is reaching the position of the Rescuer.
 A this time, the probability @ref WalkerPar "rescue_prob" is tested and:
 - if the test passes, the fiber nearest end state is set to STATE_GREEN.
 - otherwise the Rescuer detaches.
 .
 
 The parameter @ref WalkerPar "rescue_prob" is a one-shot probability and 
 is not dependent on `time_step`.

 Note: While inducing a rescue, the Rescuer may be pushed a bit toward the MINUS_END,
 due to the discretization in time. Smaller `time_step` minimize this effect. 
 
 See Examples and the @ref RescuerPar.
 @ingroup HandGroup
 */
class Rescuer : public Hand
{
private:
    
    /// disabled default constructor
    Rescuer();
    
public:
    
    /// Property
    RescuerProp const* prop;
    
    /// constructor
    Rescuer(RescuerProp const*, HandMonitor*);
    
    /// destructor
    ~Rescuer() {}
    
    /// this is called when the attachment point is beyond the PLUS_END
    void   handleDisassemblyM();
    
    /// this is called when the attachment point is below the MINUS_END
    void   handleDisassemblyP();

    /// simulate when `this` is attached but not under load
    void   stepUnloaded();
    
    /// simulate when `this` is attached and under load
    void   stepLoaded(Vector const& force, real force_norm);
    
};

#endif

