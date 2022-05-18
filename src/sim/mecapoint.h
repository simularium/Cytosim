// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MECAPOINT_H
#define MECAPOINT_H

#include "vector.h"
#include "mecable.h"
#include "matrix.h"

/// Indicates one Vertex of a Mecable
/**
 This class used to be called 'PointExact' before 12.2017 and 'ModelPoint' until 7.2018
 */
class Mecapoint
{    
private:
    
    /// Mecable containing the point-of-interest 
    Mecable const* mec_;
    
    /// Index of the point-of-interest in the Mecable
    unsigned int   pti_;
    
public:
        
    /// Reset the member variables (refers to nothing)
    void clear() { mec_ = nullptr;  pti_ = 0; }
    
    /// Default constructor reset variables
    Mecapoint() : mec_(nullptr), pti_(0) { }

    /// Build to refer to point p in ps
    Mecapoint(const Mecable * m, unsigned p) : mec_(m), pti_(p) { }
    
    /// Set to refer to point p in ps
    void   set(const Mecable * m, unsigned p) { mec_ = m; pti_ = p; }
    
    /// Constant pointer to the Mecable 
    Mecable const* mecable()       const { return mec_; }
    
    /// true if the pointer seems to be valid.
    bool           valid()         const { return mec_ == nullptr || pti_ < mec_->nbPoints(); }
    
    /// Index of point in object
    unsigned int   point()         const { return pti_; }
        
    /// Position of the point-of-interest in space
    Vector         pos()           const { return mec_->posPoint(pti_); }
    
    /// Index of the point-of-interest in the big matrix
    index_t        matIndex()      const { return mec_->matIndex() + pti_; }
    
    /// Write to file
    void           write(Outputter&) const;
    
    /// Read from file
    void           read(Inputter&, Simul&);
    
    /// test if `this` shares one point with the argument
    bool           overlapping(const Mecapoint &) const;

    /// test if `this` is one point away from the argument
    bool           near(const Mecapoint &) const;

    /// Human friendly ouput
    void           print(std::ostream&) const;
};

/// output operator for debugging purpose
std::ostream& operator << (std::ostream&, const Mecapoint&);


#endif
