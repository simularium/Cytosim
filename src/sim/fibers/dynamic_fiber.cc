// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "dynamic_fiber.h"
#include "dynamic_fiber_prop.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
#include "space.h"


/**
 By default, both ends are growing
 */
DynamicFiber::DynamicFiber(DynamicFiberProp const* p) : Fiber(p), prop(p)
{
    // set PLUS_END as growing
    unitP[0] = 1;
    unitP[1] = 1;
    unitP[2] = 1;
    mStateP  = calculateStateP();
    mGrowthP = 0;
    
    nextGrowthP = RNG.exponential();
    nextHydrolP = RNG.exponential();
    nextShrinkP = RNG.exponential();

    // set MINUS_END as growing
    unitM[0] = 1;
    unitM[1] = 1;
    unitM[2] = 1;
    mStateM  = calculateStateM();
    mGrowthM = 0;

    nextGrowthM = RNG.exponential();
    nextHydrolM = RNG.exponential();
    nextShrinkM = RNG.exponential();
}


DynamicFiber::~DynamicFiber()
{
    prop = nullptr;
}


//------------------------------------------------------------------------------
#pragma mark -


state_t DynamicFiber::calculateStateM() const
{ 
    return 4 - unitM[0] - 2 * unitM[1];
}


state_t DynamicFiber::dynamicStateM() const
{
    return STATE_WHITE;
    assert_true( mStateM == calculateStateM() );
    return mStateM;
}


void DynamicFiber::setDynamicStateM(state_t s)
{
    if ( s < 1 || 4 < s )
        throw InvalidParameter("Invalid AssemblyState for DynamicFiber MINUS_END");
    
    if ( s != mStateM )
    {
        mStateM  = s;
        unitM[1] = ( 4 - s ) / 2;
        unitM[0] = ( 4 - s - 2*unitM[1] );
        assert_true( 0==unitP[0] || unitP[0]==1 );
        assert_true( 0==unitP[1] || unitP[1]==1 );
        assert_true( mStateM == calculateStateM() );
    }
}


