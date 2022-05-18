// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_capsule.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "mecapoint.h"
#include "meca.h"


SpaceCapsule::SpaceCapsule(SpaceProp const* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("capsule is only defined for DIM = 2 and 3");
}


void SpaceCapsule::resize(Glossary& opt)
{
    real len = length_, rad = radius_;
    
    if ( opt.set(rad, "diameter") )
        rad *= 0.5;
    else opt.set(rad, "radius");
    // total length is specified:
    if ( opt.set(len, "length") )
        len = ( len - 2 * rad ) * 0.5;

    if ( len < 0 )
        throw InvalidParameter("capsule:length must be >= 2 * radius");
    if ( rad < 0 )
        throw InvalidParameter("capsule:radius must be >= 0");
    
    length_ = len;
    radius_ = rad;
    update();
}


void SpaceCapsule::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-radius_-length_,-radius_,-radius_);
    sup.set( radius_+length_, radius_, radius_);
}


real SpaceCapsule::volume() const
{
#if ( DIM >= 3 )
    return ( length_ + (2/3.0) * radius_ ) * radiusSqr_ * ( 2 * M_PI );
#else
    return 4 * length_ * radius_ + M_PI * radiusSqr_;
#endif
}

bool SpaceCapsule::inside(Vector const& w) const
{
    real n = w.normYZSqr() + square(std::max((real)0, fabs(w.XX)-length_));
    
    return ( n <= radiusSqr_ );
}


bool SpaceCapsule::allInside(Vector const& w, const real rad) const
{
    assert_true( rad >= 0 );
    real n = w.normYZSqr() + square(std::max((real)0, fabs(w.XX)-length_));
    
    return ( n <= square(radius_-rad) );
}

//------------------------------------------------------------------------------
Vector SpaceCapsule::project(Vector const& w) const
{
    Vector p;
    real n = w.normYZSqr();
    
    //calculate the projection on the axis, within boundaries:
    if ( fabs(w.XX) > length_ )
    {
        real L = std::copysign(length_, w.XX);
        n += square( w.XX - L );
        //normalize from this point on the axis
        if ( n > 0 ) n = radius_ / sqrt(n);
        
        p.XX = L + n * ( w.XX - L );
    }
    else
    {
        //normalize from this point on the axis
        if ( n > 0 ) n = radius_ / sqrt(n);
        
        p.XX = w.XX;
    }
    
    if ( n > 0 )
    {
#if ( DIM > 1 )
        p.YY = n * w.YY;
#endif
#if ( DIM >= 3 )
        p.ZZ = n * w.ZZ;
#endif
    }
    else
    {
        //we project on a arbitrary point on the cylinder
#if ( DIM > 1 )
        p.YY = radius_;
#endif
#if ( DIM >= 3 )
        p.ZZ = 0;
#endif
    }
    return p;
}


Vector SpaceCapsule::randomPlace() const
{
    size_t nb_trials = 1<<13;
    size_t ouf = 0;
    Vector res;

    do {
        
#if ( DIM == 1 )
        res.set((length_+radius_)*RNG.sreal());
#elif ( DIM == 2 )
        res.set((length_+radius_)*RNG.sreal(), radius_*RNG.sreal());
#else
        const Vector2 V = Vector2::randB(radius_);
        res.set((length_+radius_)*RNG.sreal(), V.XX, V.YY);
#endif
        
        if ( ++ouf > nb_trials )
        {
            std::clog << "placement failed in SpaceCapsule::randomPlace()" << std::endl;
            return Vector(0,0,0);
        }
        
    } while ( ! inside(res) );
    
    return res;
}

//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCapsule::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff, const real len, const real rad)
{
    if ( fabs(pos.XX) > len )
        meca.addSphereClamp(pos, pe, Vector(std::copysign(len, pos.XX),0,0), rad, stiff);
    else
        meca.addCylinderClampX(pe, rad, stiff);
}


/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCapsule::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length_, radius_);
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCapsule::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca & meca, real stiff) const
{
    if ( rad < radius_ )
        setInteraction(pos, pe, meca, stiff, length_, radius_-rad);
    else
        setInteraction(pos, pe, meca, stiff, length_, 0);
}

//------------------------------------------------------------------------------

void SpaceCapsule::write(Outputter& out) const
{
    out.put_characters("capsule", 16);
    out.writeUInt16(2);
    out.writeFloat(length_);
    out.writeFloat(radius_);
}


void SpaceCapsule::setLengths(const real len[])
{
    length_ = len[0];
    radius_ = len[1];
    update();
}

void SpaceCapsule::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "capsule");
    setLengths(len);
}


//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpaceCapsule::draw() const
{
    //number of sections in the quarter-circle
    constexpr size_t fin = ((DIM==2) ? 32 : 8) * gle::finesse;
    
    GLfloat c[4*fin+1], s[4*fin+1];
    gle::circle(4*fin, c, s, 1);
    
    GLfloat L = (GLfloat)length_;
    GLfloat R = (GLfloat)radius_;
    
#if ( DIM <= 2 )
    
    //display a loop in X/Y plane
    glBegin(GL_LINE_LOOP);
    
    for ( size_t n = 0;     n <= 2*fin; ++n )
        glVertex2f(R*s[n]+L, R*c[n]);
    
    for ( size_t n = 2*fin; n <= 4*fin; ++n )
        glVertex2f(R*s[n]-L, R*c[n]);
    
    glEnd();
    
#else
    
    //display strips along the side of the volume:
    for ( size_t sc = 0; sc < 4*fin; ++sc )
    {
        //compute the transverse angles:
        GLfloat ctb  = c[sc  ],   stb  = s[sc  ];
        GLfloat cta  = c[sc+1],   sta  = s[sc+1];
        GLfloat ctbR = R*ctb,     stbR = R*stb;
        GLfloat ctaR = R*cta,     staR = R*sta;
        
        //draw one strip of the oval:
        glBegin(GL_TRIANGLE_STRIP);
        for ( size_t ii=0; ii <= fin; ++ii )
        {
            GLfloat ca = c[ii], sa = s[ii];
            glNormal3f( ca, cta*sa, sta*sa );
            glVertex3f( +L+R*ca, ctaR*sa, staR*sa );
            glNormal3f( ca, ctb*sa, stb*sa );
            glVertex3f( +L+R*ca, ctbR*sa, stbR*sa );
        }
        for ( int ii=fin; ii >= 0; --ii)
        {
            GLfloat ca = -c[ii], sa = s[ii];
            glNormal3f( ca, cta*sa, sta*sa );
            glVertex3f( -L+R*ca, ctaR*sa, staR*sa );
            glNormal3f( ca, ctb*sa, stb*sa );
            glVertex3f( -L+R*ca, ctbR*sa, stbR*sa );
        }
        glEnd();
    }
    
    if ( 1 )
    {
        //draw 2 rings on the surface
        glPushMatrix();
        glTranslatef(L, 0, 0);
        glScalef(R, R, R);
        glRotated(90, 0, 1, 0);
        gle::gleArrowedBand(24, 0.25);
        glTranslatef(0, 0, -2*L/R);
        glRotated(180, 0, 1, 0);
        gle::gleArrowedBand(24, 0.25);
        glPopMatrix();
    }

#endif
    return true;
}

#else

bool SpaceCapsule::draw() const
{
    return false;
}

#endif
