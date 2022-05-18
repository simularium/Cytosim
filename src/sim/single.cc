// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "sim.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "glossary.h"
#include "iowrapper.h"
#include "single.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"
#include "meca.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------
Single::Single(SingleProp const* p, Vector const& w)
: sPos(w), sHand(nullptr), prop(p)
{
    assert_true(prop->hand_prop);
    sHand = prop->hand_prop->newHand(this);
    assert_true(sHand);
}


Single::~Single()
{
    if ( sHand  &&  sHand->attached() )
        sHand->detach();

    if ( linked() )
        objset()->remove(this);
    
    delete(sHand);
    sHand = nullptr;
    prop = nullptr;
}

//------------------------------------------------------------------------------
#pragma mark -

void Single::afterAttachment(Hand const*)
{
    assert_true( attached() );
    // link into correct SingleSet sublist:
    SingleSet * set = static_cast<SingleSet*>(objset());
    if ( set )
        set->relinkA(this);
}


void Single::beforeDetachment(Hand const* h)
{
    assert_true( h == sHand );
    
#if ( DIM < 2 )
    /*
     Relocate Single to the position where it is attached.
     This is necessary to start the diffusion process from the correct location
     */
    sPos = h->posHand();
#else
    /*
     Set position near the attachment point, but offset in the perpendicular
     direction at a random distance within the range of attachment of the Hand.
     
     This is necessary to achieve detailed balance, which in particular implies
     that rounds of binding/unbinding should not get the Singles closer to
     the Filaments to which they bind.
     */
    sPos = h->posHand() + h->dirFiber().randOrthoB(h->prop->binding_range);
#endif

    // link into correct SingleSet sublist:
    SingleSet * set = static_cast<SingleSet*>(objset());
    if ( set )
        set->relinkD(this);
}


//------------------------------------------------------------------------------
#pragma mark -


Vector Single::position() const
{
    if ( sHand->attached() )
        return sHand->pos();
    return sPos;
}

void Single::foldPosition(Modulo const* m)
{
    m->fold(sPos);
}

void Single::randomizePosition()
{
    sPos = prop->confine_space_ptr->randomPlace();
}


void Single::stepF(Simul& sim)
{
    assert_false( sHand->attached() );

#if NEW_MOBILE_SINGLE
    // translation:
    sPos += prop->speed_dt;
#endif

    // diffusion:
    sPos.addRand(prop->diffusion_dt);
    
    // confinement:
    if ( prop->confine == CONFINE_INSIDE )
    {
        if ( !prop->confine_space_ptr->inside(sPos) )
            sPos = prop->confine_space_ptr->bounce(sPos);
        if ( modulo )
            modulo->fold(sPos);
    }
    else if ( prop->confine == CONFINE_ON )
    {
        sPos = prop->confine_space_ptr->project(sPos);
    }
    
    sHand->stepUnattached(sim, sPos);
}


void Single::stepA()
{
    assert_true( sHand->attached() );
    assert_true( !hasForce() );

#if NEW_MOBILE_SINGLE
    // translation:
    sPos += prop->speed_dt;
#endif
    
    sHand->stepUnloaded();
}

/**
 Add confinement force to the bound fiber
 */
void Single::setInteractions(Meca & meca) const
{
    assert_true( sHand->attached() );
}

//------------------------------------------------------------------------------
#pragma mark -

void Single::write(Outputter& out) const
{
    sHand->write(out);
    out.writeFloats(sPos, DIM);
}


void Single::read(Inputter& in, Simul& sim, ObjectTag tag)
{
    const bool s = attached();
    sHand->read(in, sim);
    in.readFloats(sPos, DIM);
    
    /*
     Because the SingleSet has 2 lists where Single are stored depending
     on their bound/unbound state, we need to unlink and relink here, in
     case the state stored on file is different from the current state.
     */
    if ( s != attached() )
    {
        SingleSet * set = static_cast<SingleSet*>(objset());
        if ( set )
        {
            if ( s )
                set->relinkD(this);
            else
                set->relinkA(this);
        }
    }
}

