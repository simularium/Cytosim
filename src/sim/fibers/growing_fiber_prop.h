// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GROWING_FIBER_PROP
#define GROWING_FIBER_PROP

#include "fiber_prop.h"


/// additional Property for GrowingFiber
/**
 @ingroup Properties
 Assembly is continuous, and can occur at both ends.
 The dynamic states of the end are ignored with this model.
 */
class GrowingFiberProp : public FiberProp
{
    
    friend class GrowingFiber;
    
public:
    
    /**
     @defgroup GrowingFiberPar Parameters of GrowingFiber
     @ingroup Parameters
     Inherits @ref FiberPar.
     @{
     */
    
    /// see @ref GrowingFiber
    
    /// Speed of assembly (must be positive)
    /**
     Antagonistic force decrease assembly rate exponentially if it is directed against the assembly:

         if ( force < 0 )
             speed = growing_speed * free_polymer * exp( force / growing_force ) + growing_off_speed;
         else
             speed = growing_speed * free_polymer + growing_off_speed;
     
     The parameters are:
     - `growing_speed`, the force-dependent and concentration-dependent assembly rate.
     - `growing_off_speed`, a constant term, normally negative to represent spontaneous disassembly.
     - `growing_force`, the characteristic force
     .
     In this equation, `free_polymer` represents the fraction of free monomers in [0,1].
     Antagonistic force is negative ( force < 0 ) if it is directed against fiber assembly.
     */
    real    growing_speed[2];
    
    /// Constant term in the growing speed equation (can be negative or positive)
    real    growing_off_speed[2];

    /// Characteristic force for polymer assembly
    real    growing_force[2];

    /// Flag to enable special dynamics in which growth is determined by the position of the fiber end
    /**
     If 'shrink_outside[0] == true', the PLUS_END will:
     - grow if the PLUS_END is inside the confining Space
     - shrink if the PLUS_END is outside the confining Space.
     .
     Similarly, 'shrink_outside[1]' determines the behavior of the MINUS_END.

     The shrinking speed is equal to the specified growing_speed.
     */
    bool    shrink_outside[2];
    
    /// Shrinking speed of end that are outside, for option 'shrink_outside'
    real    shrinking_speed[2];

    /// @}
    
private:
    
    /// derived variable
    real    growing_speed_dt[2];
    
    /// derived variable
    real    growing_off_speed_dt[2];
    
    /// derived variable
    real    shrinking_speed_dt[2];

public:
    
    /// constructor
    GrowingFiberProp(const std::string& n) : FiberProp(n) { clear(); }

    /// destructor
    ~GrowingFiberProp() { }
    
    /// return a Fiber with this property
    Fiber* newFiber() const;
    
    /// set default values
    void clear();
       
    /// set using a Glossary
    void read(Glossary&);
   
    /// check and derive parameter values
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new GrowingFiberProp(*this); }

    /// write
    void write_values(std::ostream&) const;

};

#endif

