// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_ELLIPSE_H
#define SPACE_ELLIPSE_H

#include "dim.h"
#include "space.h"


//#define HAS_SPHEROID

/// ellipse in 2D, ellipsoid or spheroid in 3D 
/**
 The ellipse/ellipsoid is aligned with the principal axes X, Y and Z.
 
 Parameters:
     - length = total length in X, Y and Z directions
     .
 
 The projection of one point on the surface of the ellipse is done numerically. 
 In 3D this is the only solution if the 3 axes have different length.
 setInteraction() relies on project() and thus uses the tangent plane at the
 projection point to approximate the confinement force.

 @ingroup SpaceGroup
 */

class SpaceEllipse : public Space
{
protected:

    /// dimensions
    real length_[3];
  
    /// dimensions squared
    real lengthSqr_[3];

#ifdef HAS_SPHEROID
    /// indicates that two axes are equal
    int mSpheroid;
#endif
    
    void update();
    
public:
        
    /// creator
    SpaceEllipse(SpaceProp const*);
        
    /// change dimensions
    void        resize(Glossary& opt);
 
    /// return bounding box in `inf` and `sup`
    void        boundaries(Vector& inf, Vector& sup) const;
    
    /// direct normal direction calculation
    Vector      normalToEdge(Vector const& pos) const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(Vector const&) const;
    
    /// set `proj` as the point on the edge that is closest to `point`
    Vector1     project1D(Vector1 const&) const;
    
    /// set `proj` as the point on the edge that is closest to `point`
    Vector2     project2D(Vector2 const&) const;
    
    /// set `proj` as the point on the edge that is closest to `point`
    Vector3     project3D(Vector3 const&) const;
    
    /// set `proj` as the point on the edge that is closest to `point`
    Vector      project(Vector const& pos) const
    {
#if ( DIM == 1 )
        return project1D(pos);
#elif ( DIM == 2 )
        return project2D(pos);
#else
        return project3D(pos);
#endif
    }
    
    
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

