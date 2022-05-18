// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "hand_prop.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "sim.h"
#include "property_list.h"
#include "simul_prop.h"
#include "simul.h"
#include "hand.h"
#include "hand_monitor.h"

#include "motor_prop.h"
#include "walker_prop.h"
#include "slider_prop.h"
#include "nucleator_prop.h"
#include "regulator_prop.h"
#include "rescuer_prop.h"
#include "tracker_prop.h"
#include "cutter_prop.h"
#include "chewer_prop.h"
#include "mighty_prop.h"
#include "actor_prop.h"


/// Switch to enable Myosin, Kinesin and Dynein
#define NEW_HAND_TYPES 1

#if NEW_HAND_TYPES
#include "kinesin_prop.h"
#include "dynein_prop.h"
#include "myosin_prop.h"
#endif


/**
 @defgroup HandGroup Hand and related
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Hand can bind to a Fiber, and derived class can do more things.

 A plain Hand can only bind and unbind from a Fiber.
 Derived classes are available that implement more complex functionalities,
 for example molecular motors or severing enzymes.
 
 List of classes accessible by specifying `hand:activity`:
 
 @ref HandGroup
 
 `activity`    | Class         | Parameters         | Property     |
 --------------|---------------|--------------------|---------------
 `bind`        | Hand          | @ref HandPar       | HandProp
 `move`        | Motor         | @ref MotorPar      | MotorProp
 `nucleate`    | Nucleator     | @ref NucleatorPar  | NucleatorProp
 `slide`       | Slider        | @ref SliderPar     | SliderProp
 `track`       | Tracker       | @ref TrackerPar    | TrackerProp
 `rescue`      | Rescuer       | @ref RescuerPar    | RescuerProp
 `regulate`    | Regulator     | @ref RegulatorPar  | RegulatorProp
 `cut`         | Cutter        | @ref CutterPar     | CutterProp
 `chew`        | Chewer        | @ref ChewerPar     | ChewerProp
 `mighty`      | Mighty        | @ref MightyPar     | MightyProp
 `act`         | Actor         | @ref ActorPar      | ActorProp
 
 <h2>Digital Hands:</h2>
 
 `activity`    | Class         | Parameters         | Property     |
 --------------|---------------|--------------------|---------------
 `digit`       | Digit         | @ref DigitPar      | DigitProp
 `walk`        | Walker        | @ref WalkerPar     | WalkerProp
 `kinesin`*    | Kinesin       | @ref KinesinPar    | KinesinProp
 `dynein`*     | Dynein        | @ref DyneinPar     | DyneinProp
 `myosin`*     | Myosin        | @ref MyosinPar     | MyosinProp
 
 * Unfinished classes.
 
 Example:

     set hand motor
     {
       binding = 10, 0.05
       unbinding = 0.2, 3
 
       activity = move
       unloaded_speed = 1
       stall_force = 5
     }
 
 */
HandProp * HandProp::newProperty(const std::string& nom, Glossary& glos)
{
    std::string a;
    if ( glos.peek(a, "activity") )
    {
        if ( a == "move" || a == "motor" )
            return new MotorProp(nom);
        if ( a == "digit" )
            return new DigitProp(nom);
        if ( a == "walk" )
            return new WalkerProp(nom);
        if ( a == "slide" )
            return new SliderProp(nom);
        if ( a == "nucleate" )
            return new NucleatorProp(nom);
        if ( a == "regulate" )
            return new RegulatorProp(nom);
        if ( a == "track" )
            return new TrackerProp(nom);
        if ( a == "rescue" )
            return new RescuerProp(nom);
        if ( a == "cut" )
            return new CutterProp(nom);
        if ( a == "chew" )
            return new ChewerProp(nom);
        if ( a == "mighty" )
            return new MightyProp(nom);
        if ( a == "act" )
            return new ActorProp(nom);
        if ( a == "bind" )
            return new HandProp(nom);
#if NEW_HAND_TYPES
        if ( a == "kinesin" )
            return new KinesinProp(nom);
        if ( a == "dynein" )
            return new DyneinProp(nom);
        if ( a == "myosin" )
            return new MyosinProp(nom);
#endif
#if ( 0 )
        throw InvalidParameter("unknown hand:activity `"+a+"'");
#else
        // try to proceed anyhow:
        std::cerr << "WARNING: unknown hand:activity `" << a << "'" << std::endl;
#endif
    }
    return new HandProp(nom);
}


