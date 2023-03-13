// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "simul.h"
#include "display2.h"
#include "modulo.h"

#include "fake.h"
#include "fiber_disp.h"
#include "line_disp.h"
#include "point_disp.h"

#include "opengl.h"
#include "gle.h"
#include "gle_color_list.h"
#include "glut.h"

using namespace gle;
extern Modulo const* modulo;


#define ENABLE_EXPLODE_DISPLAY ( DIM < 3 )


Display2::Display2(DisplayProp const* dp) : Display(dp)
{
}


void Display2::drawSimul(Simul const& sim)
{
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    drawFields(sim.fields);
    
    glEnable(GL_LIGHTING);
#if ( DIM > 2 )
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
#endif
    drawSpaces(sim.spaces);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    if ( prop->couple_select & 1 )
        drawCouplesF(sim.couples);

    if ( prop->single_select & 1 )
        drawSinglesF(sim.singles);
    
#if ( 0 )
    // bypass the normal display to improve performance:
    glEnableClientState(GL_VERTEX_ARRAY);
    // display Fibers in a random (ever changing) order:
    for ( Fiber * fib = sim.fibers.first(); fib ; fib=fib->next() )
    {
        if ( fib->disp->visible )
        {
            lineWidth(fib->prop->disp->line_width);
            fib->disp->color.load();
            glVertexPointer(DIM, GL_DOUBLE, 0, fib->data());
            glDrawArrays(GL_LINE_STRIP, 0, fib->nbPoints());
        }
    }
    glDisableClientState(GL_VERTEX_ARRAY);
#else
    drawFibers(sim.fibers);
#endif

    if ( prop->couple_select & 2 )
        drawCouplesA(sim.couples);
    
#if ( DIM == 3 )
    
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

#endif
    
    drawBeads(sim.beads);
    drawSolids(sim.solids);
    drawSpheres(sim.spheres);
    
#if ( DIM == 3 )
    
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

#endif

    if ( prop->couple_select & 4 )
        drawCouplesB(sim.couples);
    
    if ( prop->single_select & 2 )
        drawSinglesA(sim.singles);
    
#if ( DIM == 3 )
    
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

#endif

    drawOrganizers(sim.organizers);
    drawMisc(sim);
}

//------------------------------------------------------------------------------
#pragma mark -

void Display2::drawBall(Vector const& pos, real radius)
{
    glPushMatrix();
    gleTranslate(pos);
    gleScale(radius);
    if ( DIM == 3 )
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        gleSphere2B();
        glCullFace(GL_BACK);
        gleSphere4B();
    }
    else
        gleDiscB();
    glPopMatrix();
}


/// this version usually draws a little sphere
inline void Display2::drawPoint(Vector const& pos, PointDisp const* disp)
{
    if ( disp->perceptible )
    {
#if ( 0 )
        // draw a OpenGL point
        pointSize(disp->size);
        glBegin(GL_POINTS);
        gleVertex(pos);
        glEnd();
#else
        /// draw a little sphere
        glPushMatrix();
        gleTranslate(pos);
        gleScale(disp->size*sFactor);
        gleSphere1B();
        glPopMatrix();
#endif
    }
}

//------------------------------------------------------------------------------
#pragma mark -


void Display2::drawFiber(Fiber const& fib)
{
#if ENABLE_EXPLODE_DISPLAY
    //translate whole display to display the Fiber
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    gleTranslate(0, fib.disp->explode_shift, 0);
#endif
    
    Display::drawFiber(fib);

#if ENABLE_EXPLODE_DISPLAY
    glPopMatrix();
#endif
}

//------------------------------------------------------------------------------
#pragma mark -

void Display2::drawBead(Bead const& obj)
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
        bodyColor2(disp, obj.signature());
        lineWidth(disp->width);
        gleObject(obj.position(), obj.radius(), gleCircleB);
    }
#endif
}


/**
 Display a semi-transparent disc / sphere
 */
void Display2::drawBeadT(Bead const& obj)
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

