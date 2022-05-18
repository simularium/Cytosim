// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef SPACE_CYLINDERP_H
#define SPACE_CYLINDERP_H

#include "space.h"
#include "modulo.h"

/// a periodic cylinder aligned with X
/**
 Space `cylinderP` is a cylinder with periodic boundary conditions
 along the X-axis. It has no ends and loops on itself like a torus,
 but without the curvature.

 Parameters:
     - length = length of the cylinder in X
     - radius = radius of the cylinder
     .

 To display a periodic Space, use simul:display parameter 'tile'.
 @ingroup SpaceGroup
 */
class SpaceCylinderP : public Space
{
private:
    
    /// half the length of the central cylinder
    real  length_;
    
    /// the radius of the cylinder
    real  radius_;

    /// Object to handle periodic boundary conditions
    Modulo modulo_;
    
public:
        
    ///creator
    SpaceCylinderP(SpaceProp const*);

    /// change dimensions
    void        resize(Glossary& opt);
 
    /// return Modulo Object
    Modulo const* getModulo() const { return &modulo_; }
    
    /// match sizes of Modulo object
    void        update();

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
    
    /// direct normal direction calculation
    Vector      normalToEdge(Vector const&) const;
    
    /// direct surface placement
    Vector      randomPlaceOnEdge(real) const;

    /// set `proj` as the point on the edge that is closest to `point`
    Vector      project(Vector const& pos) const;

    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, Mecapoint const&, Meca&, real stiff) const;
    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, Mecapoint const&, real rad, Meca&, real stiff) const;

    
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

