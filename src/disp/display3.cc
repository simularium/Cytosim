// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "simul.h"
#include "display3.h"
#include "modulo.h"

#include "fake.h"
#include "fiber_disp.h"
#include "line_disp.h"
#include "point_disp.h"

#include "opengl.h"
#include "gle_color_list.h"
#include "glut.h"
#include "gle.h"

using namespace gle;
extern Modulo const* modulo;


Display3::Display3(DisplayProp const* dp) : Display(dp)
{
}

//------------------------------------------------------------------------------
#pragma mark -

void Display3::drawSimul(Simul const& sim)
{
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    drawFields(sim.fields);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    drawSpaces(sim.spaces);
    
    glDisable(GL_CULL_FACE);

    if ( stencil_ )
    {
        /*
         We use here the stencil test to make sure that nothing else is drawn
         where the inner side of the fibers is visible. This improves the
         display with clipping planes, as fibers appear as cut solid objects
         */
        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, ~0);
        
        // draw inner surfaces of fibers:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        drawFibers(sim.fibers);
        
        // draw front surfaces of fibers:
        glCullFace(GL_BACK);
        glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
        drawFibers(sim.fibers);
        
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_EQUAL, 0, ~0);
    }
    else
    {
        /**
         If the display is 'cut', we might see the inner sides,
         but rendering would be faster with Culling enabled
        */
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        drawFibers(sim.fibers);
    }
    
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    drawBeads(sim.beads);
    drawSolids(sim.solids);
    drawSpheres(sim.spheres);
    
    if ( prop->single_select & 1 )
        drawSinglesF(sim.singles);
    
    if ( prop->couple_select & 1 )
        drawCouplesF(sim.couples);

    if ( prop->couple_select & 2 )
        drawCouplesA(sim.couples);

    if ( prop->couple_select & 4 )
        drawCouplesB(sim.couples);

    if ( prop->single_select & 2 )
        drawSinglesA(sim.singles);
    
    if ( stencil_ )
    {
        glClearStencil(0);
        glDisable(GL_STENCIL_TEST);
    }

    drawOrganizers(sim.organizers);
    glDisable(GL_CULL_FACE);
    drawMisc(sim);
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::drawBall(Vector const& pos, real radius) const
{
    glEnable(GL_CULL_FACE);
    assert_true(glIsEnabled(GL_LIGHTING));
    glPushMatrix();
    gleTranslate(pos);
    gleScale(radius);
    glCullFace(GL_FRONT);
    gleSphere2B();
    glCullFace(GL_BACK);
    gleSphere4B();
    glPopMatrix();
}


void Display3::drawPoint(Vector const& pos, PointDisp const* disp) const
{
    if ( disp->perceptible )
    {
        glPushMatrix();
        gleTranslate(pos);
        gleScale(disp->size*sFactor);
        gleSphere1B();
        
#if ( 0 )
        if ( disp->symbol )
        {
            glDisable(GL_LIGHTING);
            disp->symbol_color.load();
            glRasterPos2f(0,0);
            glBitmap(0,0,0,0,-5,-4,0);
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, disp->symbol);
            glEnable(GL_LIGHTING);
        }
#endif
        glPopMatrix();
    }
}


void drawCap(int sty, Vector const& pos, Vector const& dir, real rad)
{
    if ( sty == 1 )
        gleObject(pos, dir, rad, gleDiscB);
    else if ( sty == 2 )
    {
        glEnable(GL_CLIP_PLANE4);
        setClipPlane(GL_CLIP_PLANE4, dir, pos);
        gleObject(pos, rad, gleSphere4);
        glDisable(GL_CLIP_PLANE4);
    }
}


//------------------------------------------------------------------------------
#pragma mark - Advanced Fiber Rendering