void Display2::drawSolid(Solid const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    //display points:
    if ( disp->style & 2  &&  disp->size > 0 )
    {
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
            drawPoint(obj.posP(ii), disp);
    }
    
    //display outline of spheres
    if ( disp->style & 4 )
    {
        bodyColor2(disp, obj.signature());
        lineWidth(disp->width);
#if ( DIM == 2 )
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
        {
            if ( obj.radius(ii) > 0 )
                gleObject(obj.posP(ii), obj.radius(ii), gleCircleB);
        }
#elif ( DIM == 3 )
        //special display for ParM simulations (DYCHE)
        if ( obj.mark()  &&  obj.nbPoints() >= 3 )
            gleObject(obj.posP(0), obj.diffPoints(1, 0), obj.radius(0), gleCircleB);
#endif
    }
    
    //print the number for each Solid
    if ( disp->style & 8 )
    {
        char tmp[32];
        bodyColor2(disp, obj.signature());
        snprintf(tmp, sizeof(tmp), "%u", obj.identity());
        gleDrawText(obj.posP(0), tmp, GLUT_BITMAP_HELVETICA_10);
    }
    
    //draw polygon around vertices of Solid
    if ( disp->style & 16 )
    {
        lineWidth(disp->width);
        bodyColor2(disp, obj.signature());
        glBegin(GL_LINE_LOOP);
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
            gleVertex(obj.posPoint(ii));
        glEnd();
    }
}

/**
 Display a semi-transparent disc / sphere
 */
void Display2::drawSolidT(Solid const& obj, unsigned int ii)
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

void Display2::drawSphere(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    //display center and surface points
    if ( disp->style & 2  &&  disp->perceptible )
    {
        bodyColor(disp, obj.signature());
        drawPoint(obj.posP(0), disp);
        for ( unsigned ii = obj.nbRefPoints; ii < obj.nbPoints(); ii++ )
            drawPoint(obj.posP(ii), disp);
    }
    
    //display reference points
    if ( disp->style & 8  &&  disp->perceptible )
    {
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 1; ii < obj.nbRefPoints; ii++ )
            drawPoint(obj.posP(ii), disp);
    }
}


void Display2::drawSphereT(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    if ( disp->style & 5 )
    {
        bodyColorT(disp, obj.signature());
        lineWidth(disp->width);
        
#if ( DIM < 3 )
        
        if ( disp->style & 1 )
            gleObject(obj.posP(0), obj.radius(), gleDiscB);
        else
            gleObject(obj.posP(0), obj.radius(), gleCircleB);
        
#else
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        /* Note: The rotation matrix for the sphere calculated below from the
         reference points, includes scaling by the radius of the sphere.
         We then use a primitive for a sphere of radius 1.
         */
        const Vector C = obj.posP(0);
        gleTransRotate(obj.posP(1)-C, obj.posP(2)-C, obj.posP(3)-C, C);
        
        //draw transparent envelope
        if ( disp->style & 1 )
            gleDualPass(gleSphere4B);
        if ( disp->style & 4 )
        {
            disp->color2.load_front();
            gleThreeBands(64);
        }
        glPopMatrix();
        
#endif
    }
}

