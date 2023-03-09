// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "digit.h"
#include "digit_prop.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "messages.h"
#include "glossary.h"
#include "lattice.h"
#include "simul.h"


Digit::Digit(DigitProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}

bool Digit::attachmentAllowed(FiberSite& sit) const
{
    if ( Hand::attachmentAllowed(sit) )
    {
#if FIBER_HAS_LATTICE
        FiberLattice * lat = &sit.fiber()->lattice();
        
        if ( !lat->ready() )
            throw InvalidParameter("fiber:lattice was not defined");
        
        // index to site containing given abscissa:
        lati_t s = lat->index(sit.abscissa());

        if ( lat->outsideMP(s) || unavailable(lat, s) )
            return false;
        
        // adjust to match selected lattice site:
        sit.engageLattice(lat, s, prop->site_shift);
#endif
        return true;
    }
    return false;
}


/**
 Digit::attachmentAllowed() should have been called before,
 such that the `sit` already points to a valid lattice site
 */
void Digit::attach(FiberSite const& sit)
{
    Hand::attach(sit);
    assert_true(vacant(site()));
    inc();
}


void Digit::detach()
{
    dec();
#if 0
    // testing the value of the lattice after detachment:
    FiberLattice::cell_t c = lattice()->data(site());
    if ( c & prop->footprint )
        std::clog << *this << " detach " << (int)c << "\n";
#endif
    Hand::detach();
}

//------------------------------------------------------------------------------
#pragma mark -

void Digit::hop(lati_t s)
{
    assert_true( attached() );
#if FIBER_HAS_LATTICE
    dec();
    fbSite = s;
    inc();
    fbAbs = s * lattice()->unit() + prop->site_shift;
#else
    fbAbs = s * prop->step_size + prop->site_shift;
#endif
    reinterpolate();
}

//------------------------------------------------------------------------------

/**
 Try to move `n` sites in the PLUS_END direction,
 stopping if any intermediate position is already occupied.
 */
void Digit::crawlP(const int n)
{
    lati_t s = site();
    lati_t e = s + n;
    
    while ( s < e )
    {
        if ( vacant(s+1) )
            ++s;
        else
            break;
    }
    if ( s != site() )
        hop(s);
}


/**
 Try to move `n` sites in the MINUS_END direction,
 stopping if any intermediate position is already occupied.

 */
void Digit::crawlM(const int n)
{
    lati_t s = site();
    lati_t e = s - n;
    
    while ( s > e )
    {
        if ( vacant(s-1) )
            --s;
        else
            break;
    }
    if ( s != site() )
        hop(s);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 The Digit normally does not move by itself
 */
void Digit::handleDisassemblyM()
{
    assert_true( attached() );
    
    if ( RNG.test(prop->hold_shrinking_end) )
    {
        jumpToEndM();
        if ( site() < lattice()->indexM() )
            detach();
    }
    else
        detach();
}


/**
 The Digit normally does not move by itself
 */
void Digit::handleDisassemblyP()
{
    assert_true( attached() );
    
    if ( prop->hold_shrinking_end )
    {
        jumpToEndP();
        if ( site() > lattice()->indexP() )
            detach();
    }
    else
        detach();
}


/**
tests detachment
 */
void Digit::stepUnloaded()
{
    assert_true( attached() );
    testDetachment();
}


/**
(see @ref Stochastic)
 */
void Digit::stepLoaded(Vector const& force, real force_norm)
{
    assert_true( attached() );
    assert_true( nextDetach >= 0 );

    testKramersDetachment(force_norm);
}

//------------------------------------------------------------------------------
#pragma mark -

std::ostream& operator << (std::ostream& os, Digit const& obj)
{
    os << obj.property()->name() << "(" << obj.fiber()->reference() << ", " << obj.abscissa();
    os << ", " << obj.site() << ")";
    return os;
}


#if FIBER_HAS_LATTICE
void Fiber::resetLattice()
{
    frLattice.clear();
    
    for ( Hand * ha = handListFront; ha; ha = ha->next() )
    {
        if ( ha->lattice() == &frLattice )
            static_cast<Digit*>(ha)->inc();
    }
}
#endif

