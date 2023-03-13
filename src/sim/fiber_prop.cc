// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_prop.h"
#include "sim.h"
#include <cmath>
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "property_list.h"
#include "single_prop.h"
#include "simul_prop.h"
#include "simul.h"
#include "fiber.h"

/**
 This is virtualized to return a derived Fiber if appropriate
 */
Fiber* FiberProp::newFiber() const
{
    return new Fiber(this);
}


/**
 Return the length specified by the user in 'opt'
 */
real FiberProp::newFiberLength(Glossary& opt) const
{
    real len = 1.0, var = 0.0;

    if ( 0 < opt.nb_values("fiber_length") )
    {
        opt.set_from_least_used_value(len, "fiber_length");
        return len;
    }
    
    opt.set(len, "length");

    if ( opt.value_is("length", 1, "exponential") )
    {
        // exponential distribution, truncated (Julio M.Belmonte's student):
        real L;
        do {
            L = len * RNG.exponential();
        } while (( L < min_length ) | ( L > max_length ));
        len = L;
    }
    else if ( opt.set(var, "length", 1) )
    {
        // add variability without changing mean:
        len += var * RNG.sreal();
    }
    
    len = std::max(len, min_length);
    len = std::min(len, max_length);
    return len;
}


/**
 @addtogroup FiberGroup
 @{
 <hr>
 
 When creating a new Fiber, you may specify:
 - the initial length,
 - the initial state of the PLUS_END and MINUS_END,
 - if the position refers to the center or to the tip of the fiber
 - the shape, using a set of points
 .
 
 Syntax:
 
     new filament
     {
       length = REAL, LENGTH_MODIFIER
       end_state = PLUS_END_STATE, MINUS_END_STATE
       reference = REFERENCE
     }
 
 The optional LENGTH_MODIFIER can be:
 - `exponential`,
 - REAL
 .
 This introduces variability, without changing the mean length.
 The second form generates a flat distribution of width 2*LENGTH_MODIFIER.
 
 The initial states PLUS_END_STATE and MINUS_END_STATE can be:
 - 0 = `white` or `static`
 - 1 = `green` or `grow`
 - 4 = `red`   or `shrink`
 .
 
 Optional reference specificiation:
 - center [default]
 - plus_end
 - minus_end
 .
 
 To use a persistent random walk as initial shape, set:
 
     new filament
     {
       equilibrate = PERSISTENCE_LENGTH
       position = POSITION
       direction = DIRECTION
     }
 
 In this case the shape will be random and different for each filament, and
 characterized by the given persistence length (units of length). The shape will
 be translated to bring its center of gravity at the specified position, and
 rotated to match the direction specified with the average filament direction.
 As usual, if 'position' and 'direction' are not specified, they are random.

 To specify the shape of a Fiber directly, use:
 
     new filament
     {
         shape = POSITION, POSITION, ...
     }
 
 Examples:
 
     new filament
     {
       length = 1
       plus_end = grow
       minus_end = static
     }

 which is equivalent to:

     new filament
     {
       length = 1
       end_state = green, white
     }

     new filament
     {
       position = 0 0 0
       orientation = 1 0 0
       shape = -4 -3 0, -3 0 0, -1 2 0, 1  3 0
     }
 
 @}
 */
