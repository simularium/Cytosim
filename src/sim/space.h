// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_H
#define SPACE_H

#include <string>

#include "sim.h"
#include "real.h"
#include "node.h"
#include "vector.h"
#include "object.h"
#include "common.h"
#include "modulo.h"
#include "space_prop.h"


class Mecapoint;
class Interpolation;
class FiberSet;
class Modulo;
class Simul;
class Meca;

//------------------------------------------------------------------------------

/// Defines the spatial constrains in cytosim
/**
The Space defines a few important functions:\n
 - volume(), which returns the volume contained inside the boudaries,
 - inside(x), which tells if a position `x` is inside the space or not,
 - project(x,p), which calculates `p`, the closest point to `x` on the edge of the space.
 .
The edges are considered to be inside.
*/
class Space : public Object
{
protected:
    
    /// read numbers from file
    static void read_data(Inputter&, real*, std::string const&);

public:
    
    /// parameters
    SpaceProp const* prop;
    
    /// constructor
    Space(SpaceProp const*);
    
    /// destructor
    virtual ~Space();
    
    //------------------------------ BASIC -------------------------------------
    
    /// this is called if any length has been changed
    virtual void resize(Glossary& opt) {};

    /// initialize Modulo if this Space has some periodic dimensions
    virtual Modulo const* getModulo() const { return nullptr; }
    
    /// radius used for piston effect (and defined only for certain shapes)
    virtual real thickness() const { return 0; }

    //------------------------------ OBJECT ------------------------------------
    
    /// the volume inside in 3D, or the surface area in 2D
    virtual real   volume() const { return 1; }

    /// return the bounds for the coordinates of the points inside the Space
    /**
     set inf as [ min(X), min(Y), min(Z) ]
     and sup as [ max(X), max(Y), max(Z) ]
     for any point (X, Y, Z) contained inside the Space.
     
     It thus defines a cuboid aligned with the main axes, and containing the entire volume.
     */
    virtual void   boundaries(Vector& inf, Vector& sup) const { inf.set(-1,-1,-1); sup.set(1,1,1); }
    
    /// true if `point` is inside or on the edge of this Space
    virtual bool   inside(Vector const&) const { return true; }
    
    /// set `proj` as the point on the edge that is closest to `point`
    /*
     If the edge is a smooth surface, this should correspond to the usual orthogonal projection.
     */
    virtual Vector project(Vector const& pos) const { ABORT_NOW("base Space is unbounded"); };
    
    /// apply a force directed towards the edge of this Space, for a point located at `pos`
    virtual void   setInteraction(Vector const& pos, Mecapoint const&, Meca &, real stiff) const;
    
    /// apply a force directed towards the edge of this Space deflated by `radius`
    virtual void   setInteraction(Vector const& pos, Mecapoint const&, real rad, Meca &, real stiff) const;
    
#if ( 0 )
    /// apply a force directed towards the edge of this Space
    void   setInteraction(Vector const&, Interpolation const&, Meca &, real stiff) const;
#endif
    
    /// true if all points of the sphere (`center`, `radius`) are inside this Space
    virtual bool   allInside(Vector const&, real rad) const;
    
    /// true if no point of the sphere (`center`, `radius`) is inside this Space
    virtual bool   allOutside(Vector const&, real rad) const;
    
    //---------------------------- DERIVED -------------------------------------
    
    /// returns the maximum absolute value of any coordinate
    real           max_extension() const;

    /// true if `point` is outside this Space ( defined as !inside(point) )
    bool           outside(Vector const& pos)  const { return ! inside(pos); }
    
    /// project `point` on this Space deflated by `radius`, putting the result in `proj`
    Vector         projectDeflated(Vector const&, real rad) const;
    
    
    /// the square of the distance to the edge of this Space
    real           distanceToEdgeSqr(Vector const&) const;
    
    /// the distance to the edge, always positive
    real           distanceToEdge(Vector const& pos) const { return sqrt(distanceToEdgeSqr(pos)); }
    
    /// the distance to the edge, positive if `point` is outside, and negative if inside
    real           signedDistanceToEdge(Vector const&) const;
    
    /// bring a position back inside, as if it bounced off the walls of the Space
    Vector         bounce(Vector) const;
    
    
    /// a Vector perpendicular to the space edge at `point`, directed towards the outside
    virtual Vector normalToEdge(Vector const& pos) const;
    
    /// a random position inside the volume, uniformly distributed in the volume
    virtual Vector randomPlace() const;

    /// a random position located inside and at most at distance `radius` from the edge
    virtual Vector randomPlaceNearEdge(real rad, unsigned long nb_trials = 10000) const;
    
    /// a random position located on the edge
    Vector         randomPlaceOnEdge(real rad, size_t nb_trials = 10000) const;
    
    /// estimate Volume using a crude Monte-Carlo method with `cnt` calls to Space::inside()
    real           estimateVolume(size_t cnt) const;

    //------------------------------ SIMULATION ---------------------------------
    
    /// one Monte-Carlo simulation step
    virtual void   step() {}
    
    /// add interactions to a Meca
    virtual void   setInteractions(Meca &, FiberSet const&) const {}

    //------------------------------ READ/WRITE --------------------------------
    
    /// a unique character identifying the class
    static const ObjectTag TAG = 'e';
    
    /// return unique character identifying the class
    ObjectTag      tag() const { return TAG; }
    
    /// return associated Property
    Property const* property() const { return prop; }
    
    /// returns the name of the Property
    std::string    name() const { return prop->name(); }

    /// a static_cast<> of Node::next()
    Space*         next() const { return static_cast<Space*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Space*         prev() const { return static_cast<Space*>(nPrev); }
    
    /// write to file
    void           write(Outputter&) const;

    /// read from file
    void           read(Inputter&, Simul&, ObjectTag);
    
    /// get dimensions from array `len`
    virtual void   setLengths(const real len[8]) {}

    //------------------------------ DISPLAY ----------------------------------
    
    /// a shape-specific openGL display function, return true if display was done
    /**
     In 2D, this should draw the edge of the surface using lines.
     in 3D, this should draw the surface of the volume, using triangles.
     */
    virtual bool   draw()  const { return false; }

    /// Default 2D display, tracing the outline of a section of the Volume
    void           drawSection(int dim, real pos, real step) const;

};

#endif

