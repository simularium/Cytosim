// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_ring.h"
#include "exceptions.h"
#include "mecapoint.h"
#include "iowrapper.h"
#include "glossary.h"
#include "meca.h"


SpaceRing::SpaceRing(SpaceProp const* p)
: Space(p)
{
    if ( DIM < 3 )
        throw InvalidParameter("ring is only valid in 3D: use rectangle instead");
    length_ = 0;
    radius_ = 0;
}


void SpaceRing::resize(Glossary& opt)
{
    real len = length_, rad = radius_;
    
    if ( opt.set(rad, "diameter") )
        rad *= 0.5;
    else opt.set(rad, "radius");
    if ( opt.set(len, "length") )
        len *= 0.5;

    if ( len < 0 )
        throw InvalidParameter("ring:length must be > 0");
    if ( rad < 0 )
        throw InvalidParameter("ring:radius must be >= 0");

    length_ = len;
    radius_ = rad;
    update();
}


void SpaceRing::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-length_,-radius_,-radius_);
    sup.set( length_, radius_, radius_);
}


real  SpaceRing::volume() const
{
    return 2 * M_PI * length_ * radius_ * radius_;
}


Vector SpaceRing::randomPlace() const
{
#if ( DIM >= 3 )
    const Vector2 V = Vector2::randB(radius_);
    return Vector(length_*RNG.sreal(), V.XX, V.YY);
#elif ( DIM > 1 )
    return Vector(length_*RNG.sreal(), radius_*RNG.sreal());
#else
    return Vector(length_*RNG.sreal());
#endif
}


//------------------------------------------------------------------------------
bool SpaceRing::inside(Vector const& w) const
{
#if ( DIM > 2 )
    const real RT = w.YY * w.YY + w.ZZ * w.ZZ;
    return ( fabs(w.XX) <= length_  &&  RT <= radiusSqr_ );
#else
    return false;
#endif
}

bool SpaceRing::allInside(Vector const& w, const real rad ) const
{
    assert_true( rad >= 0 );

#if ( DIM > 2 )
    const real RT = w.YY * w.YY + w.ZZ * w.ZZ;
    return ( fabs(w.XX) + rad <= length_  &&  RT <= square(radius_-rad) );
#else
    return false;
#endif
}

//------------------------------------------------------------------------------
/**
 Project always on the surface of the cylinder
 */
Vector SpaceRing::project(Vector const& w) const
{
    Vector p;
    if ( w.XX >  length_ )
        p.XX =  length_;
    else if ( w.XX < -length_ )
        p.XX = -length_;
    else
        p.XX = w.XX;
    
#if ( DIM > 2 )
    real n = sqrt( w.YY*w.YY+ w.ZZ*w.ZZ );
    
    if ( n > 0 )
    {
        n = radius_ / n;
        p.YY = n * w.YY;
        p.ZZ = n * w.ZZ;
    }
    else
    {
        p.YY = radius_;
        p.ZZ = 0;
    }
#endif
    return p;
}

//------------------------------------------------------------------------------

/**
 This applies a force directed to the surface of the cylinder
 */
void SpaceRing::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff, const real len, const real rad)
{
    const index_t inx = DIM * pe.matIndex();

    if ( pos.XX > len )
    {
        meca.mC(inx, inx) -= stiff;
        meca.base(inx)    += stiff * len;
    }
    else if ( pos.XX < -len )
    {
        meca.mC(inx, inx) -= stiff;
        meca.base(inx)    -= stiff * len;
    }
    
    meca.addCylinderClampX(pe, rad, stiff);
}


/**
 This applies a force directed to the surface of the cylinder
 */
void SpaceRing::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length_, radius_);
}

/**
 This applies a force directed to the surface of the cylinder
 */
void SpaceRing::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length_, radius_);
}

//------------------------------------------------------------------------------

void SpaceRing::write(Outputter& out) const
{
    out.put_characters("ring", 16);
    out.writeUInt16(2);
    out.writeFloat(length_);
    out.writeFloat(radius_);
}


void SpaceRing::setLengths(const real len[])
{
    length_ = len[0];
    radius_ = len[1];
    update();
}


void SpaceRing::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "ring");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpaceRing::draw() const
{
#if ( DIM > 2 )

    const size_t fin = 512;
    GLfloat c[fin+1], s[fin+1];
    gle::circle(fin, c, s, GLfloat(radius_));

    GLfloat L = GLfloat(length_);
    
    glBegin(GL_TRIANGLE_STRIP);
    for ( size_t n = 0; n <= fin; ++n )
    {
        glNormal3f( 0, c[n], s[n]);
        glVertex3f(+L, c[n], s[n]);
        glVertex3f(-L, c[n], s[n]);
    }
    glEnd();
    
#endif
    return true;
}

#else

bool SpaceRing::draw() const
{
    return false;
}

#endif