Hand * HandProp::newHand(HandMonitor* m) const
{
    return new Hand(this, m);
}


//------------------------------------------------------------------------------
void HandProp::clear()
{
    binding_rate       = 0;
    binding_range      = 0;
    binding_key        = ~0U;  //all bits at 1
    unbinding_rate     = 0;
    unbinding_force    = INFINITY;
    unbinding_force_inv = 0;

    bind_also_end      = 0;
    bind_only_end      = NO_END;
    bind_end_range     = 0;
#if NEW_BIND_ONLY_FREE_END
    bind_only_free_end = false;
#endif
    hold_growing_end   = 0;
    hold_shrinking_end = 0;
    
    activity           = "bind";
    display            = "";
    display_fresh      = false;
}


void HandProp::read(Glossary& glos)
{
    glos.set(binding_rate,  "binding_rate")  || glos.set(binding_rate,  "binding", 0);
    glos.set(binding_range, "binding_range") || glos.set(binding_range, "binding", 1);
    glos.set(binding_key,   "binding_key")   || glos.set(binding_key,   "binding", 2);
    
    glos.set(unbinding_rate,  "unbinding_rate")  || glos.set(unbinding_rate,  "unbinding", 0);
    glos.set(unbinding_force, "unbinding_force") || glos.set(unbinding_force, "unbinding", 1);
    
    
    glos.set(bind_also_end, "bind_also_end", {{"off",       NO_END},
                                              {"plus_end",  PLUS_END},
                                              {"minus_end", MINUS_END},
                                              {"both_ends", BOTH_ENDS}});

    glos.set(bind_only_end, "bind_only_end", {{"off",       NO_END},
                                              {"plus_end",  PLUS_END},
                                              {"minus_end", MINUS_END},
                                              {"both_ends", BOTH_ENDS}});
    
    glos.set(bind_end_range, "bind_end_range") || glos.set(bind_end_range, "bind_only_end", 1);

#ifdef BACKWARD_COMPATIBILITY
    glos.set(bind_also_end, "bind_also_ends", {{"off",       NO_END},
                                               {"plus_end",  PLUS_END},
                                               {"minus_end", MINUS_END},
                                               {"both_ends", BOTH_ENDS}});
    glos.set(bind_only_end,      "bind_end", {{"off",       NO_END},
                                              {"plus_end",  PLUS_END},
                                              {"minus_end", MINUS_END},
                                              {"both_ends", BOTH_ENDS}});
    glos.set(bind_end_range,     "bind_end", 1);
#endif
    
    
    glos.set(hold_growing_end,   "hold_growing_end");
    glos.set(hold_shrinking_end, "hold_shrinking_end");
#if NEW_BIND_ONLY_FREE_END
    glos.set(bind_only_free_end, "bind_only_free_end");
#endif
    
    glos.set(activity,           "activity");
    if ( glos.set(display, "display") )
        display_fresh = true;

#ifdef BACKWARD_COMPATIBILITY
    if ( glos.set(hold_growing_end, "hold_growing_ends") )
        Cytosim::warn << "hand:hold_growing_ends was renamed hold_growing_end\n";
#endif
}


