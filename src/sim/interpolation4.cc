// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "interpolation4.h"
#include "interpolation.h"
#include "mecapoint.h"
#include "simul.h"
#include "meca.h"

/**
Set a point of index 'P' on Mecable
*/
void Interpolation4::set(Mecable const* m, unsigned p)
{
    assert_true( !m || p < m->nbPoints() );
    
    mec_ = m;
    ref_ = p;
    
    coef_[0] = 1.0;
    coef_[1] = 0.0;
    coef_[2] = 0.0;
    coef_[3] = 0.0;
    ord_ = 1;
}

/**
Coefficient 'c' determines the position between 'P' and 'Q':
- with ( c == 0 ) the point is equal to Q
- with ( c == 1 ) the point is equal to P
.
*/
void Interpolation4::set(Mecable const* m, unsigned p, unsigned q, real c)
{
    assert_true(m);
    assert_true(q == p+1);
    assert_true(q < m->nbPoints());
    
    mec_ = m;
    ref_ = p;
    
    coef_[0] = c;
    coef_[1] = 1.0 - c;
    coef_[2] = 0.0;
    coef_[3] = 0.0;
    ord_ = 2;
}

/**
The Vector 'vec' determines the interpolation coefficients:
    - if ( vec == [0, 0, 0] ) this interpolates P exactly
    - if ( vec == [1, 0, 0] ) this interpolates P+1
    - if ( vec == [0, 1, 0] ) this interpolates P+2
    - if ( vec == [0, 0, 1] ) this interpolates P+3
    .
This is used when the four vertices [P ... P+3] define a orthonormal reference,
as the components of 'vec' are then simply the coordinates of the position of the
interpolation in this reference frame.
*/
void Interpolation4::set(Mecable const* m, unsigned p, Vector const& vec)
{
    assert_true(m);
    
    mec_ = m;
    ref_ = p;

    coef_[1] = vec.XX;
#if ( DIM == 1 )
    coef_[2] = 0.0;
    coef_[3] = 0.0;
    coef_[0] = 1.0 - vec.XX;
#elif ( DIM == 2 )
    coef_[2] = vec.YY;
    coef_[3] = 0.0;
    coef_[0] = 1.0 - vec.XX - vec.YY;
#else
    coef_[2] = vec.YY;
    coef_[3] = vec.ZZ;
    coef_[0] = 1.0 - vec.XX - vec.YY - vec.ZZ;
#endif

    if ( vec.norm_inf() < REAL_EPSILON )
        ord_ = 1;
    else
        ord_ = 1+DIM;

    // the last point to be interpolated is ( ref_ + ord_ -1 )
    assert_true(ref_+ord_ <= m->nbPoints());
}


Vector Interpolation4::pos() const
{
    unsigned top = std::min(ord_, mec_->nbPoints());
    Vector res = coef_[0] * mec_->posPoint(ref_);
    for ( unsigned i = 1; i < top; ++i )
        res += coef_[i] * mec_->posPoint(ref_+i);
    return res;
}


void Interpolation4::addLink(Meca& meca, Interpolation const& arg, const real weight) const
{
    unsigned off = mec_->matIndex() + ref_;
    unsigned pts[] = { off, off+1, off+2, off+3 };
    
    switch ( ord_ )
    {
        case 0:
            break;
        case 1:
            meca.addLink1(arg, off, weight);
            break;
        case 2:
            meca.addLink2(arg, pts, coef_, weight);
            break;
        case 3:
            meca.addLink3(arg, pts, coef_, weight);
            break;
        case 4:
            meca.addLink4(arg, pts, coef_, weight);
        break;
    }
}


void Interpolation4::addLink(Meca& meca, Mecapoint const& arg, const real weight) const
{
    unsigned off = mec_->matIndex() + ref_;
    unsigned pts[] = { off, off+1, off+2, off+3 };
    
    switch ( ord_ )
    {
        case 0:
            break;
        case 1:
            meca.addLink(arg, Mecapoint(mec_, ref_), weight);
            break;
        case 2:
            meca.addLink2(arg, pts, coef_, weight);
            break;
        case 3:
            meca.addLink3(arg, pts, coef_, weight);
            break;
        case 4:
            meca.addLink4(arg, pts, coef_, weight);
        break;
    }
}


void Interpolation4::write(Outputter& out) const
{
    Object::writeReference(out, mec_);
    out.writeUInt16(ref_);
    for ( int d = 1; d < 4; ++d )
        out.writeFloat(coef_[d]);
}


void Interpolation4::read(Inputter& in, Simul& sim)
{
    ObjectTag g;
    mec_ = Simul::toMecable(sim.readReference(in, g));
    ref_ = in.readUInt16();
    
    for ( int d = 1; d < 4; ++d )
        coef_[d] = in.readFloat();
    
    coef_[0] = 1.0 - coef_[1] - coef_[2] - coef_[3];
    
    ord_ = 4;
    while ( fabs(coef_[ord_-1]) < REAL_EPSILON )
        --ord_;
}


void Interpolation4::print(std::ostream& out) const
{
    const unsigned w = 9;
    if ( mec_ )
    {
        out << "(" << mec_->reference() << "  " << std::setw(w) << coef_[0];
        for ( int d = 1; d < 4; ++d )
            out << " " << std::setw(w) << coef_[d];
        out << ")";
    }
    else
        out << "(void)";
}

