// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef DISPLAY_H
#define DISPLAY_H

#include "real.h"
#include "array.h"
#include "fiber.h"
#include "mecable.h"
#include "mecapoint.h"
#include "display_prop.h"
#include "property_list.h"
#include "gle_color.h"

class Simul;
class Mecable;
class SingleSet;
class CoupleSet;
class Couple;
class FiberSet;
class Solid;
class SolidSet;
class Organizer;
class OrganizerSet;
class Space;
class SpaceSet;
class Sphere;
class SphereSet;
class Bead;
class BeadSet;
class FieldSet;
class FiberProp;
class FiberDisp;
class PointDisp;
class LineDisp;

/// defining the DISPLAY keyword enables display code in included files
#define DISPLAY 1


///Base class to display Cytosim state using OpenGL
class Display
{
protected:
    
    /**
     @brief A display element with a depth coordinate
     
     zObject is used to depth-sort and display transparent objects
     */
    class zObject
    {
        /// pointer to object
        Mecapoint  point_;
        
        /// distance to the imaging plane
        real       depth_;

    public:
        
        zObject() : depth_(0.0) { }
        
        zObject(Mecable const* m) : point_(m, 0), depth_(0.0) { }

        zObject(Mecable const* m, unsigned i) : point_(m, i), depth_(0.0) { }
        
        /// position
        Vector position() const { return point_.pos(); }
        
        /// query depth
        real   depth()    const { return depth_; }
        
        /// set depth
        void   depth(real z)    { depth_ = z; }
        
        /// display object
        void   draw(Display*) const;

    };
    
    /// function to sort zObjects according to their position 'depth'
    static int closer(const void * ap, const void * bp)
    {
        zObject const* a = (const zObject*)(ap);
        zObject const* b = (const zObject*)(bp);
        
        if ( a->depth() > b->depth() ) return  1;
        if ( a->depth() < b->depth() ) return -1;
        return 0;
    }

    /// array of transparent objects to be displayed last
    Array<zObject> zObjects;

private:
    
    /// set default value of FiberProp
    void prepareFiberDisp(FiberProp*, PropertyList&, gle_color);

    /// set values of fiber's LineDisp
    void prepareLineDisp(Fiber const*);

    template < typename T >
    void preparePointDisp(T * prop, PropertyList&, gle_color);

protected:
    
    /// the pixel size for this particular display
    real           pixelSize;
    
    /// scaling factors to convert 'size' parameter into pixels used in glPointSize() and glLineWidth()
    real           uFactor;
    
    /// scaling factors to convert 'size' parameter into real dimensions used in glScale()
    real           sFactor;
    
    
    /// flag used to calculate clusterAnalysis only once
    unsigned       fiber_prep;
    
    /// used to calculate clusterAnalysis only once
    real           prep_time;
    
    /// min and max age used to adjust color range with COLORING_AGE
    real           age_scale, age_min;
    
    /// use OpenGL stencil test:
    bool           stencil_;

public:
    
    /// associated parameters
    DisplayProp const* prop;
    
    /// constructor
    Display(DisplayProp const*);
    
    /// virtual destructor needed, as class is base to others
    virtual ~Display() {}
    
    /// display opaque internal objects using OpenGL commands
    virtual void drawSimul(Simul const&);
    
    /// enable/disable stencil usage
    void setStencil(bool s) { stencil_ = s; }

    /// set current pixel-size and the value of the point in pixels
    void setPixelFactors(GLfloat pixel_size, GLfloat uFactor);
    
    /// get ready to display
    void prepareForDisplay(Simul const&, PropertyList&);

    /// display the whole simulation
    void display(Simul const&);
    
    /// display for periodic systems
    void displayTiled(Simul const&, int nine);
    
        
    /// find an individual color
    void bodyColor(PointDisp const*, unsigned) const;
    
    /// find an individual color
    void bodyColor2(PointDisp const*, unsigned) const;

    /// find an individual color that may be transparent
    void bodyColorT(PointDisp const*, unsigned) const;

    /// set OpenGL line width
    void lineWidth(real w) const { glLineWidth(std::max((GLfloat)(w*uFactor), 0.25f)); }

    /// set OpenGL point size
    void pointSize(real w) const { glPointSize(std::max((GLfloat)(w*uFactor), 0.25f)); }
    
    
    /// draw a scalar field
    void drawFields(FieldSet const&);
    
