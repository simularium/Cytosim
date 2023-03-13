// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "space_polygon.h"
#include "exceptions.h"
#include "mecapoint.h"
#include "glossary.h"
#include "polygon.h"
#include "meca.h"
#include <fstream>


SpacePolygon::SpacePolygon(SpaceProp const* p)
: Space(p)
{
    inf_.reset();
    sup_.reset();
    surface_ = 0;
    height_ = 0;

    if ( DIM == 1 )
        throw InvalidParameter("polygon is not usable in 1D");
}


SpacePolygon::~SpacePolygon()
{
}


/**
 recalculate bounding box, volume
 and points offsets that are used to project
 */
void SpacePolygon::resize(Glossary& opt)
{
    int ord = 6;
    std::string file;
    
    if ( opt.set(file, "file") )
        poly_.read(file);
    else if ( opt.has_key("points") )
    {
        // specify vertices directly:
        unsigned nbp = opt.nb_values("points");
        poly_.allocate(nbp);
        for ( unsigned p = 0; p < nbp; ++p )
        {
            Vector2 vec(0,0);
            if ( ! opt.set(vec, "points", p) )
                throw InvalidParameter("polygon:points must be a list of comma-separated points: X Y, X Y, X Y, etc.");
            poly_.setPoint(p, vec.XX, vec.YY);
        }
    }
    else if ( opt.set(ord, "order") )
    {
        real rad = 1, ang = 0;
        opt.set(rad, "radius");
        opt.set(ang, "angle");
        poly_.set(ord, rad, ang);
    }
    else
        return;
    
    if ( poly_.surface() < 0 )
    {
        //std::clog << "flipping clockwise polygon `" << file << "'" << std::endl;
        poly_.flip();
    }

    real x;
    if ( opt.set(x, "scale") )
        poly_.scale(x, x);

    if ( opt.set(x, "inflate") )
        poly_.inflate(x);
    
#if ( DIM == 3 )
    x = height_;
    if ( opt.set(x, "height") )
        x *= 0.5;
    if ( x < 0 )
        throw InvalidParameter("polygon:height must be >= 0");
    height_ = x;
#endif

    update();
}


void SpacePolygon::update()
{
    surface_ = poly_.surface();
    assert_true( surface_ > 0 );
    
    if ( poly_.complete(REAL_EPSILON) )
        throw InvalidParameter("unfit polygon: consecutive points may overlap");

    real box[4];
    poly_.find_extremes(box);
    inf_.set(box[0], box[2], 0);
    sup_.set(box[1], box[3], 0);
}


bool SpacePolygon::inside(Vector const& w) const
{
#if ( DIM > 2 )
    if ( fabs(w.ZZ) > height_ )
        return false;
#endif
#if ( DIM > 1 )
    return poly_.inside(w.XX, w.YY, 1);
#else
    return false;
#endif
}


Vector SpacePolygon::randomPlace() const
{
    if ( surface_ <= 0 )
        throw InvalidParameter("cannot pick point inside polygon of null surface");
    return Space::randomPlace();
}


Vector SpacePolygon::project(Vector const& w) const
{
    Vector p;
#if ( DIM == 1 )
    
    p.XX = w.XX;
    
#elif ( DIM == 2 )
    
    int hit;
    poly_.project(w.XX, w.YY, p.XX, p.YY, hit);
    
#elif ( DIM > 2 )
    
    if ( fabs(w.ZZ) > height_ )
    {
        if ( poly_.inside(w.XX, w.YY, 1) )
        {
            // too high or too low in the Z axis, but inside XY
            p.XX = w.XX;
            p.YY = w.YY;
        }
        else
        {
            // outside in Z and XY
            int hit;
            poly_.project(w.XX, w.YY, p.XX, p.YY, hit);
        }
        p.ZZ = std::copysign(height_, w.ZZ);
    }
    else
    {
        int hit;
        poly_.project(w.XX, w.YY, p.XX, p.YY, hit);
        if ( poly_.inside(w.XX, w.YY, 1) )
        {
            // inside in the Z axis and the XY polygon:
            // to the polygonal edge in XY plane:
            real hh = (w.XX-p.XX)*(w.XX-p.XX) + (w.YY-p.YY)*(w.YY-p.YY);
            // to the top/bottom plates:
            real v = height_ - fabs(w.ZZ);
            // compare distances
            if ( v * v < hh )
                return Vector(w.XX, w.YY, std::copysign(height_, w.ZZ));
        }
        p.ZZ = w.ZZ;
    }
    
#endif
    return p;
}


/**
 The current procedure tests the vertices of fibers against the segments of the polygon.
 This fails for non-convext polygon since the re-entrant corners can intersect the fibers.
 
 @todo Also project re-entrant polygon corners on the segments of the Fiber.
 */
