// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "space_cylinderP.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "mecapoint.h"
#include "glossary.h"
#include "meca.h"


SpaceCylinderP::SpaceCylinderP(SpaceProp const* p)
: Space(p)
{
    if ( DIM < 3 )
        throw InvalidParameter("cylinderP is only valid in 3D: use strip instead");
    length_ = 0;
    radius_ = 0;
}

void SpaceCylinderP::resize(Glossary& opt)
{
    real len = length_, rad = radius_;
    
    if ( opt.set(rad, "diameter") )
        rad *= 0.5;
    else opt.set(rad, "radius");

    if ( opt.set(len, "length") )
        len *= 0.5;

    if ( rad < 0 )
        throw InvalidParameter("cylinderP:radius must be >= 0");

    if ( len <= 0 )
        throw InvalidParameter("cylinderP:length must be > 0");
    
    length_ = len;
    radius_ = rad;

    update();
}


void SpaceCylinderP::update()
{
    modulo_.reset();
    modulo_.enable(0, 2*length_);
}


void SpaceCylinderP::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-length_,-radius_,-radius_);
    sup.set( length_, radius_, radius_);
}


real SpaceCylinderP::volume() const
{
    return 2 * M_PI * length_ * square(radius_);
}


bool SpaceCylinderP::inside(Vector const& W) const
{
#if ( DIM > 2 )
    const real RT = W.YY * W.YY + W.ZZ * W.ZZ;
    return ( RT <= square(radius_) );
#elif ( DIM > 1 )
    return ( fabs(W.YY) <= radius_ );
#else
    return false;
#endif
}


bool SpaceCylinderP::allInside(Vector const& W, const real rad) const
{
    assert_true( rad >= 0 );
#if ( DIM > 2 )
    const real RT = W.YY * W.YY + W.ZZ * W.ZZ;
    return ( RT <= square(radius_-rad) );
#elif ( DIM > 1 )
    return ( fabs(W.YY) <= radius_-rad );
#else
    return false;
#endif
}


Vector SpaceCylinderP::randomPlace() const
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


Vector SpaceCylinderP::normalToEdge(Vector const& pos) const
{
#if ( DIM >= 3 )
    real n = 1.0 / pos.normYZ();
    return Vector(0, n * pos.YY, n * pos.ZZ);
#elif ( DIM >= 2 )
    return Vector(0, std::copysign((real)1, pos.YY), 0);
#endif
    return Vector(0, 0, 0);  // intentionally invalid!
}


Vector SpaceCylinderP::randomPlaceOnEdge(real) const
{
#if ( DIM >= 3 )
    const Vector2 YZ = Vector2::randU(radius_);
    return Vector(0, YZ.XX, YZ.YY);
#endif
    return Vector(0, radius_*RNG.sflip(), 0);
}


//------------------------------------------------------------------------------
Vector SpaceCylinderP::project(Vector const& W) const
{
    Vector P(W);
    
#if ( DIM > 2 )
    real n = W.normYZ();
    if ( n > REAL_EPSILON )
    {
        P.YY = W.YY * ( radius_ / n );
        P.ZZ = W.ZZ * ( radius_ / n );
    }
    else
    {
        const Vector2 V = Vector2::randU();
        P.YY = radius_ * V.XX;
        P.ZZ = radius_ * V.YY;
    }
#endif
    return P;
}

//------------------------------------------------------------------------------

/**
 This applies forces towards the cylindrical surface only
 */
void SpaceCylinderP::setInteraction(Vector const& pos, Mecapoint const& pe, Meca& meca, real stiff) const
{
    meca.addCylinderClampX(pe, radius_, stiff);
}

/**
 This applies forces towards the cylindrical surface only
 */
void SpaceCylinderP::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca& meca, real stiff) const
{
    real R = std::max((real)0, radius_ - rad);

    meca.addCylinderClampX(pe, R, stiff);
}

//------------------------------------------------------------------------------

void SpaceCylinderP::write(Outputter& out) const
{
    out.put_characters("cylinderP", 16);
    out.writeUInt16(2);
    out.writeFloat(length_);
    out.writeFloat(radius_);
}


void SpaceCylinderP::setLengths(const real len[])
{
    length_ = len[0];
    radius_ = len[1];
    update();
}


void SpaceCylinderP::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "cylinderP");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpaceCylinderP::draw() const
{
#if ( DIM > 2 )

    const size_t fin = 512;
    GLfloat c[fin+1], s[fin+1];
    gle::circle(fin, c, s, 1);

    GLfloat L = (GLfloat)length_;
    GLfloat R = (GLfloat)radius_;

    glBegin(GL_TRIANGLE_STRIP);
    for ( size_t n = 0; n <= fin; ++n )
    {
        glNormal3f( 0, c[n], s[n] );
        glVertex3f( +L, R*c[n], R*s[n] );
        glVertex3f( -L, R*c[n], R*s[n] );
    }
    glEnd();
    
    if ( 1 )
    {
        //draw dotted-rings to indicate periodicity
        glLineStipple(1, 0x000F);
        glEnable(GL_LINE_STIPPLE);
        glPushMatrix();
        glTranslatef(L, 0, 0);
        glScalef(R, R, R);
        glRotated(90, 0, 1, 0);
        gle::gleCircle();
        glTranslatef(0, 0, -2*L/R);
        glRotated(180, 0, 1, 0);
        gle::gleCircle();
        glPopMatrix();
        glDisable(GL_LINE_STIPPLE);
    }

#endif
    return true;
}

#else

bool SpaceCylinderP::draw() const
{
    return false;
}

#endif