/**
This draws the model-segments from `inx` to `last`.
The function `set_color` is used to set the color of the segments
*/
void Display3::drawJoinedFiberLines(Fiber const& fib, bool minus_cap, bool plus_cap, real rad,
                                    unsigned inx, const unsigned last,
                                    void (*set_color)(Fiber const&, unsigned, real), real beta) const
{
    Vector pos = fib.posPoint(inx), old;
    Vector nxt = fib.posPoint(inx+1);
    Vector dir = normalize(nxt-pos);  // could use _mm_rsqrt_ss

    assert_true( last <= fib.lastSegment() );
    
    set_color(fib, inx, beta);
    if ( minus_cap )
        drawCap(fib.prop->disp->line_caps, pos, -dir, rad);
    
#if ( DIM > 1 )
    glEnable(GL_CLIP_PLANE4);
    glEnable(GL_CLIP_PLANE5);
    setClipPlane(GL_CLIP_PLANE4, dir, pos);
    
    // draw inner segments
    while ( inx < last )
    {
        old = pos;
        pos = nxt;
        nxt = fib.posPoint(inx+2);
        dir = normalize(nxt-old);
        set_color(fib, inx++, beta);
        setClipPlane(GL_CLIP_PLANE5, -dir, pos);
        gleTube(old, pos, rad, gleLongTube2B);
        setClipPlane(GL_CLIP_PLANE4,  dir, pos);
    }

    // draw last segment:
    dir = normalize(nxt-pos);
    set_color(fib, last, beta);
    setClipPlane(GL_CLIP_PLANE5, -dir, nxt);
    gleTube(pos, nxt, rad, gleLongTube2B);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
#else
    for ( ; inx < last; ++inx )
    {
        pos = nxt;
        nxt = fib.posP(inx+1);
        set_color(fib, inx, beta);
        gleTube(pos, nxt, rad, gleTube2B);
    }
    dir = fib.dirEndP();
#endif
    
    if ( plus_cap )
        drawCap(fib.prop->disp->line_caps, nxt, dir, rad);
}


/**
This draws segments of length 'len' which may not correspond to the vertices
used to model the Fiber. All abscissa is with respect to the MINUS_END.
The function `set_color` is used to set the color of the segments.
*/
void Display3::drawJoinedFiberLinesL(Fiber const& fib, bool minus_cap, bool plus_cap, real rad,
                                     long inx, const long last, real abs, const real inc,
                                     void (*set_color)(Fiber const&, long, real), real beta) const
{
    // draw MINUS_END
    Vector pos = fib.posM(abs), old;
    Vector nxt = fib.posM(abs+inc);
    Vector dir = normalize(nxt-pos);

    set_color(fib, inx, beta);
    if ( minus_cap )
        drawCap(fib.prop->disp->line_caps, pos, -dir, rad);

#if ( DIM > 1 )
    glEnable(GL_CLIP_PLANE4);
    glEnable(GL_CLIP_PLANE5);
    setClipPlane(GL_CLIP_PLANE4, dir, pos);
    
    // draw inner segments
    while ( inx < last )
    {
        old = pos;
        pos = nxt;
        nxt = fib.posM(abs+2*inc);
        dir = normalize(nxt-old);
        set_color(fib, inx, beta);
        setClipPlane(GL_CLIP_PLANE5, -dir, pos);
        gleTube(old, pos, rad, gleLongTube2B);
        // draw a circle to obturate the tube
        //gleObject(0.5*(old+pos), normalize(pos-old), rad, gleDiscB);
        setClipPlane(GL_CLIP_PLANE4,  dir, pos);
        abs += inc;
        ++inx;
    }
    
    // draw last segment:
    dir = normalize(nxt-pos);
    set_color(fib, last, beta);
    setClipPlane(GL_CLIP_PLANE5, -dir, nxt);
    gleTube(pos, nxt, rad, gleLongTube2B);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
#else
    for ( ; inx < last; ++inx )
    {
        pos = nxt;
        nxt = fib.posM(abs+inc);
        set_color(fib, inx, beta);
        gleTube(pos, nxt, rad, gleTube2B);
        abs += inc;
    }
    dir = fib.dirEndP();
#endif
    
    if ( plus_cap )
        drawCap(fib.prop->disp->line_caps, nxt, dir, rad);
}

