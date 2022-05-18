// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "space_torus.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"

SpaceTorus::SpaceTorus(SpaceProp const* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("torus is not usable in 1D");
    bWidth  = 2;
    bRadius = INFINITY;
}


void SpaceTorus::resize(Glossary& opt)
{
    real rad = bRadius, wid = bWidth;
    
    if ( opt.set(wid, "width") )
        wid *= 0.5;
    opt.set(bRadius, "radius");

    if ( rad <= 0 )
        throw InvalidParameter("torus:radius must be > 0");
    if ( wid <= 0 )
        throw InvalidParameter("torus:width must be > 0");
    if ( wid > rad )
        throw InvalidParameter("torus:width must be < radius");
    
    bWidth = wid;
    bRadius = rad;
    update();
}


real SpaceTorus::volume() const
{
#if ( DIM == 2 )
    return 4 * M_PI * bRadius * bWidth;
#else
    return 2 * M_PI * M_PI * bRadius * bWidthSqr;
#endif
}


void SpaceTorus::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-bRadius-bWidth,-bRadius-bWidth,-bWidth);
    sup.set( bRadius+bWidth, bRadius+bWidth, bWidth);
}


///project on the backbone circle in the XY plane:
Vector SpaceTorus::backbone(Vector const& pos) const
{
#if ( DIM > 1 )
    real n = bRadius / pos.normXY();
    return Vector(n * pos.XX, n * pos.YY, 0);
#else
    return Vector(0, 0, 0);
#endif
}


bool SpaceTorus::inside(Vector const& pos) const
{
    Vector prj = backbone(pos);
    return ( distanceSqr(prj, pos) <= bWidthSqr );
}


Vector SpaceTorus::project(Vector const& pos) const
{
    Vector cen = backbone(pos);
    Vector ax = pos - cen;
    real n = ax.normSqr();
    n = bWidth / sqrt(n);
    return cen + n * ax;
}


//------------------------------------------------------------------------------

void SpaceTorus::write(Outputter& out) const
{
    out.put_characters("torus", 16);
    out.writeUInt16(2);
    out.writeFloat(bRadius);
    out.writeFloat(bWidth);
}


void SpaceTorus::setLengths(const real len[])
{
    bRadius = len[0];
    bWidth = len[2];
    update();
}

void SpaceTorus::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "torus");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY

#include "gle.h"
using namespace gle;

bool SpaceTorus::draw() const
{
#if ( DIM == 2 )
    
    constexpr size_t fin = 16 * gle::finesse;
    GLfloat cir[2*fin+2];

    glEnableClientState(GL_VERTEX_ARRAY);

    gle::circle(fin, cir, GLfloat(bRadius-bWidth));
    glVertexPointer(2, GL_FLOAT, 0, cir);
    glDrawArrays(GL_LINE_STRIP, 0, fin+1);

    gle::circle(fin, cir, GLfloat(bRadius+bWidth));
    glVertexPointer(2, GL_FLOAT, 0, cir);
    glDrawArrays(GL_LINE_STRIP, 0, fin+1);

    glDisableClientState(GL_VERTEX_ARRAY);
    return true;
    
#elif ( DIM > 2 )
    
    gleTorus(bRadius, bWidth);
    return true;
    
#else
    return false;
#endif
}

#else

bool SpaceTorus::draw() const
{
    return false;
}


#endif