Fiber* FiberProp::newFiber(Glossary& opt) const
{
    Fiber * fib = newFiber();
    
#if ( 1 )
    // specify the vertices directly:
    if ( opt.has_key("points") )
    {
        unsigned nbp = opt.nb_values("points");
        fib->setNbPoints(nbp);
        for ( unsigned p = 0; p < nbp; ++p )
        {
            Vector vec(0,0,0);
            if ( ! opt.set(vec, "points", p) )
                throw InvalidParameter("fiber:points must be a list of comma-separated vectors");
            fib->setPoint(p, vec);
        }
        if ( opt.has_key("length") )
            fib->imposeLength(newFiberLength(opt));
    }
    else
#endif
    if ( opt.has_key("shape") )
    {
        unsigned nbp = opt.nb_values("shape");
        
        if ( nbp < 2 )
            throw InvalidParameter("fiber:shape must be a list of comma-separated points");

        real* tmp = new_real(DIM*nbp);
        for ( unsigned p = 0; p < nbp; ++p )
        {
            Vector vec(0,0,0);
            if ( ! opt.set(vec, "shape", p) )
                throw InvalidParameter("fiber:shape must be a list of comma-separated points");
            vec.store(tmp+DIM*p);
        }
        fib->setShape(tmp, nbp, 0);
        if ( fib->nbPoints() < 2 )
            throw InvalidParameter("the vectors specified in fiber:shape must not overlap");
        free_real(tmp);
    }
    else
    {
        const real len = newFiberLength(opt);
        real pl = 0; // persistence length
        // place fiber horizontally with center at the origin:
        if ( opt.set(pl, "equilibrate") && pl > 0 )
            fib->setEquilibrated(len, pl);
        else
            fib->setStraight(Vector(-0.5*len,0,0), Vector(1,0,0), len);
        
        FiberEnd ref = CENTER;
        if ( opt.set(ref, "reference", {{"plus_end", PLUS_END}, {"minus_end", MINUS_END}, {"center", CENTER}}) )
            fib->placeEnd(ref);
    }
    
    // possible dynamic states of the ends
    Glossary::dict_type<state_t> keys{{"white",     STATE_WHITE},
                                      {"green",     STATE_GREEN},
                                      {"yellow",    STATE_YELLOW},
                                      {"orange",    STATE_ORANGE},
                                      {"red",       STATE_RED},
                                      {"static",    STATE_WHITE},
                                      {"grow",      STATE_GREEN},
                                      {"growing",   STATE_GREEN},
                                      {"shrink",    STATE_RED},
                                      {"shrinking", STATE_RED}};
    
    
    // set state of plus ends:
    state_t p = STATE_WHITE;
#ifdef BACKWARD_COMPATIBILITY
    if ( opt.set(p, "plus_end_state") )
    {
        fib->setDynamicStateP(p);
        Cytosim::warn << "use `plus_end = STATE` instead of `plus_end_state = STATE`\n";
    }
#endif
    if ( opt.set(p, "plus_end", keys) || opt.set(p, "end_state", keys) )
        fib->setDynamicStateP(p);

    // set state of minus ends:
    state_t m = STATE_WHITE;
#ifdef BACKWARD_COMPATIBILITY
    if ( opt.set(m, "minus_end_state") )
    {
        Cytosim::warn << "use `minus_end = STATE` instead of `minus_end_state = STATE`\n";
        fib->setDynamicStateM(m);
    }
#endif
    if ( opt.set(m, "minus_end", keys) || opt.set(m, "end_state", 1, keys) )
        fib->setDynamicStateM(m);

#ifdef BACKWARD_COMPATIBILITY
    if ( fib->prop->activity != "none" && m == 0 && p == 0 )
        Cytosim::warn << "Fiber may not grow as both ends are in state `white`\n";
#endif
    
    fib->updateFiber();

    return fib;
}


//------------------------------------------------------------------------------
void FiberProp::clear()
{
    rigidity            = -1;
    segmentation        = 1;
    min_length          = 0.010;      // suitable for actin/microtubules
    max_length          = INFINITY;
    total_polymer       = INFINITY;
    persistent          = false;

    viscosity           = -1;
    drag_radius         = 0.0125;  // radius of a Microtubule
    drag_length         = 5;
    drag_model          = false;
    drag_gap            = 0;
    
    binding_key         = ~0U;  //all bits at 1

    lattice             = 0;
    lattice_unit        = 0;

    confine             = CONFINE_OFF;
    confine_stiffness   = 0;
    confine_space       = "first";
    confine_space_ptr   = nullptr;

    steric              = 0;
    steric_radius       = 0;
    steric_range        = 0;
    
    glue                = 0;
    glue_single         = "none";
    glue_prop           = nullptr;
    
#if NEW_COLINEAR_FORCE
    colinear_force      = 0;
#endif
#if NEW_FIBER_CHEW
    max_chewing_speed   = 0;
#endif
#if NEW_FIBER_LOOP
    loop                = 0;
#endif

    activity            = "none";
    display             = "";
    display_fresh       = false;
    
    used_polymer        = 0;
    free_polymer        = 1;
    
#if OLD_SQUEEZE_FORCE
    squeeze             = 0;
    squeeze_force       = 0;
    squeeze_range       = 1;
#endif
}

