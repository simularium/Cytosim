// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CHEWER_H
#define CHEWER_H

#include "hand.h"
class ChewerProp;

/// A Hand that can eat Fibers at their ends
/**
 The Chewer can bind and unbind from fibers. It will diffuse on the lattice,
 (set by `diffusion`) and it will induce depolymerization at the end of the Fiber.
 To be active in depolymerization, it needs to reach the end by 1D diffusion,
 after binding onto the lattice. It is equally active on both ends.
 The speed of polymer destruction is set by @ref ChewerPar `chewing_speed`.
 
 See Examples and the @ref ChewerPar.
 
 This model is inspired from:
 <em>
 <b>The kinesin-related protein MCAK is a microtubule depolymerase that forms 
 an ATP-hydrolyzing complex at microtubule ends.</b>\n
 Hunter, A. W. et al.  Mol Cell 11, 445–457 (2003).
 http://dx.doi.org/10.1016/S1097-2765(03)00049-2
 </em>


 @ingroup HandGroup
 */
class Chewer : public Hand
{
private:
    
    /// the fiber end that the Chewer has reached, or NO_END
    FiberEnd engaged;
    
    /// disabled default constructor
    Chewer();

public:
    
    /// Property
    ChewerProp const* prop;
    
    /// constructor
    Chewer(ChewerProp const*, HandMonitor*);
    
    /// destructor
    ~Chewer() {}
    
    
    /// attach and update variables
    void   attach(FiberSite const&);

    /// simulate when `this` is attached but not under load
    void   stepUnloaded();
    
    /// simulate when `this` is attached and under load
    void   stepLoaded(Vector const& force, real force_norm);
    
};

#endif

