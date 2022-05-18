// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "space_periodic.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"


SpacePeriodic::SpacePeriodic(SpaceProp const* p)
: Space(p)
{
    for ( int d = 0; d < 3; ++d )
        length_[d] = 0;
}


void SpacePeriodic::resize(Glossary& opt)
{
    for ( unsigned d = 0; d < DIM; ++d )
    {
        real len = length_[d];
        if ( opt.set(len, "length", d) )
            len *= 0.5;
        if ( len <= 0 )
            throw InvalidParameter("periodic:length[",d,"] must be > 0");
        length_[d] = len;
    }
    update();
}


void SpacePeriodic::update()
{
    modulo_.reset();
    for ( unsigned d = 0; d < DIM; ++d )
        modulo_.enable(d, 2*length_[d]);
}


void SpacePeriodic::boundaries(Vector& inf, Vector& sup) const
{
    inf.set(-length_[0],-length_[1],-length_[2]);
    sup.set( length_[0], length_[1], length_[2]);
}

//------------------------------------------------------------------------------
#pragma mark -

#if ( DIM == 1 )

real SpacePeriodic::volume() const
{
    return 2.0 * length_[0];
}

bool SpacePeriodic::inside(Vector const& point) const
{
    return true;
}


Vector SpacePeriodic::project(Vector const&) const
{
    throw InvalidParameter("A periodic space has no edge!");
    return Vector(0, 0, 0);
}

#endif


//------------------------------------------------------------------------------

#if ( DIM == 2 )

real SpacePeriodic::volume() const
{
    return 4.0 * length_[0] * length_[1];
}

bool SpacePeriodic::inside(Vector const& point) const
{
    return true;
}


Vector SpacePeriodic::project(Vector const&) const
{
    throw InvalidParameter("A periodic space has no edge!");
}

#endif

//------------------------------------------------------------------------------

#if ( DIM >= 3 )

real SpacePeriodic::volume() const
{
    return 8.0 * length_[0] * length_[1] * length_[2];
}

bool SpacePeriodic::inside(Vector const& point) const
{
    return true;
}

Vector SpacePeriodic::project(Vector const&) const
{
    throw InvalidParameter("A periodic space has no edge!");
    return Vector(0, 0, 0);
}

#endif

//------------------------------------------------------------------------------

void SpacePeriodic::write(Outputter& out) const
{
    out.put_characters("periodic", 16);
    out.writeUInt16(4);
    out.writeFloat(length_[0]);
    out.writeFloat(length_[1]);
    out.writeFloat(length_[2]);
    out.writeFloat(0.f);
}


void SpacePeriodic::setLengths(const real len[])
{
    length_[0] = len[0];
    length_[1] = len[1];
    length_[2] = len[2];
    update();
}


void SpacePeriodic::read(Inputter& in, Simul&, ObjectTag)
{
    real len[8] = { 0 };
    read_data(in, len, "periodic");
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

bool SpacePeriodic::draw() const
{
    const real X = length_[0];
    const real Y = ( DIM > 1 ) ? length_[1] : 10;
    const real Z = ( DIM > 2 ) ? length_[2] : 0;
    
    glLineStipple(1, 0x000F);
    glEnable(GL_LINE_STIPPLE);

#if ( DIM == 1 )
    glBegin(GL_LINES);
    gleVertex(  X, -Y, 0 );
    gleVertex(  X,  Y, 0 );
    gleVertex( -X,  Y, 0 );
    gleVertex( -X, -Y, 0 );
    glEnd();
#endif
    
#if ( DIM > 1 )
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, Z );
    gleVertex(  X, -Y, Z );
    gleVertex( -X, -Y, Z );
    gleVertex( -X,  Y, Z );
    glEnd();
#endif

#if ( DIM > 2 )
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    glEnd();
#endif

    glDisable(GL_LINE_STIPPLE);
    return true;
}

#else

bool SpacePeriodic::draw() const
{
    return false;
}

#endif

