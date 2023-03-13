// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "display.h"
#include "organizer.h"
#include "hand_prop.h"
#include "sphere_prop.h"
#include "fiber_prop.h"
#include "point_disp.h"
#include "fiber_disp.h"
#include "line_disp.h"
#include "opengl.h"
#include "modulo.h"
#include "simul.h"
#include "field.h"
#include "gle.h"
#include "gle_color.h"
#include "gle_color_list.h"
#include "glapp.h"
#include "glut.h"

extern Modulo const* modulo;


//------------------------------------------------------------------------------
#pragma mark -


Display::Display(DisplayProp const* dp)
: pixelSize(1), uFactor(1), sFactor(1), prop(dp)
{
    assert_true(dp);
    
    prep_time = -1;
}

void Display::setPixelFactors(GLfloat ps, GLfloat u)
{
    pixelSize = ps;
    uFactor   = u;
    /*
     the 0.5 below comes from the fact that glPointSize uses diameter
     while most gle::primitives use radius as arguments
     */
    sFactor = 0.5f * u * ps;
}


void Display::drawSimul(Simul const& sim)
{
    gle::gleDrawText(Vector(0,0,0), "Empty Display::display", GLUT_BITMAP_8_BY_13);
}


void Display::display(Simul const& sim)
{
    // clear list of transparent objects
    zObjects.clear();

#if ( DIM >= 3 )
    glEnable(GL_LIGHTING);
#else
    glDisable(GL_LIGHTING);
#endif
    
    /*
     Draw opaque objects:
     - depth buffer is writable
     - glColor specifies the Front material color
     */

#if ( DIM >= 3 )
    
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    
#endif
    
    drawSimul(sim);
    
    /*
     Draw translucent objects:
     - make depth buffer readible only
     - objects are depth-sorted, from far to near
     - Dual pass is used to display back before front
     */

#if ( DIM >= 3 )
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    
    if ( zObjects.size() )
        drawTransparentObjects(zObjects);
    
    glEnable(GL_CULL_FACE);
    drawTransparentSpaces(sim.spaces);

#endif
    
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    
#ifndef NDEBUG
    gle::gleReportErrors(stderr, "in Display::display()");
#endif
}


/**
 To get correct display, it would be necessary to display all opaque objects first,
 and then all transparent objects for all tiles. Here, we calls Display::display()
 a number of times, and objects are sorted within each tile. The result is not perfect.
 */
