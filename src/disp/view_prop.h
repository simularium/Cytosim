// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VIEW_PROP_H
#define VIEW_PROP_H

#include "real.h"
#include "vector3.h"
#include "quaternion.h"
#include "property.h"
#include "gle_color.h"

///properties needed to define a view
class ViewProp : public Property
{
public:
    
    /// number of OpenGL clipping planes
    static constexpr int NB_CLIP_PLANES = 4;
    
    /// zoom factor = ratio between visible area and `view_size`
    GLfloat          zoom;
    
    /// size of area visible in the window, in sim-units (default=10)
    GLfloat          view_size;
    
    /// enables the display area to be set from the size of the simulation space
    /**
     If ( `auto_scale` > 0 ), `view_size` is set automatically to match the simulation space.
     This is on by default.
     */
    unsigned int     auto_scale;
    
    /// the point that is in the center of the window in real-world coordinates
    Vector3          focus;
    
    /// additional translation used by autoTrack
    Vector3          focus_shift;
    
    /// orientation of display
    Quaternion<real> rotation;
    
    /// flag to enable perspective view in 3D
    /**
     By default, cytosim uses a orthographic projection to view the 3D space,
     but it will use a 3D perspective if 'perspective==true'.
     This is only meaningful in 3D mode.
     */
    int              perspective;
    
    /// modifies the display to show only the front, the back or a slice of the world
    /**
     possible values are:
     - `off`    (0)
     - `front`  (1)
     - `back`   (2)
     - `slice`  (3)
     .
     */
    unsigned int     slice;

    /// color of background
    gle_color        back_color;
    
    /// color used to highlight objects
    gle_color        front_color;

    /// flag to use a double buffer for smoother rendering (default=1)
    /**
     http://en.wikipedia.org/wiki/Multiple_buffering#Double_buffering_in_computer_graphics
     */
    bool             buffered;

    /// flag to enable OpenGL depth buffer (default=1)
    /**
     This is useful for 3D rendering.
     http://en.wikipedia.org/wiki/Z-buffering
     */
    int              depth_test;
    
    /// flag to perform depth-clamp (default=false)
    /** http://www.opengl.org/registry/specs/NV/depth_clamp.txt */
    int              depth_clamp;

    /// flag to enable native device resolution on mac osx
    /**
     This works only if you use Renaud Blanch's modified GLUT
     http://iihm.imag.fr/blanch/software/glut-macosx
     */
    int              retina;
    
    /// flag to enable OpenGL stencil buffer (default=0)
    int              stencil;
    
    /// if > 0, enables OpenGL full scene anti-aliasing (default=0)
    /**
     This defines the number of samples used to build an image.
     Higher values result in nicer (but slower) display.
     http://en.wikipedia.org/wiki/Multisample_anti-aliasing
     Many graphic cards only support 8 samples max, so try 4 or 8.
     */
    int              multisample;
    
    
    /// string at start of `message` (if `none` is specified, no message is shown)
    std::string      label;

    /**
     @defgroup ViewPar Display Parameters: View
     @ingroup DisplayParameters
     @{
     */
    
    /// automatically adjust view to keep fibers in window
    /**
     Possible values:
     - 0 : off
     - 1 : translate to track the center of gravity of the cloud of fiber-points
     - 2 : rotate to align the principal direction of the fiber
     - 3 : translate and rotate ( 1 and 2 are combined )
     - 4 : rotate to align two principal directions
     - 5 : translate and rotate ( 1 and 4 are combined )
     .
     The translation defined by focus is applied after this adjustment.
     */
    unsigned int     track_fibers;
    
    /// position of window on screen (top-left corner, in pixels)
    int              window_position[2];
    
    /// desired size of window in pixels (also known as `size`)
    int              window_size[2];
    
    /// size of scale-bar in sim-world units (set as `scale_bar[0]`)
    real             scale_bar_size;
    
    /// color of scale-bar (set as `scale_bar[1]`)
    gle_color        scale_bar_color;
    
    /// display flag for scale-bar (default=0, set as `scale_bar[2]`)
    unsigned int     scale_bar_mode;

    /// display flag for displaying X-Y-Z axes
    unsigned int     draw_axes;
    
    /// length of axes (set a `draw_axes[1]`, default=1)
    real             axes_size;

    /// on/off flags for clipping (defined as `clip_plane?`)
    /**
     Up to 4 clipping planes can be defined: clip_plane0 to clip_plane3
     
     Syntax:
     
         clip_plane? = BOOL, VECTOR, REAL
     
     The Boolean enables the clipping plane.
     The plane is specified by a normal vector `n` (VECTOR) and a scalar `a` (REAL).
     The visible half-space corresponds to <em> n.pos + a > 0 </em>
     
     Example:
     To define a slice perpendicular to the X-axis of width 2: 
     
         set system display
         {
            clip_plane1 = 1,  1 0 0, 1
            clip_plane2 = 1, -1 0 0, 1
         }
     
     */
    int              clip_plane_mode[NB_CLIP_PLANES];

    /// direction perpendicular to clipping plane (defined as `clip_plane?[1]`)
    Vector3          clip_plane_vector[NB_CLIP_PLANES];
    
    /// scalar offset defining the equation of the clipping plane (defined as `clip_plane?[2]`)
    real             clip_plane_scalar[NB_CLIP_PLANES];

    
    /// characteristics of OpenGL fog (also known as `fog[0]`)
    int              fog_type;
    
    /// density of fog (also known as `fog[1]`)
    GLfloat          fog_param;
    
    /// color of fog (also known as `fog[2]`)
    gle_color        fog_color;
 
    /// @}
    
    /// text displayed in center of window
    std::string      memo;
    
    /// flag to display information on screen
    int              draw_memo;

public:
   
    /// constructor
    ViewProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~ViewProp()  { }
    
    /// identifies the property
    std::string category() const { return "view"; }
     
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new ViewProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;

};

#endif
