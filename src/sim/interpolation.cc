// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_segment.h"
#include "interpolation.h"
#include "mecapoint.h"


Interpolation::Interpolation(FiberSegment const& loc, real abs)
{
    mec_  = loc.fiber();
    pt1_  = loc.point();
    pt2_  = loc.point()+1;
    coef_ = abs / loc.len();
}


bool Interpolation::overlapping(const Mecapoint & p) const
{
    return ( mec_==p.mecable() && ( pt1_==p.point() || pt2_==p.point() ));
}


bool Interpolation::overlapping(const Interpolation & p) const
{
    return ( mec_==p.mec_ &&
            ( pt1_==p.pt1_ || pt1_==p.pt2_ || pt2_==p.pt1_ || pt2_==p.pt2_ ));
}


void Interpolation::print(std::ostream& os) const
{
    if ( mec_ )
        os << "(" << mec_->reference() << " " << pt1_ << " " << pt2_ << " " << coef_ << ")";
    else
        os << "(null)";
}


std::ostream& operator << (std::ostream& os, Interpolation const& obj)
{
    obj.print(os);
    return os;
}
