// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef POLYGON_H
#define POLYGON_H

#include "assert_macro.h"
#include <fstream>
#include <iomanip>

#ifndef REAL_H
    #include "real.h"
#endif

/// Data and functions representing a closed 2D polygon
class Polygon
{
public:

    /// holds the coordinates of a point in 2D + info about the segment
    struct Point2D 
    {
        real xx, yy;   ///< coordinates of point
        real dx, dy;   ///< normalized direction to next point
        long info;     ///< indicates the type of edge
        real len;      ///< distance to next point
        
        /// constructor
        Point2D() { info = 0; }
        
        /// set coordinates of point
        Point2D(real sx, real sy)
        {
            xx = sx;
            yy = sy;
            info = 0;
        }
        
        /// test if given point overlap with *this
        bool operator == (const Point2D& p)
        {
            return ( xx == p.xx  &&  yy == p.yy );
        }
        
        /// print
        void write(std::ostream& os) const
        {
            const int w = (int)os.width();
            os << "Point2D(";
            os << " " << std::fixed << std::setw(w) << xx;
            os << " " << std::fixed << std::setw(w) << yy;
            os << "}";
        }
     };
    
    /// list of points. The array is allocated to hold index = 1+npts_
    Point2D* pts_;
    
    /// number of points
    unsigned npts_;
    
public:
    
    /// constructor
    Polygon();
    
    /// destructor
    ~Polygon();
    
    /// number of points
    unsigned nbPoints() const { return npts_; }
    
    /// set number of points and allocate memory
    void     allocate(unsigned s);
    
    /// set as regular polygon with `ord` sides (4 : square)
    void     set(unsigned ord, real radius, real angle = 0);

    /// return copy of point at index `inx`
    Point2D  point(unsigned inx) { assert_true( inx < npts_ ); return pts_[inx]; }

    /// set coordinates of point at index `inx`:
    void     setPoint(unsigned inx, real x, real y, long c = 0);
    
    /// subfunction
    static unsigned read(std::istream&, Point2D *pts, unsigned pts_size);
    
    /// read polygon from stream
    void     read(std::istream&);
    
    /// read polygon from file
    void     read(std::string const&);
    
    /// write a polygon to stream
    void     write(std::ostream&) const;

    /// flip the order of the points
    void     flip();
    
    /// move all points by given amount
    void     translate(real dx, real dy);
    
    /// scale all points by given factors in X and Y
    void     scale(real sx, real sy);
    
    /// move all segments sideways, to uniformly increase the surface of polygon
    void     inflate(real eps);
    
    /// calculate the offsets necessary for the other functions. Return 0 if OK
    int      complete(real epsilon);
    
    /// tell if a point is inside a polygon
    int      inside(real x, real y, int edge, real threshold = REAL_EPSILON) const;
    
    /// calculate the projection (pX, pY) of the point (x,y) on a polygon
    int      project(real x, real y, real& pX, real& pY, int& hit) const;
    
    /// calculate the bounding box [xmin, xmax, ymin, ymax] of a polygon
    void     find_extremes(real box[4]) const;
    
    /// calculate the surface of a polygon
    real     surface() const;
    
    /// printout
    void     dump(std::ostream&) const;
    
    /// printout
    void     print(FILE*) const;
};

#endif

