// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef COUPLE_PROP_H
#define COUPLE_PROP_H

#include "real.h"
#include "property.h"
#include "hand_prop.h"
#include "common.h"

class Couple;
class Space;

/// Property for Couple
/**
 @ingroup Properties
*/
class CoupleProp : public Property
{
    friend class Couple;
    
public:
    
    /// indicates a specificity for crosslinking two fibers
    /** 
     The criteria is based on the angle defined by the two fibers.
     Since fibers are oriented, the angle in Radian is well defined between -PI and +PI:
     - 0 corresponds to parallel fibers having the same orientation,
     - PI/2 correspond to orthogonal fibers,
     - -PI corresponds to parallel fibers having opposite orientation (antiparallel)
     .
     The allowed angle correspond to +/- 60 degree, which is rather arbitrary.
     */
    enum Specificity 
    {
        BIND_ALWAYS,           ///< can bind in any configuration
        BIND_PARALLEL,         ///< can link two fibers only if `cosine(angle) > 0.5`
        BIND_NOT_PARALLEL,     ///< can link two fibers only if `cosine(angle) < 0.5`
        BIND_ANTIPARALLEL,     ///< can link two fibers only if `cosine(angle) < -0.5`
        BIND_NOT_ANTIPARALLEL, ///< can link two fibers only if `cosine(angle) > -0.5`
        BIND_ORTHOGONAL        ///< can link two fibers only if `-sqrt(3)/2 < cosine(angle) < sqrt(3)/2`
    };
    
    /**
     @defgroup CouplePar Parameters of Couple
     @ingroup Parameters
     @{
     */
    
    
    /// name of first Hand in Couple
    std::string  hand1;
    
    /// name of second Hand in Couple
    std::string  hand2;
    
    /// stiffness of link between the two Hands while linking (pN/um)
    real         stiffness;
    
    /// resting length of the link (um)
    real         length;
    
    /// diffusion coefficient while unattached (um^2/s)
    real         diffusion;
    
    /// if set > 0, assumes uniform concentration of diffusing Couple
    /**
     The possible values for `fast_diffusion` are:
     
     - 0: disabled.
       Every Couple is explicitly represented by a diffusing point. The
       unattached Couple move randomly, with the specified diffusion constant,
       and they may accumulate at certain regions of the simulation space. That
       happens in particular with asters when the motors move inward. In fact,
       the aster can act like a black hole for the motors, in certain conditions.

     - 1: attach Couple along the length of filaments
       It is assumed that free Couples are uniformly distributed, and they are 
       thus not explicitly represented. In this mode, Cytosim will only keep a 
       count of the number of free Couples, and directly attach these Couples by
       one of their Hand, at positions of the fibers selected randomly, irrespective
       of the manner in which the filaments are distributed spatially.
       In this mode, the Couple diffusion constant is not relevant.

     - 2: attach Couple only at growing PLUS_ENDS
       Similar to mode 1, but Hand1 of Couple is directly attached to the plus tips
       of growing fibers. The number of attachments is proportional to the new
       polymer mass, which is `time_step * growing_speed`, but occurs otherwise
       equally to every filaments.
     .
     
     Enabling `fast_diffusion` can make the simulation a bit faster when there
     are a lot of Couples, but another reason to use this mode is that it makes
     the model simpler, since the fraction of bound/free Couples can then be calculated
     analytically. This also avoid the accumulation of motors in aster and other
     organized structures, which are stronger in a 2D geometry, compared to 3D. 
     Thus one may wish to enable `fast_diffusion` in a 2D simulation to better
     represent a 3D system.
     
     `fast_diffusion` does not affect Couples in the attached state.
     */
    int          fast_diffusion;
    
    /// if ( trans_activated == 1 ), Hand2 is active only if Hand1 is bound
    /**
     Both Hands of a Couple are normally equally active. With this feature,
     the activity of Hand2 is conditioned on Hand1 being attached.
     In other words, Hand2 remains inactive if Hand1 is not attached.
     This is very useful for example to make a nucleator that nucleates only after
     it has docked on an existing filament.
     */
    bool         trans_activated;

    /// prevents both Hands from binding at the same position on a Fiber (default=true)
    /**
     By default, the two Hands of a Couple may not bind at the same position on 
     the same Fiber. Such a degenerate binding may be impossible due to the molecular 
     configuration of the complex. Note that no restriction is imposed if the Hands
     attempt to bind to different fibers, or to the same fiber at distant positions. 
     The exact criteria is that the Hands cannot bind onto the same segment, or 
     to segments that have a point in common.
     
     Setting `stiff==false`, however, allows the two hands from the Couple to bind 
     nearby positions on the same fiber. In such a degenerate configuration, the 
     link is unproductive as it cannot produce force, but the feature may be useful 
     to combine activities such as cutting and motors.
     */
    bool         stiff;
    
    /// Specificity of binding to a pair a Fiber
    /**
     Set to limit the binding to only certain configurations:
     - `off`             : no restriction (default)
     - `parallel`        : parallel filaments with an angle below 60 degrees
     - `antiparallel`    : anti-parallel filaments with an angle below 60 degrees
     - `not_parallel`    : anti-parallel filaments with an angle below 120 degrees
     - `not_antiparallel`: parallel filaments with an angle below 120 degrees
     - `orthogonal`      : filaments with an angle between 60 and 120 degrees
     .
     This limit the attachment of a Hand when the other Hand is already attached,
     as a function of the angle that is defined by the already bound filament,
     and the potential new one. The first attachment of any Hand is unrestricted.
     */
    Specificity  specificity;

    /// Confinement can be `off`, `inside` (default) or `surface`
    Confinement  confine;
    
    /// Unused Parameter: confinement stiffness (also known as `confine[1]`)
    //real         confine_stiffness;
    
    /// name of space used for confinement (also known as `confine[2]`)
    std::string  confine_space;
    
    /// specialization
    /**
     @copydetails CoupleGroup
     */
    std::string  activity;
    
    /// @}
    
    /// pointer to Property of Hand 1
    HandProp *    hand1_prop;
    /// pointer to Property of Hand 2
    HandProp *    hand2_prop;

protected:
    
    real          diffusion_dt;
    /// pointer to actual confinement Space, derived from `confine_space`
    Space const*  confine_space_ptr;

public:
    
    /// constructor
    CoupleProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~CoupleProp() { }
    
    /// create a Couple having this property
    virtual Couple * newCouple(Glossary * = nullptr) const;
    
    /// identifies the property
    std::string category() const { return "couple"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute derived parameter values
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new CoupleProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
  
    /// return volume of confine_space
    real spaceVolume() const;
};

#endif

