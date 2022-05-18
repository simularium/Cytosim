// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "polygon.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include <string>
#include <cmath>
#include "exceptions.h"


Polygon::Polygon()
: pts_(nullptr), npts_(0)
{
}


Polygon::~Polygon()
{
    delete[] pts_;
}


void Polygon::allocate(unsigned s)
{
    delete[] pts_;
    pts_  = new Point2D[s+1];
    npts_ = s;
}


void Polygon::set(unsigned ord, real rad, real ang)
{
    allocate(ord);
    real a = 2 * M_PI / ord;
    for ( unsigned i = 0; i < ord; ++i )
    {
        pts_[i].xx = rad * cos(i*a+ang);
        pts_[i].yy = rad * sin(i*a+ang);
    }
    pts_[ord] = pts_[0];
}


void Polygon::setPoint(unsigned i, real x, real y, long c)
{
    if ( pts_ && i < npts_ )
    {
        pts_[i].xx = x;
        pts_[i].yy = y;
        pts_[i].info = c;
    }
}


/**
 Each point should be on its own line: X Y
 This will set coordinates in pts_[] within the limit given by `alc`.
 but it will return the number of points in the stream, even if this is greater than `alc`.
 Thus two calls to this function should be enough to:
 - determine the number of points in the file
 - allocate an appropriate array
 - read the coordinates
 */
unsigned Polygon::read(std::istream& in, Point2D* pts, unsigned pts_size)
{
    unsigned i = 0;
    char str[2048];
    real x, y;
    long k;

    while ( in.good() )
    {
        in.getline(str, sizeof(str));
        //std::clog << "polygone file " << in.gcount() << " " << str << "\n";

        if ( in.fail() && in.gcount() )
            throw InvalidIO("Could not read polygon coordinate files");
       
        char const* ptr = str;
        char * end = nullptr;
        
        x = strtod(ptr, &end);
        
        if ( end == ptr )
            continue;

        ptr = end;
        y = strtod(ptr, &end);
        
        if ( end == ptr )
        {
            y = 0;
            k = 0;
        }
        else
        {
            ptr = end;
            errno = 0;
            k = strtol(ptr, &end, 10);
            if ( errno || end == ptr )
                k = 0;
        }
        
        if ( i < pts_size )
        {
            pts[i].xx = x;
            pts[i].yy = y;
            pts[i].info = k;
        }
        ++i;
    }
    return i;
}


void Polygon::read(std::istream& in)
{
    unsigned n = read(in, nullptr, 0);
    allocate(n);
    in.clear();
    in.seekg(0);
    read(in, pts_, npts_);
}


void Polygon::read(std::string const& file)
{
    if ( file.empty() )
        throw InvalidParameter("a polygon file should be specified");
    
    std::ifstream in(file.c_str(), std::ifstream::in);
    
    if ( ! in.good() )
        throw InvalidParameter("polygon file `"+file+"' not found");
    
    //std::clog << "reading polygon from " << file << std::endl;
    
    read(in);
    
    if ( nbPoints() < 3 )
        throw InvalidParameter("polygon: too few points specified in `"+file+"'");
    
    in.close();
}


void Polygon::write(std::ostream& os) const
{
    os.precision(6);
    os << std::fixed;
    for ( unsigned i = 1; i < npts_; ++i )
    {
        os << std::setw(12) << pts_[i].xx << "  " << std::setw(12) << pts_[i].yy;
        if ( pts_[i].info )
            os << " " << pts_[i].info;
        std::endl(os);
    }
}


/**
 This makes a clockwise Polygon counter-clockwise and vice-versa
 */
void Polygon::flip()
{
    unsigned n = 1;
    unsigned p = npts_-1;
    
    while ( n < p )
    {
        Point2D X = pts_[p];
        pts_[p] = pts_[n];
        pts_[n] = X;
        ++n;
        --p;
    }
}


void Polygon::translate(real dx, real dy)
{
    for ( unsigned n = 0; n <= npts_; ++n )
    {
        pts_[n].xx += dx;
        pts_[n].yy += dy;
    }
}


void Polygon::scale(real sx, real sy)
{
    for ( unsigned n = 0; n <= npts_; ++n )
    {
        pts_[n].xx *= sx;
        pts_[n].yy *= sy;
    }
}


/**
 This will increase the area if the Polygon is counterclockwise
 */
