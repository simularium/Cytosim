// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_H
#define FIBER_H

#include <set>
#include <stdint.h>
#include "mecafil.h"
#include "fiber_prop.h"
#include "node_list.h"
#include "lattice.h"
#include "sim.h"


class Hand;
class Field;
class Single;
class FiberSet;
class FiberSegment;
class LineDisp;


/// Flag to associate a Lattice to the Fiber {0, 1}
#define FIBER_HAS_LATTICE 0

/// Lattice composed of integers, appropriate for discrete occupancy
typedef Lattice<uint64_t> FiberLattice;

/// a Mecafil to which many Hand may bind
/**
 The Fiber extends the Mecafil (itself build on Chain), adding in particular
 methods that are necessary to simulate the attachment/detachment of Hand.
 It also adds a Lattice object and a FiberProp to hold parameters.
 
 - `FiberProp * prop` points to the physical properties (ie. parameters) of the Fiber.
 - `FiberDisp * disp` points to display parameters (not used in sim).
 - `handListFront` and `handListBack` keep track of all attached Hands.
 .
 
 if prop->lattice is true, the Fiber will have a Lattice,
 which can be used by Digit and derived Hands, and other features.
 
 Fibers are stored in a FiberSet.
 */
class Fiber: public Mecafil
{
private:
    
    /// Disabled copy constructor
    Fiber(Fiber const&);
    
    /// disabled assignment operator
    Fiber& operator =(const Fiber&);

    /// Stores the information needed to sever a Fiber
    class SeverPos
    {
    public:
        real    abs;      ///< abscissa of the cut, from the reference
        state_t stateM;   ///< state of the new MINUS_END
        state_t stateP;   ///< state of the new PLUS_END
        
        /// constructor (abscissa, new_plus_end_state, new_minus_end_state)
        SeverPos(real a, state_t p, state_t m) { abs=a; stateP=p; stateM=m; }
        
        /// sort from PLUS_END to MINUS_END, i.e. with decreasing abscissa
        bool operator < (SeverPos const&b) const { return abs > b.abs; }
    };
    
    /// ordered list of future severing positions
    std::set<SeverPos>  pendingCuts;

    /// Pointer to hold a list of attached Hands
    mutable Hand *      handListFront;
    
    /// Pointer to hold a list of attached Hands
    mutable Hand *      handListBack;

#if FIBER_HAS_LATTICE
    /// Associated Lattice
    FiberLattice        frLattice;
#endif
    
    /// a grafted used to immobilize the Fiber
    Single *            frGlue;
    
protected:
#if NEW_FIBER_CHEW
    /// stored chewing at the end
    real                frChewM, frChewP;
#endif

    /// cut Fiber at point `pti`, return section `[ pti - PLUS_END ]`
    virtual Fiber* severPoint(unsigned int pti);
    
    /// return index of point where there is a kink
    unsigned       hasKink(real) const;

    
    /// viscous drag coefficient for a cylinder moving close to a surface
    static real    dragCoefficientSurface(real len, FiberProp const*);
    
    /// viscous drag coefficient for a cylinder moving in an infinite volume of fluid
    static real    dragCoefficientEllipsoid(real len, FiberProp const*);
    
    /// viscous drag coefficient for a cylinder moving in an infinite volume of fluid
    static real    dragCoefficientCylinder(real len, FiberProp const*);

public:
    
    /// the Property of this object
    FiberProp const*    prop;
    
    /// the display parameters
    LineDisp mutable*   disp;

    //--------------------------------------------------------------------------

    /// constructor
    Fiber(FiberProp const*);
    
    /// destructor
    virtual ~Fiber();

    //--------------------------------------------------------------------------
    
    /// calculate viscous drag coefficient
    void           setDragCoefficient();
    
    /// prepare for Meca
    void           prepareMecable();
    
    /// add interactions to a Meca
    void           setInteractions(Meca &) const;
    

    /// invert polarity and adjust abscissa of Hands to keep them at the same place
    void           flipHandsPolarity();
    
    /// remove the portion of size `len` that includes the MINUS_END
    void           cutM(real len);
    
    /// remove the portion of size `len` that includes the PLUS_END
    void           cutP(real len);
    
    /// Cut all segments intersecting the plane defined by <em> n.pos + a = 0 </em>
    void           planarCut(Vector const& n, real a, state_t stateP, state_t stateM);
    
    /// cut fiber at distance `abs` from the MINUS_END; returns section `[ abs - PLUS_END ]`
    Fiber *        severM(real abs);

    /// register a cut at abscissa `a` from the ORIGIN, with `m` and `p` the states of the new ends
    void           sever(real a, state_t p, state_t m) { pendingCuts.insert(SeverPos(a, p, m)); }
    
    /// perform all the cuts registered by sever()
    void           severNow();

#if NEW_FIBER_CHEW
    /// register a chewing quantity
    void           chew(const real x, FiberEnd end) { if ( end == PLUS_END ) frChewP += x; else frChewM += x; }
#endif

    /// call Chain::join(), and transfer Hands (caller should delete `fib`).
    virtual void   join(Fiber * fib);
    
    /// simulation step
    virtual void   step();
    
    /// called if a Fiber tip has elongated or shortened
    void           updateFiber();
    
    //--------------------------------------------------------------------------

