// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef HAND_PROP
#define HAND_PROP

#include "real.h"
#include "common.h"
#include "property.h"


/// enables "bind_only_free_end" to limit binding of Hands to Fibers
#define NEW_BIND_ONLY_FREE_END 0

class Hand;
class HandMonitor;
class PointDisp;

/// Property for Hand
/**
 @ingroup Properties
*/
class HandProp : public Property
{
    friend class Hand;
    
public:
      
    /// return one of the Property derived from HandProp
    static HandProp * newProperty(const std::string& n, Glossary&);
    
public:
    
    /**
     @defgroup HandPar Parameters of Hand
     @ingroup Parameters
     @{
     */
    
    /// rate of attachment when the Hand is within `binding_range` (also known as `binding[0]`)
    /**
     This has units of 1/second.
     The molecular binding_rate of conventional kinesin is 4.7 +/- 2.4 /s:
         Leduc et al. PNAS 2004 vol. 101 no. 49 17096-17101
         http://dx.doi.org/10.1073/pnas.0406598101 \n
         http://www.pnas.org/content/101/49/17096.abstract
     
     <em>default value = 0</em>
     */
    real         binding_rate;
    
    
    /// maximum distance at which the Hand can bind (also known as `binding[1]`)
    real         binding_range;
    
    
    /// can be set to restrict binding to certain type of Fiber
    /**
     The binding to a fiber is allowed only if the keys of the Hand and Fiber match.
     The test uses a BITWISE-AND of the two keys:

         if ( fiber:binding_key & hand:binding_key )
             allowed = true;
         else
             allowed = false;

     Thus, with `binding_key==0' attachment is completely disabled, and in general
     one needs to look at the bit-pattern of the number in base 2. For example
     `1` can bind to `3` but not to `2`.
     
     <em>default value = all-bits-at-1</em>
     */
    unsigned int binding_key;
    
    
    /// detachment rate in the absence of load (also known as `unbinding[0]`)
    /**
     This defines a detachment opportunity that is proportional to time.
     Kramers theory specifies that the detachment rate depends on the force
     in the link:
     
         RATE = unbinding_rate * exp( FORCE / unbinding_force )
     
     where FORCE is the norm of the tension in the link holding the Hand,
     and `unbinding_rate' and `unbinding_force' are two parameters.
     By setting `unbinding_force=inf', unbinding is made independent of load.
     
     Two articles:
         Mechanics of the kinesin step
         Carter, N. & Cross, R. Nature 435, 308–312 (2005).
         http://dx.doi.org/doi:10.1038/nature03528
     and
         Examining kinesin processivity within a general gating framework
         Andreasson et al. eLife 2015;4:e07403
         http://dx.doi.org/10.7554/eLife.07403
     provide similar values for conventional kinesin:

         unbinding_rate = 1 / s
         unbinding_force ~ 2 pN
     
     <em>default value = 0</em>
     (see @ref Stochastic)
     */
    real         unbinding_rate;
    
    
    /// characteristic force of unbinding (also known as `unbinding[1]`)
    /**
     @copydetails unbinding_rate
     
     <em>default value = inf</em>
     */
    real         unbinding_force;
    
    
    /// if true, the Hand can also bind directly to the tip of fibers
    /**
     The value of `bind_also_end` affects Hands that are located at a position
     for which the orthogonal projection on the fiber backbone is beyond one
     of the end. In this case, the attachement will occur only if `bind_also_end`
     is set and matches this end. Attachment will occur at the end of the fiber,
     if the distance is shorter than `binding_range`.
     
     Values for  are `off`, `minus_end`, `plus_end` and `both_ends`.
     
     In other words, setting 'bind_also_end==true', will extend the capture
     regions of the fibers to include one or two hemi-spheres at the ends of
     the fibers, with a radius `binding_range`.
     
     <em>default value = off</em>
     */
    int          bind_also_end;
    
    
    /// if true, the Hand can bind only near the ends of the fibers
    /**
     This determines that a Hand can only bind near the ends of the fiber.
     This parameter can be 'none', 'plus_end', 'minus_end' or 'both_ends'.
     Binding is allowed on positions located within a distance 'bind_end_range'
     from the specified end ('bind_end_range' is specified as `bind_only_end[1]`).
     
     <em>default value = off</em>
     */
    FiberEnd     bind_only_end;
    
    
    /// cutoff associated with `bind_only_end` where hand may bind (set as `bind_only_end[1]`)
    real         bind_end_range;

#if NEW_BIND_ONLY_FREE_END
    /// if true, only bind fiber tip if no other hand is bound already
    bool         bind_only_free_end;
#endif
    
    /// if false, the Hand will detach immediately upon reaching a growing or a static fiber end
    /**
     A Hand may reach the tip of the fiber on which it is bound, because it has
     moved, and `hold_growing_end` will determine the probability of detachment
     in this case. A value of 0 leads to immediate detachment.
     With a value of 1, the hand will remain attached.
     
     <em>default = 0</em>
     */
    real         hold_growing_end;
    
    
    /// if false, the Hand will detach immediately upon reaching a shrinking fiber end
    /**
     A Hand may reach the tip of the fiber on which it is bound,
     of the tip of the fiber may reach a immobile hand because it is disassembling.
     When this happens, `hold_shrinking_end` will determine if the Hand
     will detach or not.
     If `hold_shrinking_end` is true, the hand will be relocated to track the end.

     <em>default = false</em>
     */
    real         hold_shrinking_end;
    
    
    /// specialization
    /**
     @copydetails HandGroup
     */
    std::string  activity;
    
    
    /// display parameters (see @ref PointDispPar)
    std::string  display;
    
    /** @} */

public:

    /// derived variable: 1.0/unbinding_force
    real   unbinding_force_inv;
    
    /// derived variable = probability to bind in one `time_step`;
    real   binding_prob;
    
    /// derived variable = square(binding_range);
    real   binding_range_sqr;
    
    /// derived variable = unbinding_rate * time_step;
    real   unbinding_rate_dt;
    
    /// flag to indicate that `display` has a new value
    bool   display_fresh;
    
public:

    /// the display parameters for this category of Hand
    PointDisp * disp;
    
    /// constructor
    HandProp(const std::string& n) : Property(n), disp(nullptr) { clear(); }
    
    /// destructor
    ~HandProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor*) const;

    /// identifies the property
    std::string category() const { return "hand"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    virtual void read(Glossary&);
    
    /// compute values derived from the parameters
    virtual void complete(Simul const&);
    
    /// perform additional tests for the validity of parameters, given the elasticity
    virtual void checkStiffness(real stiff, real len, real mul, real kT) const;
    
    /// Attachment rate per unit length of fiber
    real bindingSectionRate() const;
    
    /// Attachment probability per unit length of fiber in one time_step
    real bindingSectionProb() const;

    /// write all values
    void write_values(std::ostream&) const;
    
    /// return a carbon copy of object
    Property* clone() const { return new HandProp(*this); }
};


#endif