    /// draw a Space
    void drawSpace(Space const*, bool);

    /// draw all Spaces (in 3D the back side)
    void drawSpaces(SpaceSet const&);
    
    /// draw the front-side of Spaces in 3D
    void drawTransparentSpaces(SpaceSet const&);

    
    /// draw Fiber MINUS_END
    virtual void drawFiberMinusEnd(Fiber const&, int style, real size) const;
    
    /// draw Fiber PLUS_END
    virtual void drawFiberPlusEnd(Fiber const&, int style, real size) const;

    
    /// draw Fiber linear features
    virtual void drawFiberLines(Fiber const&) const;
    
    /// draw one segment of a Fiber (used to display transparent fibers)
    virtual void drawFiberLinesT(Fiber const&, unsigned) const;

    /// actin-like rendering using a sphere to represent each monomer
    void         drawFilament(Fiber const& fib, gle_color const&, gle_color const&, gle_color const&) const;

    /// actin-like rendering using a sphere to represent each monomer
    void         drawActin(Fiber const& fib, gle_color const&, gle_color const&, gle_color const&) const;
    
    /// microtubule-like rendering using a sphere to represent each monomer
    void         drawMicrotubule(Fiber const& fib, gle_color const&, gle_color const&, gle_color const&) const;
    
    /// draw Fiber point-like features
    virtual void drawFiberPoints(Fiber const&) const;
    
    /// draw Fiber Speckles
    virtual void drawFiberSpeckles(Fiber const&) const;
   
    /// display lattice subtance using color
    virtual void drawFiberLattice1(Fiber const&, real width) const;
    
    /// display lattice subtance using color
    virtual void drawFiberLattice2(Fiber const&, real width) const;
   
    /// display lattice cell edges
    virtual void drawFiberLatticeEdges(Fiber const&, real size) const;

    /// display Labels for a Fiber
    void         drawFiberLabels(Fiber const&, void* font) const;
    
    /// display forces acting on the vertices
    void         drawFiberForces(Fiber const&, real scale) const;
    
    /// draw all features of Fiber
    virtual void drawFiber(Fiber const&);
    
    /// draw Fibers
    virtual void drawFibers(FiberSet const&);
    
    /// draw the average fiber for the pool defined by func(obj, val) == true
    void drawAverageFiber(ObjectList const&);
    
    /// draw the averaged fiber
    void drawAverageFiber1(FiberSet const&, void const*);
    
    /// draw the average for left-pointing and right-pointing fibers
    void drawAverageFiber2(FiberSet const&, void const*);

    
    /// draw a Bead
    virtual void drawBead(Bead const&) = 0;

    /// draw translucent elements of a Bead
    virtual void drawBeadT(Bead const&) = 0;
    
    /// draw the Beads
    void drawBeads(BeadSet const&);

    
    /// draw opaque elements of a Solid
    virtual void drawSolid(Solid const&) = 0;
    
    /// draw translucent elements of a Solid
    virtual void drawSolidT(Solid const&, unsigned int) = 0;
    
    /// draw the Solids
    void drawSolids(SolidSet const&);
    
    
    /// draw the Sphere
    virtual void drawSphere(Sphere const&) = 0;

    /// draw translucent elements of a Sphere
    virtual void drawSphereT(Sphere const&) = 0;
    
    /// draw the Spheres
    void drawSpheres(SphereSet const&);
    
    
    /// draw the free Singles
    virtual void drawSinglesF(SingleSet const&) const = 0;
    
    /// draw the attached Singles
    virtual void drawSinglesA(SingleSet const&) const = 0;

    /// draw the free Couples, showing Hand1
    virtual void drawCouplesF(CoupleSet const&) const = 0;
    
    /// draw the attached Couples
    virtual void drawCouplesA(CoupleSet const&) const = 0;
    
    /// draw one bridging Couple
    virtual void drawCoupleB(Couple const*) const { }

    /// draw the bridging Couples
    virtual void drawCouplesB(CoupleSet const&) const;

    /// draw Organizer
    virtual void drawOrganizer(Organizer const&) const = 0;
    
    /// draw the Organizers
    void drawOrganizers(OrganizerSet const&);


    /// draw translucent objects after depth-sorting
    void drawTransparentObjects(Array<zObject>&);

    
    /// draw additional items
    void drawMisc(Simul const&);
    
};


#endif