int DynamicFiber::stepMinusEnd()
{
    int res = 0;
    real chewing_rate = 0;
    
    // add chewing rate to stochastic off rate:
    
#if NEW_FIBER_CHEW
    
    // convert chewing rate to stochastic off rate:
    if ( frChewM > prop->max_chewing_speed_dt )
        chewing_rate = prop->max_chewing_speed_dt / prop->unit_length;
    else
        chewing_rate = frChewM / prop->unit_length;
    
    //std::clog << " chewing rate M " << chewing_rate / prop->time_step << std::endl;
    frChewM = 0;
    
#endif
    
    nextShrinkM -= prop->shrinking_rate_dt[1] + chewing_rate;
    while ( nextShrinkM <= 0 )
    {
        --res;
        nextShrinkM += RNG.exponential();
    }
    return res;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 The microscopic state correspond to:
 - STATE_GREEN for growth,
 - STATE_RED for shrinkage
 .
 */
state_t DynamicFiber::calculateStateP() const
{
    return 4 - unitP[0] - 2 * unitP[1];
}


state_t DynamicFiber::dynamicStateP() const
{
    assert_true( mStateP == calculateStateP() );
    return mStateP;
}


void DynamicFiber::setDynamicStateP(state_t s)
{
    if ( s < 1 || 4 < s )
        throw InvalidParameter("Invalid AssemblyState for DynamicFiber PLUS_END");
    
    if ( s != mStateP )
    {
        mStateP   = s;
        unitP[1] = ( 4 - s ) / 2;
        unitP[0] = ( 4 - s - 2*unitP[1] );
        assert_true( 0==unitP[0] || unitP[0]==1 );
        assert_true( 0==unitP[1] || unitP[1]==1 );
        assert_true( mStateP == calculateStateP() );
    }
}


/**
 Using a modified Gillespie scheme with a variable rate.
 
 returns the number of units added (if result > 0) or removed (if < 0)
 */
int DynamicFiber::stepPlusEnd()
{
    int res = 0;
    real chewing_rate = 0;
    
#if NEW_FIBER_CHEW
    
    // convert chewing rate to stochastic off rate:
    ///@todo implement smooth saturation using logistic function
    if ( frChewP > prop->max_chewing_speed_dt )
        chewing_rate = prop->max_chewing_speed_dt / prop->unit_length;
    else
        chewing_rate = frChewP / prop->unit_length;
    
    //std::clog << " chewing rate P " << chewing_rate / prop->time_step << std::endl;
    frChewP = 0;
#endif
    
    if ( mStateP == STATE_RED )
    {
        nextShrinkP -= prop->shrinking_rate_dt[0] + chewing_rate;
        while ( nextShrinkP <= 0 )
        {
            --res;
            nextShrinkP += RNG.exponential();
        }
    }
    else
    {
        // calculate the force acting on the point at the end:
        real force = projectedForceEndP();
        
        // growth is reduced if free monomers are scarce:
        real growth_rate = prop->growing_rate_dt[0] * prop->free_polymer;
        
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( force < 0  &&  prop->growing_force[0] < INFINITY )
            growth_rate *= exp(force/prop->growing_force[0]);

        real hydrol_rate = prop->hydrolysis_rate_2dt[0];
        
#if ( 0 )
        // change Hydrolysis rate if PLUS_END is far from origin:
        if ( posEndP().normSqr() > prop->zone_radius_sqr )
            hydrol_rate = prop->zone_hydrolysis_rate_2dt[0];
        
        if ( prop->zone_space_ptr && !prop->zone_space_ptr->inside(posEndP()) )
            hydrol_rate = prop->zone_hydrolysis_rate_2dt[0];
#endif
        
        // @todo detach_rate should depend on the state of the subunit
        real detach_rate = prop->growing_off_rate_dt[0] + chewing_rate;
        
        nextGrowthP -= growth_rate;
        nextShrinkP -= detach_rate;
        nextHydrolP -= hydrol_rate;
        
        while ( nextGrowthP < 0 || nextShrinkP < 0 || nextHydrolP < 0 )
        {
            // Select the earliest event:
            real a = nextGrowthP/growth_rate;
            real b = nextHydrolP/hydrol_rate;
            int ii = ( b < a );
            if ( detach_rate != 0 )
            {
                real c = nextShrinkP/detach_rate;
                if ( c < std::min(a,b) )
                    ii = 2;
            }
            
            switch ( ii )
            {
                case 0:
                    // add fresh unit, shifting old terminal to penultimate position
                    unitP[2] = unitP[1];
                    unitP[1] = unitP[0];
                    unitP[0] = 1;
                    ++res;
                    nextGrowthP += RNG.exponential();
                    break;
                    
                case 1:
                    // hydrolyze one of the unit with equal chance:
                    unitP[RNG.flip()] = 0;
                    nextHydrolP += RNG.exponential();
                    break;

                case 2:
                    // remove last unit, assuming hydrolysis already occurred below
                    unitP[0] = unitP[1];
                    unitP[1] = unitP[2];
                    unitP[2] = 0;
                    --res;
                    nextShrinkP += RNG.exponential();
                    break;
            }
            
            mStateP = calculateStateP();
        }
    }
    return res;
}


//------------------------------------------------------------------------------
#pragma mark -

void DynamicFiber::step()
{
    // perform stochastic simulation:
    int incP = stepPlusEnd();
    int incM = stepMinusEnd();

    mGrowthP = incP * prop->unit_length;
    mGrowthM = incM * prop->unit_length;

    if ( incM || incP )
    {
        if ( length() + mGrowthM + mGrowthP < prop->min_length )
        {
            // do something if the fiber is too short:
            if ( !prop->persistent )
            {
                delete(this);
                // exit to avoid doing anything with a dead object:
                return;
            }
            // possibly rescue:
            if ( RNG.test(prop->rebirth_prob[0]) )
                setDynamicStateP(STATE_GREEN);
        }
        else if ( length() + mGrowthM + mGrowthP < prop->max_length )
        {
            if ( incP ) growP(mGrowthP);
            if ( incM ) growM(mGrowthM);
            //std::clog << reference() << " " << mGrowthM << " " << mGrowthP << " " << length() << '\n';
        }
    }

    Fiber::step();
}


//------------------------------------------------------------------------------
#pragma mark -


void DynamicFiber::write(Outputter& out) const
{
    Fiber::write(out);
    
    /// write variables describing the dynamic state of the ends:
    writeHeader(out, TAG_DYNAMIC);
    out.writeUInt8(unitM[0]);
    out.writeUInt8(unitM[1]);
    out.writeUInt8(unitP[0]);
    out.writeUInt8(unitP[1]);
}


void DynamicFiber::read(Inputter& in, Simul& sim, ObjectTag tag)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( tag == TAG_DYNAMIC || ( tag == TAG && in.formatID() < 44 ) )
#else
    if ( tag == TAG_DYNAMIC )
#endif
    {
        unitM[0] = in.readUInt8();
        unitM[1] = in.readUInt8();
        mStateM  = calculateStateM();

        unitP[0] = in.readUInt8();
        unitP[1] = in.readUInt8();
        mStateP  = calculateStateP();
    }
#ifdef BACKWARD_COMPATIBILITY
    if ( tag != TAG_DYNAMIC || in.formatID() < 44 )
#else
    else
#endif
        Fiber::read(in, sim, tag);
}

