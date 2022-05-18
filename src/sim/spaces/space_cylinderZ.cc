// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_cylinderZ.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "mecapoint.h"
#include "glossary.h"
#include "meca.h"


SpaceCylinderZ::SpaceCylinderZ(SpaceProp const* p)
: Space(p)
{
    if ( DIM < 3 )
        throw InvalidParameter("cylinderZ is only valid in 3D: use sphere instead");
    bot_ = 0;
    top_ = 0;
    radius_ = 0;
}


void SpaceCylinderZ::resize(Glossary& opt)
{
    real rad = radius_, top = top_, bot = bot_;

    if ( opt.set(rad, "diameter") )
        rad *= 0.5;
    else opt.set(rad, "radius");
    opt.set(bot, "bottom");
    opt.set(top, "top");
    
    if ( rad < 0 )
        throw InvalidParameter("cylinderZ:radius must be >= 0");

    if ( top < bot )
        throw InvalidParameter("cylinerZ:bottom must be <= top");
    
    bot_ = bot;
    top_ = top;
    radius_ = rad;
}


void SpaceCylinderZ::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-radius_,-radius_, bot_);
    sup.set( radius_, radius_, top_);
}


real  SpaceCylinderZ::volume() const
{
    return M_PI * ( top_ - bot_ ) * radius_ * radius_;
}


bool  SpaceCylinderZ::inside(Vector const& w) const
{
#if ( DIM > 2 )
    if ( w.ZZ < bot_ ) return false;
    if ( w.ZZ > top_ ) return false;
#endif
#if ( DIM > 1 )
    return ( w.XX*w.XX + w.YY*w.YY <= radius_ * radius_ );
#else
    return ( fabs(w.XX) <= radius_ );
#endif
}

bool  SpaceCylinderZ::allInside(Vector const& w, const real rad ) const
{
    assert_true( rad >= 0 );
#if ( DIM > 2 )
    if ( w.ZZ - rad < bot_ ) return false;
    if ( w.ZZ + rad > top_ ) return false;
#endif
#if ( DIM > 1 )
    return ( w.XX*w.XX + w.YY*w.YY <= square(radius_-rad) );
#else
    return ( fabs(w.XX) <= radius_-rad );
#endif
}

Vector SpaceCylinderZ::randomPlace() const
{
    const Vector2 V = Vector2::randB(radius_);
    return Vector(V.XX, V.YY, bot_+RNG.preal()*(top_-bot_));
}

//------------------------------------------------------------------------------
Vector SpaceCylinderZ::project(Vector const& w) const
{
    Vector p = w;
#if ( DIM >= 3 )
    bool inZ = true;
    
    if ( w.ZZ > top_ )
    {
        p.ZZ = top_;
        inZ = false;
    }
    else if ( w.ZZ < bot_ )
    {
        p.ZZ = bot_;
        inZ = false;
    }

    real n = w.normXY();
    
    if ( n > radius_ )
    {
        n = radius_ / n;
        p.XX = n * w.XX;
        p.YY = n * w.YY;
    }
    else
    {
        if ( inZ )
        {
            if ( top_ - w.ZZ < radius_ - n )
                p.ZZ = top_;
            else if ( w.ZZ - bot_ < radius_ - n )
                p.ZZ = bot_;
            else
            {
                n = radius_ / n;
                p.XX = n * w.XX;
                p.YY = n * w.YY;
            }
        }
    }
#endif
    return p;
}

//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderZ::setInteraction(Vector const& pos, Mecapoint const& pe,
                                    Meca & meca, real stiff,
                                    const real rad, const real B, const real T)
{
#if ( DIM >= 3 )
    bool cap = false;
    bool cyl = false;
    real Z;

    // inside cylinder radius_
    if ( 2 * pos.ZZ - B > T )
    {
        Z = T;
        cap = ( pos.ZZ > T );
    }
    else
    {
        Z = B;
        cap = ( pos.ZZ < B );
    }
    
    real dis = pos.XX*pos.XX + pos.YY*pos.YY;
    
    if ( rad*rad < dis )
    {
        // outside cylinder in XY plane
        cyl = true;
    }
    else if ( ! cap )
    {
        // inside cylinder in XY plane and also inside in Z:
        if ( dis > square( rad - fabs(pos.ZZ-Z) ) )
            cyl = true;
        else
            cap = true;
    }
    
    if ( cap )
    {
        const index_t inx = 2 + DIM * pe.matIndex();
        meca.mC(inx, inx) -= stiff;
        meca.base(inx)    += stiff * Z;
    }
    
    if ( cyl )
        meca.addCylinderClampZ(pe, rad, stiff);
#endif
}


/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderZ::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, radius_, bot_, top_);
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderZ::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca & meca, real stiff) const
{
    real R = std::max((real)0, radius_ - rad);
    real T = top_ - rad;
    real B = bot_ + rad;
    
    if ( B > T )
    {
        B = 0.5 * ( top_ + bot_ );
        T = B;
    }
    
    setInteraction(pos, pe, meca, stiff, R, B, T);
}


//------------------------------------------------------------------------------

void SpaceCylinderZ::write(Outputter& out) const
{
    out.put_characters("cylinderZ", 16);
    out.writeUInt16(4);
    out.writeFloat(radius_);
    out.writeFloat(bot_);
    out.writeFloat(top_);
    out.writeFloat(0.f);
}


void SpaceCylinderZ::setLengths(const real len[])
{
    radius_ = len[0];
    bot_    = len[1];
    top_    = len[2];
}

void SpaceCylinderZ::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "cylinderZ");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpaceCylinderZ::draw() const
{
#if ( DIM > 2 )
    
    GLfloat T = top_;
    GLfloat B = bot_;
    GLfloat R = radius_;
    
    const size_t fin = 512;
    GLfloat c[fin+1], s[fin+1];
    gle::circle(fin, c, s, 1);
    
    glBegin(GL_TRIANGLE_STRIP);
    //display strips along the side of the volume:
    for ( size_t n = 0; n <= fin; ++n )
    {
        glNormal3f(c[n], s[n], 0);
        glVertex3f(R*c[n], R*s[n], T);
        glVertex3f(R*c[n], R*s[n], B);
    }
    glEnd();
    
    // draw top cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, +1);
    glVertex3f(0, 0,  T);
    for ( size_t n = 0; n <= fin; ++n )
        glVertex3f(R*c[n], R*s[n], T);
    glEnd();
    
    // draw bottom cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    glVertex3f(0, 0,  B);
    for ( size_t n = 0; n <= fin; ++n )
        glVertex3f(-R*c[n], R*s[n], B);
    glEnd();
    
#endif
    return true;
}

#else

bool SpaceCylinderZ::draw() const
{
    return false;
}

#endif