//------------------------------------------------------------------------------
void Display2::drawOrganizer(Organizer const& obj) const
{
    PointDisp const* disp = obj.disp();
    
    if ( !disp )
        return;

    if ( disp->style & 2 )
    {
        Vector P, Q;
        glDisable(GL_LIGHTING);

        bodyColor2(disp, obj.signature());
        pointSize(0.75f*disp->size);
        glBegin(GL_POINTS);
        for ( size_t i = 0; obj.getLink(i, P, Q); ++i )
            gleVertex(P);
        glEnd();

        bodyColor2(disp, obj.signature());
        lineWidth(disp->width);
        glBegin(GL_LINES);
        for ( size_t i = 0; obj.getLink(i, P, Q); ++i )
        {
            if ( modulo ) modulo->fold(Q, P);
            gleVertex(P);
            gleVertex(Q);
        }
        glEnd();
    }

    /**
     This display the Solid connecting two Aster as a spindle.
     Used for Cleo Kozlowski simulation of C. elegans
     */
    if ( disp->style & 1 && obj.tag() == Fake::TAG )
    {
        Solid const* so = static_cast<const Fake&>(obj).solid();
        if ( so && so->nbPoints() >= 4 )
        {
            bodyColor(disp, obj.signature());
#if ( DIM == 3 )
            glEnable(GL_LIGHTING);
            glPushMatrix();
            Vector3 a = 0.5*(so->posP(0) + so->posP(2));
            Vector3 b = 0.5*(so->posP(1) + so->posP(3));
            gleTransAlignZ(a, b, 1);
            gleDualPass(gleBarrel1);
            glPopMatrix();
            glDisable(GL_LIGHTING);
#else
            glBegin(GL_LINES);
            for ( unsigned ii = 0; ii < so->nbPoints(); ++ii )
                gleVertex(so->posPoint(ii));
            glEnd();
#endif
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -


inline void drawVertex(Vector const& pos, const PointDisp* disp)
{
    if ( disp->perceptible )
    {
        disp->color2.load();
        gleVertex(pos);
    }
}

#if ENABLE_EXPLODE_DISPLAY

inline void shiftedVertex(Vector const& pos, const Fiber * fib)
{
    real shift = fib->disp->explode_shift;
#if ( DIM == 3 )
    gle::gleVertex(pos.XX, pos.YY+shift, pos.ZZ);
#elif ( DIM == 2 )
    gle::gleVertex(pos.XX, pos.YY+shift);
#else
    gle::gleVertex(pos.XX, shift);
#endif
}

inline void drawVertex(Vector const& pos, const Fiber * fib, const PointDisp* disp)
{
    if ( disp->perceptible && fib->disp->visible )
    {
        disp->color.load();
        shiftedVertex(pos, fib);
    }
}


inline void drawVertex2(Vector const& pos, Fiber const* fib, PointDisp const* disp)
{
    if ( disp->perceptible && fib->disp->visible )
    {
        disp->color2.load();
        shiftedVertex(pos, fib);
    }
}

inline void drawLink(Vector const& a, Fiber const* fib, PointDisp const* disp, Vector const& b)
{
    if ( disp->visible && fib->disp->visible )
    {
        disp->color.load();
        shiftedVertex(a, fib);
        disp->color2.load();
        shiftedVertex(b, fib);
    }
}


/**
 */
inline void drawLink(Vector const& a, const Fiber * fibA, const PointDisp* dispA,
                     Vector const& b, const Fiber * fibB, const PointDisp* dispB)
{
#if ( 1 )
    //draw two segments if `explode` is enabled
    if ( dispA->visible && fibA->disp->visible )
    {
        dispA->color.load();
        shiftedVertex(a, fibA);
        dispB->color.load();
        shiftedVertex(b, fibA);
    }
    if ( dispB->visible && fibB->disp->visible && fibB->prop->disp->explode )
    {
        dispA->color.load();
        shiftedVertex(a, fibB);
        dispB->color.load();
        shiftedVertex(b, fibB);
    }
#else
    if ( fibA->disp->visible || fibB->disp->visible )
    {
        //draw one segment
        dispA->color.load();
        shiftedVertex(a, fibA);
        dispB->color.load();
        shiftedVertex(b, fibB);
    }
#endif
}

#else

// define macros without spatial shift:

inline void drawVertex(Vector const& pos, const Fiber * fib, const PointDisp* disp)
{
    assert_true(fib->disp);
    if ( disp->perceptible && fib->disp->visible )
    {
        disp->color.load();
        gleVertex(pos);
    }
}


inline void drawVertex2(Vector const& pos, const Fiber * fib, const PointDisp* disp)
{
    if ( disp->perceptible && fib->disp->visible )
    {
        disp->color2.load();
        gleVertex(pos);
    }
}


inline void drawLink(Vector const& a, const Fiber * fib, const PointDisp* disp, Vector const& b)
{
    if ( disp->perceptible && fib->disp->visible )
    {
        disp->color.load();
        gleVertex(a);
        disp->color2.load();
        gleVertex(b);
    }
}

inline void drawLink(Vector const& a, const Fiber * fibA, const PointDisp* dispA,
                     Vector const& b, const Fiber * fibB, const PointDisp* dispB)
{
    if (   dispA->perceptible && dispB->perceptible
        && ( fibA->disp->visible || fibB->disp->visible ))
    {
        dispA->color.load();
        gleVertex(a);
        dispB->color.load();
        gleVertex(b);
    }
}

#endif

//------------------------------------------------------------------------------
#pragma mark -

void Display2::drawSinglesF(const SingleSet & set) const
{
    if ( prop->point_size > 0 )
    {
        pointSize(prop->point_size);
        glBegin(GL_POINTS);
        for ( Single * obj=set.firstF(); obj ; obj=obj->next() )
        {
#if ENABLE_EXPLODE_DISPLAY && ( DIM == 1 )
            obj->disp()->color2.load();
            gleVertex(obj->posFoot().XX, obj->signature() * 0x1p-28 - 4);
#else
            drawVertex(obj->posFoot(), obj->disp());
#endif
        }
        glEnd();
    }
}


void Display2::drawSinglesA(const SingleSet & set) const
{
    // display positions of Hands
    if ( prop->point_size > 0 )
    {
        pointSize(prop->point_size);
        glBegin(GL_POINTS);
        for ( Single * obj=set.firstA(); obj ; obj=obj->next() )
            drawVertex(obj->posHand(), obj->fiber(), obj->disp());
        glEnd();
    }
    
    // display links to anchor points
    if ( prop->link_width > 0 )
    {
        lineWidth(prop->link_width);
        glBegin(GL_LINES);
        for ( Single * obj=set.firstA(); obj ; obj=obj->next() )
            if ( obj->hasForce() )
            {
                Vector ph = obj->posHand();
                Vector pf = obj->posFoot();
                if ( modulo ) modulo->fold(pf, ph);
                drawLink(ph, obj->fiber(), obj->disp(), pf);
            }
        glEnd();
    }
}

//------------------------------------------------------------------------------
#pragma mark -
/**
Always display Hand1 of Couple
 */
void Display2::drawCouplesF(CoupleSet const& set) const
{
    if ( prop->point_size > 0 )
    {
        pointSize(prop->point_size);
        
        glBegin(GL_POINTS);
        for ( Couple * obj = set.firstFF(); obj ; obj=obj->next() )
        {
#if ENABLE_EXPLODE_DISPLAY && ( DIM == 1 )
            const PointDisp * disp = obj->disp1();
            if ( disp->perceptible )
            {
                disp->color2.load();
                gleVertex(obj->posFree().XX, obj->signature() * 0x1p-28 - 4);
            }
#else
            drawVertex(obj->posFree(), obj->disp1());
#endif
        }
        glEnd();
        
#if ( DIM > 1 )
        // display inactive Couples with bitmap:
        for ( Couple * obj = set.firstFF(); obj ; obj=obj->next() )
            if ( !obj->active() && obj->disp1()->perceptible )
                obj->disp1()->drawI(obj->posFree());
#endif
    }
}


void Display2::drawCouplesA(CoupleSet const& set) const
{
    if ( prop->point_size > 0 )
    {
        // display bound couples
        pointSize(prop->point_size);
        glBegin(GL_POINTS);
        
        for ( Couple * cx=set.firstAF(); cx ; cx=cx->next() )
            drawVertex2(cx->posHand1(), cx->fiber1(), cx->disp1());
        
        for ( Couple * cx=set.firstFA(); cx ; cx=cx->next() )
            drawVertex2(cx->posHand2(), cx->fiber2(), cx->disp2());
        
        glEnd();
    }
}


void Display2::drawCouplesB(CoupleSet const& set) const
{
    // display bridging couples
    if ( prop->point_size > 0 )
    {
        pointSize(prop->point_size);
        glBegin(GL_POINTS);
        for ( Couple * cx=set.firstAA(); cx ; cx=cx->next() )
        {
#if ( 0 )
            // only display if bridging two anti-parallel filaments
            if ( prop->couple_select & 8  && cx->cosAngle() > 0 )
                continue;
            // only display if bridging two parallel filaments
            if ( prop->couple_select & 16 && cx->cosAngle() < 0 )
                continue;
#endif
            drawVertex(cx->posHand1(), cx->fiber1(), cx->disp1());
            drawVertex(cx->posHand2(), cx->fiber2(), cx->disp2());
        }
        glEnd();
    }
    
    // display the link for bridging couples
    if ( prop->link_width > 0 )
    {
        lineWidth(prop->link_width);
        glBegin(GL_LINES);
        for ( Couple * cx=set.firstAA(); cx ; cx=cx->next() )
        {
#if ( 0 )
            // only display if bridging two anti-parallel filaments
            if ( prop->couple_select & 8  && cx->cosAngle() > 0 )
                continue;
            // only display if bridging two parallel filaments
            if ( prop->couple_select & 16 && cx->cosAngle() < 0 )
                continue;
#endif
            Vector P = cx->posHand1();
            Vector Q = cx->posHand2();
            if ( modulo ) modulo->fold(Q, P);
            drawLink(P, cx->fiber1(), cx->disp1(), Q, cx->fiber2(), cx->disp2());
        }
        glEnd();
    }
}

