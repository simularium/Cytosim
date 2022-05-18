// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "couple.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "hand_prop.h"
#include "meca.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"
#include "aster.h"
#include "aster_prop.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------

Couple::Couple(CoupleProp const* p, Vector const& w)
: prop(p), cPos(w), cHand1(nullptr), cHand2(nullptr)
{
    cHand1 = prop->hand1_prop->newHand(this);
    cHand2 = prop->hand2_prop->newHand(this);

    assert_true(cHand1);
    assert_true(cHand2);
}


Couple::~Couple()
{
    if ( cHand1  &&  attached1() )
        cHand1->detach();
    
    if ( cHand2  &&  attached2() )
        cHand2->detach();
    
    if ( linked() )
        objset()->remove(this);
    
    delete(cHand1);
    cHand1 = nullptr;
    delete(cHand2);
    cHand2 = nullptr;
    prop = nullptr;
}

//------------------------------------------------------------------------------
#pragma mark -

/* category of link:
 Links on the side of the filaments:
     - Parallel
     - Antiparallel
     - X = none of the above
 Links at the ends of the filaments:
     - T
     - V
 by Jamie Li Rickman, ~2017
 */
int Couple::configuration(FiberEnd end, real len) const
{
    int e = (cHand1->abscissaFrom(end) < len) + (cHand2->abscissaFrom(end) < len);
    switch ( e )
    {
        case 0:
            if (cosAngle() > 0.5)
                return 0; // P: angle < PI/3
            else if (cosAngle() < -0.5)
                return 1; // A: angle > 2PI/3
            return 2; // X
        case 1:
            return 3; // T
        case 2:
            return 4; // V
    }
    return 5; //should not happen!
}


real Couple::stiffness() const
{
    return prop->stiffness;
}


