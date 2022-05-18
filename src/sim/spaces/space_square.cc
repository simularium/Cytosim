// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_square.h"
#include "exceptions.h"
#include "mecapoint.h"
#include "iowrapper.h"
#include "glossary.h"
#include "meca.h"


SpaceSquare::SpaceSquare(SpaceProp const* p)
: Space(p)
{
    for ( int d = 0; d < 3; ++d )
        length_[d] = 0;
}


void SpaceSquare::resize(Glossary& opt)
{
    for ( int d = 0; d < DIM; ++d )
    {
        real len = length_[d];
        if ( opt.set(len, "length", d) )
            len *= 0.5;
        if ( len < 0 )
            throw InvalidParameter("square:length[] must be >= 0");
        length_[d] = len;
    }
#if ( DIM == 2 )
    // that is for impersonating a 'cylinder' in 2D:
    if ( length_[1] <= 0 )
    {
        real rad = 0;
        if ( opt.set(rad, "radius") )
            length_[1] = rad;
    }
#endif
}


void SpaceSquare::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-length_[0],-length_[1],-length_[2]);
    sup.set( length_[0], length_[1], length_[2]);
}

//------------------------------------------------------------------------------
#pragma mark - DIM=1


#if ( DIM == 1 )

real SpaceSquare::volume() const
{
    return 2 * length_[0];
}

bool SpaceSquare::inside(Vector const& w) const
{
    return fabs(w.XX) <= length_[0];
}

bool SpaceSquare::allInside(Vector const& w, const real rad ) const
{
    assert_true( rad >= 0 );
    
    return std::max(rad-w.XX, w.XX+rad) <= length_[0];
}

Vector SpaceSquare::project(Vector const& w) const
{
    return Vector(std::copysign(length_[0], w.XX), 0, 0);
}

#endif


//------------------------------------------------------------------------------
#pragma mark - DIM=2


#if ( DIM == 2 )

real SpaceSquare::volume() const
{
    return 4 * length_[0] * length_[1];
}


bool SpaceSquare::inside(Vector const& w) const
{
    return fabs(w.XX) <= length_[0] &
           fabs(w.YY) <= length_[1];
}

bool SpaceSquare::allInside(Vector const& w, const real rad ) const
{
    assert_true( rad >= 0 );
    
    return std::max(rad-w.XX, w.XX+rad) <= length_[0] &
           std::max(rad-w.YY, w.YY+rad) <= length_[1];
}

#endif

//------------------------------------------------------------------------------
#pragma mark - DIM=3


#if ( DIM >= 3 )

real SpaceSquare::volume() const
{
    return 8 * length_[0] * length_[1] * length_[2];
}

bool SpaceSquare::inside(Vector const& w) const
{
    return fabs(w.XX) <= length_[0] &
           fabs(w.YY) <= length_[1] &
           fabs(w.ZZ) <= length_[2];
}

bool SpaceSquare::allInside(Vector const& w, const real rad ) const
{
    assert_true( rad >= 0 );
    
    return std::max(rad-w.XX, w.XX+rad) <= length_[0] &
           std::max(rad-w.YY, w.YY+rad) <= length_[1] &
           std::max(rad-w.ZZ, w.ZZ+rad) <= length_[2];
}
#endif

#if ( DIM > 1 )

Vector SpaceSquare::project(Vector const& w) const
{
    Vector p = w;
    bool in = true;
    
    if ( fabs(p.XX) > length_[0] )
    {
        p.XX = std::copysign(length_[0], p.XX);
        in = false;
    }
    if ( fabs(p.YY) > length_[1] )
    {
        p.YY = std::copysign(length_[1], p.YY);
        in = false;
    }
#if ( DIM > 2 )
    if ( fabs(p.ZZ) > length_[2] )
    {
        p.ZZ = std::copysign(length_[2], p.ZZ);
        in = false;
    }
#endif

    if ( in )
    {
        // find the dimensionality corresponding to the closest face
        real d0 = length_[0] - fabs(w.XX);
        real d1 = length_[1] - fabs(w.YY);
#if ( DIM > 2 )
        real d2 = length_[2] - fabs(w.ZZ);
        if ( d2 < d1 )
        {
            if ( d0 < d2 )
                p.XX = std::copysign(length_[0], w.XX);
            else
                p.ZZ = std::copysign(length_[2], w.ZZ);
        }
        else
#endif
        {
            if ( d0 < d1 )
                p.XX = std::copysign(length_[0], w.XX);
            else
                p.YY = std::copysign(length_[1], w.YY);
        }
    }
    return p;
}
#endif

