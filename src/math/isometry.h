// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ISOMETRY_H
#define ISOMETRY_H

#include "dim.h"
#include "vector.h"

#if ( DIM == 1 )

   #include "matrix11.h"
   typedef Matrix11 MatrixD;

#elif ( DIM == 2 )

   #include "matrix22.h"
   typedef Matrix22 MatrixD;

#elif ( DIM == 3 )

   #include "matrix33.h"
   typedef Matrix33 MatrixD;

#endif


/// A Rotation is a matrix of dimension DIM x DIM
typedef MatrixD Rotation;


/// An affine transformation in space.
/**
 A Isometry contains a vector T and a rotation matrix M,
 and represents the affine transformation:
 
     X -> M.X + T
 
 */
class Isometry
{
public:
    
    /// rotation component
    MatrixD rot;
    
    /// translation component
    Vector  mov;

public:
    
    Isometry()
    {
        mov.reset();
        rot = MatrixD::identity();
    }

    Isometry(Vector const& v)
    {
        mov = v;
        rot = MatrixD::identity();
    }

    Isometry(Vector const& v, MatrixD const& r)
    {
        mov = v;
        rot = r;
    }

    void reset()
    {
        mov.reset();
        rot = MatrixD::identity();
    }

    /// allow automatic conversion to a Vector
    operator Vector const& () const
    {
        return mov;
    }
    
    /// allow automatic conversion to a Rotation matrix
    operator MatrixD const& () const
    {
        return rot;
    }
    
    /// apply translation, after *this
    void translate(Vector const& v)
    {
        mov += v;
    }
    
    /// apply rotation, after *this
    void rotate(MatrixD const& mat)
    {
        rot = mat * rot;
        mov = mat * mov;
    }

    /// apply another isometry, after *this
    void combine(Isometry const& iso)
    {
        mov = iso.rot * mov + iso.mov;
        rot = iso.rot * rot;
    }
};


/// output operator
inline std::ostream& operator << (std::ostream& os, Isometry const& iso)
{
    os << "Isometry { " << iso.mov << " | " << iso.rot << " }";
    return os;
}


#endif
