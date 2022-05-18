// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_SQUARE_H
#define SPACE_SQUARE_H

#include "space.h"

///a rectangular region
/**
 Space `square` is a 2D or 3D rectangular volume.
 
 Parameters:
     - length = extent in X, Y and Z
     .

 @ingroup SpaceGroup
 */
class SpaceSquare : public Space
{
    /// dimensions
    real   length_[3];

private:
    
    /// apply a force directed towards the edge of the Space
    static void setInteraction(const real pos[], Mecapoint const&, Meca &, real stiff, const real dim[]);
    
public:
    
    ///creator
    SpaceSquare(SpaceProp const*);
    
    /// change dimensions
    void        resize(Glossary& opt);

    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(Vector const&) const;

    /// true if a sphere (center, radius) fits in the space, edges included
    bool        allInside(Vector const&, real rad) const;
    
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


