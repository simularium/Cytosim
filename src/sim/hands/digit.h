// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DIGIT_H
#define DIGIT_H

#include "hand.h"
#include "digit_prop.h"


/// A Hand that bind to discrete sites along a Fiber
/**
 The Digit is a Hand that can only bind at discrete positions along a Fiber,
 corresponding to the Lattice associated with this Fiber.
 
 Binding will be limited by the occupancy stored in the Lattice:
 - the digit will bind only at a vacant lattice site,
 - upon binding, the digit will occupy the site entirely
 .
 
 See Examples and the @ref DigitPar.
 @ingroup HandGroup 
 
 As defined in Hand, detachment increases exponentially with force.
 */
class Digit : public Hand
{
    /// disabled default constructor
    Digit();
    
public:
    
    /// Property
    DigitProp const* prop;
    
    /// constructor
    Digit(DigitProp const*, HandMonitor*);
    
    /// destructor
    ~Digit() {}
    
    //--------------------------------------------------------------------------

#if FIBER_HAS_LATTICE

    /// true if given Lattice's site is outside Lattice's range
    bool outsideMP(lati_t s) const { return fbLattice->outsideMP(s); }

    /// true if given Lattice's site is occupied
    bool unavailable(FiberLattice* lat, lati_t s) const { return lat->data(s) & prop->footprint; }

    /// true if given Lattice's site is unoccupied (check footprint bits)
    bool vacant(lati_t s) const { return 0 == (fbLattice->data(s) & prop->footprint); }

    /// flip footprint bits on current site
    void inc() { fbLattice->data(fbSite) ^= prop->footprint; }

    /// flip footprint bits on current site
    void dec() { fbLattice->data(fbSite) ^= prop->footprint; }
    
#else

    lati_t site() const { return std::round(fbAbs/prop->step_size); }
    bool outsideMP(lati_t s) const { return fiber()->outsideMP((s+0.5)*prop->step_size); }
    bool vacant(lati_t) const { return true; }
    void inc() {}
    void dec() {}
    
#endif
    
    /// check if attachement is possible according to properties
    bool   attachmentAllowed(FiberSite&) const;

    /// attach and update variables
    void   attach(FiberSite const&);
    
    /// detach
    void   detach();

    
    /// transfer to given site
    void   hop(lati_t);

    /// transfer to given site if it is vacant
    void   jumpTo(lati_t p) { if ( vacant(p) ) hop(p); }
    
    /// relocate without checking intermediate sites
    void   jumpToEndM() { jumpTo(lattice()->indexM()); }

    /// relocate without checking intermediate sites
    void   jumpToEndP() { jumpTo(lattice()->indexP()); }

    
    /// attempt one step towards the PLUS_END
    void   stepP()      { jumpTo(site()+1); }
    
    /// attempt one step towards the MINUS_END
    void   stepM()      { jumpTo(site()-1); }

    
    /// attempt one step of size `s` towards the PLUS_END
    void   jumpP(int s) { jumpTo(site()+s); }
    
    /// attempt one step of size `s` towards the MINUS_END
    void   jumpM(int s) { jumpTo(site()-s); }

    
    /// attempt `n` steps towards the PLUS_END, checking all intermediate sites
    void   crawlP(int n);
    
    /// attempt `n` steps towards the MINUS_END, checking all intermediate sites
    void   crawlM(int n);

    
    /// simulate when `this` is attached but not under load
    void   stepUnloaded();
    
    /// simulate when `this` is attached and under load
    void   stepLoaded(Vector const& force, real force_norm);
 
    
    /// this is called when the attachment point is beyond the PLUS_END
    void   handleDisassemblyM();
    
    /// this is called when the attachment point is below the MINUS_END
    void   handleDisassemblyP();
    
};

/// output operator
std::ostream& operator << (std::ostream&, Digit const&);

#endif

