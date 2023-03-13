// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DISPLAY2_H
#define DISPLAY2_H

#include "display.h"
class PointDisp;

///Cytosim display class for style=2
/**
 This style produces a fast 2D display.
 Some of the parameters in PointDisp are ignored.

 Point-like objects are rendered using OpenGL::Points and display-lists.
 All points are therefore displayed with the same size.
 */
class Display2 : public Display
{
    /// draw a fine spherical object
    void drawBall(Vector const&, real radius);
    
    /// draw a point
    void drawPoint(Vector const&, PointDisp const*);
    
public:
    
    ///constructor
    Display2(DisplayProp const*);
    
    ///destructor
    ~Display2() {}
    
     
    /// draw the given simulation state using OpenGL commands
    void drawSimul(Simul const&);
   
    /// draw Fibers with offset
    void drawFiber(Fiber const&);
    
    /// draw the Solids
    void drawSolid(Solid const&);
 
    /// draw the transparent part for the Solids
    void drawSolidT(Solid const&, unsigned int);
    
    /// draw a Bead
    void drawBead(Bead const&);
    
    /// draw transparent membrane of Bead
    void drawBeadT(Bead const&);
    
    /// draw a Sphere
    void drawSphere(Sphere const&);
    
    /// draw transparent membrane of Sphere
    void drawSphereT(Sphere const&);
    
    /// draw the free Singles
    void drawSinglesF(SingleSet const&) const;

    /// draw the attached Singles
    void drawSinglesA(SingleSet const&) const;

    /// draw the free Couples
    void drawCouplesF(CoupleSet const&) const;

    /// draw the attached Couples
    void drawCouplesA(CoupleSet const&) const;

    /// draw the bridging Couples
    void drawCouplesB(CoupleSet const&) const;
    
    /// draw an Organizer
    void drawOrganizer(Organizer const&) const;
};

#endif

