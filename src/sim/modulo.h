// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University.

#ifndef MODULO_H
#define MODULO_H

#include "real.h"
#include "vector.h"

/// Modulo is a helper class used to implement periodic boundary conditions
/**
 This class is used to apply periodic boundaries conditions to Vectors in one or
 in multiple dimensions in space: X, Y or Z.
 
 We follow the method B (Do not restrict the particle coordinates) described in
 [wikipedia](https://en.wikipedia.org/wiki/Periodic_boundary_conditions)

 In this way, we can tell the movements of the filaments, and track all paths
 without any loss of information. Attention: our origin is in the box center,
 unlike on Wikipedia’s description.
 */
class Modulo
{
private:
    
    /// the period in each dimension
    real  mSize[4];
    
    /// bitfield indicating the dimensions that are periodic
    int   mMode;

public:
    
    /// set as non periodic
    void reset() { mMode = 0; for (int d=0; d<4; ++d) mSize[d] = 0; }
    
    /// constructor
    Modulo() { reset(); }

    /// destructor
    ~Modulo() {}
    
    /// disable periodicity in all dimensions
    void disable() { mMode = 0; }
    
    /// enable periodicity in dimension 'd'
    void enable(size_t d, real size);
    
    /// true if at least one direction has periodic boundaries
    bool isPeriodic() const { return mMode; }

    /// true if direction `d` has periodic boundaries
    bool isPeriodic(size_t d) const { return mMode & (1<<d); }
    
    /// return the d-th direction of periodicity
    Vector period(size_t d) const;
    
    /// shift `pos` to its canonical image, which is the one closest to the origin
    void   fold(Vector& pos) const;
    
    /// shift `pos` to its image which is closest to `ref`
    void   fold(Vector& pos, Vector const& ref) const;
    
    /// return translation necessary to bring `pos` to its canonical image
    Vector offset(Vector const& pos) const;
    
    /// set `pos` to its canonical image, and return offset = pos - fold(pos)
    void   foldOffset(Vector& pos, Vector& off) const;
    
};

#endif