void Couple::setInteractions(Meca & meca) const
{
    assert_true( attached1() && attached2() );
    
    meca.addLink(cHand1->interpolation(), cHand2->interpolation(), prop->stiffness);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Simulates:
 - diffusive motion
 - attachment
 .
 */
void Couple::stepFF(Simul& sim)
{
    diffuse();
    
    // confinement:
    if ( prop->confine == CONFINE_INSIDE )
    {
        /**
         @todo dirichlet boundary conditions
         Set concentration of molecules at edges of Space by letting molecules
         out, and put some back at a constant rate
         */
        if ( !prop->confine_space_ptr->inside(cPos) )
            cPos = prop->confine_space_ptr->bounce(cPos);
        if ( modulo )
            modulo->fold(cPos);
    }
    else if ( prop->confine == CONFINE_ON )
    {
        cPos = prop->confine_space_ptr->project(cPos);
    }

    /*
     To attachment a Couple, we flip a coin to give equal chance to each Hand,
     as if they were sharing the two half of a spherical cap.
     Note that this divides by two the effective binding rate of the Hands.
     */
    if ( RNG.flip() )
        cHand1->stepUnattached(sim, cPos);
    else if ( !prop->trans_activated )
        cHand2->stepUnattached(sim, cPos);
}


/**
 Simulates:
 - attachment of cHand2
 - attached activity of cHand1
 .
 */
void Couple::stepAF(Simul& sim)
{
    //we use cHand1->pos() first, because stepUnloaded() may detach cHand1
    cHand2->stepUnattached(sim, cHand1->pos());
    cHand1->stepUnloaded();
}


/**
 Simulates:
 - attachment of cHand1
 - attached activity of cHand2
 .
 */
void Couple::stepFA(Simul& sim)
{
    //we use cHand2->pos() first, because stepUnloaded() may detach cHand2
    cHand1->stepUnattached(sim, cHand2->pos());
    cHand2->stepUnloaded();
}


/**
 Simulates:
 - attached activity of cHand1
 - attached activity of cHand2
 .
 */
void Couple::stepAA()
{
    Vector f = force();
    real fn = f.norm();
    cHand1->stepLoaded( f, fn);
    cHand2->stepLoaded(-f, fn);
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 @return:
 - True if attachment is possible
 - False if attachment is forbiden
 .
 
 If ( couple:stiff == true ), the two Hands of the Couple will refuse to be attached
 to the same segment, or to two neighboring segments on the same fiber.
 
 We cannot calculate the force of such 'degenerate' links, and they are undesired in
 most cases.
 
 */

bool Couple::allowAttachment(FiberSite const& sit)
{
    Hand const* that = attachedHand();
    
    if ( !that )
        return true;
    
    if ( prop->stiff )
    {
        if ( that->fiber() == sit.fiber()
            && fabs(sit.abscissa()-that->abscissa()) <= 2*sit.fiber()->segmentation() )
        return false;
    }
    
    /*
     Allow or disallow binding based on the angle made between the two Fibers.
     The threshold on the cosine of the angle are here somewhat arbitrary
     */
    switch( prop->specificity )
    {
        case CoupleProp::BIND_ALWAYS:
            return true;
            
        case CoupleProp::BIND_PARALLEL:
            if ( dot(sit.dirFiber(), that->dirFiber()) < 0.5 )
                return false;
            break;
            
        case CoupleProp::BIND_NOT_PARALLEL:
            if ( dot(sit.dirFiber(), that->dirFiber()) > 0.5 )
                return false;
            break;
  
        case CoupleProp::BIND_ANTIPARALLEL:
            if ( dot(sit.dirFiber(), that->dirFiber()) > -0.5 )
                return false;
            break;
            
        case CoupleProp::BIND_NOT_ANTIPARALLEL:
            if ( dot(sit.dirFiber(), that->dirFiber()) < -0.5 )
                return false;
            break;
            
        case CoupleProp::BIND_ORTHOGONAL:
            if ( fabs(dot(sit.dirFiber(), that->dirFiber())) > 0.866025 )
                return false;
            break;
            
        default:
            throw InvalidParameter("unknown couple:specificity");
    }

    //attachment is allowed by default:
    return true;
}


void Couple::afterAttachment(Hand const* h)
{
    // link into correct CoupleSet sublist:
    CoupleSet * set = static_cast<CoupleSet*>(objset());
    if ( set )
    {
        if ( h == cHand1 )
            set->relinkA1(this);
        else
            set->relinkA2(this);
    }
}


void Couple::beforeDetachment(Hand const* h)
{
    assert_true(h->attached());
 
#if ( DIM < 2 )
    /*
     Relocate the Couple unbound position vector to where it is attached.
     This ensures that the diffusion process starts from the correct location
     */
    cPos = h->posHand();
#else
    /*
     Set position near the attachment point, but offset in the perpendicular
     direction at a random distance within the range of attachment of the Hand
     
     This is necessary to achieve detailed balance, which in particular implies
     that rounds of binding/unbinding should not get the Couples closer to
     the Filaments to which they bind.
     */
    cPos = h->posHand() + h->dirFiber().randOrthoB(h->prop->binding_range);
#endif
    
    // link into correct CoupleSet sublist:
    CoupleSet * set = static_cast<CoupleSet*>(objset());
    if ( set )
    {
        if ( h == cHand1 )
            set->relinkD1(this);
        else
            set->relinkD2(this);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 The position is:
 - cPos if the Couple is free,
 - the position of the attached Hand if only one is attached
 - the average position of the two hands if they are both attached
.
 */
Vector Couple::position() const
{
    if ( attached2() )
    {
        if ( attached1() )
            return 0.5 * ( cHand2->pos() + cHand1->pos() );
        return cHand2->pos();
    }
    if ( attached1() )
    {
        return cHand1->pos();
    }
    return cPos;
}


void Couple::foldPosition(Modulo const* m)
{
    m->fold(cPos);
}

void Couple::randomizePosition()
{
    cPos = prop->confine_space_ptr->randomPlace();
}

//------------------------------------------------------------------------------
#pragma mark -

Vector Couple::force() const
{
    Vector d = cHand2->pos() - cHand1->pos();
    
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}


Hand * Couple::attachedHand() const
{
    if ( attached1() )
        return cHand1;
    else if ( attached2() )
        return cHand2;
    else
        return nullptr;
}


Hand* Couple::otherHand(Hand const* h) const
{
    if ( h == cHand1 )
        return cHand2;
    else
        return cHand1;
}


Vector Couple::otherPosition(Hand const* h) const
{
    if ( h == cHand1 )
    {
        if ( attached2() )
            return cHand2->pos();
        throw Exception("otherPosition() called for unattached Hand2");
    }
    else
    {
        if ( attached1() )
            return cHand1->pos();
        throw Exception("otherPosition() called for unattached Hand1");
    }
}


Vector Couple::otherDirection(Hand const* h) const
{
    if ( h == cHand1 )
    {
        if ( attached2() )
            return cHand2->dirFiber();
        else
            return Vector::randU();
    }
    else
    {
        if ( attached1() )
            return cHand1->dirFiber();
        else
            return Vector::randU();
    }
}


//------------------------------------------------------------------------------
#pragma mark -


void Couple::write(Outputter& out) const
{
    //std::clog << "- writing " << state() << " at " << out.pos() << '\n';
    cHand1->write(out);
    cHand2->write(out);
    if ( !attached1() && !attached2() )
        out.writeFloats(cPos, DIM);
}


void Couple::read(Inputter& in, Simul& sim, ObjectTag tag)
{
    const bool s1 = attached1();
    const bool s2 = attached2();
    
    cHand1->read(in, sim);
    cHand2->read(in, sim);
    
    if ( attached1() || attached2() )
        cPos = position();
    else
        in.readFloats(cPos, DIM);
    
    /*
     Because the CoupleSet contains 4 sublists where Couple are stored depending
     on their bound/unbound state, we need to relink *this Couple now,
     since the state stored on file could be different from the current state.
     */
    if ( s1 != attached1() || s2 != attached2() )
    {
        CoupleSet * set = static_cast<CoupleSet*>(objset());
        if ( set )
            set->relink(this, s1, s2);
    }
}

