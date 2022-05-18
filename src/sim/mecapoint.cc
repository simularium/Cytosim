// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "mecapoint.h"
#include "assert_macro.h"
#include "interpolation.h"
#include "iowrapper.h"
#include "simul.h"


void Mecapoint::read(Inputter& in, Simul& sim)
{
    ObjectTag g;
    mec_ = Simul::toMecable(sim.readReference(in, g));
    if ( mec_ )
        pti_ = in.readUInt16();
    else
        pti_ = 0;
}


void Mecapoint::write(Outputter& out) const
{
    out.writeSoftSpace();
    if ( mec_ ) {
        Object::writeReference(out, mec_);
        out.writeUInt16(pti_);
    }
    else {
        Object::writeNullReference(out);
    }
}


bool Mecapoint::overlapping(const Mecapoint & p) const
{
    return ( mec_ == p.mec_  &&  pti_ == p.pti_ );
}


bool Mecapoint::near(const Mecapoint & p) const
{
    return ( mec_ == p.mec_  &&
            ( pti_ == p.pti_ || pti_ == p.pti_+1 || pti_+1 == p.pti_ ));
}


void Mecapoint::print(std::ostream& os) const
{
    if ( mec_ )
        os << "(" << mec_->reference() << "  " << point() << ")";
    else
        os << "(void)";
}


std::ostream& operator << (std::ostream& os, Mecapoint const& obj)
{
    obj.print(os);
    return os;
}
