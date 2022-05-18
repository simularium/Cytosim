// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPHERE_PROP_H
#define SPHERE_PROP_H

#include "real.h"
#include "property.h"
#include "common.h"

class PointDisp;
class Space;


/// Property for Sphere
/**
 @ingroup Properties
*/
class SphereProp : public Property
{
    friend class Sphere;

public:
   
    /**
     @defgroup SpherePar Parameters of Sphere
     @ingroup Parameters
     @{
     */
    
    
    /// mobility of points on the surface
    real         point_mobility;
    
    /// effective viscosity (if unspecified, simul:viscosity is used)
    /**
     Set the effective `viscosity` to lower or increase the drag coefficient of a particular class of Sphere.\n
     If unspecified, the global `simul:viscosity` is used.
     */
    real         viscosity;
    
    /// if true, use special formula to calculate mobilities
    /**
     This formula is derived from Lubrication theory and applies
     in the case where the sphere fits tightly in an elongated volume.
     */
    bool         piston_effect;
    
    /// flag to include steric interaction for this object
    int          steric;
    
    /// distance added to the radius to set the steric interaction distance
    real         steric_range;
    
    /// flag to confine this object
    Confinement  confine;
    
    /// confinement stiffness (also known as `confine[1]`)
    real         confine_stiffness;
    
    /// name of space used for confinement (also known as `confine[2]`)
    std::string  confine_space;
    
    /// display parameters (see @ref PointDispPar)
    std::string  display;
    
    /// @}
    
    /// derived variable: flag to indicate that `display` has a new value
    bool         display_fresh;

    /// derived variable: parameters derived from string `display`
    PointDisp *  disp;

private:
    
    /// pointer to actual confinement Space, derived from `confine_space`
    Space const* confine_space_ptr;

public:
        
    /// constructor
    SphereProp(const std::string& n) : Property(n), disp(nullptr) { clear(); }

    /// destructor
    ~SphereProp() { }
    
    /// identifies the property
    std::string category() const { return "sphere"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(Simul const&);

    /// return a carbon copy of object
    Property* clone() const { return new SphereProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

