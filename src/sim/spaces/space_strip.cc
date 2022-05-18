// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "space_strip.h"
#include "exceptions.h"
#include "mecapoint.h"
#include "iowrapper.h"
#include "glossary.h"
#include "meca.h"


SpaceStrip::SpaceStrip(SpaceProp const* p)
: Space(p), bot_(0), top_(0)
{
    if ( DIM == 1 )
        throw InvalidParameter("strip is not usable in 1D");
    for ( unsigned d = 0; d < 3; ++d )
        length_[d] = 0;
}


void SpaceStrip::resize(Glossary& opt)
{
    for ( unsigned d = 0; d < DIM-1; ++d )
    {
        real len = length_[d];
        if ( opt.set(len, "length", d) )
            len *= 0.5;
        if ( len < 0 )
            throw InvalidParameter("strip:length[] must be >= 0");
        length_[d] = len;
    }
    
    real bot = bot_, top = top_;
    if ( opt.set(top, "length", DIM-1) )
    {
        bot = -0.5 * top;
        top =  0.5 * top;
    }
    else
    {
        opt.set(bot, "bottom");
        opt.set(top, "top");
    }

#if ( DIM == 2 )
    // that is for impersonating a 'cylinderP' in 2D:
    if ( top <= bot )
    {
        real rad = 0;
        if ( opt.set(rad, "radius") )
        {
            top =  rad;
            bot = -rad;
        }
    }
#endif

    if ( top < bot )
        throw InvalidParameter("strip:top must be >= strip:bottom");
    
    bot_ = bot;
    top_ = top;
    
    update();
}


void SpaceStrip::update()
{
    modulo_.reset();
    for ( unsigned d = 0; d < DIM-1; ++d )
        modulo_.enable(d, 2*length_[d]);
}


void SpaceStrip::boundaries(Vector& inf, Vector& sup) const
{
#if ( DIM == 2 )
    inf.set(-length_[0], bot_, 0);
    sup.set( length_[0], top_, 0);
#else
    inf.set(-length_[0],-length_[1], bot_);
    sup.set( length_[0], length_[1], top_);
#endif
}

//------------------------------------------------------------------------------
#pragma mark -


#if ( DIM == 1 )

real SpaceStrip::volume() const
{
    return ( top_ - bot_ );
}

bool SpaceStrip::inside(Vector const& point) const
{
    return (( bot_ <= point.XX ) & ( point.XX <= top_ ));
}


Vector SpaceStrip::project(Vector const& pos) const
{
    real X = sign_select(2 * pos.XX - bot_ - top_, bot_, top_);
    return Vector(X);
}

#endif


//------------------------------------------------------------------------------

#if ( DIM == 2 )

real SpaceStrip::volume() const
{
    return 2.0 * length_[0] * ( top_ - bot_ );
}

bool SpaceStrip::inside(Vector const& point) const
{
    return (( bot_ <= point.YY ) & ( point.YY <= top_ ));
}


Vector SpaceStrip::project(Vector const& pos) const
{
    real Y = sign_select(2 * pos.YY - bot_ - top_, bot_, top_);
    return Vector(pos.XX, Y);
}

#endif

//------------------------------------------------------------------------------

#if ( DIM >= 3 )

real SpaceStrip::volume() const
{
    return 4.0 * length_[0] * length_[1] * ( top_ - bot_ );
}

bool SpaceStrip::inside(Vector const& point) const
{
    return (( bot_ <= point.ZZ ) & ( point.ZZ <= top_ ));
}

Vector SpaceStrip::project(Vector const& pos) const
{
    real Z = sign_select(2 * pos.ZZ - bot_ - top_, bot_, top_);
    return Vector(pos.XX, pos.YY, Z);
}

#endif

//------------------------------------------------------------------------------

