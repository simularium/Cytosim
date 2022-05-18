// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_BANANA_H
#define SPACE_BANANA_H

#include "space.h"

/// a bent cylinder of constant diameter terminated by hemispheric caps
/**
 Space `banana` is comprised from a section of a torus,
 terminated by two hemispheres.
 
 Parameters:
     - `length` = the overall length
     - `width` = the diameter of the torus in its cross sections
     - `curvature` = the main radius of the torus centerline
     .
 
 This class was first conceived by Dietrich Foethke, to simulate S. pombe.
 
 */
class SpaceBanana : public Space
{
private:
    
    /// dimensions
    real  bRadius;
    real  bLength;
    real  bWidth, bWidthSqr;

    /// angle covered by torus section
    real bAngle;
    
    /// X and Y coordinates of the right end
    real bEnd[2];

    /// coordinates of the center of the torus
    Vector bCenter;
    
    void update();
    
    /// project on the backbone circle
    Vector backbone(Vector const& pos) const;

public:
        
    /// constructor
    SpaceBanana(SpaceProp const*);
    
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
