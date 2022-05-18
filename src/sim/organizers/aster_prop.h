// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ASTER_PROP_H
#define ASTER_PROP_H

#include "real.h"
#include "property.h"
#include "common.h"

class FiberSet;


/// Property for Aster
/**
 @ingroup Properties
 
 Note: the Aster can be constrained by confining the Solid around which it is built.
 */
class AsterProp : public Property
{
    friend class Aster;
    
public:
    
    /**
     @defgroup AsterPar Parameters of Aster
     @ingroup Parameters
     @{
     */

    /// stiffness of links between the Solid and the Fiber
    /**
     - stiffness[0] is the link between the central point of the Solid 
       and the focus-end of the Fiber
     - stiffness[1] is the link between a secondary point of the Solid and an
       intermediate point of the Fiber. This holds the Fiber in direction.
     .
    */
    real          stiffness[2];
    
    /// designates which end of the fiber is towards the center
    FiberEnd      focus;
    
    /// rate at which a new fiber is created at an unoccupied site (known as nucleate)
    real          fiber_rate;
    
    /// name of Fiber that make up the Aster (know as `nucleate[1]`)
    std::string   fiber_type;
    
    /// specifications for initial fibers (also known as `nucleate[2]`)
    std::string   fiber_spec;

    /// @}
    
    
private:
    
    /// probability of nucleation
    real         fiber_prob;

public:
    
    /// constructor
    AsterProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~AsterProp() { }
    
    /// identifies the property
    std::string category() const { return "aster"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new AsterProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

