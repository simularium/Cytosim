// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "shackle_long.h"
#include "shackle_prop.h"
#include "modulo.h"
#include "meca.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------
ShackleLong::ShackleLong(ShackleProp const* p, Vector const& w)
: Shackle(p, w), mArm(nullTorque)
{
}


//------------------------------------------------------------------------------
#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real ShackleLong::calcArm(const Interpolation & pt, Vector const& pos, real len)
{
    Vector vec = pt.pos() - pos;
    if ( modulo )
        modulo->fold(vec);
    return std::copysign(len, cross(vec, pt.diff()) );
}

#elif ( DIM >= 3 )

/**
 Return a vector of norm `len`, perpendicular to the Fiber referenced by `pt` and aligned with the link.
 @todo update to match interSideLink3D when available
 */
Vector ShackleLong::calcArm(const Interpolation & pt, Vector const& pos, real len)
{
    Vector a  = pt.diff();
    Vector as = pos - pt.pos();
    if ( modulo )
        modulo->fold(as);
    // here a.normSqr() == cHand1->fiber().segmentationSqr();
    Vector p = ( as - ( dot(as, a) / a.normSqr() ) * a );
    real pn = p.normSqr();
    if ( pn > REAL_EPSILON )
        return p * ( len / sqrt(pn) );
    else
        return a.randOrthoU(len);
    //return cross( pt.pos()-pos, pt.diff() ).normalized(len);
}

#endif

//------------------------------------------------------------------------------

/*
Note that, since `mArm` is calculated by setInteraction(),
the result of sidePos() will be incorrect if 'solve=0'
*/
Vector ShackleLong::sidePos() const
{
#if ( DIM == 1 )
    
    return cHand1->pos();
    
#elif ( DIM == 2 )
    
    return cHand1->pos() + cross(mArm, cHand1->dirFiber());
    
#else
    
    ///\todo: change formula to match interSideLink3D
    return cHand1->pos() + mArm;
    
#endif
}

/**
 Calculates the force for the interSideLink()
 */
Vector ShackleLong::force() const
{
    Vector d = cHand2->pos() - ShackleLong::sidePos();
        
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}

//------------------------------------------------------------------------------
/**
 The interaction is slipery on hand1
 */
void ShackleLong::setInteractions(Meca & meca) const
{
    Interpolation const& pt1 = cHand1->interpolation();
    Interpolation const& pt2 = cHand2->interpolation();

#if ( DIM == 2 )
    
    mArm = calcArm(pt1, pt2.pos(), prop->length);
    meca.addSideSlidingLink2D(pt1, pt2, mArm, prop->stiffness);
    
#elif ( DIM >= 3 )
    
    //@todo: update to interSideSlidingLink3D() 
    mArm = calcArm(pt1, pt2.pos(), prop->length);
    meca.addSideSlidingLinkS(pt1, pt2, mArm, prop->length, prop->stiffness);
    
#endif
}

