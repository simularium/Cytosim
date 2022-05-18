// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef GROWING_FIBER_H
#define GROWING_FIBER_H

#include "sim.h"
#include "vector.h"
#include "node_list.h"
#include "fiber.h"

class GrowingFiberProp;


/// A Fiber with independent assembly/disassembly at both ends
/**
 The growing speed of each end are set independently.
 The basic parameters are:
 
 * `growing_speed`, the base assembly rate in um/s.
 * `growing_off_speed`, a constant term, normally negative to represent spontaneous disassembly.
 * `growing_force`, the characteristic force of polymerization in pN.
 
 Positive values of the rate correspond to assembly, and negative values to disassembly. 
 Assembly is exponentially decreased by antagonistic force, and linearly dependent 
 on the availability of polymer.  Disassembly always occurs at the specified rate.
 Only the component of the force parallel to the direction of the fiber at the end 
 is taken into account:
 
     force = force_vector * fiber_direction;
 
 The projected force is negative ( antagonistic ) if it is directed against fiber assembly.
 
     if ( force < 0 )
         speed = growing_speed * free_polymer * exp( force / growing_force ) + growing_off_speed;
     else
         speed = growing_speed * free_polymer + growing_off_speed;
 
 In this equation, `free_polymer` is a number in [0,1], representing the fraction of free monomers.
 It is defined as:
 
    free_polymer = 1.0 - sum(all_fiber_length) / total_polymer
 
 The length of a fiber will not exceed `fiber:max_length`, and any Fiber shorter than `fiber:min_length`
 will be deleted.

 See the @ref GrowingFiberPar.
 @ingroup FiberGroup
 */
class GrowingFiber : public Fiber
{
private:
    
    /// state of PLUS_END (static or growing)
    state_t    mStateP;
    
    /// assembly at PLUS_END during last time-step
    real       mGrowthP;
    
    /// state of MINUS_END (static or growing)
    state_t    mStateM;
    
    /// assembly at MINUS_END during last time-step
    real       mGrowthM;
    
public:
    
    /// the Property of this object
    GrowingFiberProp const* prop;
    
    /// constructor
    GrowingFiber(GrowingFiberProp const*);
    
    /// destructor
    virtual ~GrowingFiber();
    
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of MINUS_END
    state_t     dynamicStateM() const { return mStateM; };
    
    /// change state of MINUS_END
    void        setDynamicStateM(state_t s);
    
    /// length increment at MINUS_END during last time-step
    real        freshAssemblyM() const { return mGrowthM; }

    
    /// return assembly/disassembly state of PLUS_END
    state_t     dynamicStateP() const { return mStateP; };
    
    /// change state of PLUS_END
    void        setDynamicStateP(state_t s);

    /// length increment at PLUS_END during last time-step
    real        freshAssemblyP() const { return mGrowthP; }

    
    /// monte-carlo step
    void        step();
    
    //--------------------------------------------------------------------------
    
    /// write to Outputter
    void        write(Outputter&) const;
    
    /// read from Inputter
    void        read(Inputter&, Simul&, ObjectTag);
    
};


#endif