void Polygon::inflate(real eps)
{
    complete(REAL_EPSILON);
    
    if ( npts_ < 3 )
        return;
    
    // tangent 'T' to previous segment
    real tx = pts_[npts_-1].dx;
    real ty = pts_[npts_-1].dy;
    
    // previous point shifted out by 'eps'
    real px = pts_[npts_-1].xx + eps * ty;
    real py = pts_[npts_-1].yy - eps * tx;

    for ( unsigned n = 0; n < npts_; ++n )
    {
        // normal 'N' to current segment
        real nx =  pts_[n].dy;
        real ny = -pts_[n].dx;

        /*
         Calculate interestion of the shifted lines
         First line is parametric:     X = P + a * T
         Second line is defined by:  ( X - Q ) . N = eps
         where P = previous point, Q = current point
         */
        real s = tx * nx + ty * ny;
        
        if ( fabs(s) < REAL_EPSILON )
        {
            px = pts_[n].xx + eps * nx;
            py = pts_[n].yy + eps * ny;
        }
        else
        {
            real a = eps + ( pts_[n].xx - px ) * nx + ( pts_[n].yy - py ) * ny;
            px += tx * a / s;
            py += ty * a / s;
        }
        //std::clog << " n " << n << "  " << px << "  " << py << "\n";
        
        tx = pts_[n].dx;
        ty = pts_[n].dy;
    
        pts_[n].xx = px;
        pts_[n].yy = py;
    }
    
    if ( npts_ > 1 )
    {
        pts_[npts_].xx = pts_[0].xx;
        pts_[npts_].yy = pts_[0].yy;
    }
}


/**
 box[] = { xmin, xmax, ymin, ymax }
 
 result is undefined if ( npts == 0 ).
 */
void Polygon::find_extremes(real box[4]) const
{
    if ( npts_ > 0 )
    {
        box[0] = pts_[0].xx;
        box[1] = pts_[0].xx;
        box[2] = pts_[0].yy;
        box[3] = pts_[0].yy;
    }
    
    for ( unsigned i = 1; i < npts_; ++i )
    {
        if ( pts_[i].xx < box[0] )  box[0] = pts_[i].xx;
        if ( pts_[i].xx > box[1] )  box[1] = pts_[i].xx;
        if ( pts_[i].yy < box[2] )  box[2] = pts_[i].yy;
        if ( pts_[i].yy > box[3] )  box[3] = pts_[i].yy;
    }
}


/**
 pre-calculate offset of successive points,
 and length of segments, used in project for efficiency.
 Also copy two points to simplify the calculations:
 - point[nbpts  ] <- point[0]
 - point[nbpts+1] <- point[1]
 .
 
 The array should be allocated to hold (npts+2) Point2D
 */
int Polygon::complete(real epsilon)
{
    int res = 0;
    if ( npts_ > 1 )
    {
        pts_[npts_] = pts_[0];
   
        unsigned i = 0, n = 0, p = 0;
        do {
            real dx = 0, dy = 0, d = 1;
            // skip consecutive points that are too close from each other:
            do {
                if ( ++p > npts_ )
                    goto finish;
                dx = pts_[p].xx - pts_[n].xx;
                dy = pts_[p].yy - pts_[n].yy;
                d = sqrt( dx * dx + dy * dy );
            } while ( d < epsilon );
            //normalize the vector:
            pts_[i]     = pts_[n];
            pts_[i].dx  = dx / d;
            pts_[i].dy  = dy / d;
            pts_[i].len = d;
            n = p;
            ++i;
        } while ( n <= npts_ );
    
finish:
        if ( i < npts_ )
        {
            std::clog << "Polygon had " << npts_-i << " degenerate vertices\n";
            npts_ = i;
            res = 1;
        }
        
        if ( npts_ > 1 )
        {
            pts_[npts_] = pts_[0];
            pts_[npts_].dx = pts_[0].dx;
            pts_[npts_].dy = pts_[0].dy;
        }
    }

    return res;
}

    
/**
 calculate volume of polygon, using an algorithm that return a negative value
 for a polygon defined clockwise and a positive value for anti-clockwise.
 http://mathworld.wolfram.com/PolygonArea.html
 */
real Polygon::surface() const
{
    if ( npts_ < 3 )
        return 0;
    
    real S = pts_[npts_-1].xx * ( pts_[0].yy - pts_[npts_-2].yy );
    for ( unsigned ii = 2; ii < npts_; ++ii )
        S += pts_[ii-1].xx * ( pts_[ii].yy - pts_[ii-2].yy );
    
    return S * 0.5;
}


