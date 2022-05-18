// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DUO_H
#define DUO_H

#include "couple.h"
class DuoProp;

/// A specialized kind of Couple
/**
 The Duo is a couple that can be active or inactive:
 - it is activated instantly inside a given space,
 - is is deactivated spontaneously with the given rate.
 .
 See DuoProp
 
 The DuoLong is automatically selected for non-zero-resting length.
 @ingroup CoupleGroup
 */
class Duo : public Couple
{
    friend class DuoProp;
    
    /// Gillespie countdown timer for deactivation event
    real    gspTime;
    
    /// switch on activity flag
    void    activate();
    
    /// switch off activity flag
    void    deactivate();
    
    /// check for deactivation
    void    deactivation();
    
protected:
    
    /// active flag
    int     mActive;
    
public:
    
    /// property
    DuoProp const* prop;
    
    /// constructor
    Duo(DuoProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Duo();
    
    /// activity flag
    bool    active() const { return mActive; }
    
    /// simulation step for a free Duo
    void    stepFF(Simul&);
    
    /// simulation step for a Duo attached by Hand1
    void    stepAF(Simul&);
    
    /// simulation step for a Duo attached by Hand2
    void    stepFA(Simul&);
    
    /// simulation step for a linking Duo
    void    stepAA();

    /// write to file
    void    write(Outputter&) const;
    
    /// read from file
    void    read(Inputter&, Simul&, ObjectTag);

};


#endif

