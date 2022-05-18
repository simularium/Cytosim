// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_TORUS_H
#define SPACE_TORUS_H

#include "space.h"

///a torus of constant diameter centered on the origin
/**
 Space `torus` is defined by two parameters: 
 
 Parameters:
     - `radius` = the main radius of the torus centerline
     - `width`  = the diameter of the torus in its cross sections.
     .

 @ingroup SpaceGroup
 */
class SpaceTorus : public Space
{
private:
    
    /// main radius
    real  bRadius;
    
    /// thickness
    real  bWidth, bWidthSqr;
    
    /// set bWidthSqr
    void update() { bWidthSqr = square(bWidth); }
    
    /// project on the backbone
    Vector backbone(Vector const& pos) const;
    
public:
        
    /// constructor
    SpaceTorus(SpaceProp const*);
        
    /// change dimensions
    void        resize(Glossary& opt);

    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// radius
    real        thickness() const { return bWidth; }

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
