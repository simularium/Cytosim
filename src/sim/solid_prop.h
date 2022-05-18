// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SOLID_PROP_H
#define SOLID_PROP_H

#include "real.h"
#include "property.h"
#include "common.h"
#include "vector.h"

class PointDisp;
class Space;

/// new convergent radial flow code made for Maria Burdyniuk
#define NEW_RADIAL_FLOW 0

/// new option to immobilize Solid by attaching one point
#define NEW_SOLID_CLAMP 0


/// Property for Bead and Solid
/*
 @ingroup Properties
 */
class SolidProp : public Property
{
    friend class Bead;
    friend class Solid;
    
public:
    
    /**
     @defgroup SolidPar Parameters of Solid and Bead
     @ingroup Parameters
     @{
     */
    
    /// drag of the bead, defining how easy it is to move the bead ( force = drag * speed )
    /**
     If `drag` is not specified, its value is calculated using Stokes' law,
     using the effective viscosity:
     
         drag = 6 * M_PI * viscosity * radius;

     */
    real         drag;
    
    /// effective viscosity (if not specified, simul:viscosity is used)
    /**
     Set the effective `viscosity` to lower or increase the drag coefficient of a particular class of beads/solid.\n
     Example: It possible to put the majority of the drag coefficient of an aster at its center, by reducing the effecitve viscosity of the fibers, and conserving (or increasing) the viscosity of the solid which forms its core.
     If unspecified, the global `simul:viscosity` is used.
     */
    real         viscosity;
    
    /// flag to include steric interactions
    int          steric;
    
    /// distance added to radius to set the steric interaction distance
    real         steric_range;
    
    /// flag to confine the object
    /**
     Most objects will accept these values:
     - `off` (default)
     - `on` or `surface`
     - `inside`
     - `outside`
     - `all_inside`
     .
     */
    Confinement  confine;
    
    /// confinement stiffness (also known as `confine[1]`)
    real         confine_stiffness;
    
    /// name of space used for confinement (also known as `confine[2]`)
    std::string  confine_space;
    
#if NEW_RADIAL_FLOW
    /// for the additional force
    real         flow_time[2];
    
    /// for the additional force
    Vector       flow_center;
#endif
    
#if NEW_SOLID_CLAMP
    /// position of clamping that applies to point 0 of Solid (known as `clamp`)
    Vector       clamp_pos;
    
    /// stiffness of clamping force (known as `clamp[1]`)
    real         clamp_stiff;
#endif
    
    /// display string (see @ref PointDispPar)
    std::string  display;
    
    /// @}
    
    
    /// flag to indicate that `display` has a new value
    bool         display_fresh;
    
    /// parameters derived from string `display`
    PointDisp *  disp;
    
private:
    
    /// pointer to actual confinement Space, derived from `confine_space`
    Space const* confine_space_ptr;
    
    /// customized Label can be 'bead' or 'solid'
    std::string  mCategory;
    
public:
    
    /// constructor
    SolidProp(const std::string& k, const std::string& n) : Property(n), disp(nullptr), mCategory(k) { clear(); }
    
    /// destructor
    ~SolidProp() { }
    
    /// identifies the property
    std::string category() const { return mCategory; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// derive parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new SolidProp(*this); }
    
    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