/**
 Count the number of time a ray from (xx, yy) to (infinity, yy) crosses the polygon
 The point is inside if the result is odd. 
 This works for both clockwise and anticlockwise polygons.
 
 @return
 0    : point is ouside
 1    : point is inside
 edge : point is near the boundary, within distance `threshold`
 .
*/
int Polygon::inside(real xx, real yy, int edge, real threshold) const
{
    int cross = 0;
    
    Point2D p1, p2 = pts_[0];
    
    //check all edges of polygon
    for ( unsigned ii = 1; ii <= npts_; ++ii )
    {
        p1 = p2;
        p2 = pts_[ii];

        // check if edge cannot interesect with ray
        if (( yy <= p1.yy && yy < p2.yy ) || ( yy >= p1.yy && yy > p2.yy ))
            continue;
        
        // ray may go through p2
        if ( yy == p2.yy )
        {
            // check for horizontal edge
            if ( p1.yy == p2.yy )
            {
                if ( xx > p1.xx && xx > p2.xx )
                    continue;
                if ( xx < p1.xx && xx < p2.xx )
                    continue;
                return edge;
            }
            
            if ( p2.xx < xx )
                continue;
            
            if ( xx == p2.xx )
                return edge;
            
            // next vertex
            const Point2D& p3 = pts_[ii+1];
         
            // check that p2 is not a corner
            if (( p1.yy < yy && yy < p3.yy ) || ( p3.yy < yy && yy < p1.yy ))
                ++cross;
            
            continue;
        }
        
        // xx is left of edge
        if ( xx <= p1.xx || xx <= p2.xx )
        {
            // intersection of ray with edge
            real xi = ( yy - p1.yy ) * ( p2.xx - p1.xx ) / ( p2.yy - p1.yy ) + p1.xx;
            
            // overlies on an edge
            if ( fabs( xx - xi ) < threshold )
                return edge;
                
            // xx left of intersection
            if ( xx < xi )
                ++cross;
        }
    }
    
    //std::clog << " polygon::inside " << cross << " for " << xx << " " << yy << std::endl;
    return ( cross & 1 );
}

/**
 Find the closest point on the polygon to (x, y)
 
 @return
 0 : projects on an edge
 1 : projects on a vertex
 .
 The function will also set `hit` to be the index of the point,
 or the segment where the projection landed

 */
int Polygon::project(real xx, real yy, real& pX, real& pY, int& hit) const
{
    if ( npts_ < 1 )
        throw InvalidParameter("Cannot project on uninitialized polygon");
    int res = 0;
    
    //initialize with first point:
    hit = 0;
    pX = pts_[0].xx;
    pY = pts_[0].yy;
    
    real dis = ( xx - pX ) * ( xx - pX ) + ( yy - pY ) * ( yy - pY );
    
    for ( unsigned ii = 0; ii < npts_; ++ii )
    {
        real x = xx - pts_[ii].xx;
        real y = yy - pts_[ii].yy;
        // distance to polygon point:
        real d = x * x + y * y;
        // abscissa of projection on segment [ii, ii+1] of the polygon:
        real a = pts_[ii].dx * x + pts_[ii].dy * y;
        
        if ( a > 0 )
        {
            if ( a < pts_[ii].len )
            {
                // distance from segment to point:
                real da = d - a * a;
                
                if ( da < dis )
                {
                    dis = da;
                    pX  = pts_[ii].xx + a * pts_[ii].dx;
                    pY  = pts_[ii].yy + a * pts_[ii].dy;
                    hit = ii;
                    res = 1;
                }
            }
        }
        else
        {
            if ( d < dis )
            {
                dis = d;
                pX  = pts_[ii].xx;
                pY  = pts_[ii].yy;
                hit = ii;
                res = 0;
            }
        }
    }
    
    return res;
}


void Polygon::dump(std::ostream& os) const
{
    os << "polygon " << npts_ << "\n";
    for ( unsigned n = 0; n <= npts_; ++n )
    {
        os << " " << std::setw(10) << pts_[n].xx << " " << std::setw(10) << pts_[n].yy << " " << pts_[n].info;
        os << " " << std::setw(10) << pts_[n].dx << " " << std::setw(10) << pts_[n].dy << "\n";
    }
}


void Polygon::print(FILE * f) const
{
    fprintf(f, "polygon %i\n", npts_);
    for ( unsigned n = 0; n <= npts_; ++n )
    {
        fprintf(f, "%10.2f %10.2f %4li", pts_[n].xx, pts_[n].yy, pts_[n].info);
        fprintf(f, "  %10.2f %10.2f\n", pts_[n].dx, pts_[n].dy);
    }
}

