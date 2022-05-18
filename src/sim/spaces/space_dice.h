// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_DICE_H
#define SPACE_DICE_H

#include "space.h"

/// A rectangle ( or a cube ) with rounded edges. 
/**
 Space `dice` is a cube with smooth edges.

 It is build by expanding a cube by a distance `radius` in all directions.
 Mathematically, a point is inside the `dice` if it is at most at distance
 `radius` from the inner cube obtained by subtracting `radius` to the sizes.
 The dice is thus included in the rectangular space of similar size.

 Parameters:
     - length = total extent along X, Y and Z
     - radius = rounding radius of edges
     .

 Note: Dice::setInteraction() relies on project(), and numerical instabilities
 may arise in particular if `radius << size`, because determining the tangent
 plane to a point becomes imprecise.
 
 @ingroup SpaceGroup
 */
class SpaceDice : public Space
{
public:
    
    /// dimensions
    real      length_[3];
    
    /// the radius by which the corners are smoothed
    real      radius_;
    
    /// the square of the radius
    real      radiusSqr_;
    
    /// calculate radiusSqr
    void  update() { radiusSqr_ = square(radius_); }

public:
        
    /// constructor
    SpaceDice(SpaceProp const*);

    /// change dimensions
    void        resize(Glossary& opt);
 
    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(Vector const&) const;
    
    /// set `proj` as the point on the edge that is closest to `point`
    Vector      project(Vector const& pos) const;

    
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
