// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "space_banana.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"


SpaceBanana::SpaceBanana(SpaceProp const* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("banana is not edible in 1D");
    bLength = 10;
    bWidth  = 2;
    bRadius = 20;
}


void SpaceBanana::resize(Glossary& opt)
{
    real len = bLength, wid = bWidth, rad = bRadius;
    
    if ( opt.set(wid, "radius") )
        wid *= 2;
    else opt.set(wid, "diameter");
    opt.set(rad, "curvature");
    opt.set(len, "length");

    len = len - 2 * wid;

    if ( len <= 0 )
        throw InvalidParameter("banana:length must be >= 2 * width");
    if ( rad <= 0 )
        throw InvalidParameter("banana:radius must be >= 0");
    if ( wid < 0 )
        throw InvalidParameter("banana:width must be > 0");
    if ( wid > bRadius )
        throw InvalidParameter("banana:width must be <= radius");
    
    bLength = len;
    bWidth = wid;
    bRadius = rad;
    update();
}


void SpaceBanana::update()
{
    bAngle = 0.5 * bLength / bRadius;
    
    if ( bAngle > M_PI )
    {
        bAngle = M_PI;
        std::cerr << "banana:length should not exceed 2*PI*radius\n";
    }

    bWidthSqr = bWidth * bWidth;
    bEnd[0] = bRadius * sin(bAngle);
    bEnd[1] = 0.5*bRadius*(1-cos(bAngle));
    
    bCenter[0] = 0;
    bCenter[1] = bRadius - bEnd[1];
    bCenter[2] = 0;
}


real SpaceBanana::volume() const
{
#if ( DIM > 2 )
    return 2*bAngle*M_PI*bRadius*bWidthSqr + 4/3.*M_PI*bWidthSqr*bWidth;
#else
    return 4*bAngle*bRadius*bWidth + M_PI*bWidthSqr;
#endif
}


void SpaceBanana::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-bEnd[0]-bWidth,-bWidth,-bWidth);
    sup.set( bEnd[0]+bWidth, bEnd[1]+bWidth, bWidth);
}


/// project on the backbone circular arc in the XY plane:
Vector SpaceBanana::backbone(Vector const& pos) const
{
    Vector cp = pos - bCenter;
    
    real n = bRadius / cp.normXY();
    Vector prj;

    prj[0] = bCenter[0] + n * cp[0];
    prj[1] = bCenter[1] + n * cp[1];
    
    if ( prj[1] > bEnd[1] )
    {
        prj[0] = std::copysign(bEnd[0], pos[0]);
        prj[1] = bEnd[1];
    }
    
    if ( DIM > 2 )
        prj[2] = 0;
    return prj;
}


bool SpaceBanana::inside(Vector const& pos) const
{
    Vector prj = backbone(pos);
    return ( distanceSqr(pos, prj) <= bWidthSqr );
}


Vector SpaceBanana::project(Vector const& pos) const
{
    Vector cen = backbone(pos);
    Vector dif = pos - cen;
    real n = dif.normSqr();
    return cen + (bWidth / sqrt(n)) * dif;
}


//------------------------------------------------------------------------------

void SpaceBanana::write(Outputter& out) const
{
    out.put_characters("banana", 16);
    out.writeUInt16(4);
    out.writeFloat(bLength);
    out.writeFloat(bWidth);
    out.writeFloat(bRadius);
    out.writeFloat(0.f);
}


void SpaceBanana::setLengths(const real len[])
{
    bLength = len[0];
    bWidth  = len[1];
    bRadius = len[2];
    update();
}

void SpaceBanana::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "banana");
    setLengths(len);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY

#include "gle.h"
using namespace gle;

bool SpaceBanana::draw() const
{
#if ( DIM == 2 )
    
    //number of sections in the quarter-circle
    constexpr size_t fin = 8 * gle::finesse;
    GLfloat c[fin+1], s[fin+1];
    
    GLfloat A = -bAngle + M_PI_2;
    GLfloat B =  bAngle - M_PI_2;
    
    glBegin(GL_LINE_LOOP);
    // lower swing
    gle::arc(fin, c, s, bRadius+bWidth, A-M_PI, B, bCenter[0], bCenter[1]);
    for ( size_t i = 0; i < fin; ++i )
        gleVertex(c[i], s[i], 0);
    
    // right cap
    gle::arc(fin, c, s, bWidth, B, B+M_PI, bEnd[0], bEnd[1]);
    for ( size_t i = 0; i < fin; ++i )
        gleVertex(c[i], s[i], 0);
    
    // upper swing
    gle::arc(fin, c, s, bRadius-bWidth, B, A-M_PI, bCenter[0], bCenter[1]);
    for ( size_t i = 0; i < fin; ++i )
        gleVertex(c[i], s[i], 0);
        
    // left cap
    gle::arc(fin, c, s, bWidth, A, A+M_PI, -bEnd[0], bEnd[1]);
    for ( size_t i = 0; i < fin; ++i )
        gleVertex(c[i], s[i], 0);

    glEnd();
    
#elif ( DIM > 2 )
    
    glMatrixMode(GL_MODELVIEW);

    GLdouble plane1[] = { -cos(bAngle), -sin(bAngle), 0, 0 };
    GLdouble plane2[] = {  cos(bAngle), -sin(bAngle), 0, 0 };
    GLdouble plane1i[] = {  cos(bAngle), sin(bAngle), 0, 0 };
    GLdouble plane2i[] = { -cos(bAngle), sin(bAngle), 0, 0 };
    
    const GLenum glp1 = GL_CLIP_PLANE4;
    const GLenum glp2 = GL_CLIP_PLANE5;
    
    glEnable(glp1);
    glEnable(glp2);
    
    //center part:
    glPushMatrix();
    glTranslated(bCenter[0], bCenter[1], 0);
    glClipPlane(glp1, plane1);
    glClipPlane(glp2, plane2);
    gleTorus(bRadius, bWidth);
    glPopMatrix();

    glDisable(glp2);

    //right cap:
    glPushMatrix();
    glTranslated(bEnd[0], bEnd[1], 0);
    glClipPlane(glp1, plane1i);
    gleScale(bWidth);
    gleSphere8B();
    glPopMatrix();

    //left cap:
    glPushMatrix();
    glTranslated(-bEnd[0], bEnd[1], 0);
    glClipPlane(glp1, plane2i);
    gleScale(bWidth);
    gleSphere8B();
    glPopMatrix();
    
    glDisable(glp1);
    
#endif
    return true;
}

#else

bool SpaceBanana::draw() const
{
    return false;
}


#endif
