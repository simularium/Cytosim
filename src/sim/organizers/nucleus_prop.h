// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NUCLEUS_PROP_H
#define NUCLEUS_PROP_H

#include "real.h"
#include "property.h"


/// Property for Nucleus
/**
 @ingroup Properties
*/
class NucleusProp : public Property
{
    friend class Nucleus;
        
public:
    
    /**
     @defgroup NucleusPar Parameters of Nucleus
     @ingroup Parameters
     @{
     */

    /// stiffness of links
    real          stiffness;
    
    /// @}

public:
 
    /// constructor
    NucleusProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~NucleusProp() { }
    
    /// identifies the property
    std::string category() const { return "nucleus"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(Simul const&);
    
    /// return a carbon copy of object
    Property* clone() const { return new NucleusProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};

#endif

