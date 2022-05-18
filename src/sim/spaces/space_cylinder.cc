// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_cylinder.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "mecapoint.h"
#include "glossary.h"
#include "meca.h"


SpaceCylinder::SpaceCylinder(SpaceProp const* p)
: Space(p)
{
    if ( DIM < 3 )
        throw InvalidParameter("cylinder is only valid in 3D: use rectangle instead");
    length_ = 0;
    radius_ = 0;
}


void SpaceCylinder::resize(Glossary& opt)
{
    real len = length_, rad = radius_;
    
    if ( opt.set(rad, "diameter") )
        rad *= 0.5;
    else opt.set(rad, "radius");
    if ( opt.set(len, "length") )
        len *= 0.5;

    if ( len < 0 )
        throw InvalidParameter("cylinder:length must be >= 0");

    if ( rad < 0 )
        throw InvalidParameter("cylinder:radius must be >= 0");
    
    length_ = len;
    radius_ = rad;
}


void SpaceCylinder::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-length_,-radius_,-radius_);
    sup.set( length_, radius_, radius_);
}


real SpaceCylinder::volume() const
{
    return 2 * M_PI * length_ * radius_ * radius_;
}


bool SpaceCylinder::inside(Vector const& w) const
{
#if ( DIM > 2 )
    const real RT = w.YY * w.YY + w.ZZ * w.ZZ;
    return ( fabs(w.XX) < length_  &&  RT <= radius_ * radius_ );
#elif ( DIM > 1 )
    return ( fabs(w.XX) < length_  &&  fabs(w.YY) <= radius_ );
#else
    return false;
#endif
}


bool SpaceCylinder::allInside(Vector const& w, const real rad) const
{
    assert_true( rad >= 0 );
#if ( DIM > 2 )
    const real RT = w.YY * w.YY + w.ZZ * w.ZZ;
    return ( fabs(w.XX) + rad < length_  &&  RT <= square(radius_-rad) );
#elif ( DIM > 1 )
    return ( fabs(w.XX) + rad < length_  &&  fabs(w.YY) <= radius_-rad );
#else
    return false;
#endif
}


Vector SpaceCylinder::randomPlace() const
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
Vector SpaceCylinder::project(Vector const& w) const
{
    Vector p = w;
#if ( DIM >= 3 )
    bool inX = 1;
    
    if ( fabs(w.XX) > length_ )
    {
        p.XX = std::copysign(length_, w.XX);
        inX = 0;
    }
    
    real n = w.normYZ();
    
    if ( n > radius_ )
    {
        n = radius_ / n;
        p.YY = n * w.YY;
        p.ZZ = n * w.ZZ;
    }
    else
    {
        if ( inX )
        {
            if ( length_ - fabs(w.XX) < radius_ - n )
            {
                p.XX = std::copysign(length_, w.XX);
            }
            else
            {
                n = radius_ / n;
                p.YY = n * w.YY;
                p.ZZ = n * w.ZZ;
            }
        }
    }
#endif
    return p;
}

//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical part and the caps.
 */
void SpaceCylinder::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca,
                                   real stiff, const real len, const real rad)
{
    bool cap = ( fabs(pos.XX) > len );
    bool cyl = false;
    real X = std::copysign(len, pos.XX);
    
#if ( DIM > 2 )
    
    real dis = pos.YY*pos.YY + pos.ZZ*pos.ZZ;
    
    if ( rad*rad < dis )
    {
        // outside cylinder in YZ plane
        cyl = true;
    }
    else if ( ! cap )
    {
        // inside cylinder in YZ plane and also inside in X:
        if ( dis > square( rad - fabs(pos.XX-X) ) )
            cyl = true;
        else
            cap = true;
    }
    
#endif

    if ( cap )
    {
        const index_t inx = DIM * pe.matIndex();
        meca.mC(inx, inx) -= stiff;
        meca.base(inx)    += stiff * X;
    }
  
    if ( cyl )
        meca.addCylinderClampX(pe, rad, stiff);
}


/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinder::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length_, radius_);
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinder::setInteraction(Vector const& pos, Mecapoint const& pe,
                                   real rad, Meca & meca, real stiff) const
{
    real eRadius = radius_ - rad;
    if ( eRadius < 0 ) eRadius = 0;
    real eLength = length_ - rad;
    if ( eLength < 0 ) eLength = 0;
    
    setInteraction(pos, pe, meca, stiff, eLength, eRadius);
}

//------------------------------------------------------------------------------

void SpaceCylinder::write(Outputter& out) const
{
    out.put_characters("cylinder", 16);
    out.writeUInt16(2);
    out.writeFloat(length_);
    out.writeFloat(radius_);
}


void SpaceCylinder::setLengths(const real len[])
{
    length_ = len[0];
    radius_ = len[1];
}

void SpaceCylinder::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "cylinder");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpaceCylinder::draw() const
{
#if ( DIM > 2 )

    const size_t fin = 512;

    GLfloat L = (GLfloat)length_;
    GLfloat R = (GLfloat)radius_;
    
    GLfloat c[fin+1], s[fin+1];
    gle::circle(fin, c, s, 1);
    
    glBegin(GL_TRIANGLE_STRIP);
    for ( size_t sc = 0; sc <= fin; ++sc )
    {
        GLfloat ca = c[sc], sa = s[sc];
        glNormal3f( 0, ca, sa );
        glVertex3f( +L, R*ca, R*sa );
        glVertex3f( -L, R*ca, R*sa );
    }
    glEnd();
    
    // draw the cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( +1, 0, 0 );
    glVertex3f( +L, 0, 0 );
    for ( size_t sc = 0; sc <= fin; ++sc )
        glVertex3f( +L, R*c[sc], R*s[sc] );
    glEnd();
    
    // draw the cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( -1, 0, 0 );
    glVertex3f( -L, 0, 0 );
    for ( size_t sc = 0; sc <= fin; ++sc )
        glVertex3f( -L,-R*c[sc], R*s[sc] );
    glEnd();
    
#endif
    return true;
}

#else

bool SpaceCylinder::draw() const
{
    return false;
}

#endif