//------------------------------------------------------------------------------
#pragma mark - Interaction

/// apply a force directed towards the edge of the box
/**
 When the point is in the center of the box.
 
 When a point is along the edge of the cube, the interaction
 is flat in one direction, and curved in the two others.

 */

void SpaceSquare::setInteraction(const real pos[], Mecapoint const& pe, Meca & meca, real stiff, const real dim[])
{
    bool in = true;
    
    index_t inx = DIM * pe.matIndex();
    
    for ( int d = 0; d < DIM; ++d )
    {
        assert_true( dim[d] >= 0 );
        if ( fabs(pos[d]) > dim[d] )
        {
            meca.mC(inx+d, inx+d) -= stiff;
            meca.base(inx+d)      += stiff * std::copysign(dim[d], pos[d]);
            in = false;
        }
    }

    if ( in ) 
    {
        // find the dimensionality 'dip' corresponding to the closest face
        int  dip = 0;
        
        real l = dim[0] - fabs(pos[0]);
#if ( DIM > 1 )
        real u = dim[1] - fabs(pos[1]);
        if ( u < l ) { dip = 1; l = u; };
#endif
#if ( DIM > 2 )
        u = dim[2] - fabs(pos[2]);
        if ( u < l )  dip = 2;
#endif
        meca.mC(inx+dip, inx+dip) -= stiff;
        meca.base(inx+dip) += stiff * std::copysign(dim[dip], pos[dip]);
    }
}


void SpaceSquare::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length_);
}


void SpaceSquare::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca & meca, real stiff) const
{
    real dim[DIM];
    for ( int d = 0; d < DIM; ++d )
        dim[d] = std::max((real)0, length_[d] - rad);

    setInteraction(pos, pe, meca, stiff, dim);
}

//------------------------------------------------------------------------------

void SpaceSquare::write(Outputter& out) const
{
    out.put_characters("square", 16);
    out.writeUInt16(4);
    out.writeFloat(length_[0]);
    out.writeFloat(length_[1]);
    out.writeFloat(length_[2]);
    out.writeFloat(0.f);
}


void SpaceSquare::setLengths(const real len[])
{
    length_[0] = len[0];
    length_[1] = len[1];
    length_[2] = len[2];
}

void SpaceSquare::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "square");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"
using namespace gle;

bool SpaceSquare::draw() const
{
    const real X = length_[0];
    const real Y = length_[1];
    const real Z = ( DIM > 2 ) ? length_[2] : 0;
  
    glPushAttrib(GL_ENABLE_BIT);

#if ( DIM > 2 )

    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X,  Y,  Z );
    gleVertex(  X, -Y, -Z );
    gleVertex(  X, -Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex( -X, -Y, -Z );
    gleVertex( -X, -Y,  Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X,  Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -Z );
    gleVertex( -X,  Y, -Z );
    gleVertex(  X,  Y,  Z );
    gleVertex( -X,  Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X, -Y,  Z );
    gleVertex( -X, -Y,  Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X, -Y, -Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y,  Z );
    gleVertex( -X,  Y,  Z );
    gleVertex(  X, -Y,  Z );
    gleVertex( -X, -Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X, -Y, -Z );
    glEnd();

    glDisable(GL_LIGHTING);
    glLineWidth(0.5);
    glColor3f(1,1,1);
    
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, Z );
    gleVertex(  X, -Y, Z );
    gleVertex( -X, -Y, Z );
    gleVertex( -X,  Y, Z );
    glEnd();
    
    glBegin(GL_LINES);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X,  Y,  Z );
    gleVertex(  X, -Y, -Z );
    gleVertex(  X, -Y,  Z );
    gleVertex( -X, -Y, -Z );
    gleVertex( -X, -Y,  Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X,  Y,  Z );
    glEnd();

#endif
  
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    glEnd();

    glPopAttrib();
    return true;
}

#else

bool SpaceSquare::draw() const
{
    return false;
}

#endif
