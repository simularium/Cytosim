// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "space_sphere.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"
#include "random.h"
#include "meca.h"

SpaceSphere::SpaceSphere(SpaceProp const* p)
: Space(p), radius_(0), radiusSqr_(0)
{
}


void SpaceSphere::resize(Glossary& opt)
{
    real rad = radius_;
    
    if ( opt.set(rad, "diameter") )
        rad *= 0.5;
    else opt.set(rad, "radius");
    
    if ( rad < 0 )
        throw InvalidParameter(prop->name()+":radius must be >= 0");
    
    radius_ = rad;
    update();
}

void SpaceSphere::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-radius_,-radius_,-radius_);
    sup.set( radius_, radius_, radius_);
}


real SpaceSphere::volume() const
{
#if ( DIM == 1 )
    return 2 * radius_;
#elif ( DIM == 2 )
    return M_PI * square(radius_);
#else
    return 4*M_PI/3.0 * cube(radius_);
#endif
}

bool SpaceSphere::inside(Vector const& pos) const
{
    return pos.normSqr() <= radiusSqr_;
}

Vector SpaceSphere::project(Vector const& pos) const
{
    real n = pos.normSqr();
    
    if ( n > 0 ) {
        return pos * ( radius_ / sqrt(n) );
    }
    else {
        //select a random point on the surface
        return radius_ * Vector::randU();
    }
}

//------------------------------------------------------------------------------

void SpaceSphere::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
    meca.addSphereClamp(pos, pe, Vector(0,0,0), radius_, stiff);
}


void SpaceSphere::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca & meca, real stiff) const
{
    if ( radius_ > rad )
        meca.addSphereClamp(pos, pe, Vector(0,0,0), radius_-rad, stiff);
    else {
        meca.addPointClamp(pe, Vector(0,0,0), stiff);
        std::cerr << "object is too big to fit in SpaceSphere\n";
    }
}


//------------------------------------------------------------------------------

void SpaceSphere::write(Outputter& out) const
{
    out.put_characters("sphere", 16);
    out.writeUInt16(2);
    out.writeFloat(radius_);
    out.writeFloat(0.f);
}


void SpaceSphere::setLengths(const real len[])
{
    radius_ = len[0];
    update();
}


void SpaceSphere::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "sphere");
    setLengths(len);
}


//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY

#include "gle.h"

bool SpaceSphere::draw() const
{

#if ( DIM <= 2 )
 
    //number of sections in the quarter-circle
    constexpr size_t fin = ((DIM==2) ? 32 : 8) * gle::finesse;
    
    GLfloat cir[2*fin+2];
    gle::circle(fin, cir, (GLfloat)radius_);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, cir);
    glDrawArrays(GL_LINE_STRIP, 0, fin+1);
    glDisableClientState(GL_VERTEX_ARRAY);

#else
    
    GLfloat R = (GLfloat)radius_;
    glPushMatrix();
    glScalef(R, R, R);
    gle::gleSphere8B();
    gle::gleThreeBands(128);
    glPopMatrix();
    
#endif
    
    return true;
}

#else

bool SpaceSphere::draw() const
{
    return false;
}

#endif
