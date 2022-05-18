// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "wrist_long.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo const* modulo;


WristLong::WristLong(SingleProp const* sp, Mecable const* mec, const unsigned pti)
: Wrist(sp, mec, pti), mArm(nullTorque)
{
}


WristLong::~WristLong()
{
}

//------------------------------------------------------------------------------

#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real WristLong::calcArm(const Interpolation & pt, Vector const& pos, real len)
{
    Vector vec = pt.pos() - pos;
    if ( modulo )
        modulo->fold(vec);
    return std::copysign(len, cross(vec, pt.diff()) );
}

#elif ( DIM >= 3 )

/**
 Return a vector of norm len, that is perpendicular to the Fiber referenced by `pt`,
 and also perpendicular to the link.
 */
Vector WristLong::calcArm(const Interpolation & pt, Vector const& pos, real len)
{
    Vector vec = pt.pos() - pos;
    if ( modulo )
        modulo->fold(vec);
    Vector a = cross( vec, pt.diff() );
    real an = a.normSqr();
    if ( an > REAL_EPSILON )
        return a * ( len / sqrt(an) );
    else
        return pt.diff().randOrthoU(len);
}

#endif


//------------------------------------------------------------------------------
Vector WristLong::force() const
{
    assert_true( sHand->attached() );
    Vector d = posFoot() - WristLong::sidePos();
    
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}


//------------------------------------------------------------------------------

/*
Note that, since `mArm` is calculated by setInteraction(),
the result of sidePos() will be incorrect if 'solve=0'
*/
Vector WristLong::sidePos() const
{
#if ( DIM > 1 )
    return sHand->pos() + cross(mArm, sHand->dirFiber());
#endif
    return sHand->pos();
}

/**
 Using a Meca::interSideLink()
 */
void WristLong::setInteractions(Meca & meca) const
{
    Interpolation const& pt = sHand->interpolation();
    
    /* 
     The 'arm' is recalculated each time, but in 2D at least,
     this maybe not necessary, as switching should be rare.
     */
    
#if ( DIM == 2 )
    
    mArm = calcArm(pt, posFoot(), prop->length);
    if ( anchor.rank() == 1 )
        meca.addSideLink2D(pt, anchor.vertex0(), mArm, prop->stiffness);
    else
        throw InvalidParameter("unfinished WristLong::setInteractions(length>0, Interpolation4)");

#elif ( DIM >= 3 )
    
    mArm = calcArm(pt, posFoot(), prop->length);
    if ( anchor.rank() == 1 )
        meca.addSideLinkS(pt, anchor.vertex0(), mArm, prop->length, prop->stiffness);
    else
        throw InvalidParameter("unfinished WristLong::setInteractions(length>0, Interpolation4)");

    //@todo WristLong:setInteractions() use interSideLink3D()
    
#endif
}

