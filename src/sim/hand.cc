// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "hand.h"
#include "hand_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "fiber_prop.h"
#include "fiber_grid.h"
#include "simul.h"
#include "sim.h"

//------------------------------------------------------------------------------

Hand::Hand(HandProp const* p, HandMonitor* m)
 : haNext(nullptr), haPrev(nullptr), haMonitor(m), prop(p)
{
    // initialize in unattached state:
    nextDetach = 0;
}


Hand::~Hand()
{
    // the Hands should be detached in ~Couple and ~Single
    assert_true(!fbFiber);
    prop = nullptr;
}


Hand * Hand::otherHand() const
{
    return haMonitor->otherHand(this);
}


Vector Hand::otherPosition() const
{
    return haMonitor->otherPosition(this);
}


real Hand::linkStiffness() const
{
    return haMonitor->linkStiffness();
}


void Hand::resetTimers()
{
    // initialize the Gillespie counters:
    if ( attached() )
    {
        nextDetach = RNG.exponential();
    }
    else
    {
        nextDetach = 0;
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void Hand::relocate(Fiber* f)
{
    assert_true(f);
    if ( fbFiber )
    {
        fbFiber->removeHand(this);
        fbFiber = f;
#if FIBER_HAS_LATTICE
        if ( fbLattice )
            fbLattice = &f->lattice();
#endif
    }
    f->addHand(this);
    reinterpolate();
}


void Hand::relocate(Fiber* f, const real a)
{
    assert_true(f);
    if ( f != fbFiber )
    {
        if ( fbFiber )
        {
            fbFiber->removeHand(this);
            fbFiber = f;
#if FIBER_HAS_LATTICE
            if ( fbLattice )
                fbLattice = &f->lattice();
#endif
        }
        f->addHand(this);
    }
    fbAbs = a;
    reinterpolate();
}


void Hand::moveToEnd(const FiberEnd end)
{
    assert_true(fbFiber);
    assert_true(end==PLUS_END || end==MINUS_END);
    
    if ( end == PLUS_END )
        FiberSite::relocateP();
    else
        FiberSite::relocateM();
}

//------------------------------------------------------------------------------
#pragma mark -

/**
Checks that all the conditions required for attachment are met
 */
bool Hand::attachmentAllowed(FiberSite& sit) const
{
    assert_true( sit.attached() );
    
    /*
     Check that the two binding keys match, allowing binding 
     according to the BITWISE AND of the two keys:
     */
    if ( ! ( prop->binding_key & sit.fiber()->prop->binding_key ) )
        return false;
        
    // check end-on binding:
    if ( sit.abscissaFromM() < 0 )
    {
        if ( prop->bind_also_end & MINUS_END )
            sit.relocateM();
        else
            return false;
    }
    else if ( sit.abscissaFromP() < 0 )
    {
        if ( prop->bind_also_end & PLUS_END )
            sit.relocateP();
        else
            return false;
    }
    
    FiberEnd end = NO_END;

    switch ( prop->bind_only_end )
    {
        case NO_END:
            break;
        case MINUS_END:
            if ( sit.abscissaFromM() > prop->bind_end_range )
                return false;       // too far from fiber end
            end = MINUS_END;
            break;
        case PLUS_END:
            if ( sit.abscissaFromP() > prop->bind_end_range )
                return false;       // too far from fiber end
            end = PLUS_END;
            break;
        case BOTH_ENDS:
        {
            if ( sit.abscissaFromM() > prop->bind_end_range )
            {
                // too far from MINUS_END
                if ( sit.abscissaFromP() > prop->bind_end_range )
                    return false;       // too far from PLUS_END
                end = PLUS_END;
            }
            else
            {
                // close from MINUS_END
                if ( sit.abscissaFromP() > prop->bind_end_range )
                    end = MINUS_END;    // too far from PLUS_END
                else
                    end = RNG.choice(MINUS_END, PLUS_END);
            }
        } break;
        default:
            throw Exception("illegal value of hand:bind_only_end");
    }

#if NEW_BIND_ONLY_FREE_END
    // check occupancy near the end (should be done with FiberLattice)
    if ( end != NO_END && prop->bind_only_free_end )
    {
        if ( 0 < sit.fiber()->nbHandsNearEnd(prop->bind_end_range, end) )
            return false;
    }
#endif
    
    // also check the Monitor's permissions:
    return haMonitor->allowAttachment(sit);
}


void Hand::locate(Fiber* f, real a)
{
    assert_true(f);
    assert_true(!fbFiber);
    //assert_true(f->abscissaM() <= a + REAL_EPSILON);
    //assert_true(a <= f->abscissaP() + REAL_EPSILON);

    fbAbs   = a;
    fbFiber = f;
    f->addHand(this);
    reinterpolate();
    haMonitor->afterAttachment(this);
    nextDetach = RNG.exponential();
}


void Hand::attach(FiberSite const& s)
{
    assert_true(s.attached());
    assert_true(!fbFiber);

    locate(s.fiber(), s.abscissa());
#if FIBER_HAS_LATTICE
    fbLattice = s.lattice();
    fbSite = s.site();
#endif
}


void Hand::detach()
{
    assert_true( attached() );
    haMonitor->beforeDetachment(this);
    fbFiber->removeHand(this);
    fbFiber = nullptr;
#if FIBER_HAS_LATTICE
    fbLattice = nullptr;
#endif
}


void Hand::detachHand()
{
    assert_true( attached() );
    fbFiber->removeHand(this);
    fbFiber = nullptr;
#if FIBER_HAS_LATTICE
    fbLattice = nullptr;
#endif
}

//------------------------------------------------------------------------------
#pragma mark -


void Hand::checkFiberRange()
{
    assert_true( attached() );
    
    if ( fbAbs < fbFiber->abscissaM() )
        handleDisassemblyM();
    else if ( fbAbs > fbFiber->abscissaP() )
        handleDisassemblyP();
}


void Hand::handleDisassemblyM()
{
    if ( RNG.test(prop->hold_shrinking_end) )
        relocateM();
    else
        detach();
}

void Hand::handleDisassemblyP()
{
    if ( RNG.test(prop->hold_shrinking_end) )
        relocateP();
    else
        detach();
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Test for attachment to nearby Fibers
 */
void Hand::stepUnattached(Simul& sim, Vector const& pos)
{
    assert_true( unattached() );

    sim.fiberGrid.tryToAttach(pos, *this);
}


/**
 By default, the unloaded Hand does nothing
 */
void Hand::stepUnloaded()
{
    assert_true( attached() );
    
    testDetachment();
}


/**
 By default, the loaded Hand does nothing
 */
void Hand::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );
    
    if ( prop->unbinding_force_inv > 0 )
        testKramersDetachment(force_norm);
    else
        testDetachment();
}


//------------------------------------------------------------------------------
#pragma mark -


void Hand::write(Outputter& out) const
{
    /*
     it is not necessary to write the property number here,
     since it is set when the Hand is created in class Single or Couple.
     */
    FiberSite::write(out);
}


void Hand::read(Inputter& in, Simul& sim)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 32 )
        prop = sim.findProperty<HandProp>("hand",in.readUInt16());
#endif
    
    Fiber * fib = fbFiber;
    FiberSite::read(in, sim);
    resetTimers();
    
    // update fiber's lists:
    if ( fib != fbFiber )
    {
        if ( fib )
            fib->removeHand(this);
        if ( fbFiber )
            fbFiber->addHand(this);
    }
}

std::ostream& operator << (std::ostream& os, Hand const& obj)
{
    os << "hand(" << obj.fiber()->reference() << ", " << obj.abscissa() << ")";
    return os;
}
