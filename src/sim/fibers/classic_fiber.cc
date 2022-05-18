// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "classic_fiber.h"
#include "classic_fiber_prop.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "messages.h"
#include "simul.h"
#include "space.h"


//------------------------------------------------------------------------------

ClassicFiber::ClassicFiber(ClassicFiberProp const* p) : Fiber(p), prop(p)
{
    mStateM  = STATE_WHITE;
    mStateP  = STATE_WHITE;
    mGrowthM = 0;
    mGrowthP = 0;
}


ClassicFiber::~ClassicFiber()
{
    prop = nullptr;
}


void ClassicFiber::setDynamicStateM(state_t s)
{
    if ( s==STATE_WHITE || s==STATE_GREEN || s==STATE_RED )
        mStateM = s;
    else
        throw InvalidParameter("Invalid AssemblyState for ClassicFiber MINUS_END");
}


void ClassicFiber::setDynamicStateP(state_t s)
{
    if ( s==STATE_WHITE || s==STATE_GREEN || s==STATE_RED )
        mStateP = s;
    else
        throw InvalidParameter("Invalid AssemblyState for ClassicFiber PLUS_END");
}


//------------------------------------------------------------------------------
#pragma mark -

/** 
 The catastrophe rate depends on the growth rate of the corresponding tip,
 which is itself reduced by antagonistic force. 
 The correspondance is : 1/rate = a + b * growthSpeed.
 
 For no force on the growing tip: rate = catastrophe_rate * time_step
 For very large forces          : rate = catastrophe_rate_stalled * time_step
 
 cf. `Dynamic instability of MTs is regulated by force`
 M.Janson, M. de Dood, M. Dogterom. JCB 2003, Figure 2 C.
 */
void ClassicFiber::step()
{
    constexpr int P = 0, M = 1;
    const real len = length();

    if ( mStateM == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real force = projectedForceEndM();
        
        // growth is reduced if free monomers are scarce:
        real spd = prop->growing_speed_dt[M] * prop->free_polymer;
        
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( force < 0  &&  prop->growing_force[M] < INFINITY )
            mGrowthM = spd * exp(force/prop->growing_force[M]) + prop->growing_off_speed_dt[M];
        else
            mGrowthM = spd + prop->growing_off_speed_dt[M];
        
        
        // catastrophe may be constant, or it may depend on the growth rate
        real cata;
        if ( prop->catastrophe_coef[M] > 0 )
            cata = prop->catastrophe_rate_stalled_dt[M] / ( 1.0 + prop->catastrophe_coef[M] * mGrowthM );
        else
            cata = prop->catastrophe_rate_dt[M];

        if ( RNG.test(cata) )
            mStateM = STATE_RED;
    }
    else if ( mStateM == STATE_RED )
    {
        mGrowthM = prop->shrinking_speed_dt[M];
        
        if ( RNG.test(prop->rescue_prob[M]) )
            mStateM = STATE_GREEN;
    }
    
    
    if ( mStateP == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real force = projectedForceEndP();
        
        // growth is reduced if free monomers are scarce:
        real spd = prop->growing_speed_dt[P] * prop->free_polymer;
        
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( force < 0  &&  prop->growing_force[P] < INFINITY )
            mGrowthP = spd * exp(force/prop->growing_force[P]) + prop->growing_off_speed_dt[P];
        else
            mGrowthP = spd + prop->growing_off_speed_dt[P];
        
        
        // catastrophe may be constant, or it may depend on the growth rate
        real cata;
        if ( prop->catastrophe_coef[P] > 0 )
            cata = prop->catastrophe_rate_stalled_dt[P] / ( 1.0 + prop->catastrophe_coef[P] * mGrowthP );
        else
            cata = prop->catastrophe_rate_dt[P];
        
#if NEW_CATASTROPHE_OUTSIDE
        // Catastrophe rate is multiplied if the PLUS_END is outside
        if ( prop->catastrophe_space_ptr->outside(posEndP()) )
        {
            LOG_ONCE("Fiber's plus-end catastrophe rate is different outside the Space\n");
            cata *= prop->catastrophe_outside;
        }
#endif

#if NEW_LENGTH_DEPENDENT_CATASTROPHE
        /*
         Ad-hoc length dependence, used to simulate S. pombe with catastrophe_length=5
         Foethke et al. MSB 5:241 - 2009
         */
        if ( prop->catastrophe_length > 0 )
        {
            LOG_ONCE("Using ad-hoc length-dependent catastrophe rate\n");
            cata *= length() / prop->catastrophe_length;
        }
#endif
        
        if ( RNG.test(cata) )
            mStateP = STATE_RED;
    }
    else if ( mStateP == STATE_RED )
    {
        mGrowthP = prop->shrinking_speed_dt[P];
        
        if ( RNG.test(prop->rescue_prob[P]) )
            mStateP = STATE_GREEN;
    }
    
    real inc = mGrowthP + mGrowthM;
    if ( inc < 0  &&  len + inc < prop->min_length )
    {
        // the fiber is too short, we may delete it:
        if ( !prop->persistent )
        {
            delete(this);
            return;
        }
   
        // we may regrow:
        if ( mStateM == STATE_RED && RNG.test(prop->rebirth_prob[M]) )
            mStateM = STATE_GREEN;
    
        if ( mStateP == STATE_RED && RNG.test(prop->rebirth_prob[P]) )
            mStateP = STATE_GREEN;
    }
    else if ( len + inc < prop->max_length )
    {
        if ( mGrowthM ) growM(mGrowthM);
        if ( mGrowthP ) growP(mGrowthP);
    }
    else if ( 0 < inc  &&  len < prop->max_length )
    {
        // the remaining possible growth is distributed to the two ends:
        inc = ( prop->max_length - len ) / inc;
        if ( mGrowthM ) growM(inc*mGrowthM);
        if ( mGrowthP ) growP(inc*mGrowthP);
    }
    else
    {
        mGrowthM = 0;
        mGrowthP = 0;
    }
    
    Fiber::step();
}


//------------------------------------------------------------------------------
#pragma mark -


void ClassicFiber::write(Outputter& out) const
{
    Fiber::write(out);
    
    /// write variables describing the dynamic state of the ends:
    writeHeader(out, TAG_DYNAMIC);
    out.writeUInt16(mStateM);
    out.writeUInt16(mStateP);
}


void ClassicFiber::read(Inputter& in, Simul& sim, ObjectTag tag)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( tag == TAG_DYNAMIC || ( tag == TAG && in.formatID() < 44 ) )
#else
    if ( tag == TAG_DYNAMIC )
#endif
    {
        unsigned m = 0, p = 0;
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 42 )
            p = in.readUInt8();
        else
#endif
        {
            m = in.readUInt16();
            p = in.readUInt16();
        }

#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 46 )
            setDynamicStateP(p);
        else
#endif
        {
            setDynamicStateM(m);
            setDynamicStateP(p);
        }
    }
#ifdef BACKWARD_COMPATIBILITY
    if ( tag != TAG_DYNAMIC || in.formatID() < 44 )
#else
    else
#endif
        Fiber::read(in, sim, tag);
}

