// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DYNAMIC_FIBER_PROP
#define DYNAMIC_FIBER_PROP

#include "fiber_prop.h"


/// additional Property for DynamicFiber
/**
 @ingroup Properties
 */
class DynamicFiberProp : public FiberProp
{
    friend class DynamicFiber;
    
public:
    
    /**
     @defgroup DynamicFiberPar Parameters of DynamicFiber
     @ingroup Parameters
     Inherits @ref FiberPar.
     @{
     */
    
    /// see @ref DynamicFiber
    
    /// Length of discrete units of assembly/disassembly
    real    unit_length;
    
    /// Speed of assembly
    real    growing_speed[2];
    
    /// Spontaneous disassembly in the growing state, independent of free tubulin concentration
    real    growing_off_speed[2];

    /// Characteristic force for polymer assembly (default=+inf)
    /**
     Filament      |   Max. possible    | Measured stall force |
     --------------|--------------------|-----------------------
     Microtubule   | ~ 1GTP per 8nm/13  | ~ 1.67 pN
     Actin         | ~ 1ATP per 4nm/2   | ~ 1 pN

     With 1 ATP bringing ~ 80--100 pN.nm of energy
     
     <em>
     <b>Direct measurement of force generation by actin filament polymerization using an optical trap.</b>\n
     Footer et al.\n
     PNAS vol. 104 no. 7; 2007\n
     http://doi.org/10.1073/pnas.0607052104
     
     <b>Measurement of the Force-Velocity Relation for Growing Microtubules</b>\n
     Marileen Dogterom and Bernard Yurke\n
     Science Vol 278 pp 856-860; 1997\n
     http://www.sciencemag.org/content/278/5339/856.abstract
     </em>
     */
    real    growing_force[2];

    /// Hydrolysis rate of G-units, which defines the catastrophe rate
    /**
     Without spontaneous off rate (`growing_off_rate==0`), 
     the catastrophe rate is set by
     
         catastrophe_rate = 3 * hydrolysis_rate ^2 / growing_rate;
     
     with `growing_rate = growing_speed / unit_length`
     */
    real    hydrolysis_rate[2];

    /// Speed of disassembly
    real    shrinking_speed[2];
    
    /// switching rate to the growing state for a fiber shorter than `min_length` (default=0)
    real    rebirth_rate[2];

    
    /// Space defining the zone in which hydrolysis_rate is different
    std::string zone_space;
    
    /// Radius of zone in which the hydrolysis_rate is different
    real    zone_radius;
    
    /// Hydrolysis rate of G-units, outside the zone of radius `zone_radius`.
    /** this create an circular zone around the origin */
    real    zone_hydrolysis_rate[2];

    /// @}
    
private:
    
    real    growing_rate_dt[2];
    real    growing_off_rate_dt[2];
    real    hydrolysis_rate_2dt[2];
    real    zone_hydrolysis_rate_2dt[2];
    real    shrinking_rate_dt[2];
    real    rebirth_prob[2];
    
    real    zone_radius_sqr;
    Space const*  zone_space_ptr;
    
public:
    
    /// constructor
    DynamicFiberProp(const std::string& n) : FiberProp(n) { clear(); }

    /// destructor
    ~DynamicFiberProp() { }
    
    /// return a Fiber with this property
    Fiber* newFiber() const;
    
    /// set default values
    void clear();
       
    /// set using a Glossary
    void read(Glossary&);
   
    /// check and derive parameter values
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new DynamicFiberProp(*this); }

    /// write
    void write_values(std::ostream&) const;

    /// print predicted length and time
    void splash(std::ostream&, real, real) const;
};

#endif

