// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "picket_long.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo const* modulo;


//------------------------------------------------------------------------------

PicketLong::PicketLong(SingleProp const* p, Vector const& w)
: Picket(p, w), mArm(nullTorque)
{
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=fixed");
#endif
}


PicketLong::~PicketLong()
{
    //std::clog<<"~PicketLong("<<this<<")"<<std::endl;
}

//------------------------------------------------------------------------------

#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real PicketLong::calcArm(const Interpolation & pt, Vector const& pos, real len)
{
    Vector vec = pt.pos() - pos;
    if ( modulo )
        modulo->fold(vec);
    return std::copysign(len, cross(vec, pt.diff()) );
}

#elif ( DIM >= 3 )

/**
 Return a vector of norm `len`, perpendicular to the Fiber referenced by `pt` and aligned with the link.
*/
Vector PicketLong::calcArm(const Interpolation & pt, Vector const& pos, real len)
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

/*
 Note that, since `mArm` is calculated by setInteraction(),
 the result of sidePos() will be incorrect if 'solve=0'
 */
Vector PicketLong::sidePos() const
{
#if ( DIM > 1 )
    return sHand->pos() + cross(mArm, sHand->dirFiber());
#endif
    return sHand->pos();
}

//------------------------------------------------------------------------------
/**
 This calculates the force corresponding to the interSideLink()
 */
Vector PicketLong::force() const
{
    assert_true( sHand->attached() );
    Vector d = sPos - PicketLong::sidePos();
 
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}

//------------------------------------------------------------------------------
void PicketLong::setInteractions(Meca & meca) const
{
#if ( DIM == 1 )
    meca.addPointClamp(sHand->interpolation(), sPos, prop->stiffness);
#else
    Interpolation const& pt = sHand->interpolation();
    
    /* 
     The 'arm' is recalculated every time, but in 2D at least,
     this may not be necessary, as flipping should occur rarely.
     */
    
    mArm = calcArm(pt, sPos, prop->length);
    
#if ( DIM == 2 )
    meca.addSidePointClamp2D(pt, sPos, mArm, prop->stiffness);
#elif ( DIM >= 3 )
    meca.addSidePointClamp3D(pt, sPos, mArm, prop->stiffness);
#endif

#endif
}


