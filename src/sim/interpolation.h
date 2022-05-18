// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "real.h"
#include "vector.h"
#include "matrix.h"
#include "mecable.h"
#include "mecapoint.h"

class FiberSegment;

/// Indicates an intermediate position between two points of a Mecable
/**
 This class defines an interpolation between two points on the same Mecable.
 The Mecable is designated by a pointer, and two vertices by their indices.
 An interpolation coefficient in [0,1] then defines an intermediate position
 between the two vertices:
 
     pos = (1-coef) * pt1_ + coef * pt2_
 
 This class used to be called 'PointInterpolated' before 12.2017
 */
class Interpolation
{
private:
    
    /// Mecable from which the points are interpolated 
    Mecable const*  mec_;

    /// index of interpolated point 1 in mec_
    unsigned int    pt1_;

    /// index of interpolated point 2 in mec_
    unsigned int    pt2_;
    
    /// interpolation coefficient: pos = (1-coef) * pt1_ + coef * pt2_
    real           coef_;
    
public:

    Interpolation(Interpolation const&) { }
    
    /// reset member variables
    Interpolation() : mec_(nullptr), pt1_(0), pt2_(0), coef_(0) { }
    
    /// set to interpolate p1 and p2 on ps, with coefficient c
    Interpolation(const Mecable * m, unsigned p, unsigned q, real c)
    : mec_(m), pt1_(p), pt2_(q), coef_(c) { }

    /// set to interpolate given fiber segment, with abscissa `c` 
    Interpolation(FiberSegment const&, real abs);
    
    
    /// Reset member variables (refers to nothing)
    void clear()
    {
        mec_ = nullptr;  pt1_ = 0;  pt2_ = 0;  coef_ = 0;
    }
    
    /// Set to interpolate p1 and p2 on ps, with coefficient c, on the same Mecable
    void set(unsigned p, unsigned q, const real c)
    {
        pt1_ = p;  pt2_ = q;  coef_ = c;
    }
    
    /// Index of point 1 in the matrix of dynamics (Meca)
    index_t         matIndex1() const { return mec_->matIndex() + pt1_; }
    
    /// Index of point 2 in the matrix of dynamics (Meca)
    index_t         matIndex2() const { return mec_->matIndex() + pt2_; }
    
    /// true if the pointer seems to be valid.
    bool            valid()    const { return mec_ == nullptr || ( pt1_ < mec_->nbPoints() && pt2_ < mec_->nbPoints() ); }

    /// Constant pointer to the Mecable
    Mecable const*  mecable()  const { return mec_; }

    /// Mecapoint corresponding to first point
    Mecapoint       exact1()   const { return Mecapoint(mec_, pt1_); }
    
    /// Mecapoint corresponding to second point
    Mecapoint       exact2()   const { return Mecapoint(mec_, pt2_); }
    
    /// Index of point 1 in object
    unsigned int    point1()   const { return pt1_; }
  
    /// Index of point 2 in object
    unsigned int    point2()   const { return pt2_; }

    /// interpolation coefficient on second point
    real            coef1()    const { return coef_; }

    /// interpolation coefficient on first point
    real            coef2()    const { return 1.0-coef_; }

    /// Set interpolation coefficient
    void            coef(real c)     { coef_ = c; }
    
    /// Interpolated position in space
    Vector          pos()      const { return mec_->interpolatePoints(pt1_, pt2_, coef_); }
    
    /// position of first point
    Vector          pos1()     const { return mec_->posP(pt1_); }
    
    /// position of second point 
    Vector          pos2()     const { return mec_->posP(pt2_); }
    
    /// that is pos2() - pos1()
    Vector          diff()     const { return mec_->diffPoints(pt1_, pt2_); }
    
    /// distance between point1 and point2
    real            len()      const { return diff().norm(); }

    /// squared distance between point1 and point2
    real            lenSqr()   const { return diff().normSqr(); }

    /// normalize(pos2() - pos1())
    Vector          dir()      const { return normalize(diff()); }

    /// true if the coefficient is in [0, 1]
    bool            inside()   const { return ( 0 <= coef_ ) && ( coef_ <= 1.0 ); }

    /// test if `this` has a common point with argument
    bool            overlapping(const Mecapoint &) const;
    
    /// test if `this` has a common point with argument
    bool            overlapping(const Interpolation &) const;

    /// Human friendly ouput
    void            print(std::ostream&) const;
};

/// output operator for debugging purpose
std::ostream& operator << (std::ostream&, Interpolation const&);

#endif