void SpaceStrip::setInteraction(Vector const& pos, Mecapoint const& pe, Meca& meca, real stiff) const
{
    index_t inx = DIM-1 + DIM * pe.matIndex();
    
    meca.mC(inx, inx) -= stiff;

#if ( DIM == 2 )
    real Y = sign_select(2 * pos.YY - bot_ - top_, bot_, top_);
    meca.base(inx) += stiff * Y;
#elif ( DIM > 2 )
    real Z = sign_select(2 * pos.ZZ - bot_ - top_, bot_, top_);
    meca.base(inx) += stiff * Z;
#endif
}


void SpaceStrip::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca& meca, real stiff) const
{
    index_t inx = DIM-1 + DIM * pe.matIndex();
    
    meca.mC(inx, inx) -= stiff;

#if ( DIM == 2 )
    real Y = sign_select(2 * pos.YY - bot_ - top_, bot_ + rad, top_ - rad);
    meca.base(inx) += stiff * Y;
#elif ( DIM > 2 )
    real Z = sign_select(2 * pos.ZZ - bot_ - top_, bot_ + rad, top_ - rad);
    meca.base(inx) += stiff * Z;
#endif
}

//------------------------------------------------------------------------------

void SpaceStrip::write(Outputter& out) const
{
    out.put_characters("strip", 16);
    out.writeUInt16(4);
    out.writeFloat(length_[0]);
    out.writeFloat(length_[1]);
    out.writeFloat(bot_);
    out.writeFloat(top_);
}


void SpaceStrip::setLengths(const real len[])
{
    length_[0] = len[0];
    length_[1] = len[1];
    bot_ = len[2];
    top_ = len[3];
#ifdef BACKWARD_COMPATIBILITY
    // changed from 'length[2]' to 'bot_' & 'top_' on 12.06.2020
    if ( bot_ > top_ && top_ == 0 )
    {
        top_ =  0.5 * bot_;
        bot_ = -0.5 * bot_;
    }
#endif
    update();
}

void SpaceStrip::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "strip");
    setLengths(len);
}


//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------
#pragma mark -

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"
using namespace gle;

bool SpaceStrip::draw() const
{
    const real X = length_[0];
    const real T = top_;
    const real B = bot_;

#if ( DIM >= 3 )
    const real Y = length_[1];
    // draw faces:
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0, 0, 1);
    gleVertex(-X,  Y, B);
    gleVertex( X,  Y, B);
    gleVertex(-X, -Y, B);
    gleVertex( X, -Y, B);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0, 0, -1);
    gleVertex(-X,  Y, T);
    gleVertex(-X, -Y, T);
    gleVertex( X,  Y, T);
    gleVertex( X, -Y, T);
    glEnd();
    // draw outline:
    glBegin(GL_LINE_STRIP);
    gleVertex(-X,  Y, B);
    gleVertex( X,  Y, B);
    gleVertex( X, -Y, B);
    gleVertex(-X, -Y, B);
    gleVertex(-X,  Y, B);
    glEnd();
    glBegin(GL_LINE_STRIP);
    gleVertex(-X,  Y, T);
    gleVertex(-X, -Y, T);
    gleVertex( X, -Y, T);
    gleVertex( X,  Y, T);
    gleVertex(-X,  Y, T);
    glEnd();
#else
    glBegin(GL_LINES);
    gleVertex(-X, T, 0);
    gleVertex( X, T, 0);
    gleVertex( X, B, 0);
    gleVertex(-X, B, 0);
    glEnd();
#endif

    // draw outline:
    glLineStipple(1, 0x000F);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
#if ( DIM >= 3 )
    gleVertex( X,  Y, T);
    gleVertex( X,  Y, B);
    gleVertex( X, -Y, T);
    gleVertex( X, -Y, B);
    gleVertex(-X,  Y, T);
    gleVertex(-X,  Y, B);
    gleVertex(-X, -Y, T);
    gleVertex(-X, -Y, B);
#else
    gleVertex( X, T, 0);
    gleVertex( X, B, 0);
    gleVertex(-X, T, 0);
    gleVertex(-X, B, 0);
#endif
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    return true;
}

#else

bool SpaceStrip::draw() const
{
    return false;
}

#endif