void HandProp::complete(Simul const& sim)
{    
    if ( sim.time_step() < REAL_EPSILON )
        throw InvalidParameter("simul:time_step is not defined");
    
    binding_range_sqr = square(binding_range);
    binding_prob = -std::expm1(-binding_rate * sim.time_step());
    unbinding_rate_dt = unbinding_rate * sim.time_step();
    
    if ( binding_range < 0 )
        throw InvalidParameter(name()+":binding_range must be >= 0");
    
    if ( binding_rate < 0 )
        throw InvalidParameter(name()+":binding_rate must be positive");
    
    if ( unbinding_rate < 0 )
        throw InvalidParameter(name()+":unbinding_rate must be positive");
    
    if ( hold_growing_end < 0 )
        throw InvalidParameter(name()+":hold_growing_end must be >= 0");
    
    if ( hold_shrinking_end < 0 )
        throw InvalidParameter(name()+":hold_shrinking_end must be >= 0");

    if ( sim.ready() )
    {
        if ( binding_prob > sim.prop->acceptable_prob )
            Cytosim::warn << name() << ":binding_rate is too high: decrease time_step\n";
    
        if ( unbinding_rate_dt > sim.prop->acceptable_prob )
            Cytosim::warn << name() << ":unbinding_rate is too high: decrease time_step\n";
    }
    
#ifdef BACKWARD_COMPATIBILITY
    if ( unbinding_force == 0 )
    {
        Cytosim::warn << "assuming that hand:unbinding_force=+inf, since the set value was zero\n";
        unbinding_force = INFINITY;
    }
#endif
    
    if ( unbinding_force <= 0 )
        throw InvalidParameter(name()+":unbinding_force must be > 0");

    // this should be zero if 'unbinding_force = inf':
    unbinding_force_inv = 1.0 / unbinding_force;

    /*
     The exponential term in Kramer's theory can easily become numerically "infinite",
     and since `zero * infinite` is undefined, we disable here
     the exponential argument if the unbinding rate is null:
     */
    if ( unbinding_rate == 0 )
        unbinding_force_inv = 0;

    //std::clog << name() << " unbinding_force_inv = " << unbinding_force_inv << std::endl;
}


/**
 Compare the energy in a link when it binds at its maximum distance,
 with the Thermal energy
 
 @todo the warning may not be relevant for long Links
 */
void HandProp::checkStiffness(real stiff, real len, real mul, real kT) const
{
    real dis = binding_range - len;
    real en = ( stiff * dis * dis ) / kT;
    
    if ( en > 10.0 && binding_rate > 0 )
    {
        Cytosim::warn << "binding of `" << name() << "' is thermodynamically unfavorable (stiffness * binding_range^2 = " << en << " kT)\n";
        //<< PREF << "you could decrease stiffness or binding_range\n";
    }
    
    
    real ap = exp( stiff * dis * unbinding_force_inv );
    
    if ( ap > 10.0 )
    {
        Cytosim::warn << "hand `" << name() << "' may unbind just after binding:\n"\
        << PREF << "exp( stiffness * binding_range / unbinding_force ) = " << ap << "\n";
        //<< PREF << "you could decrease stiffness or binding_range\n";
    }
}


void HandProp::write_values(std::ostream& os) const
{
    write_value(os, "binding",            binding_rate, binding_range);
    write_value(os, "binding_key",        binding_key);
    write_value(os, "unbinding",          unbinding_rate, unbinding_force);
    
    write_value(os, "bind_also_end",      bind_also_end);
    write_value(os, "hold_growing_end",   hold_growing_end);
    write_value(os, "hold_shrinking_end", hold_shrinking_end);
    write_value(os, "bind_only_end",      bind_only_end, bind_end_range);
#if NEW_BIND_ONLY_FREE_END
    write_value(os, "bind_only_free_end", bind_only_free_end);
#endif
    write_value(os, "display",            "("+display+")");
    write_value(os, "activity",           activity);
}


/**
 Attachment rate per unit length of fiber
 */
real HandProp::bindingSectionRate() const
{
#if ( DIM == 2 )
    return 2 * binding_range * binding_rate;
#elif ( DIM == 3 )
    return M_PI * binding_range * binding_range * binding_rate;
#else
    return 0;
#endif
}


/**
 Attachment probability per unit length of fiber in one time_step
 */
real HandProp::bindingSectionProb() const
{
#if ( DIM == 2 )
    return 2 * binding_range * binding_prob;
#elif ( DIM == 3 )
    return M_PI * binding_range * binding_range * binding_prob;
#else
    return 0;
#endif
}

