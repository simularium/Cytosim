// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_CYLINDERZ_H
#define SPACE_CYLINDERZ_H

#include "space.h"

///a cylinder of axis Z
/**
 Space `cylinderZ` is radial symmetric along the Z axis.
 The cross section in the XY plane is a disc.

 Parameters:
     - radius = radius of cylinder
     - bottom = smallest Z
     - top = highest Z
     .

 @ingroup SpaceGroup
 */
class SpaceCylinderZ : public Space
{    
    /// apply a force directed towards the edge of the Space
    static void setInteraction(Vector const& pos, Mecapoint const&, Meca &, real stiff, real, real, real);

private:
    
    /// the radius of the cylinder
    real        radius_;
    
    /// position in Z of the bottom limit
    real        bot_;
    
    /// position in Z of the top limit
    real        top_;
    
public:
        
    ///creator
    SpaceCylinderZ(SpaceProp const*);

    /// change dimensions
    void        resize(Glossary& opt);
 
    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// radius
    real        thickness() const { return radius_; }

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