/**
This draws a segment from 'abs1' to 'abs2', with respect to the MINUS_END.
*/
void Display3::drawFiberSegment(Fiber const& fib, bool minus_cap, bool plus_cap, real rad,
                                const real abs1, const real abs2) const
{
    // draw MINUS_END
    Vector pos = fib.posM(abs1);
    Vector nxt = fib.posM(abs2);
    Vector dir = normalize(nxt-pos);

    if ( minus_cap )
        drawCap(fib.prop->disp->line_caps, pos, -dir, rad);
    gleTube(pos, nxt, rad, gleTube2B);
    if ( plus_cap )
        drawCap(fib.prop->disp->line_caps, nxt, dir, rad);
}


void set_color_not(Fiber const&, long, real)
{
}

void set_color_not(Fiber const&, unsigned, real)
{
}

void set_color_tension(Fiber const& fib, unsigned seg, real beta)
{
    real x = beta * fib.tension(seg);
#if ( 1 )
    if ( x > 0 )  // invert color for extended fibers
        fib.disp->color.inverted().load_front(x);
    else          // use normal color compressed fibers
        fib.disp->color.load_front(-x);
#else
    // use rainbow coloring, where Lagrange multipliers are negative under compression
    gle_color::jet_color(1-x, fib.disp->color.a()).load_front();
#endif
}

void set_color_curvature(Fiber const& fib, unsigned seg, real)
{
    if ( fib.nbPoints() > 2 )
    {
        real c = fib.curvature(std::max(seg, 1u));
        real d = fib.curvature(std::min(seg+1, fib.lastSegment()));
        gle_color::jet_color(0.5*(c+d)).load_front();
    }
    else
        gle_color::jet_color(0).load_front();
}

void set_color_direction(Fiber const& fib, unsigned seg, real)
{
    gle::radial_color(fib.dirSegment(seg)).load_front();
}


void Display3::drawFiberLines(Fiber const& fib) const
{
    FiberDisp const*const disp = fib.prop->disp;
    const real rad = disp->line_width * sFactor;
    
    if ( disp->line_style == 1 )
    {
#if ( 1 )
        drawJoinedFiberLines(fib, true, true, rad, 0, fib.lastSegment(), set_color_not, 1.0);
#else
        // this is a basic rendering where tubes would not join properly:
        drawCap(fib.prop->disp->line_caps, fib.posEndM(), -fib.dirEndM(), rad);
        for ( unsigned s = 0; s < fib.nbSegments(); ++s )
            gleTube(fib.posP(s), fib.posP(s+1), rad, gleTube2B);
        drawCap(fib.prop->disp->line_caps, fib.posEndP(), fib.dirEndP(), rad);
#endif
    }
    else if ( disp->line_style == 2 )
    {
        real beta = 1.0 / disp->tension_scale;
        drawJoinedFiberLines(fib, true, true, rad, 0, fib.lastSegment(), set_color_tension, beta);
    }
    else if ( disp->line_style == 3 )
    {
        drawJoinedFiberLines(fib, true, true, rad, 0, fib.lastSegment(), set_color_curvature, 1.0);
    }
    else if ( disp->line_style == 4 )
    {
        drawJoinedFiberLines(fib, true, true, rad, 0, fib.lastSegment(), set_color_direction, 1.0);
    }
}