//------------------------------------------------------------------------------
void FiberProp::read(Glossary& glos)
{
    glos.set(rigidity,          "rigidity");
    glos.set(segmentation,      "segmentation");
    glos.set(min_length,        "min_length");
    glos.set(max_length,        "max_length");
    glos.set(total_polymer,     "total_polymer");
    glos.set(persistent,        "persistent");
#ifdef BACKWARD_COMPATIBILITY
    bool ds;
    if ( glos.set(ds, "delete_stub") )
    {
        persistent = !ds;
        Cytosim::warn << "use `persistent="<<!ds<<"' instead of `delete_stub="<<ds<<"'\n";
    }
#endif
    
    glos.set(viscosity,    "viscosity");
    glos.set(drag_radius,  "drag_radius");
    glos.set(drag_length,  "drag_length");
    glos.set(drag_model,   "drag_model");
    glos.set(drag_gap,     "drag_model", 1);
#ifdef BACKWARD_COMPATIBILITY
    glos.set(drag_radius,  "hydrodynamic_radius");
    glos.set(drag_length,  "hydrodynamic_radius", 1);
    glos.set(drag_model,   "surface_effect");
    glos.set(drag_gap,     "surface_effect", 1);
#endif

    glos.set(binding_key,  "binding_key");
    
    glos.set(lattice,           "lattice");
    glos.set(lattice_unit,      "lattice", 1);
    glos.set(lattice_unit,      "lattice_unit");
    
    glos.set(confine,           "confine", {{"off",       CONFINE_OFF},
                                            {"on",        CONFINE_ON},
                                            {"inside",    CONFINE_INSIDE},
                                            {"outside",   CONFINE_OUTSIDE},
                                            {"none",      CONFINE_OFF},
                                            {"surface",   CONFINE_ON},
                                            {"plus_end",  CONFINE_PLUS_END},
                                            {"minus_end", CONFINE_MINUS_END},
                                            {"both_ends", CONFINE_BOTH_ENDS},
                                            {"plus_out",  CONFINE_PLUS_OUT}});
    
    glos.set(confine_stiffness, "confine", 1);
    glos.set(confine_space,     "confine", 2);
    glos.set(confine_stiffness, "confine_stiffness");
    glos.set(confine_space,     "confine_space");
    
#ifdef BACKWARD_COMPATIBILITY
    if ( confine_space == "current" )
        confine_space = "last";

    glos.set(confine,           "confined", {{"none",      CONFINE_OFF},
                                             {"inside",    CONFINE_INSIDE},
                                             {"outside",   CONFINE_OUTSIDE},
                                             {"surface",   CONFINE_ON},
                                             {"minus_end", CONFINE_MINUS_END},
                                             {"plus_end",  CONFINE_PLUS_END}});
    
    glos.set(confine_stiffness, "confined", 1);
#endif

#if OLD_SQUEEZE_FORCE
    glos.set(squeeze,           "squeeze");
    glos.set(squeeze_force,     "squeeze", 1);
    glos.set(squeeze_range,     "squeeze", 2);
#endif
    
    glos.set(steric,            "steric");
    glos.set(steric_radius,     "steric", 1);
    glos.set(steric_range,      "steric", 2);
    glos.set(steric_radius,     "steric_radius");
    glos.set(steric_range,      "steric_range");
    
    glos.set(glue,              "glue");
    glos.set(glue_single,       "glue", 1);
    
#if NEW_COLINEAR_FORCE
    glos.set(colinear_force,    "colinear_force");
#endif
#if NEW_FIBER_CHEW
    glos.set(max_chewing_speed, "max_chewing_speed");
#endif
#if NEW_FIBER_LOOP
    glos.set(loop,              "loop");
#endif

    glos.set(activity, "activity");
    if ( glos.set(display, "display") )
        display_fresh = true;
}


