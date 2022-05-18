// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "shackle.h"
#include "shackle_prop.h"
#include "meca.h"


Shackle::Shackle(ShackleProp const* p, Vector const & w)
: Couple(p, w), prop(p)
{
}


Shackle::~Shackle()
{
    prop = nullptr;
}


/**
 The interaction is slipery on hand1
 */
void Shackle::setInteractions(Meca & meca) const
{
    Interpolation const& pt1 = cHand1->interpolation();
    Interpolation const& pt2 = cHand2->interpolation();

    meca.addSlidingLink(pt1, pt2, prop->stiffness);
}


void Shackle::stepAA()
{
    real dis;
    
    // project the position of cHand2 to set abscissa of cHand1
    real a = cHand1->fiber()->projectPoint(cHand2->pos(), dis);
    
    //std::clog << "Shackle " << proj.abscissa() - cHand1->abscissa() << std::endl;
    cHand1->moveTo(a);
    
    if ( attached1() )
    {
        Vector f = force();
        real fn = f.norm();
        cHand1->stepLoaded( f, fn);
        cHand2->stepLoaded(-f, fn);
    }
}