// this is for display with transparency:
void Display3::drawFiberLinesT(Fiber const& fib, unsigned i) const
{
    FiberDisp const*const disp = fib.prop->disp;
    const real rad = disp->line_width * sFactor;
 
    fib.disp->color.load_both();

    Vector A = fib.posP(i);
    Vector B = fib.posP(i+1);
    
    if ( i == 0 )
    {
        drawCap(fib.prop->disp->line_caps, A, normalize(A-B), rad);
        setClipPlane(GL_CLIP_PLANE5, normalize(B-A), A);
    }
    else
    {
        setClipPlane(GL_CLIP_PLANE5, normalize(B-fib.posP(i-1)), A);
    }
    
    if ( i == fib.lastSegment() )
    {
        drawCap(fib.prop->disp->line_caps, B, normalize(B-A), rad);
        setClipPlane(GL_CLIP_PLANE4, normalize(A-B), B);
    }
    else
    {
        setClipPlane(GL_CLIP_PLANE4, normalize(A-fib.posP(i+2)), B);
    }
    
    glEnable(GL_CLIP_PLANE5);
    glEnable(GL_CLIP_PLANE4);
    gleTube(A, B, rad, gleLongTube2B);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
}


//------------------------------------------------------------------------------
#pragma mark -

void set_color_alternate(Fiber const& fib, long ix, real)
{
    if ( ix & 1 )
        fib.disp->color.load_front();
    else
        fib.disp->color.darken(0.75).load_front();
}


void set_color_lattice(Fiber const& fib, long ix, real scale)
{
#if FIBER_HAS_LATTICE
    fib.disp->color.darken(scale * fib.lattice().data(ix)).load_front();
#else
    fib.disp->color.load_front();
#endif
}


void set_rainbow_lattice(Fiber const& fib, long ix, real scale)
{
#if FIBER_HAS_LATTICE
    gle_color::jet_color(scale * fib.lattice().data(ix)).load_front();
#else
    fib.disp->color.load_front();
#endif
}


void Display3::drawFiberLattice(Fiber const& fib, real width,
                                 void (*set_color)(Fiber const&, long, real)) const
{
    FiberLattice const& lat = *fib.drawableLattice();
    glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT);
    GLfloat blk[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  blk);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  blk);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blk);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, blk);
    glMateriali (GL_FRONT_AND_BACK, GL_SHININESS, 32);
    
    FiberDisp const*const disp = fib.prop->disp;

    const real fac = 1.0 / disp->lattice_scale;
    const real uni = lat.unit();
    const real rad = width * sFactor;
    
    auto inf = lat.indexM();
    auto sup = lat.indexP();

    real lenM = uni * inf - fib.abscissaM();
    real lenP = fib.abscissaP() - uni * sup;
    
    bool capM = ( lenM > 0 );
    bool capP = ( lenP > 0 );

    if ( capM )
    {
        set_color(fib, inf-1, disp->lattice_rescale ? fac*uni/lenM : fac);
        drawFiberSegment(fib, 1, 0, rad, 0, lenM);
    }

    drawJoinedFiberLinesL(fib, !capM, !capP, rad, inf, sup, lenM, uni, set_color, fac);
    
    if ( capP )
    {
        set_color(fib, sup, disp->lattice_rescale ? fac*uni/lenP : fac);
        drawFiberSegment(fib, 0, 1, rad, uni*sup-fib.abscissaM(), fib.length());
    }
    glPopAttrib();
}


void Display3::drawFiberLattice1(Fiber const& fib, real width) const
{
    drawFiberLattice(fib, width, set_color_lattice);
}

void Display3::drawFiberLattice2(Fiber const& fib, real width) const
{
    drawFiberLattice(fib, width, set_rainbow_lattice);
}

