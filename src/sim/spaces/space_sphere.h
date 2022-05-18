// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_SPHERE_H
#define SPACE_SPHERE_H

#include "space.h"

/// sphere centered at the origin.
/**
 Space `sphere` is a sphere centered around the origin
 
 Parameters:
    - radius = radius of the sphere
    .
 
 @ingroup SpaceGroup
 */

class SpaceSphere : public Space
{
protected:
    
    /// the radius of the sphere
    real  radius_;
    
    /// square of the radius
    real  radiusSqr_;
    
    /// calculate radiusSqr
    void  update() { radiusSqr_ = square(radius_); }
    
public:
    
    /// constructor
    SpaceSphere(SpaceProp const*);

    /// check number and validity of specified lengths
    void        resize(Glossary& opt);

    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// radius
    real        thickness() const { return radius_; }

    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(Vector const&) const;
    
    /// a random position inside the volume
    Vector      randomPlace() const { return Vector::randB(radius_); }
    
    /// direct normal direction calculation
    Vector      normalToEdge(Vector const& pos) const { return normalize(pos); }

    /// set `proj` as the point on the edge that is closest to `point`
    Vector      project(Vector const& pos) const;
    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, Mecapoint const&, Meca &, real stiff) const;

    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, Mecapoint const&, real rad, Meca &, real stiff) const;
    
    /// OpenGL display function; returns true if successful
    bool        draw() const;
    
    /// write to file
    void        write(Outputter&) const;

    /// get dimensions from array `len`
    void        setLengths(const real len[8]);
    
    /// read from file
    void        read(Inputter&, Simul&, ObjectTag);

};

#endif

