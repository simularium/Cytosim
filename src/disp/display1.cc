// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "simul.h"
#include "display1.h"
#include "modulo.h"

#include "fake.h"
#include "line_disp.h"
#include "point_disp.h"

#include "opengl.h"
#include "gle.h"
#include "gle_color_list.h"
#include "glut.h"

using namespace gle;
extern Modulo const* modulo;

//------------------------------------------------------------------------------

Display1::Display1(DisplayProp const* dp) : Display(dp)
{
}


void Display1::drawSimul(Simul const& sim)
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
    
    if ( prop->couple_select & 2 )
        drawCouplesA(sim.couples);
    
    if ( prop->single_select & 1 )
        drawSinglesF(sim.singles);
    
    drawFibers(sim.fibers);
    
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

void Display1::drawBall(Vector const& pos, real radius) const
{
    glPushMatrix();
    gleTranslate(pos);
    gleScale(radius);
    if ( DIM == 3 )
    {
        glCullFace(GL_FRONT);
        gleSphere2B();
        glCullFace(GL_BACK);
        gleSphere4B();
    }
    else
        gleDiscB();
    glPopMatrix();
}


/// draw a little sphere
inline void Display1::drawPoint(Vector const& pos, PointDisp const* disp) const
{
    glPushMatrix();
    gleTranslate(pos);
    gleScale(disp->size*sFactor);
    gleSphere1B();
    glPopMatrix();
}


//------------------------------------------------------------------------------
#pragma mark -

void Display1::drawBead(Bead const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    // display center:
    if ( disp->style & 2  && disp->perceptible )
    {
        bodyColor(disp, obj.signature());
        drawPoint(obj.position(), disp);
    }
    
#if ( DIM == 2 )
    // display outline:
    if ( disp->style & 4 )
    {
        lineWidth(disp->width);
        bodyColor(disp, obj.signature());
        gleObject(obj.position(), obj.radius(), gleCircleB);
    }
#endif
}


/**
 Display a semi-transparent disc / sphere
 */
void Display1::drawBeadT(Bead const& obj)
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

void Display1::drawSolid(Solid const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    //display points of the Solids
    if ( disp->style & 2  &&  disp->size > 0  && disp->perceptible )
    {
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
            drawPoint(obj.posP(ii), disp);
    }
    
    //display outline of spheres
    if ( disp->style & 4 )
    {
        lineWidth(disp->width);
        bodyColor(disp, obj.signature());
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
    
    //print the number for each solid
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
        lineWidth(disp->width);
        bodyColor(disp, obj.signature());
        glBegin(GL_LINE_LOOP);
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
            gleVertex(obj.posPoint(ii));
        glEnd();
    }
}

/**
 Display a semi-transparent disc / sphere
 */
void Display1::drawSolidT(Solid const& obj, unsigned int ii)
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

void Display1::drawSphere(Sphere const& obj)
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
    if ( disp->style & 8  &&  disp->perceptible  )
    {
        bodyColor(disp, obj.signature());
        for ( unsigned ii = 1; ii < obj.nbRefPoints; ii++ )
            drawPoint(obj.posP(ii), disp);
    }
}


void Display1::drawSphereT(Sphere const& obj)
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
void Display1::drawOrganizer(Organizer const& obj) const
{
    PointDisp const* disp = obj.disp();
    
    if ( !disp )
        return;

    if ( disp->style & 2 )
    {
        Vector P, Q;
        
        glDisable(GL_LIGHTING);
        
        for ( size_t i = 0; obj.getLink(i, P, Q); ++i )
            disp->drawI(P);

        lineWidth(disp->width);
        bodyColor(disp, obj.signature());
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

/**
 display the attached position of free singles
 */
void Display1::drawSinglesF(const SingleSet & set) const
{
    for ( Single * obj=set.firstF(); obj ; obj=obj->next() )
        obj->disp()->drawF(obj->posFoot());
}


void Display1::drawSinglesA(const SingleSet & set) const
{
    // display the Hands
    for ( Single * obj=set.firstA(); obj ; obj=obj->next() )
    {
        const PointDisp * disp = obj->disp();
        if ( disp->perceptible  &&  obj->fiber()->disp->visible )
        {
            Vector ph = obj->posHand();
            
            disp->drawA(ph);
            
            if ( obj->hasForce() && disp->width > 0 )
            {
                Vector ps = obj->sidePos();
                Vector pf = obj->posFoot();
                if ( modulo )
                {
                    modulo->fold(pf, ph);
                    modulo->fold(ps, ph);
                }
                
                disp->color.load();
#if ( DIM >= 3 )
                gleCone(pf, ph-pf, disp->width*sFactor);
#else
                gleBand(ph, disp->width*sFactor, ps, disp->width*sFactor);
                gleBand(ps, disp->width*sFactor, disp->color, pf, disp->width*sFactor, disp->color.alpha_scaled(0.5));
#endif
            }
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Always display Hand1 of the Couple.
 */
void Display1::drawCouplesF(CoupleSet const& set) const
{
    for ( Couple * cx = set.firstFF() ; cx ; cx=cx->next() )
    {
        if ( cx->active() )
            cx->disp1()->drawF(cx->posFree());
        else
            cx->disp1()->drawI(cx->posFree());
    }
}


void Display1::drawCouplesA(CoupleSet const& set) const
{
    // display bound couples
    for ( Couple * cx=set.firstAF(); cx ; cx=cx->next() )
    {
        if ( cx->fiber1()->disp->visible )
        {
            if ( cx->active() )
                cx->disp1()->drawF(cx->posHand1());
            else
                cx->disp1()->drawI(cx->posHand1());
        }
    }
    
    for ( Couple * cx=set.firstFA(); cx ; cx=cx->next() )
    {
        if ( cx->fiber2()->disp->visible )
        {
            if ( cx->active() )
                cx->disp2()->drawF(cx->posHand2());
            else
                cx->disp2()->drawI(cx->posHand2());
            
        }
    }
}


void Display1::drawCoupleB(Couple const* cx) const
{
    const PointDisp * pd1 = cx->disp1();
    const PointDisp * pd2 = cx->disp2();
    
    Vector p1 = cx->posHand1();
    Vector p2 = cx->posHand2();
    
    if ( modulo ) modulo->fold(p2, p1);
    
    if ( pd1 == pd2 )
    {
        if ( pd1->perceptible )
        {
            pd1->color.load();
#if ( DIM == 2 )
            //gleBand(p1, pd1->width*sFactor, p2, pd2->width*sFactor);
            real rad = pd1->width*sFactor;
            gleMan(p1, rad * cx->dirFiber1(), p2, rad * cx->dirFiber2());
#else
            lineWidth(pd1->width);
            glBegin(GL_LINES);
            gleVertex(p1);
            gleVertex(p2);
            glEnd();
#endif
        }
    }
    else
    {
        if ( pd1->perceptible || pd2->perceptible )
        {
#if ( DIM == 2 )
            //gleBand(p1, pd1->width*sFactor, pd1->color, p2, pd2->width*sFactor, pd2->color);
            gleMan(p1, ( pd1->width * sFactor ) * cx->dirFiber1(), pd1->color,
                   p2, ( pd2->width * sFactor ) * cx->dirFiber2(), pd2->color);
#else
            lineWidth(pd1->width);
            glBegin(GL_LINES);
            gleVertex(p1);
            gleVertex(p2);
            glEnd();
#endif
        }
    }
    
    if ( cx->active() )
    {
        pd1->drawA(p1);
        pd2->drawA(p2);
    }
    else
    {
        pd1->drawI(p1);
        pd2->drawI(p2);
    }
}

