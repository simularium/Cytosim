// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University.

#include "modulo.h"
#include "dim.h"
#include "exceptions.h"

constexpr int PERIODIC_XYZ = ( 1 << DIM ) - 1;
constexpr int PERIODIC_XY  = PERIODIC_XYZ & 3;
constexpr int PERIODIC_X   = 1;


/// enable periodicity in dimension 'd'
void Modulo::enable(size_t d, real size)
{
    if ( size > 0 )
    {
        mMode |= 1<<d;
        mSize[d] = size;
    }
    else
        ;//throw InvalidParameter("periodic:length[",d,"] must be > 0");
}


Vector Modulo::period(size_t d) const
{
    Vector vec(0,0,0);
    if ( d < DIM && ( mMode & 1<<d ))
        vec[d] = mSize[d];
    return vec;
}


void Modulo::fold(Vector& vec) const
{
    if ( mMode == PERIODIC_XYZ )
    {
        vec.XX = fold_real(vec.XX, mSize[0]);
#if ( DIM > 1 )
        vec.YY = fold_real(vec.YY, mSize[1]);
#endif
#if ( DIM > 2 )
        vec.ZZ = fold_real(vec.ZZ, mSize[2]);
#endif
    }
    else if ( mMode == PERIODIC_XY )
    {
        vec.XX = fold_real(vec.XX, mSize[0]);
#if ( DIM > 1 )
        vec.YY = fold_real(vec.YY, mSize[1]);
#endif
    }
    else if ( mMode == PERIODIC_X )
    {
        vec.XX = fold_real(vec.XX, mSize[0]);
    }
    else
    {
        if ( mMode & 1 ) vec.XX = fold_real(vec.XX, mSize[0]);
#if ( DIM > 1 )
        if ( mMode & 2 ) vec.YY = fold_real(vec.YY, mSize[1]);
#endif
#if ( DIM > 2 )
        if ( mMode & 4 ) vec.ZZ = fold_real(vec.ZZ, mSize[2]);
#endif
    }
}


//this makes modulo around the center 'ref'
void Modulo::fold(Vector& pos, Vector const& ref) const
{
    Vector img = pos - ref;
    fold(img);
    pos = img + ref;
}


//calculate the offset from the canonical image to actual 'pos'
Vector Modulo::offset(Vector const& pos) const
{
    Vector img = pos;
    fold(img);
    return pos - img;
}


//calculate the canonical image of 'pos' and return the associated shift
void Modulo::foldOffset(Vector& pos, Vector& off) const
{
    Vector vec = pos;
    fold(pos);
    off = vec - pos;
}
