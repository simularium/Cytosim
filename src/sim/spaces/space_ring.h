// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_RING_H
#define SPACE_RING_H

#include "space.h"

/// a cylinder of axis X, but without caps
/**
 Space `ring` is radial symmetric along the X-axis.
 The cross section in the YZ plane is a disc.
 The end discs are not part of the surface.
 project() will always project on the curvy surface of the cylinder.

 Parameters:
     - length = total extent of the cylinder in X
     - radius = radius of the cylinder
     .

 @ingroup SpaceGroup
 */
class SpaceRing : public Space
{    
    /// apply a force directed towards the edge of the Space
    static void setInteraction(Vector const& pos, Mecapoint const&, Meca &, real stiff, real len, real rad);

private:
    
    /// half the length of the cylinder
    real  length_;
    
    /// the radius of the ring
    real  radius_;
    
    /// the square of the radius
    real  radiusSqr_;
    
    /// calculate radiusSqr
    void  update() { radiusSqr_ = square(radius_); }

public:
        
    ///creator
    SpaceRing(SpaceProp const*);
    
    /// change dimensions
    void        resize(Glossary& opt);

    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(Vector const&) const;
    
    /// true if the bead is inside the Space
    bool        allInside(Vector const&, real rad) const;
    
    /// a random position inside the volume
    Vector      randomPlace() const;

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

