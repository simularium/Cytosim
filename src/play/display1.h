// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DISPLAY1_H
#define DISPLAY1_H

#include "display.h"
class PointDisp;

///Cytosim display class for style=1
/**
 This is the standard 2D display.
 It implements most of the characteristics in PointDisp and FiberDisp
 */
class Display1 : public Display
{
    /// draw a fine spherical object
    void drawBall(Vector const&, real radius) const;
    
    /// draw a point
    void drawPoint(Vector const&, PointDisp const*) const;

public:
    
    ///constructor
    Display1(DisplayProp const*);
    
    ///destructor
    ~Display1() {}
    
    
    /// draw the given simulation state using OpenGL commands
    void drawSimul(Simul const&);
    
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
    void drawCoupleB(Couple const*) const;
    
    /// draw an Organizer
    void drawOrganizer(Organizer const&) const;
};

#endif

