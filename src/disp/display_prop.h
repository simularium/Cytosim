// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef DISPLAY_PROP_H
#define DISPLAY_PROP_H

#include "property.h"
#include "opengl.h"


/// Parameters for Play
class DisplayProp : public Property
{

public:
    
    /**
     @defgroup DisplayPar Display parameters: World
     @ingroup DisplayParameters
     @{
     */
    
    /// style of display { 1, 2, 3 }
    /**
     3 styles are implemented:
     - style 1 used OpenGL lines and points. It is suitable for 2D work.
     - style 2 is a faster display, also suitable for 2D.
     - style 3 draw real tubes and uses OpenGL lighting for rendering. It is nice for 3D.
     .
     */
    unsigned       style;

    /// if true, repeat the display along periodic boundary directions
    /**
    This is only useful is the main Space is periodic, strip or cylinderP
    In this case, the whole system will be displayed multiple times,
    shifted appropriately in the directions that are periodic.
    */
    int            tile;
    
    /// if true, translate objects to place them in the root cell for periodic boundary conditions
    int            fold;
    
    /// default diameter of points
    float          point_size;
    
    /// default width of hookean links
    float          link_width;

    /// default width of lines
    float          line_width;
    
    /// if set > 0, this defines the unit size used for `point_size` and `line_width` 
    /**
     Set this parameter to specify the fiber radius and point size in real units.

     `point_size` and `line_width` are usually set in pixels, but if `point_value` is set,
     these specifications are interpreted as multiples of `point_value`,
     which itself is given in simulation unit (i.e. real distance).
     
     For example, if you set `line_width=5` and `point_value=0.01`,
     the fibers will be displayed with a diameter of 0.050 (i.e. 50 nanometers).
     
     <em> default = 0 </em>
     */
    float          point_value;
    
    /// selection bitfield for Couples
    unsigned       couple_select;
    
    /// selection bitfield for Singles
    unsigned       single_select;
    
    /// flag to display Meca's links
    bool           draw_links;

    /// @}

public:
    
    /// constructor
    DisplayProp(const std::string& n) : Property(n) { clear(); }
    
    /// destructor
    ~DisplayProp() { }
    
    /// identifies the property
    std::string category() const { return "simul:display"; }
        
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new DisplayProp(*this); }
    
    /// write all values
    void write_values(std::ostream&) const;

};


#endif


