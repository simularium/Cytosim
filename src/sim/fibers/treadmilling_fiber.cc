// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "assert_macro.h"
#include "treadmilling_fiber.h"
#include "treadmilling_fiber_prop.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
#include "space.h"


//------------------------------------------------------------------------------

TreadmillingFiber::TreadmillingFiber(TreadmillingFiberProp const* p) : Fiber(p), prop(p)
{
    mStateM  = STATE_WHITE;
    mGrowthM = 0;
    
    mStateP  = STATE_WHITE;
    mGrowthP = 0;
}


TreadmillingFiber::~TreadmillingFiber()
{
    prop = nullptr;
}


//------------------------------------------------------------------------------
#pragma mark -

void TreadmillingFiber::setDynamicStateM(state_t s)
{
    if ( s == STATE_WHITE || s == STATE_GREEN || s == STATE_RED )
        mStateM = s;
    else
        throw InvalidParameter("Invalid AssemblyState for TreadmillingFiber MINUS_END");
}


void TreadmillingFiber::setDynamicStateP(state_t s)
{
    if ( s == STATE_WHITE || s == STATE_GREEN || s == STATE_RED )
        mStateP = s;
    else
        throw InvalidParameter("Invalid AssemblyState for TreadmillingFiber PLUS_END");
}


//------------------------------------------------------------------------------

void TreadmillingFiber::step()
{    
    constexpr int P = 0, M = 1;

    if ( mStateP == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real forceP = projectedForceEndP();
        
        // growth is reduced if free monomers are scarce:
        mGrowthP = prop->growing_speed_dt[P] * prop->free_polymer;
        
        assert_true(mGrowthP>=0);
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( forceP < 0  &&  prop->growing_force[P] < INFINITY )
            mGrowthP *= exp(forceP/prop->growing_force[P]);
    }
    else if ( mStateP == STATE_RED )
    {
        mGrowthP = prop->shrinking_speed_dt[P];
    }
    else
    {
        mGrowthP = 0;
    }
    
    
    if ( mStateM == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real forceM = projectedForceEndM();
        
        // growth is reduced if free monomers are scarce:
        mGrowthM = prop->growing_speed_dt[M] * prop->free_polymer;

        assert_true(mGrowthM>=0);
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( forceM < 0  &&  prop->growing_force[M] < INFINITY  )
            mGrowthM *= exp(forceM/prop->growing_force[M]);
    }
    else if ( mStateM == STATE_RED )
    {
        mGrowthM = prop->shrinking_speed_dt[M];
    }
    else
    {
        mGrowthM = 0;
    }
    
    
    real len = length();
    real inc = mGrowthP + mGrowthM;
    if ( inc < 0  &&  len + inc < prop->min_length )
    {
        // the fiber is too short, we delete it:
        delete(this);
        return;
    }
    else if ( len + inc < prop->max_length )
    {
        if ( mGrowthM ) growM(mGrowthM);
        if ( mGrowthP ) growP(mGrowthP);
    }
    else if ( len < prop->max_length )
    {
        // the remaining possible growth is distributed to the two ends:
        inc = ( prop->max_length - len ) / inc;
        if ( mGrowthM ) growM(inc*mGrowthM);
        if ( mGrowthP ) growP(inc*mGrowthP);
    }

    Fiber::step();
}

                  
//------------------------------------------------------------------------------
#pragma mark -


void TreadmillingFiber::write(Outputter& out) const
{
    Fiber::write(out);

    /// write variables describing the dynamic state of the ends:
    writeHeader(out, TAG_DYNAMIC);
    out.writeUInt16(mStateM);
    out.writeUInt16(mStateP);
}


void TreadmillingFiber::read(Inputter& in, Simul& sim, ObjectTag tag)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( tag == TAG_DYNAMIC || ( tag == TAG && in.formatID() < 44 ) )
#else
    if ( tag == TAG_DYNAMIC )
#endif
    {
        mStateM = in.readUInt16();
        mStateP = in.readUInt16();
    }
#ifdef BACKWARD_COMPATIBILITY
    if ( tag != TAG_DYNAMIC || in.formatID() < 44 )
#else
    else
#endif
        Fiber::read(in, sim, tag);
}

