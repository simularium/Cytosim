// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SLIDER_H
#define SLIDER_H

#include "hand.h"
class SliderProp;

/// A Hand that will move on a Fiber passively with viscous resistance
/**
 The Slider is a Hand, and can thus bind and unbind from fibers.
 
 A bound Slider will move on the fiber it is pulled by external force.
 The Slider is a passive element that may move which staying attached to the fiber.
 
 The @ref SliderPar "mobility" defines the ratio between speed and force
 
     real load = force * direction_of_fiber;
     real displacement = load * mobility * time_step;
 
 See Examples and the @ref SliderPar.
 @ingroup HandGroup 
 */
class Slider : public Hand
{
private:
    
    /// disabled default constructor
    Slider();
    
public:
    
    /// Property
    SliderProp const* prop;
    
    /// constructor
    Slider(SliderProp const*, HandMonitor*);

    /// destructor
    ~Slider() {}

    
    /// simulate when `this` is attached but not under load
    void   stepUnloaded();

    /// simulate when `this` is attached and under load
    void   stepLoaded(Vector const & force, real force_norm);
    
};

#endif