    /// the energy due to bending rigidity: 1/2 * rigidity * sum( curvature(s)^2 ds ),
    real           bendingEnergy() const { return bendingEnergy0()*prop->rigidity; }
    
    /// return the abscissa of the closest position to `w` on this Fiber, and set `dis` to the square of the distance
    real           projectPoint(Vector const& w, real & dis) const;
    
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of MINUS_END
    virtual state_t dynamicStateM() const { return STATE_WHITE; }

    /// return assembly/disassembly state of PLUS_END
    virtual state_t dynamicStateP() const { return STATE_WHITE; }

    /// return assembly/disassembly state of the FiberEnd
    state_t         dynamicState(FiberEnd end) const;

    
    /// change state of MINUS_END
    virtual void   setDynamicStateM(state_t) {}

    /// change state of PLUS_END
    virtual void   setDynamicStateP(state_t) {}

    /// change state of FiberEnd `end` to `s`
    void           setDynamicState(FiberEnd end, state_t s);
    
    
    /// the length of freshly assembled polymer at the MINUS_END during the last time step
    virtual real   freshAssemblyM() const { return 0; }

    /// the length of freshly assembled polymer at the PLUS_END during the last time step
    virtual real   freshAssemblyP() const { return 0; }

    /// the length of freshly assembled polymer during the last time step
    real           freshAssembly(FiberEnd end) const;
    
    
    /// true if the tip `end` has grown in the last time step ( freshAssembly(which) > 0 )
    bool           isGrowing(FiberEnd end) const { return freshAssembly(end) > 0; }
    
    /// true if the tip `end` has shrunk in the last time step ( freshAssembly(which) < 0 )
    bool           isShrinking(FiberEnd end) const { return freshAssembly(end) < 0; }
    
    //--------------------------------------------------------------------------
    
    /// register a new Hands that attached to this Fiber
    void           addHand(Hand*) const;
    
    /// unregister bound Hands (which has detached)
    void           removeHand(Hand*) const;
    
    /// update all Hands bound to this
    void           updateHands() const;

    /// detach all Hands
    void           detachHands() const;
    
    /// sort Hands by order of increasing abscissa
    void           sortHands() const;
    
    /// return Hand bound to this fiber (use ->next() to access all other Hands)
    Hand *         firstHand() const { return handListFront; }
   
    /// number of attached Hands
    unsigned       nbHands() const;
    
    /// a function to count Hands using a custom criteria
    int            nbHands(int (*count)(Hand const*)) const;

    /// number of Hands attached within a range of abscissa
    unsigned       nbHandsInRange(real abs_min, real abs_max, FiberEnd ref) const;
    
    /// number of Hands attached at a distance less than 'len' from the specified FiberEnd
    unsigned       nbHandsNearEnd(real len, FiberEnd end) const;
    
    //--------------------------------------------------------------------------
#if FIBER_HAS_LATTICE
    /// modifiable reference to Fiber's Lattice
    FiberLattice&  lattice() { return frLattice; }
    
    /// const reference to Fiber's Lattice
    FiberLattice const&  lattice() const { return frLattice; }

    /// recalculate occupancy lattice from bound Hands
    void           resetLattice();
#else
    /// does nothing
    void           resetLattice() {}
#endif
    
    /// record minium, maximum and sum of lattice values
    void           infoLattice(real& len, unsigned&, real& sm, real& mn, real& mx) const;

    /// print Lattice data (for debugging purpose)
    void           printLattice(std::ostream&, FiberLattice const&) const;

    /// lattice to be displayed
    FiberLattice const* drawableLattice() const;

    //--------------------------------------------------------------------------
    
    /// set the box glue for pure pushing
    void           setGlue1(Single* glue, FiberEnd, Space const* space);
    
    /// set the box glue for pure pulling
    void           setGlue2(Single* glue, FiberEnd, Space const* space);
    
    /// set the box glue for pushing and pulling
    void           setGlue3(Single* glue, Space const* space);
    
    /// a setGlue to rule them all
    void           setGlue(Single*& glue, FiberEnd, Space const* space);
    
    /// create a Single that can be used as glue
    void           makeGlue(Single*& glue);
    
    //--------------------------------------------------------------------------

    /// a static_cast<> of Node::next()
    Fiber *  next()  const  { return static_cast<Fiber*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Fiber *  prev()  const  { return static_cast<Fiber*>(nPrev); }

    //--------------------------------------------------------------------------
    
    /// a unique character identifying the class
    static const ObjectTag TAG = 'f';
    
    /// identifies data for dynamic ends of fibers
    static const ObjectTag TAG_DYNAMIC = 'F';
    
    /// identifies FiberLattice data
    static const ObjectTag TAG_LATTICE = 'l';
    
    /// return unique character identifying the class
    ObjectTag       tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }
    
    /// convert pointer to Fiber* if the conversion seems valid; returns 0 otherwise
    static Fiber* toFiber(Object * obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Fiber*>(obj);
        return nullptr;
    }
    
    /// convert pointer to Fiber* if the conversion seems valid; returns 0 otherwise
    static Fiber const* toFiber(Object const* obj)
    {
        if ( obj  &&  obj->tag() == TAG )
            return static_cast<Fiber const*>(obj);
        return nullptr;
    }

    //--------------------------------------------------------------------------

    /// write to file
    void        write(Outputter&) const;
    
    /// read from file
    void        read(Inputter&, Simul&, ObjectTag);

};

#endif