void Display3::drawFiberLatticeEdges(Fiber const& fib, real width) const
{
    drawFiberLattice(fib, width, set_color_alternate);
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Display the MINUS_END of a Fiber, according to `style`:
 - 1: draw a sphere
 - 2: draw a cone
 - 3: draw a flat cylinder
 - 4: draw an arrow-head
 - 5: arrow-head in reverse direction
 .
 with 3D objects
 */
void Display3::drawFiberMinusEnd(Fiber const& fib, int style, real size) const
{
    real width = size * sFactor;
    if ( width > 0 )
    {
        switch (style)
        {
            case 1:
                gleObject(fib.posEndM(), width, gleSphere2B);
                break;
            case 2:
                gleObject(fib.posEndM(), -fib.dirEndM(), width, gleLongConeB);
                break;
            case 3:
                gleObject(fib.posEndM(), -fib.dirEndM(), width, gleCylinderB);
                break;
            case 4:
                gleObject(fib.posEndM(),  fib.dirEndM(), width, gleArrowTailB);
                break;
            case 5:
                gleObject(fib.posEndM(), -fib.dirEndM(), width, gleArrowTailB);
                break;
        }
    }
}


/**
 Display the PLUS_END of a Fiber, according to `style`:
 - 1: draw a sphere
 - 2: draw a cone
 - 3: draw a flat cylinder
 - 4: draw an arrow-head
 - 5: arrow-head in reverse direction
 .
 with 3D objects
 */
void Display3::drawFiberPlusEnd(Fiber const& fib, int style, real size) const
{
    real width = size * sFactor;
    if ( width > 0 )
    {
        switch (style)
        {
            case 1:
                gleObject(fib.posEndP(), width, gleSphere2B);
                break;
            case 2:
                gleObject(fib.posEndP(), fib.dirEndP(), width, gleLongConeB);
                break;
            case 3:
                gleObject(fib.posEndP(), fib.dirEndP(), width, gleCylinderB);
                break;
            case 4:
                gleObject(fib.posEndP(), fib.dirEndP(), width, gleArrowTailB);
                break;
            case 5:
                gleObject(fib.posEndP(), -fib.dirEndP(), width, gleArrowTailB);
                break;
        }
    }
}


void Display3::drawFiberSpeckles(Fiber const& fib) const
{
    FiberDisp const*const disp = fib.prop->disp;
    // diameter of lines and points in space units:
    real rad = disp->speckle_size * sFactor;
    
    if ( disp->point_size * uFactor < 2 )
        return;

    // display random speckles:
    if ( disp->speckle_style == 1 )
    {
        /*
         A simple random number generator seeded by fib.signature()
         is used to distribute points always at the same position
         with respect to the lattice of each fiber.
         */
        
        const real spread = disp->speckle_interval;
        const real S = 0x1p-32;
        // draw speckles below the origin of abscissa:
        if ( fib.abscissaM() < 0 )
        {
            uint32_t z = fib.signature();
            real a = spread * log(z*S);
            while ( a > fib.abscissaP() )
            {
                z = lcrng2(z);
                a += spread * log(z*S);
            }
            while ( a >= fib.abscissaM() )
            {
                gleObject(fib.pos(a), rad, gleSphere1B);
                z = lcrng2(z);
                a += spread * log(z*S);
            }
        }
        // draw speckles above the origin of abscissa:
        if ( fib.abscissaP() > 0 )
        {
            uint32_t z = ~fib.signature();
            real a = -spread * log(z*S);
            while ( a < fib.abscissaM() )
            {
                z = lcrng1(z);
                a -= spread * log(z*S);
            }
            while ( a <= fib.abscissaP() )
            {
                gleObject(fib.pos(a), rad, gleSphere1B);
                z = lcrng1(z);
                a -= spread * log(z*S);
            }
        }
    }
    else if ( disp->speckle_style == 2 )
    {
        //we distribute points regularly along the center line
        const real grad = disp->speckle_interval;
        real ab = grad * ceil( fib.abscissaM() / grad );
        while ( ab <= fib.abscissaP() )
        {
            gleObject(fib.pos(ab), rad, gleSphere1B);
            ab += grad;
        }
    }
}


void Display3::drawFiberPoints(Fiber const& fib) const
{
    FiberDisp const*const disp = fib.prop->disp;
    // diameter of lines and points in space units:
    real rad = disp->point_size * sFactor;
    
    if ( disp->point_size * uFactor < 2 )
        return;

    if ( disp->point_style == 1 )
    {
        // display vertices:
        for ( unsigned ii = 0; ii < fib.nbPoints(); ++ii )
            gleObject(fib.posP(ii), rad, gleSphere2B);
    }
    else if ( disp->point_style == 2 )
    {
        // display arrowheads along the fiber:
        const real siz = disp->point_size*sFactor;
        const real sep = disp->point_interval;
        real ab = ceil(fib.abscissaM()/sep) * sep;
        for ( ; ab <= fib.abscissaP(); ab += sep )
            gleCone(fib.pos(ab), fib.dir(ab), siz);
    }
    else if ( disp->point_style == 3 )
    {
        // display middle of fiber:
        gleObject(fib.posMiddle(), 2*disp->point_size, gleSphere2B);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::drawBead(Bead const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    // display center:
    if ( disp->style & 2 )
    {
        bodyColor(disp, obj.signature());
        drawPoint(obj.position(), disp);
    }
    
#if ( DIM == 2 )
    // display outline:
    if ( disp->style & 4 )
    {
        bodyColor(disp, obj.signature());
        lineWidth(disp->width);
        gleObject(obj.position(), obj.radius(), gleCircleB);
    }
#endif
}

/**
 Display a bead as a sphere
 */
void Display3::drawBeadT(Bead const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    if ( disp->style & 1 )
    {
        bodyColorT(disp, obj.signature());
        drawBall(obj.position(), obj.radius());
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void Display3::drawSolid(Solid const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    //display points:
    if ( disp->style & 2  &&  disp->size > 0 )
    {
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
            drawPoint(obj.posP(ii), disp);
    }
    
#if ( DIM == 3 )
    //special display for ParM simulations (DYCHE)
    if ( obj.mark()  &&  disp->style & 4  &&  obj.nbPoints() >= 3 )
    {
        bodyColor(disp, obj.signature());
        gleObject(obj.posP(0), obj.diffPoints(1, 0), obj.radius(0), gleCircleB);
    }
#endif
    
    //display a signature for each Solid
    if ( disp->style & 8 )
    {
        char tmp[32];
        bodyColor(disp, obj.signature());
        snprintf(tmp, sizeof(tmp), "%u", obj.identity());
        gleDrawText(obj.posP(0), tmp, GLUT_BITMAP_HELVETICA_10);
    }
    
    //draw polygon around vertices of Solid
    if ( disp->style & 16 )
    {
        const real rad = disp->width * sFactor;
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 1; ii < obj.nbPoints(); ++ii )
            gleTube(obj.posPoint(ii-1), obj.posPoint(ii), rad, gleTube2B);
    }
}


/**
 Display a semi-transparent disc / sphere
 */
void Display3::drawSolidT(Solid const& obj, unsigned int ii)
{
    const PointDisp * disp = obj.prop->disp;

    if ( disp->style & 1  &&  obj.radius(ii) > 0 )
    {
        bodyColorT(disp, obj.signature());
        drawBall(obj.posP(ii), obj.radius(ii));
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::drawSphere(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    //display center and surface points
    if ( disp->size > 0  &&  disp->style & 2 )
    {
        bodyColor(disp, obj.signature());
        drawPoint(obj.posP(0), disp);
        for ( unsigned ii = obj.nbRefPoints; ii < obj.nbPoints(); ++ii )
            drawPoint(obj.posP(ii), disp);
    }

    //display reference points
    if ( disp->size > 0  &&  disp->style & 8 )
    {
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 1; ii < obj.nbRefPoints; ii++ )
            drawPoint(obj.posP(ii), disp);
    }
}

void Display3::drawSphereT(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    //display the envelope
    if ( disp->style & 5 )
    {
        bodyColorT(disp, obj.signature());
        lineWidth(disp->width);
        
        glPushMatrix();
        
#if ( DIM < 3 )
        
        gleTranslate(obj.posP(0));
        gleTorus(obj.radius(), disp->size*sFactor);
        
#else
        
        /* Note: The rotation matrix for the sphere calculated below from the
            reference points, includes scaling by the radius of the sphere.
            We then use a primitive for a sphere of radius 1.
            */
        const Vector C = obj.posP(0);
        gleTransRotate(obj.posP(1)-C, obj.posP(2)-C, obj.posP(3)-C, C);
        
        if ( disp->style & 1 )
        {
            glCullFace(GL_FRONT);
            gleSphere2B();
            glCullFace(GL_BACK);
            gleSphere4B();
        }
        if ( disp->style & 4 )
        {
            disp->color2.load_front();
            gleThreeBands(64);
        }
        
#endif
        glPopMatrix();
    }
}

//------------------------------------------------------------------------------

void Display3::drawOrganizer(Organizer const& obj) const
{
    PointDisp const* disp = obj.disp();
    
    if ( !disp )
        return;

    const real w = disp->width*sFactor;

    if ( disp->style & 2 )
    {
        Vector P, Q;
        bodyColor(disp, obj.signature());
        
        for ( size_t i = 0; obj.getLink(i, P, Q); ++i )
            drawPoint(P, disp);

        for ( size_t i = 0; obj.getLink(i, P, Q); ++i )
        {
            if ( modulo ) modulo->fold(Q, P);
            gleTube(P, Q, w, gleTube1B);
        }
    }
    /**
     This displays the Solid connecting two Aster as a spindle.
     Used for Cleo Kozlowski simulation of C. elegans
     */
    if ( disp->style & 1 && obj.tag() == Fake::TAG )
    {
        Solid const* so = static_cast<const Fake&>(obj).solid();
        if ( so && so->nbPoints() >= 4 )
        {
            bodyColor(so->prop->disp, so->signature());
#if ( DIM == 3 )
            glPushMatrix();
            Vector3 a = 0.5 * (so->posP(0) + so->posP(2));
            Vector3 b = 0.5 * (so->posP(1) + so->posP(3));
            gleTransAlignZ(a, b, 1);
            glColor3f(0.6f,0.6f,0.6f);
            gleDualPass(gleBarrel1);
            glPopMatrix();
#else
            for ( unsigned ii = 0; ii < so->nbPoints(); ii+=2 )
                gleTube(so->posPoint(ii), so->posPoint(ii+1), w, gleHexTube1B);
#endif
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void Display3::drawSinglesF(SingleSet const& set) const
{
    for ( Single * obj=set.firstF(); obj ; obj=obj->next() )
    {
        obj->disp()->color2.load_both();
        drawPoint(obj->posFoot(), obj->disp());
    }
}


void Display3::drawSinglesA(SingleSet const& set) const
{
    for ( Single * obj=set.firstA(); obj ; obj=obj->next() )
    {
        if ( obj->fiber()->disp->visible )
        {
            const PointDisp * disp = obj->disp();
            Vector ph = obj->posHand();
            
            if ( obj->hasForce() && disp->width > 0 )
            {
                Vector pf = obj->posFoot();
                if ( modulo ) modulo->fold(pf, ph);
                disp->color2.load_both();
#if ( DIM >= 3 )
                gleTube(ph, pf, disp->width*sFactor);
#else
                gleBand(ph, disp->width*sFactor, disp->color, pf, disp->width*sFactor, disp->color.alpha_scaled(0.5));
#endif
            }
            
            disp->color.load_both();
            drawPoint(ph, disp);
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -
/**
 Display always Hand1 of Couple
 */
void Display3::drawCouplesF(CoupleSet const& set) const
{
    for ( Couple * cx = set.firstFF(); cx ; cx=cx->next() )
        drawHand2(cx->posFree(), cx->disp1());
}


void Display3::drawCouplesA(CoupleSet const& set) const
{
    for ( Couple * cx=set.firstAF(); cx ; cx=cx->next() )
    {
        if ( cx->fiber1()->disp->visible && cx->disp1()->visible )
        {
#if ( 0 )
            // ENDOCYTOSIS 2015
            if ( cx->fiber1()->disp->color.transparent() )
                cx->disp1()->color.load_both(cx->fiber1()->disp->color.transparency());
            else
#endif
            drawHand(cx->posHand1(), cx->disp1());
        }
    }
    
    for ( Couple * cx=set.firstFA(); cx ; cx=cx->next() )
    {
        if ( cx->fiber2()->disp->visible && cx->disp2()->visible )
        {
#if ( 0 )
            // ENDOCYTOSIS 2015
            if ( cx->fiber2()->disp->color.transparent() )
                cx->disp1()->color.load_both(cx->fiber2()->disp->color.transparency());
            else
#endif
            drawHand(cx->posHand2(), cx->disp2());
        }
    }
}

void Display3::drawCoupleBfast(Couple const* cx) const
{
    const PointDisp * pd1 = cx->disp1();
    const PointDisp * pd2 = cx->disp2();
    
    Vector p1 = cx->posHand1();
    Vector p2 = cx->posHand2();
    
    if ( pd1 == pd2 )
    {
        if ( pd1->visible )
        {
            pd1->color.load_both();
            gleTube(p1, p2, pd2->width*sFactor, gleHexTube1B);
            drawHand(p1, pd1);
            drawHand(p2, pd2);
        }
    }
    else if ( pd1->visible || pd2->visible )
    {
        pd1->color.load_both();
        gleTube(p1, p2, pd1->width*sFactor, gleTube1B);
        if ( pd1->visible ) drawHand(p1, pd1);
        if ( pd2->visible ) drawHand(p2, pd2);
    }
}


void Display3::drawCoupleB(Couple const* cx) const
{
    const PointDisp * pd1 = cx->disp1();
    const PointDisp * pd2 = cx->disp2();
    
    Vector p1 = cx->posHand1();
    Vector p2 = cx->posHand2();
    if ( modulo ) modulo->fold(p2, p1);
    
    Vector dif = p2 - p1;
    real dns = dif.normSqr();
    
#if ( 1 )
    if ( dns > 1e-6 )
    {
        dns = sFactor / sqrt(dns);
        // position the heads at the surface of the filaments:
        const real rad1 = cx->fiber1()->prop->disp->line_width;
        const real rad2 = cx->fiber2()->prop->disp->line_width;
        p1 += dif * std::min((real)0.5, rad1*dns);
        p2 -= dif * std::min((real)0.5, rad2*dns);
    }
#endif
    
    if ( pd1 == pd2 )
    {
        if ( pd1->visible )
        {
            pd1->color.load_both();
            gleTube(p1, p2, pd2->width*sFactor, gleHexTube1B);
            drawPoint(p1, pd1);
            drawPoint(p2, pd2);
        }
    }
    else if ( dns > 1e-6 )
    {
        Vector mid = 0.5 * ( p1 + p2 );
        
        glEnable(GL_CLIP_PLANE5);
        if ( pd1->visible )
        {
            setClipPlane(GL_CLIP_PLANE5, -dif, mid);
            pd1->color.load_front();
            gleTube(p1, p2, pd1->width*sFactor, gleTube1B);
            drawHand(p1, pd1);
        }
        
        if ( pd2->visible )
        {
            setClipPlane(GL_CLIP_PLANE5,  dif, mid);
            pd2->color.load_front();
            gleTube(p2, p1, pd2->width*sFactor, gleTube1B);
            drawHand(p2, pd2);
        }
        glDisable(GL_CLIP_PLANE5);
    }
    else
    {
        if ( pd1->visible ) drawHand(p1, pd1);
        if ( pd2->visible ) drawHand(p2, pd2);
    }
}