void Display::displayTiled(Simul const& sim, int arg)
{
    assert_true(modulo);
    
    int l[3] = { 0 };
    int u[3] = { 0 };
    
    for ( int d = 0; d < DIM; ++d )
    {
        if ( modulo->isPeriodic(d) )
        {
            l[d] = (arg>1) ? -1 : 0;
            u[d] = +1;
        }
    }
    
    glMatrixMode(GL_MODELVIEW);
    
    Vector px = modulo->period(0);
    Vector py = modulo->period(1);
    Vector pz = modulo->period(2);

    for ( int dx = l[0]; dx <= u[0]; ++dx )
    for ( int dy = l[1]; dy <= u[1]; ++dy )
    for ( int dz = l[2]; dz <= u[2]; ++dz )
    {
        Vector T = dx * px + dy * py + dz * pz;
        gle::gleTranslate( T);
        display(sim);
        gle::gleTranslate(-T);
    }
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Create a FiberDisp for this Property if necessary
 */
void Display::prepareFiberDisp(FiberProp * p, PropertyList& alldisp, gle_color col)
{
    FiberDisp *& disp = p->disp;
    
    // recover existing property:
    if ( !disp )
        disp = static_cast<FiberDisp*>(alldisp.find("fiber:display", p->name()));

    // create new property with default values:
    if ( disp == nullptr )
    {
        disp = new FiberDisp(p->name());
        alldisp.push_back(disp);
        // set default:
        disp->color       = col;
        disp->back_color  = col.darken(0.5);
        disp->point_size  = prop->point_size;
        disp->line_width  = prop->line_width;
    }
    
    // parse user-provided values:
    if ( p->display_fresh )
    {
        disp->read_string(p->display, " in "+p->name()+":display");
        p->display_fresh = false;
    }
    
    if ( disp->coloring == FiberDisp::COLORING_CLUSTER )
        fiber_prep |= 1;
    
    if ( disp->line_style == 2 )
        fiber_prep |= 2;

    if ( disp->coloring == FiberDisp::COLORING_AGE )
        fiber_prep |= 4;
}


/**
 set LineDisp for given Fiber
 */
void Display::prepareLineDisp(const Fiber * fib)
{
    assert_true(fib->prop);
    FiberDisp const*const disp = fib->prop->disp;
    
    if ( !fib->disp )
        fib->disp = new LineDisp();
    LineDisp * self = fib->disp;
    
    // change body color depending on coloring mode:
    switch ( disp->coloring )
    {
        case FiberDisp::COLORING_OFF:
            self->color = disp->color;
            break;
        case FiberDisp::COLORING_RANDOM:
            self->color = gle::bright_color(fib->signature()).match_a(disp->color);
            break;
        case FiberDisp::COLORING_DIRECTION:
            self->color = gle::radial_color(fib->avgDirection());
            break;
        case FiberDisp::COLORING_MARK:
            self->color = gle::nice_color(fib->mark());
            break;
        case FiberDisp::COLORING_FLAG:
            self->color = gle::std_color(fib->flag());
            break;
        case FiberDisp::COLORING_CLUSTER:
            self->color = gle::std_color(fib->flag());
            break;
        case FiberDisp::COLORING_AGE:
            self->color = gle_color::jet_color((fib->age()-age_min)*age_scale, 1.0);
            break;
    }
    
#if ( 0 )
    // colors of ends set to match body color:
    self->end_color[0] = self->color;
    self->end_color[1] = self->color;
#else
    // colors of ends for non-dynamic filaments:
    self->end_color[0] = disp->end_color[0];
    self->end_color[1] = disp->end_color[0];
#endif
    
    // For dynamic Fibers, change colors of tips according to state:
    if ( fib->dynamicStateP() > 0 )
        self->end_color[0] = disp->end_color[fib->dynamicStateP()%5];
    
    if ( fib->dynamicStateM() > 0 )
        self->end_color[1] = disp->end_color[fib->dynamicStateM()%5];

    // hide right or left-pointing fibers:
    if ( ( disp->exclude & 1 )  &&  dot(fib->diffPoints(0), disp->exclude_axis) < 0 )
        self->color = disp->hide_color;
    if ( ( disp->exclude & 2 )  &&  dot(fib->diffPoints(0), disp->exclude_axis) > 0 )
        self->color = disp->hide_color;
    
#if ( DIM == 2 )
    // hide clockwise or counter-clockwise orientated fibers:
    if ( ( disp->exclude & 4 )  &&  cross(fib->posP(0), fib->diffPoints(0)) < 0 )
        self->color = disp->hide_color;
    if ( ( disp->exclude & 8 )  &&  cross(fib->posP(0), fib->diffPoints(0)) > 0 )
        self->color = disp->hide_color;
#elif ( DIM == 3 )
    // hide clockwise or counter-clockwise orientated fibers in the XY plane
    if ( ( disp->exclude & 4 )  &&  cross(fib->posP(0), fib->diffPoints(0)).ZZ < 0 )
        self->color = disp->hide_color;
    if ( ( disp->exclude & 8 )  &&  cross(fib->posP(0), fib->diffPoints(0)).ZZ > 0 )
        self->color = disp->hide_color;
#endif
    
#if ( 1 )
    // hide fibers depending on mask
    if ( fib->signature() & disp->mask_bitfield )
        self->color = disp->hide_color;
#else
    if ( fib->mark() & disp->mask_bitfield )
        self->color = disp->hide_color;
#endif

    // default visibility set from class:
    if ( disp->visible )
    {
        // change visibility flag according to body color:
        if ( !self->color.visible() )
            self->visible = 0;
        else if ( self->color.transparent() )
            self->visible = -1;
        else
            self->visible = 1;
    }
    else
        self->visible = 0;

#if ( 0 )
    // hide fibers which are not growing
    if ( fib->dynamicStateP() == STATE_WHITE )
    {
        LOG_ONCE("non-growing fibers made invisible\n");
        self->visible = -1;
        self->color = disp->hide_color;
        self->end_color[0] = self->color.transparency(1.0);
    }
#endif
    
    // set parameters for exploded display
    if ( disp->explode )
        self->explode_shift = ( lcrng3(fib->signature()) * 0x1p-32 - 0.5 ) * disp->explode_range;
    else
        self->explode_shift = 0;
}


/**
 Create a PointDisp for this Property if necessary
 */
template < typename T >
void Display::preparePointDisp(T * p, PropertyList& alldisp, gle_color col)
{
    assert_true(p);
        
    PointDisp *& disp = p->disp;
    
    // search for matching property:
    if ( !disp )
        disp = static_cast<PointDisp*>(alldisp.find(p->category()+":display", p->name()));
    
    // create new property:
    if ( !disp )
    {
        //std::clog <<" new " << p->category() << ":display " << p->name() << "\n";
        disp = new PointDisp(p->category()+":display", p->name());
        disp->clear();
        alldisp.push_back(disp);
        // set default:
        disp->color  = col;
        disp->color2 = col.alpha(0.5);
        disp->size   = prop->point_size;
        if ( p->category() == "hand" )
            disp->width = prop->link_width;
        else
            disp->width = prop->line_width;
    }
    
    // parse display string once:
    if ( p->display_fresh )
    {
        disp->read_string(p->display, " in "+p->name()+":display");
        p->display_fresh = false;
    }
    
    disp->prepare(uFactor, sFactor, 0);
}

/**
 Perform the operations that are necessary to display the simulation:
 - create FiberDisp, HandDisp, SphereDisp, etc. (one per Property)
 - create LineDisp (one per Fiber)
 - set default values,
 - parse display strings
 .
*/
void Display::prepareForDisplay(Simul const& sim, PropertyList& alldisp)
{
    if ( prop->fold )
        sim.foldPositions();
    
    // counter to give different colors to the objects
    unsigned int idx = 0;
    
    PropertyList plist = sim.properties.find_all("fiber");
    
    fiber_prep = 0;
    // create a FiberDisp for each FiberProp:
    for ( Property* p : plist )
        prepareFiberDisp(static_cast<FiberProp*>(p), alldisp, gle::nice_color(idx++));

    if ( prep_time != sim.time() )
    {
        // the cluster analysis only needs to be done once per state:
        //prep_time = sim.time();
        if ( fiber_prep & 1 )
            sim.flagClusters(true);
        
        // if fiber tensions are used for display, recompute them now:
        if ( fiber_prep & 2 )
            sim.computeForces();
        
        // calculate Fiber::age() range and set color scaling factor:
        if ( fiber_prep & 4 )
        {
            unsigned cnt;
            real avg, dev, mn, mx;
            FiberSet::infoBirthtime(sim.fibers.collect(), cnt, avg, dev, mn, mx);
            if ( mx > mn )
            {
                //std::clog << "=Fiber:age range [" << mn << " " << mx << " ]\n";
                age_min = sim.time() - mx;
                age_scale = 5.0 / ( mx - mn );
            }
            else
            {
                age_min = 0;
                age_scale = 1;
            }
        }
    }
    
    // attribute LineDisp, and set individual display values for all fibers
    for ( Fiber * fib = sim.fibers.first(); fib; fib = fib->next() )
        prepareLineDisp(fib);
    
    //create a PointDisp for each HandProp:
    for ( Property * i : sim.properties.find_all("hand") )
        preparePointDisp(static_cast<HandProp*>(i), alldisp, gle::nice_color(idx++));
    
    //create a PointDisp for each SphereProp:
    for ( Property * i : sim.properties.find_all("sphere") )
        preparePointDisp(static_cast<SphereProp*>(i), alldisp, gle::bright_color(idx++));
    
    //create a PointDisp for each SolidProp:
    for ( Property * i : sim.properties.find_all("solid", "bead") )
        preparePointDisp(static_cast<SolidProp*>(i), alldisp, gle::bright_color(idx++));
    
    //create a PointDisp for each SpaceProp:
    gle_color col(DIM==3?0x00000044:0xAAAAAAFF);
    for ( Property * i : sim.properties.find_all("space") )
        preparePointDisp(static_cast<SpaceProp*>(i), alldisp, col);
}


/**
 if `coloring` is enabled, this loads the N-th bright color,
 otherwise load the object' display color
 */
void Display::bodyColor(PointDisp const* disp, unsigned s) const
{
    if ( disp->coloring )
    {
        gle_color col = gle::bright_color(s);
        col.load();
        col.load_front();
        col.darken(0.5).load_back();
    }
    else
    {
        disp->color.load_load(1.0);
        disp->color2.load_back();
    }
}

/**
 if `coloring` is enabled, this loads the N-th bright color,
 otherwise load the object' display color
 */
void Display::bodyColor2(PointDisp const* disp, unsigned s) const
{
    if ( disp->coloring )
        gle::bright_color(s).match_a(disp->color).load();
    else
        disp->color.load();
}

/**
 This is used for transparent objects.
 if `coloring` is enabled, this loads the N-th bright color,
 with an alpha value matched to the one of the object's display color.
 */
void Display::bodyColorT(PointDisp const* disp, unsigned s) const
{
    if ( disp->coloring )
    {
        gle_color col = gle::bright_color(s).match_a(disp->color);
        col.load();
        col.load_both();
    }
    else
    {
        disp->color.load();
        disp->color.load_both();
    }
}


//------------------------------------------------------------------------------
#pragma mark -


/**
 Draw a transparent Spaces (3D only)
 */
void Display::drawSpace(Space const* obj, bool opaque)
{
    const PointDisp * disp = obj->prop->disp;
    
    glEnable(GL_CULL_FACE);
    // draw back side
    if ( disp->visible & 2 && disp->color2.opaque() == opaque )
    {
        lineWidth(disp->width);
        glCullFace(GL_FRONT);
        disp->color2.load_back();
        obj->draw();
    }
    // draw front side
    if ( disp->visible & 1 && disp->color.opaque() == opaque )
    {
        lineWidth(disp->width);
        glCullFace(GL_BACK);
        disp->color.load_front();
        obj->draw();
    }
}


void Display::drawSpaces(SpaceSet const& set)
{
#if ( DIM == 3 )
    
    // draw non-transparent Spaces first:
    for ( Space * obj = set.first(); obj; obj=obj->next() )
    {
        if ( obj->prop->disp->visible )
            drawSpace(obj, true);
    }

#else
    
    for ( Space * obj = set.first(); obj; obj=obj->next() )
    {
        const PointDisp * disp = obj->prop->disp;
        if ( disp->visible )
        {
            lineWidth(disp->width);
            disp->color.load_load();
            obj->draw();
        }
    }
    
#endif
}


/**
 Draw transparent Spaces
 */
void Display::drawTransparentSpaces(SpaceSet const& set)
{
    for ( Space * obj = set.first(); obj; obj=obj->next() )
    {
        if ( obj->prop->disp->visible )
            drawSpace(obj, false);
    }
}


/**
 This displays only one Field, specified by DisplayProp:field_number
 
 GL_CULL_FACE and GL_LIGHTING should be disabled
 */
void Display::drawFields(FieldSet const& set)
{
#if ( DIM == 3 )
    // get current modelview transformation:
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    
    // extract axis corresponding to vertical direction:
    Vector3 dir = normalize(Vector3(mat[2], mat[6], mat[10]));
#else
    Vector3 dir(0,0,1);
#endif
    
#if ( 1 )
    Field * obj = set.first();
#else
    for ( Field * obj = set.first(); obj; obj=obj->next() )
#endif
    
    if ( obj && obj->hasField() )
    {
        if ( obj->prop->visible == 1 )
            obj->draw(false, dir, 0);
        else if ( obj->prop->visible == 2 )
            obj->draw();
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display::drawAverageFiber(ObjectList const& objs)
{
    Vector G, D, M, P;
    real S = FiberSet::infoPosition(objs, M, G, P);
    
    if ( S > REAL_EPSILON )
    {
        Vector MP = normalize( P - M );
        gle::gleCylinder(M, MP, 10*pixelSize);
        gle::gleCone(P, MP, 10*pixelSize);
        gle::gleObject(G, 10*pixelSize, gle::gleSphere2B);
    }
}


bool selectR(Object const* obj, void const* arg)
{
    Fiber const* fib = static_cast<Fiber const*>(obj);
    return fib->prop == arg  &&  dot(fib->diffPoints(0), fib->prop->disp->exclude_axis) > 0;
}

bool selectL(Object const* obj, void const* arg)
{
    Fiber const* fib = static_cast<Fiber const*>(obj);
    return fib->prop == arg  &&  dot(fib->diffPoints(0), fib->prop->disp->exclude_axis) < 0;
}

void Display::drawAverageFiber1(FiberSet const& fibers, void const* arg)
{
    ObjectList objs = fibers.collect(match_property, arg);

#if ( 1 )
    // highlight with a black outline
    glLineWidth(3);
    glColor3f(0,0,0);
    glDepthMask(GL_FALSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawAverageFiber(objs);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthMask(GL_TRUE);
#endif
    
    glColor3f(1,1,1);
    drawAverageFiber(objs);
}


void Display::drawAverageFiber2(FiberSet const& fibers, void const* arg)
{
    ObjectList objsR = fibers.collect(selectR, arg);
    ObjectList objsL = fibers.collect(selectL, arg);

#if ( 1 )
    // highlight with a black outline
    glLineWidth(3);
    glColor3f(0,0,0);
    glDepthMask(GL_FALSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawAverageFiber(objsR);
    drawAverageFiber(objsL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthMask(GL_TRUE);
#endif
    
    // display right-pointing fibers in Red
    glColor3f(1,0,0);
    drawAverageFiber(objsR);
    
    // display left-pointing fibers in Green
    glColor3f(0,1,0);
    drawAverageFiber(objsL);
}    


void Display::drawMisc(Simul const& sim)
{
#if ( 0 )
    // display Steric Grid for visual debugging:
    glLineWidth(0.5);
    glColor3f(0, 0, 1);
    sim.pointGrid.draw();
#endif
    
    for ( Property * i : sim.properties.find_all("fiber") )
    {
        FiberProp* fp = static_cast<FiberProp*>(i);
        if ( fp->disp->draw_average == 1 )
            drawAverageFiber1(sim.fibers, fp);
        else if ( fp->disp->draw_average == 2 )
            drawAverageFiber2(sim.fibers, fp);
    }
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
 */
void Display::drawFiberMinusEnd(Fiber const& fib, int style, real size) const
{
    real width = size * sFactor;
    if ( width > 0 )
    {
        switch(style)
        {
            default:
                break;
            case 1:
                gle::gleObject(fib.posEndM(), width, gle::gleSphere2B);
                break;
            case 2:
                gle::gleCone(fib.posEndM(), -fib.dirEndM(), width);
                break;
            case 3:
                gle::gleCylinder(fib.posEndM(), -fib.dirEndM(), width);
                break;
            case 4:
                gle::gleArrowTail(fib.posEndM(), fib.dirEndM(), width);
                break;
            case 5:
                gle::gleArrowTail(fib.posEndM(), -fib.dirEndM(), width);
                break;
            case 6:
                gle::gleObject(fib.posEndM(), width, gle::gleCube1);
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
 */
void Display::drawFiberPlusEnd(Fiber const& fib, int style, real size) const
{
    real width = size * sFactor;
    if ( width > 0 )
    {
        switch(style)
        {
            default:
                break;
            case 1:
                gle::gleObject(fib.posEndP(), width, gle::gleSphere2B);
                break;
            case 2:
                gle::gleCone(fib.posEndP(), fib.dirEndP(), width);
                break;
            case 3:
                gle::gleCylinder(fib.posEndP(), fib.dirEndP(), width);
                break;
            case 4:
                gle::gleArrowTail(fib.posEndP(), fib.dirEndP(), width);
                break;
            case 5:
                gle::gleArrowTail(fib.posEndP(), -fib.dirEndP(), width);
                break;
            case 6:
                gle::gleObject(fib.posEndP(), width, gle::gleCube1);
                break;
        }
    }
}


void Display::drawFiberLines(Fiber const& fib) const
{
    FiberDisp const*const disp = fib.prop->disp;
    GLfloat alpha = disp->color.transparency();

    if ( disp->line_style == 1 )
    {
        // display plain lines:
        lineWidth(disp->line_width);
#if ( DIM > 1 ) && REAL_IS_DOUBLE
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(DIM, GL_DOUBLE, 0, fib.data());
        glDrawArrays(GL_LINE_STRIP, 0, fib.nbPoints());
        glDisableClientState(GL_VERTEX_ARRAY);
#else
        glBegin(GL_LINE_STRIP);
        for ( unsigned ii = 0; ii < fib.nbPoints(); ++ii )
            gle::gleVertex(fib.posP(ii));
        glEnd();
#endif
    }
    else if ( disp->line_style == 2 )
    {
        gle_color col = fib.disp->color;
        // display segments with color indicating internal tension
        lineWidth(disp->line_width);
        glBegin(GL_LINES);
        for ( unsigned ii = 0; ii < fib.lastPoint(); ++ii )
        {
            // the Lagrange multipliers are negative under compression
            real x = fib.tension(ii) / disp->tension_scale;
#if ( 1 )
            // adjust transparency, to make tense fibers more visible:
            if ( x <= 0 )
                col.inverted().load(-x);  // invert color for compression
            else
                col.load(x);  // extension
#else
            // use rainbow coloring, where Lagrange multipliers are negative under compression
            gle_color::jet_color(1-x, alpha).load();
#endif
            gle::gleVertex(fib.posP(ii));
            gle::gleVertex(fib.posP(ii+1));
        }
        glEnd();
    }
    else if ( disp->line_style == 3 )
    {
        // display segments with color indicating the curvature
        lineWidth(disp->line_width);
        glBegin(GL_LINE_STRIP);
        if ( fib.nbPoints() > 2 )
            gle_color::jet_color(fib.curvature(1), alpha).load();
        else
            gle_color::jet_color(0, alpha).load();
        gle::gleVertex(fib.posP(0));
        for ( unsigned ii = 1; ii < fib.lastPoint(); ++ii )
        {
            gle_color::jet_color(fib.curvature(ii), alpha).load();
            gle::gleVertex(fib.posP(ii));
        }
        gle::gleVertex(fib.posP(fib.lastPoint()));
        glEnd();
    }
    else if ( disp->line_style == 4 )
    {
        // color according to the angle with respect to the XY-plane:
        lineWidth(disp->line_width);
        glBegin(GL_LINES);
        for ( unsigned n = 0; n < fib.lastPoint(); ++n )
        {
            gle::radial_color(fib.dirSegment(n)).load();
            gle::gleVertex(fib.posP(n));
            gle::gleVertex(fib.posP(n+1));
        }
        glEnd();
    }
}


void Display::drawFiberLinesT(Fiber const& fib, unsigned i) const
{
    FiberDisp const*const disp = fib.prop->disp;
    
    fib.disp->color.load_load();
    // display plain lines:
    lineWidth(disp->line_width);
    
    glBegin(GL_LINES);
    gle::gleVertex(fib.posP(i));
    gle::gleVertex(fib.posP(i+1));
    glEnd();
}


void Display::drawFiberSpeckles(Fiber const& fib) const
{
    FiberDisp const*const disp = fib.prop->disp;
    
    // display random speckles:
    if ( disp->speckle_style == 1 )
    {
        /*
         A simple random number generator seeded by fib.signature()
         is used to distribute points always at the same position
         with respect to the lattice of each fiber.
         */
        pointSize(disp->speckle_size);
        glBegin(GL_POINTS);
        
        const real spread = disp->speckle_interval;
        const real S = 0x1p-32;
        // draw speckles below the origin of abscissa
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
                gle::gleVertex(fib.pos(a));
                z = lcrng2(z);
                a += spread * log(z*S);
            }
        }
        // draw speckles above the origin of abscissa
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
                gle::gleVertex(fib.pos(a));
                z = lcrng1(z);
                a -= spread * log(z*S);
            }
        }
        glEnd();
    }
    else if ( disp->speckle_style == 2 )
    {
        // display regular speckles
        pointSize(disp->speckle_size);
        glBegin(GL_POINTS);
        //we distribute points regularly along the center line
        const real grad = disp->speckle_interval;
        real ab = grad * ceil( fib.abscissaM() / grad );
        while ( ab <= fib.abscissaP() ) {
            gle::gleVertex( fib.pos(ab) );
            ab += grad;
        }
        glEnd();
    }
}


void Display::drawFiberPoints(Fiber const& fib) const
{
    FiberDisp const*const disp = fib.prop->disp;

    if ( disp->point_style == 1 )
    {
        // display vertices:
        pointSize(disp->point_size);
        glBegin(GL_POINTS);
        for ( unsigned ii = 0; ii < fib.nbPoints(); ++ii )
            gle::gleVertex( fib.posP(ii) );
        glEnd();
    }
    else if ( disp->point_style == 2 )
    {
        // display arrowheads along the fiber:
        const real siz = disp->point_size*sFactor;
        const real sep = disp->point_interval;
        real ab = ceil(fib.abscissaM()/sep) * sep;
        for ( ; ab <= fib.abscissaP(); ab += sep )
            gle::gleCone(fib.pos(ab), fib.dir(ab), siz);
    }
    else if ( disp->point_style == 3 )
    {
        // display only middle of fiber:
        gle::gleObject(fib.posMiddle(), 2*disp->point_size, gle::gleSphere2B);
    }
}


void set_lattice_color(Fiber const& fib, FiberLattice const& lat, real val)
{
    FiberDisp const*const disp = fib.prop->disp;
    disp->color.darken( val / disp->lattice_scale ).load();
}

void set_lattice_color(Fiber const& fib, FiberLattice const& lat, real val, real len)
{
    FiberDisp const*const disp = fib.prop->disp;
    const gle_color col = disp->color;

    if ( disp->lattice_rescale )
        // use this if the lattice cells hold a quantity:
        col.darken( val * len / ( lat.unit() * disp->lattice_scale )).load();
    else // use this if the lattice cells hold a concentration:
        col.darken( val / disp->lattice_scale ).load();
}

/**
 This style uses one vertex for each site, positionned at the center of the range
 OpenGL will interpolate the colors, and each site will be covered by a gradient.
 */
void Display::drawFiberLattice1(Fiber const& fib, real width) const
{
    FiberLattice const& lat = *fib.drawableLattice();
    const real uni = lat.unit();
    const auto inf = lat.indexM();
    const auto sup = lat.indexP();
    assert_true( inf <= sup );
    
    lineWidth(width);
    glBegin(GL_LINE_STRIP);
    if ( inf == sup )
    {
        //the Fiber is entirely covered by one site!
        real len = fib.abscissaP() - fib.abscissaM();
        set_lattice_color(fib, lat, lat.data(inf), len);
        gle::gleVertex(fib.posEndM());
        gle::gleVertex(fib.posEndP());
    }
    else
    {
        // the terminal site may be truncated
        real len = lat.abscissa(inf+1) - fib.abscissaM();
        if ( len > 0 )
            set_lattice_color(fib, lat, lat.data(inf), len);
        gle::gleVertex(fib.posEndM());
        if ( uni*(inf+0.5) > fib.abscissaM() )
            gle::gleVertex(fib.pos(uni*(inf+0.5)));
        
        for ( auto h = inf+1; h < sup; ++h )
        {
            set_lattice_color(fib, lat, lat.data(h));
            gle::gleVertex(fib.pos(uni*(h+0.5)));
        }
        
        // the terminal site may be truncated
        len = fib.abscissaP() - lat.abscissa(sup);
        if ( len > 0 )
            set_lattice_color(fib, lat, lat.data(sup), len);
        if ( uni*(sup+0.5) < fib.abscissaP() )
            gle::gleVertex(fib.pos(uni*(sup+0.5)));
        gle::gleVertex(fib.posEndP());
    }
    glEnd();
}


/**
 This style, uses two vertices for each site, positionned at the extremity of the range,
 and each site is entirely covered by the color corresponding to the value.
 */
void Display::drawFiberLattice2(Fiber const& fib, real width) const
{
    FiberLattice const& lat = *fib.drawableLattice();
    const real uni = lat.unit();
    const auto inf = lat.indexM();
    const auto sup = lat.indexP();
    assert_true( inf <= sup );
    
    lineWidth(width);
    glBegin(GL_LINE_STRIP);
    if ( inf == sup )
    {
        //the Fiber is entirely covered by one site!
        real len = fib.abscissaP() - fib.abscissaM();
        set_lattice_color(fib, lat, lat.data(inf), len);
        gle::gleVertex(fib.posEndM());
        gle::gleVertex(fib.posEndP());
    }
    else
    {
        // the terminal site may be truncated
        real len = lat.abscissa(inf+1) - fib.abscissaM();
        if ( len > 0 )
            set_lattice_color(fib, lat, lat.data(inf), len);
        gle::gleVertex(fib.posEndM());
        
        for ( auto h = inf+1; h < sup; ++h )
        {
            gle::gleVertex(fib.pos(uni*h));
            set_lattice_color(fib, lat, lat.data(h));
            gle::gleVertex(fib.pos(uni*h));
        }
        
        // the terminal site may be truncated
        len = fib.abscissaP() - lat.abscissa(sup);
        if ( len > 0 )
            set_lattice_color(fib, lat, lat.data(sup), len);
        gle::gleVertex(fib.pos(uni*sup));
        gle::gleVertex(fib.posEndP());
    }
    glEnd();
}


/**
 Indicate the edges between sites with small dots
 */
void Display::drawFiberLatticeEdges(Fiber const& fib, real size) const
{
    FiberLattice const& lat = *fib.drawableLattice();
    const real uni = lat.unit();
    const auto inf = lat.indexM();
    const auto sup = lat.indexP();

    fib.disp->color.load();
    pointSize(size);
    glBegin(GL_POINTS);
    for ( auto h = inf+1; h <= sup; ++h )
        gle::gleVertex(fib.pos(uni*h));
    glEnd();
}


void Display::drawFiberLabels(Fiber const& fib, void* font) const
{
    FiberDisp const*const disp = fib.prop->disp;
    char str[32];

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    if ( disp->label_style & 1 )
    {
        // draw fiber name and vertex indices
        int n = snprintf(str, sizeof(str), " %u ", fib.identity());
        for ( unsigned ii = 0; ii < fib.nbPoints(); ++ii )
        {
            snprintf(str+n, sizeof(str)-n, "%i", ii);
            gle::gleDrawText(fib.posP(ii), str, font);
        }
    }
    if ( disp->label_style & 2 )
    {
        // draw fiber name and abscissa value at vertices
        int n = snprintf(str, sizeof(str), " %u ", fib.identity());
        for ( unsigned ii = 0; ii < fib.nbPoints(); ++ii )
        {
            snprintf(str+n, sizeof(str)-n, "%.3f", fib.abscissaPoint(ii));
            gle::gleDrawText(fib.posP(ii), str, font);
        }
    }
    if ( disp->label_style & 4 )
    {
        // display integral abscissa along the fiber
        snprintf(str, sizeof(str), "%.3f", fib.abscissaM());
        gle::gleDrawText(fib.posEndM(), str, font);
        
        int s = (int)ceil(fib.abscissaM());
        int e = (int)floor(fib.abscissaP());
        for ( int a = s; a <= e; ++a )
        {
            snprintf(str, sizeof(str), "%i", a);
            gle::gleDrawText(fib.pos(a), str, font);
        }
        
        snprintf(str, sizeof(str), "%.3f", fib.abscissaP());
        gle::gleDrawText(fib.posEndP(), str, font);
    }
    glPopAttrib();
}


/// display forces acting on the vertices, with lines
void Display::drawFiberForces(Fiber const& fib, real scale) const
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for ( unsigned ii = 0; ii < fib.nbPoints(); ++ii )
    {
        Vector p = fib.posP(ii);
        Vector q = p + scale * fib.netForce(ii);
        gle::gleVertex(p);
        gle::gleVertex(q);
    }
    glEnd();
    glPopAttrib();
}


/// used for drawFilament
inline void drawMonomer(Vector3 const& pos, real rad)
{
    gle::gleObject(pos, rad, gle::gleSphere2B);
}


/**
 This renders a protofilament by drawing spheres of alternating colors,
 along the backbone of the `Fiber` at distance 4nm from each other.
 */
void Display::drawFilament(Fiber const& fib,
                           gle_color const& color1,
                           gle_color const& color2,
                           gle_color const& colorE) const
{
    // axial translation between two sucessive monomers:
    const real dab = 0.004;
    // enlarge radius of monomers to make them overlap
    const real rad = 0.65 * dab;
    
    real ab = 0;
    
    glEnable(GL_CLIP_PLANE4);
    
    int cnt = 0;
    // increment until we reach the MINUS_END
    while ( ab <= fib.abscissaM() )
    {
        ++cnt;
        ab += dab;
    }
    Vector3 p(fib.pos(ab)), q;
    // draw the monomers until the PLUS_END:
    while ( ab < fib.abscissaP() )
    {
        q = p;
        ab += dab;
        p = Vector3(fib.pos(ab));

        // use different tones to individualize the two strands:
        if ( ++cnt & 1 )
            color1.load_load();
        else
            color2.load_load();
        
        // change color for the last monomer:
        if ( ab + dab > fib.abscissaP() )
        {
            colorE.load_load();
            glDisable(GL_CLIP_PLANE4);
        }
        
        // set clipping plane with the next monomer
        gle::setClipPlane(GL_CLIP_PLANE4, normalize(q-p), (p+q)*0.5);
        
        drawMonomer(q, rad);
        
        // set cliping plane with the previous:
        gle::setClipPlane(GL_CLIP_PLANE5, normalize(p-q), (p+q)*0.5);
        
        glEnable(GL_CLIP_PLANE5);
    }
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
}


/**
 This renders 26 spheres positionned on a right-handed helix,
 making one turn every 74nm, with a max width of ~ 9nm.
 This is roughly Ken Holmes' model of F-actin:
 Nature 347, 44 - 49 (06 September 1990); doi:10.1038/347044a0
 which shows half a turn in 37nm containing 13 monomers.
 */
void Display::drawActin(Fiber const& fib,
                        gle_color const& color1,
                        gle_color const& color2,
                        gle_color const& colorE) const
{    
    // axial translation between two sucessive monomers:
    const real dab = 0.00275;
    // enlarge radius of monomers to make them overlap
    const real rad = 1.3 * dab;
    // distance from central axis to center of monomers
    real off = 0.0045 - dab;
    
    /*
     The filamentous actin structure can be considered to be a single stranded
     levorotatory helix with a rotation of 166° around the helical axis
     and an axial translation of 27.5 Å
    */
    // rotation angle between consecutive monomers
    const real dan = -166 * M_PI / 180;
    const real cs = cos(dan);
    const real sn = sin(dan);

    real ab = 0;
    Vector3 d(fib.dirEndM());  // unit tangent to centerline
    Vector3 n = fib.adjustedNormal(d);
    //std::clog << fib.reference() << " " << n << "    " << n.normSqr() << '\n';
    
    Vector3 p, q;
    
    glEnable(GL_CLIP_PLANE4);
    
    int cnt = 0;
    // rotate until we reach the MINUS_END
    while ( ab <= fib.abscissaM() )
    {
        ++cnt;
        ab += dab;
        n = d.rotateOrtho(n, cs, sn);
    }
    p = Vector3(fib.pos(ab)) + off * n;
    // draw the monomers until the PLUS_END:
    while ( ab < fib.abscissaP() )
    {
        q = p;
        ab += dab;
        d = Vector3(fib.dir(ab));
        n = d.rotateOrtho(n, cs, sn);
        p = Vector3(fib.pos(ab)) + off * n;
        
        // use different tones to individualize the two strands:
        if ( ++cnt & 1 )
            color1.load_load();
        else
            color2.load_load();

        // change color for the last monomer:
        if ( ab + dab > fib.abscissaP() )
        {
            colorE.load_load();
            glDisable(GL_CLIP_PLANE4);
        }
        
        // set clipping plane with the next monomer
        gle::setClipPlane(GL_CLIP_PLANE4, normalize(q-p), (p+q)*0.5);
        
        drawMonomer(q, rad);
        
        // set cliping plane with the previous:
        gle::setClipPlane(GL_CLIP_PLANE5, normalize(p-q), (p+q)*0.5);
        
        glEnable(GL_CLIP_PLANE5);
    }
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
}


/**
 This renders a Microtubule using spheres of alternating colors
 colorA for alpha-tubulin
 colorB for beta-tubulin
 */
void Display::drawMicrotubule(Fiber const& fib,
                              gle_color const& colorA,
                              gle_color const& colorB,
                              gle_color const& colorE) const
{
    // precalculated 3-start helical trajectory, for 13 monomers:
    //real dx[] = {0,0.000923,0.001846,0.002769,0.003692,0.004615,0.005538,0.006461,0.007384,0.008308,0.009231,0.010154,0.011077};
    // some protofilaments are shifted by 8 nm backward:
    real dx[] = {0,0.000923,0.001846,0.002769,0.003692,0.004615,0.005538,0.006461,0.007384,0.000308,0.001231,0.002154,0.003077};
    real dy[] = {0.8855,0.5681,0.1205,-0.3546,-0.7485,-0.9709,-0.9709,-0.7485,-0.3546,0.1205,0.5681,0.8855,1.0000};
    real dz[] = {-0.4647,-0.8230,-0.9927,-0.9350,-0.6631,-0.2393,0.2393,0.6631,0.9350,0.9927,0.8230,0.4647,0};

    // axial translation (one monomer)
    const real sa = 0.004;
    // axial translation (one heterodimer)
    const real dab = 0.008;
    // enlarged radius of monomers makes them overlap slighlty
    const real rad = 0.003;
    // distance from central axis to center of monomers, such that diameter is 25nm
    real off = 0.025 / 2 - rad;

    const real abmax = fib.abscissaP();
    real ab = dab * ceil( fib.abscissaM() / dab );
    Vector3 d(fib.dir(ab));   // unit tangent vector
    Vector3 n = fib.adjustedNormal(d);
    
    while ( ab+6*sa < abmax )
    {
        d = Vector3(fib.dir(ab));
        Vector3 p(fib.pos(ab));
        // adjust 'n' to keep it orthogonal to 'd':
        n = d.orthogonal(n, 1.0);
        // set two vectors orthogonal to 'd' of norm 'off':
        Vector3 e = n * off;
        Vector3 f = cross(d, e);

        colorA.load_load();
        for ( int i = 0; i < 13; ++i )
            drawMonomer(p+dx[i]*d+dy[i]*e+dz[i]*f, rad);

        colorB.load_load();
        for ( int i = 0; i < 13; ++i )
            drawMonomer(p+(sa+dx[i])*d+dy[i]*e+dz[i]*f, rad);

        ab += dab;
    }
    // at the plus-end, only draw monomers below the end
    while ( ab+sa < abmax )
    {
        d = Vector3(fib.dir(ab));
        Vector3 p(fib.pos(ab));
        // adjust 'n' to keep it orthogonal to 'd':
        n = d.orthogonal(n, 1.0);
        // set two vectors orthogonal to 'd' of norm 'off':
        Vector3 e = n * off;
        Vector3 f = cross(d, e);

        for ( int i = 0; i < 13; ++i )
        {
            if ( ab+sa+dx[i] < abmax )
            {
                colorA.load_load();
                drawMonomer(p+dx[i]*d+dy[i]*e+dz[i]*f, rad);
                if ( ab+5.2*sa+dx[i] < abmax )
                    colorB.load_load();
                else
                    colorE.load_load();
                drawMonomer(p+(sa+dx[i])*d+dy[i]*e+dz[i]*f, rad);
            }
        }
        ab += dab;
    }
}


void Display::drawFiber(Fiber const& fib)
{
    FiberDisp const*const disp = fib.prop->disp;
    int line_style = disp->line_style;
    
#if FIBER_HAS_LATTICE
    if ( fib.lattice().ready() )
    {
        // if the Lattice is displayed, do not draw backbone:
        switch ( disp->lattice_style )
        {
            case 1:
                drawFiberLattice1(fib, disp->line_width);
                line_style = 0;
                break;
            case 2:
                drawFiberLattice2(fib, disp->line_width);
                line_style = 0;
                break;
            case 3:
                drawFiberLatticeEdges(fib, disp->line_width);
                line_style = 0;
                break;
        }
    }
#endif
    
#if ( DIM == 3 )
    // full transparent style in 3D
    if ( line_style && fib.disp->color.transparent() )
    {
        for ( unsigned i = 0; i < fib.lastPoint(); ++i )
            zObjects.push_back(zObject(&fib, i));
        line_style = 0;
    }
#endif
    
    if ( line_style )
    {
        gle_color col1 = fib.disp->color;
        gle_color col2 = fib.disp->color.darken(0.625);
        gle_color colE = fib.disp->end_color[0];
        
        // adjust colors for front and back surfaces:
        col1.load_load();
        if ( fib.prop->disp->coloring )
            col1.load_back();
        else
            fib.prop->disp->back_color.load_back();
        
        if ( disp->line_style != 1 || disp->style == 0 )
            drawFiberLines(fib);
        else if ( disp->style == 1 )
            drawFilament(fib, col1, col2, colE);
        else if ( disp->style == 2 )
            drawActin(fib, col1, col2, colE);
        else if ( disp->style == 3 )
            drawMicrotubule(fib, col1, col2, colE);
    }

    if ( disp->point_style > 0 )
    {
        fib.disp->color.load_load();
        disp->back_color.load_back();
        drawFiberPoints(fib);
    }
    
    if ( disp->speckle_style > 0 )
    {
        fib.disp->color.load_load();
        disp->back_color.load_back();
        drawFiberSpeckles(fib);
    }

    // draw other fiber elements only if fiber is fully visible:
    //if ( fib.disp->visible > 0 )
    {
        if ( disp->label_style )
        {
            fib.disp->color.load(0.5);
            drawFiberLabels(fib, GLUT_BITMAP_HELVETICA_10);
        }
        
        if ( disp->end_style[1] )
        {
            fib.disp->end_color[1].load_load();
            //fib.disp->color.load_load();
            disp->back_color.load_back();
            drawFiberMinusEnd(fib, disp->end_style[1], disp->end_size[1]);
        }
        
        if ( disp->end_style[0] )
        {
            fib.disp->end_color[0].load_load();
            //fib.disp->color.load_load();
            disp->back_color.load_back();
            drawFiberPlusEnd(fib, disp->end_style[0], disp->end_size[0]);
        }
        
        if ( disp->force_scale > 0 )
        {
            disp->force_color.load();
            lineWidth(disp->point_size);
            drawFiberForces(fib, disp->force_scale);
        }
    }
}


void Display::drawFibers(FiberSet const& set)
{
#if ( 1 )
    // display Fibers in a random (ever changing) order:
    for ( Fiber const* fib = set.first(); fib ; fib=fib->next() )
#else
    // display the Fiber always in the same order:
    for( Fiber const* fib = set.firstID(); fib; fib=set.nextID(fib) )
#endif
    {
        if ( fib->disp->visible )
            drawFiber(*fib);
    }
}


//------------------------------------------------------------------------------
#pragma mark -


void Display::drawCouplesB(CoupleSet const& set) const
{
    for ( Couple * cx=set.firstAA(); cx ; cx=cx->next() )
    {
        // do not display Couple if the associated Fibers are both hidden
        if ( !cx->fiber1()->disp->visible && !cx->fiber2()->disp->visible )
            continue;
        
        // only display if bridging two anti-parallel filaments
        if ( prop->couple_select & 8  && cx->cosAngle() > 0 )
            continue;
        
        // only display if bridging two parallel filaments
        if ( prop->couple_select & 16 && cx->cosAngle() < 0 )
            continue;
        
        drawCoupleB(cx);
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void Display::drawSolids(SolidSet const& set)
{
    for ( Solid * obj = set.first(); obj; obj=obj->next() )
    {
        if ( obj->prop->disp->visible )
        {
            drawSolid(*obj);
#if ( DIM == 3 )
            if ( obj->prop->disp->color.transparent() )
            {
                for ( unsigned ii = 0; ii < obj->nbPoints(); ++ii )
                    zObjects.push_back(zObject(obj, ii));
            }
            else
#endif
            {
                for ( unsigned ii = 0; ii < obj->nbPoints(); ++ii )
                    drawSolidT(*obj, ii);
            }
        }
    }
}


void Display::drawBeads(BeadSet const& set)
{
    for ( Bead * obj = set.first(); obj; obj=obj->next() )
    {
        if ( obj->prop->disp->visible )
        {
            drawBead(*obj);
#if ( DIM == 3 )
            if ( obj->prop->disp->color.transparent() )
                zObjects.push_back(zObject(obj));
            else
#endif
                drawBeadT(*obj);
        }
    }
}


void Display::drawSpheres(SphereSet const& set)
{
    for ( Sphere * obj=set.first(); obj ; obj=obj->next() )
    {
        if ( obj->prop->disp->visible )
        {
            drawSphere(*obj);
#if ( DIM == 3 )
            if ( obj->prop->disp->color.transparent() )
                zObjects.push_back(zObject(obj));
            else
#endif
                drawSphereT(*obj);
        }
    }
}


void Display::drawOrganizers(OrganizerSet const& set)
{
    for ( Organizer * obj=set.first(); obj ; obj=obj->next() )
        drawOrganizer(*obj);
}


//------------------------------------------------------------------------------
#pragma mark -


/// display sub-part `inx` of object `obj`
void Display::zObject::draw(Display * disp) const
{
    Mecable const * mec = point_.mecable();
    switch( mec->tag() )
    {
        case Fiber::TAG:
            disp->drawFiberLinesT(*static_cast<const Fiber*>(mec), point_.point());
            break;
            
        case Solid::TAG:
            disp->drawSolidT(*static_cast<const Solid*>(mec), point_.point());
            break;
            
        case Bead::TAG:
            disp->drawBeadT(*static_cast<const Bead*>(mec));
            break;
            
        case Sphere::TAG:
            disp->drawSphereT(*static_cast<const Sphere*>(mec));
            break;
            
        default:
            std::cerr << "Internal error: unknown zObject pointer" << std::endl;
    }
}


#if ( DIM >= 3 )

/**
 This display objects in `zObjects` from back to front

 Depth-sorting is used in 3D to display transparent objects
 from the furthest to the nearest.
*/
void Display::drawTransparentObjects(Array<zObject>& list)
{
    // get current modelview transformation:
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    
    // extract axis corresponding to vertical direction:
    Vector3 vertical(mat[2], mat[6], mat[10]);
    
    for ( zObject & i : list )
        i.depth(dot(i.position(), vertical));
    
    // depth-sort objects:
    list.sort(closer);

    /*
     Enable polygon offset to avoid artifacts with objects of same size,
     particularly the ends of filaments with their tubular shaft.
     */
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    for ( zObject const& i : list )
        i.draw(this);
    
    glDisable(GL_POLYGON_OFFSET_FILL);
}

#endif