void FiberProp::complete(Simul const& sim)
{
    if ( viscosity < 0 )
        viscosity = sim.prop->viscosity;
    
    if ( viscosity < 0 )
        throw InvalidParameter("fiber:viscosity or simul:viscosity should be defined");
    
    confine_space_ptr = sim.findSpace(confine_space);
    
    if ( confine_space_ptr )
        confine_space = confine_space_ptr->name();

    if ( sim.ready() && confine != CONFINE_OFF )
    {
        if ( !confine_space_ptr )
            throw InvalidParameter(name()+":confine_space `"+confine_space+"' was not found");
    
        if ( confine_stiffness < 0 )
            throw InvalidParameter(name()+":confine_stiffness must be specified and >= 0");
    }

    if ( min_length < 0 )
        throw InvalidParameter("fiber:min_length should be >= 0");

    if ( max_length < 0 )
        throw InvalidParameter("fiber:max_length should be >= 0");

#ifdef BACKWARD_COMPATIBILITY
    if ( total_polymer == 0 )
        total_polymer = INFINITY;
#endif
    if ( total_polymer <= 0 )
        throw InvalidParameter("fiber:total_polymer should be > 0 (you can specify 'inf')");

    if ( glue )
    {
        if ( sim.ready() )
            glue_prop = sim.findProperty<SingleProp>("single", glue_single);
    }
    
    if ( lattice && sim.ready() )
    {
#if FIBER_HAS_LATTICE
        if ( lattice_unit <= 0 )
            throw InvalidParameter("fiber:lattice_unit (known as fiber:lattice[1]) must be specified and > 0");
#else
        throw InvalidParameter("Cytosim cannot handle fiber:lattice. Please recompile after setting FIBER_HAS_LATTICE to 1");
#endif
    }

    if ( rigidity < 0 )
        throw InvalidParameter("fiber:rigidity must be specified and >= 0");
    
    if ( segmentation <= 0 )
        throw InvalidParameter("fiber:segmentation must be > 0");

    if ( sim.ready() )
    {
        // Adjust the segmentation of all Fibers having this FiberProp
        for ( Fiber* fib = sim.fibers.first(); fib; fib=fib->next() )
            if ( fib->property() == this )
                fib->adjustSegmentation(segmentation);
    }
    
    if ( steric && steric_radius <= 0 )
        throw InvalidParameter("fiber:steric[1] (radius) must be specified and > 0");
    
    if ( drag_radius <= 0 )
        throw InvalidParameter("fiber:drag_radius must be > 0");
    
    if ( drag_length <= 0 )
        throw InvalidParameter("fiber:drag_length must be > 0");

#if OLD_SQUEEZE_FORCE
    if ( max_chewing_speed < 0 )
        throw InvalidParameter("fiber:max_chewing_speed must be >= 0");
    max_chewing_speed_dt = max_chewing_speed * sim.time_step();
#endif
    
#if ( 0 )
    //print some information on the 'stiffness' of the matrix
    Fiber fib(this);
    fib.setStraight(Vector(-5,0,0), Vector(1,0,0), 10);
    
    fib.setDragCoefficient();
    real mob_dt = sim.time_step() * fib.nbPoints() / fib.dragCoefficient();
    
    real stiffness = 100;
    real coef1 = mob_dt * stiffness;

    Cytosim::log("Numerical hardness (stiffness=%.1f): %7.2f\n", stiffness, coef1);

    real rod   = segmentation;
    real coef2 = mob_dt * rigidity / ( rod * rod * rod );
    
    Cytosim::log("Numerical hardness (rigidity=%.1f): %7.2f\n", rigidity, coef2);
#endif
}


//------------------------------------------------------------------------------

void FiberProp::write_values(std::ostream& os) const
{
    write_value(os, "rigidity",            rigidity);
    write_value(os, "segmentation",        segmentation);
    write_value(os, "min_length",          min_length);
    write_value(os, "max_length",          max_length);
    write_value(os, "total_polymer",       total_polymer);
    write_value(os, "persistent",          persistent);
    write_value(os, "viscosity",           viscosity);
    write_value(os, "drag_radius",         drag_radius);
    write_value(os, "drag_length",         drag_length);
    write_value(os, "drag_model",          drag_model, drag_gap);
#if OLD_SQUEEZE_FORCE
    write_value(os, "squeeze",             squeeze, squeeze_force, squeeze_range);
#endif
    write_value(os, "binding_key",         binding_key);
    write_value(os, "lattice",             lattice, lattice_unit);
    write_value(os, "confine",             confine, confine_stiffness, confine_space);
    write_value(os, "steric",              steric, steric_radius, steric_range);
    write_value(os, "glue",                glue, glue_single);
#if NEW_COLINEAR_FORCE
    write_value(os, "colinear_force",      colinear_force);
#endif
#if NEW_FIBER_CHEW
    write_value(os, "max_chewing_speed",   max_chewing_speed);
#endif
#if NEW_FIBER_LOOP
    write_value(os, "loop",                loop);
#endif
    write_value(os, "activity",            activity);
    write_value(os, "display",             "("+display+")");
}