void SpacePolygon::setInteraction(Vector const& pos, Mecapoint const& pe, Meca & meca, real stiff) const
{
#if ( DIM > 1 )
    index_t inx = DIM * pe.matIndex();
    
    int hit;
    real pX, pY;
    int edg = poly_.project(pos.XX, pos.YY, pX, pY, hit);
    real nX = -poly_.pts_[hit].dy;
    real nY =  poly_.pts_[hit].dx;
    
#if ( DIM > 2 )
    bool in = poly_.inside(pos.XX, pos.YY, 1);

    if ( fabs(pos.ZZ) >= height_ )
    {
        meca.mC(inx+2, inx+2) -= stiff;
        meca.base(inx+2)      += stiff * std::copysign(height_, pos.ZZ);
        if ( in ) return;
    }
    else if ( in )
    {
        // Compare distance to top/bottom plate:
        real v = height_ - fabs(pos.ZZ);
        // and distance to polygonal edge in XY plane:
        real hh = (pos.XX-pX)*(pos.XX-pX) + (pos.YY-pY)*(pos.YY-pY);
        
        if ( v * v < hh )
        {
            meca.mC(inx+2, inx+2) -= stiff;
            meca.base(inx+2)      += stiff * std::copysign(height_, pos.ZZ);
            return;
        }
    }
#endif

    if ( edg )
    {
        // projection on an edge of normal (nX, nY) already normalized
        const real pr = ( pX * nX + pY * nY ) * stiff;
        
        meca.mC(inx  , inx  ) -= nX * nX * stiff;
        meca.mC(inx  , inx+1) -= nX * nY * stiff;
        meca.mC(inx+1, inx+1) -= nY * nY * stiff;
        
        meca.base(inx  )  += nX * pr;
        meca.base(inx+1)  += nY * pr;
    }
    else
    {
        // projection on a vertex:
#if ( DIM == 2 )
        meca.mB(pe.matIndex(), pe.matIndex()) -= stiff;
#elif ( DIM > 2 )
        meca.mC(inx,   inx  ) -= stiff;
        meca.mC(inx+1, inx+1) -= stiff;
#endif
        meca.base(inx  )  += stiff * pX;
        meca.base(inx+1)  += stiff * pY;
    }
#endif
}


void SpacePolygon::setInteraction(Vector const& pos, Mecapoint const& pe, real rad, Meca & meca, real stiff) const
{
    //setInteraction(pos, pe, meca, stiff);
    std::cerr << "unfinished SpacePolygon::setInteraction(with radius)\n";
}

#include "fiber_segment.h"
#include "fiber_set.h"

void SpacePolygon::setInteractions(Meca & meca, FiberSet const& fibers) const
{
#if ( 0 )
    /// WORK IN PROGRESS
    Polygon::Point2D const* pts = poly_.pts_;
    const int n_pik = 2;
    const int inx[n_pik] = { 0, 100 };
    Vector pik[n_pik];
    
    for ( int i = 0; i < n_pik; ++i )
        pik[i].set(pts[inx[i]].xx, pts[inx[i]].yy, 0);
    
    for ( Fiber * fib=fibers.first(); fib; fib=fib->next() )
    {
        real ls = fib->segmentation();
        for ( unsigned seg = 0; seg < fib->nbSegments() ; ++seg )
        {
            FiberSegment loc(fib, seg);
            for ( int i = 0; i < n_pik; ++i )
            {
                real dis;
                real abs = loc.projectPoint(pik[i], dis);
                if ( 0 <= abs  &&  abs < ls )
                {
                    if ( !inside(loc.pos(abs)) || !inside(loc.pos1()) || !inside(loc.pos2()) )
                        meca.addPointClamp(Interpolation(loc, abs), pik[i], 100);
                }
            }
        }
    }
#endif
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpacePolygon::draw() const
{
    const unsigned npts = poly_.nbPoints();
    Polygon::Point2D const* pts = poly_.pts_;

#if ( DIM == 2 )
    
    glEnable(GL_STENCIL_TEST);
    glClearStencil(1);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
    
    //display points
    GLfloat s = 1;
    glGetFloatv(GL_LINE_WIDTH, &s);
    glPointSize(s);
    glBegin(GL_POINTS);
    for ( unsigned n=0; n < npts; ++n )
        gle::gleVertex(pts[n].xx, pts[n].yy);
    glEnd();

    //display polygon
    glBegin(GL_LINE_LOOP);
    for ( unsigned n=0; n < npts; ++n )
        gle::gleVertex(pts[n].xx, pts[n].yy);
    glEnd();
    
    glClear(GL_STENCIL_BUFFER_BIT);
    glDisable(GL_STENCIL_TEST);

#elif ( DIM > 2 )
    
    // display bottom
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    for ( unsigned n=0; n < npts; ++n )
        gle::gleVertex(pts[n].xx, pts[n].yy, -height_);
    glEnd();
    
    // display top
    glBegin(GL_LINE_LOOP);
    for ( unsigned n=npts; n > 0; --n )
        gle::gleVertex(pts[n].xx, pts[n].yy,  height_);
    glEnd();
    
    // display sides
    real Z = height_;
    glBegin(GL_TRIANGLE_STRIP);
    for ( unsigned n=0; n < npts; ++n )
    {
        gle::gleVertex(pts[n].xx, pts[n].yy, Z);
        gle::gleVertex(pts[n].xx, pts[n].yy,-Z);
    }
    if ( 0 < npts )
    {
        gle::gleVertex(pts[0].xx, pts[0].yy, Z);
        gle::gleVertex(pts[0].xx, pts[0].yy,-Z);
    }
    glEnd();
    
#endif
    
#if ( 0 )
    // display points:
    glColor3f(1,1,1);
    glPointSize(3);
    glBegin(GL_POINTS);
    for ( unsigned n=0; n < npts; ++n )
        gle::gleVertex(pts[n].xx, pts[n].yy);
    glEnd();
#endif
#if ( 0 )
    // indicate index of each point:
    char tmp[32];
    for ( unsigned n=0; n < npts; ++n )
    {
        snprintf(tmp, sizeof(tmp), "%i", n);
        Vector p(pts[n].xx, pts[n].yy, height_);
        gle::gleDrawText(p, tmp, 0);
    }
#endif

    return true;
}

#else

bool SpacePolygon::draw() const
{
    return false;
}

#endif
