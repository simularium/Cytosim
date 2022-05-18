// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_dice.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"


SpaceDice::SpaceDice(SpaceProp const* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("dice is not usable in 1D");

    for ( int d = 0; d < 3; ++d )
        length_[d] = 0;
    radius_ = 0;
}


void SpaceDice::resize(Glossary& opt)
{
    real rad = radius_;
    
    opt.set(rad, "radius") || opt.set(rad, "edge");
    if ( rad < 0 )
        throw InvalidParameter("dice:radius must be >= 0");

    for ( int d = 0; d < DIM; ++d )
    {
        real len = length_[d];
        if ( opt.set(len, "length", d) )
            len *= 0.5;
        if ( len < rad )
            throw InvalidParameter("dice:length[] must be >= 2 * radius");
        length_[d] = len;
    }
    
    radius_ = rad;
    update();
}


/**
 The `dice` is included in the rectangle
 */
void SpaceDice::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-length_[0],-length_[1],-length_[2]);
    sup.set( length_[0], length_[1], length_[2]);
}


/**
 If `radius==0`, the volume should be the volume of a rectangle
 */
real SpaceDice::volume() const
{
#if ( DIM == 1 )
    return 2 * length_[0];
#elif ( DIM == 2 )
    return 4 * length_[0]*length_[1] + (M_PI-4)*radius_*radius_;
#else
    return 8 * length_[0]*length_[1]*length_[2]
    + 2 * (M_PI-4) * ( length_[0] + length_[1] + length_[2] - 3 * radius_ ) * radius_ * radius_
    + (4*M_PI/3.0 - 8) * radius_ * radius_ * radius_;
#endif
}


//------------------------------------------------------------------------------

bool  SpaceDice::inside(Vector const& w) const
{
    real dis = 0;
    for ( int d = 0; d < DIM; ++d )
    {
        real a = fabs(w[d]) - length_[d];
        if ( a > 0 )
            return false;
        dis += square(std::max((real)0, a+radius_));
    }
    return ( dis <= radiusSqr_ );
}


//------------------------------------------------------------------------------

#if ( DIM == 1 )

Vector SpaceDice::project(Vector const& w) const
{
    return Vector(std::copysign(length_[0], w.XX), 0, 0);
}

#else

Vector SpaceDice::project(Vector const& w) const
{
    Vector p = w;
    bool in = true;
    
    //calculate projection on the inner cube obtained by subtracting radius
    for ( int d = 0; d < DIM; ++d )
    {
        real test = length_[d] - radius_;
        if ( fabs(w[d]) > test )
        {
            p[d] = std::copysign(test, w[d]);
            in = false;
        }
    }
    
    if ( in )
    {
        // find the dimensionality corresponding to the closest face
        real d0 = length_[0] - fabs(w.XX);
        real d1 = length_[1] - fabs(w.YY);
#if ( DIM > 2 )
        real d2 = length_[2] - fabs(w.ZZ);
        if ( d2 < d1 )
        {
            if ( d2 < d0 )
                p.ZZ = std::copysign(length_[2], w.ZZ);
            else
                p.XX = std::copysign(length_[0], w.XX);
        }
        else
#endif
        {
            if ( d1 < d0 )
                p.YY = std::copysign(length_[1], w.YY);
            else
                p.XX = std::copysign(length_[0], w.XX);
        }
        return p;
    }

    //normalize to radius(), and add to p to get the real projection
    real dis = radius_ / sqrt((w-p).normSqr());
    for ( int d = 0; d < DIM; ++d )
        p[d] += dis * ( w[d] - p[d] );
    
    return p;
}
#endif

//------------------------------------------------------------------------------

void SpaceDice::write(Outputter& out) const
{
    out.put_characters("dice", 16);
    out.writeUInt16(4);
    out.writeFloat(length_[0]);
    out.writeFloat(length_[1]);
    out.writeFloat(length_[2]);
    out.writeFloat(radius_);
}


void SpaceDice::setLengths(const real len[])
{
    length_[0] = len[0];
    length_[1] = len[1];
    length_[2] = len[2];
    radius_ = len[3];
    update();
}

void SpaceDice::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "dice");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"
using namespace gle;

bool SpaceDice::draw() const
{
#if ( DIM > 2 )
    
    const real X = length_[0] - radius_;
    const real Y = length_[1] - radius_;
    const real Z = length_[2] - radius_;
 
    const real XR = length_[0];
    const real YR = length_[1];
    const real ZR = length_[2];

    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  XR,  Y, -Z );
    gleVertex(  XR,  Y,  Z );
    gleVertex(  XR, -Y, -Z );
    gleVertex(  XR, -Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex( -XR, -Y, -Z );
    gleVertex( -XR, -Y,  Z );
    gleVertex( -XR,  Y, -Z );
    gleVertex( -XR,  Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  YR, -Z );
    gleVertex( -X,  YR, -Z );
    gleVertex(  X,  YR,  Z );
    gleVertex( -X,  YR,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X, -YR,  Z );
    gleVertex( -X, -YR,  Z );
    gleVertex(  X, -YR, -Z );
    gleVertex( -X, -YR, -Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y,  ZR );
    gleVertex( -X,  Y,  ZR );
    gleVertex(  X, -Y,  ZR );
    gleVertex( -X, -Y,  ZR );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -ZR );
    gleVertex(  X, -Y, -ZR );
    gleVertex( -X,  Y, -ZR );
    gleVertex( -X, -Y, -ZR );
    glEnd();
    
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    
    glLineStipple(1, 0x000F);
    glEnable(GL_LINE_STIPPLE);
    drawSection( 0, -X, 0.01 );
    drawSection( 0,  X, 0.01 );
    drawSection( 1, -Y, 0.01 );
    drawSection( 1,  Y, 0.01 );
    drawSection( 2, -Z, 0.01 );
    drawSection( 2,  Z, 0.01 );
    glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
    
#else

    drawSection( 2, 0, 0.01 );

#endif
    
    return true;
}

#else

bool SpaceDice::draw() const
{
    return false;
}

#endif


