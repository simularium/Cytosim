// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef POINT_DISP_H
#define POINT_DISP_H

#include "gle_color.h"
#include "property.h"
#include "gle.h"



/// the parameters necessary to display a point-like object
/**
 This is used by Hands, Sphere, Solid, Beads
 Note that these parameters may be interpreted differently when displaying different classes. 
 For example `coloring` is only implemented for Sphere, Beads but not for Hands,
 while `shape` and `symbol` are only implemented for Hands.
 */
class PointDisp : public Property
{    
    /// used to differentiate between different uses of the class
    std::string mKind;
    
    /// draw outline of shape
    void strokeShape() const;
    
    /// draw surface of shape
    void paintShape() const;

    /// draw active state with OpenGL vector primitives
    void strokeA() const;
    
    /// draw inactive state with OpenGL vector primitives
    void strokeI() const;

public:
    
    /**
     @defgroup PointDispPar Display parameters: Points
     @ingroup DisplayParameters
     @{
     */
    
    
    /// visibility flag : 0=hidden, 1=opaque
    /**
     For a Space, bits are used to enable the display of front/back sides,
     such that one can use these values:
     - 1 : display only faces that are facing outside,
     - 2 : display only faces that are facing inside,
     - 3 : display both sides.
     .
     */
    int        visible;
    
    /// color of object (in 3D display, the color of outer surfaces)
    gle_color  color;
    
    /// second color (set as color[1])
    /**
     This is used to display unattached Single and unbridging Couple, 
     and the inner surfaces of objects such as Sphere, Solid, Bead and Space.
     If it is not defined, `color2` is set to be a darker tone of `color`.
     */
    gle_color  color2;

    /// if true, attribute random colors to individual objects
    int        coloring;
    
    /// display diameter of points in pixel units
    float      size;
    
    /// display width of lines
    float      width;
    
    /// 'c' for circle, 'h' for hexagon, 's' for star, etc.
    char       shape;
    
    /// a bitfield to set different display options
    int        style;
    
    /// character displayed (do not set, or set as 0 to disable this feature)
    char       symbol;
    
    /// color of symbol (set as symbol[1])
    gle_color  symbol_color;
    
    /// @}
    
    /// visible and big enough to be seen
    bool       perceptible;
    
    /// this is the size in real unit
    float      realSize;
    
public:
    
    /// constructor
    PointDisp(const std::string& k, const std::string& n);
    
    /// copy constructor
    PointDisp(PointDisp const&);
    
    /// copy assignment
    PointDisp& operator =(PointDisp const&);

    /// destructor
    ~PointDisp();
    
    /// identifies the property
    std::string category() const { return mKind; }

    /// clear to default values
    void clear();
    
    /// set from glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new PointDisp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
    /// recalculate bitmaps
    void      prepare(GLfloat uf, GLfloat sf, bool make_maps);
    
    /// draw inactive state
    template < typename VECTOR >
    void drawI(VECTOR const& pos) const
    {
        if ( perceptible )
        {
            glPushMatrix();
            gle::gleTranslate(pos);
            gle::gleScale(realSize);
            color2.load();
            gle::gleDisc();
            glPopMatrix();
        }
    }

    /// draw active state, unattached
    template < typename VECTOR >
    void drawF(VECTOR const& pos) const
    {
        if ( perceptible )
        {
            glPushMatrix();
            gle::gleTranslate(pos);
            gle::gleScale(realSize);
            color2.load();
            strokeA();
            glPopMatrix();
        }
    }

    /// draw active state, attached
    template < typename VECTOR >
    void drawA(VECTOR const& pos) const
    {
        if ( perceptible )
        {
            glPushMatrix();
            gle::gleTranslate(pos);
            gle::gleScale(realSize);
            color.load();
            strokeA();
            glPopMatrix();
        }
    }

};


#endif
