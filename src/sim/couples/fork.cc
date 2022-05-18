// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fork.h"
#include "fork_prop.h"
#include "meca.h"


Fork::Fork(ForkProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
    sinus = 0;
}


Fork::~Fork()
{
    prop = nullptr;
}


void Fork::setInteractions(Meca & meca) const
{
    Interpolation const& pt1 = cHand1->interpolation();
    Interpolation const& pt2 = cHand2->interpolation();
    
    meca.addLink(pt1, pt2, prop->stiffness);
    
#if ( DIM == 2 )
    // flip the angle to match the current configuration of the bond
    if ( prop->flip )
        sinus = std::copysign(prop->sinus, cross(pt1.diff(), pt2.diff()));
    else
        sinus = prop->sinus;
    
    meca.addTorquePoliti(pt1, pt2, prop->cosinus, sinus, prop->angular_stiffness);
#elif ( DIM == 3 )
    throw InvalidParameter("Torque is not implemented in 3D");
#endif
}

