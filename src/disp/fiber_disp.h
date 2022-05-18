// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef FIBER_DISP_H
#define FIBER_DISP_H

#include "real.h"
#include "assert_macro.h"
#include "gle_color.h"
#include "property.h"
#include "vector.h"


/// Display parameters for a class of Fiber
/**
 Holds the display attributes for a certain class of Fiber.
 
 There is one FiberDisp for each FiberProp.
 */
class FiberDisp : public Property
{
public:

    /// possible values for fiber:coloring
    enum ColoringModes {
        COLORING_OFF,
        COLORING_RANDOM,
        COLORING_DIRECTION,
        COLORING_MARK,
        COLORING_FLAG,
        COLORING_CLUSTER,
        COLORING_AGE
    };
    
public:
    
    /**
     @defgroup FiberDispPar Display parameters: Fibers
     @ingroup DisplayParameters
     @{
     */
    
    /// general rendering style
    /**
     Possible values of `style`:
     - 0 or 'line'        : line or cylinders for style=3 (this is the default),
     - 1 or 'actin'       : actin-like rendering using beads for monomers,
     - 2 or 'microtubule' : microtubule-like rendering using beads for monomers.
     .
     */
    int          style;
    
    /// visibility flag
    int          visible;
    
    /// color of fiber
    gle_color    color;
    
    /// color of inner surfaces of cylinder in 3D display (set as color[1])
    gle_color    back_color;
    
    /// color for unselected objects, default=invisible (set as color[2])
    gle_color    hide_color;

    /// if true, vary the colors used to display the fibers
    /**
     This option is used to attribute a different color to each fiber,
     painting all the segments of the fiber with the same color.
     
     Effects of `coloring`:
     - 0 : no coloring,
     - 1 : color fibers randomly,
     - 2 : color fibers depending on direction, relative to `right_direction`,
     - 3 : color fibers depending on the `mark`,
     - 4 : color clusters defined by couple-connectivity,
     - 5 : color fibers according to age.
     .
     */
    int          coloring;

    /// width of lines (also known as `line[0]` or `width`)
    float        line_width;

    /// style for lines (also known as `line[1]`)
    /**
     Possible values of `line_style`:
     - 0 : hide,
     - 1 : plain lines,
     - 2 : color rendering of longitudinal tensions,
     - 3 : color rendering of local curvature,
     - 4 : color rendering of the angular orientation relative to the X-axis
     .
     */
    int          line_style;
    
    /// if true, close the end of the fiber (valid only for style==3)
    /**
     Possible values of `line_caps`:
     - 0: leave fibers open (unfinished),
     - 1: use a disc to make a flat end,
     - 2: use a hemisphere to make a round end.
     This is only valid for style==3
     */
    int          line_caps;
    
    /// diameter of points (also known as `point[0]` or `size`)
    /**
     `point_size` and `line_width` are normally set in pixels, 
     but if `display`:point_value is set, their value is understood 
     in multiples of `point_value`, which itself is a distance.
     
     For example, if you set line_width=2.5 and point_value=0.01,
     the fibers will be displayed with a diameter of 0.025.
     */
    float        point_size;
    
    /// style for display of points (also known as `point[1]`)
    /**
     Possible values for `point_style`:
     - 0 : show nothing,
     - 1 : show vertices,
     - 2 : show arrowheads separated by `point_interval`,
     - 3 : show middle point of each fiber
     .
     */
    int          point_style;
    
    /// distance between arrows for `point_style=2` (also known as `point[2]`)
    real         point_interval;
    

    /// style of fiber tips for { PLUS_END, MINUS_END }
    /**
     `end_style[0]` determines the style of the PLUS_END,
     and `end_style[1]` the style of the MINUS_END.
     
     Possible end_style:
     - 0 : hide,
     - 1 : display a disc/sphere,
     - 2 : display a cone,
     - 3 : display a disc,
     - 4 : draw arrowhead,
     - 5 : draw arrowhead in the inverted direction (for actin)
     .
     */
    int          end_style[2];
    
    /// size of fiber tips for { PLUS_END, MINUS_END }
    /**
     You can also specify:
     
         plus_end  = SIZE, STYLE
         minus_end = SIZE, STYLE
         
     */
    real         end_size[2];
    
    /// colors of the different FiberTip states
    /**
     This determines the set of color that are used to display the fiber tips,
     according to their assembly state (Fiber::dynamicState):
     - static ends (dynamic-state 0) use end_color[0],
     - growing end (dynamic-state 1), use end_color[1],
     - shrinking end (dynamic-state 4), use end_color[4]
     .
     */
    gle_color    end_color[5];
    
    
    /// if true, specify the style for displaying lattice content (also known as `lattice[0]`)
    int          lattice_style;
    
    /// defines the range of colors when displaying the lattice (also known as `lattice[1]`)
    real         lattice_scale;
    
    /// rescale concentration for the cells at the edge of reduced length
    bool         lattice_rescale;
    
    /// style of labels
    /**
     Possible `label_style`:
     - 0 : hide,
     - 1 or 2 : name of fiber and index of vertices
     - 4 : abscissa along fiber
     .
     */
    int          label_style;
    
    
    /// size for speckle display (also know as `speckles`)
    int          speckle_size;

    /// style for speckle display (also know as `speckles[1]`)
    /**
     Possible values for `speckle_style`:
     - 0 : hide,
     - 1 : random speckles, separated on average by `interval`,
     - 2 : regular speckes, separated by `interval`.
     .
     */
    int          speckle_style;

    /// average distance between speckles (also known as `speckles[2]`)
    real         speckle_interval;

    /// a bit-field to hide certain categories of fibers
    /**
     Possible values for `exclude`:
     - 0 : all fibers are displayed,
     - 1 : show only right-pointing fibers,
     - 2 : show only left-pointing fibers,
     - 4 : show only counter-clockwise fibers,
     - 8 : show only clockwise fibers.
     .
     
     You may also address each bit directly, knowning that:
     - 1st bit true: hide left-pointing fibers
     - 2nd bit true: hide right-pointing fibers
     - 3rd bit true: hide clockwise fibers
     - 4th bit true: hide counter-clockwise fibers
     .
     */
    int          exclude;
    
    /// the direction used for hiding left- or right-pointing fibers, etc. (known as `exclude[1]`)
    Vector       exclude_axis;
    
    
    
    /// number of bits equal to `1` in the mask_bitfield
    /**
     This parameter can be used to hide a fraction of the fiber.
     Each fiber will be visible with a probability `1/2^mask`.
     `mask_bitfield' is set randomly with `mask` bits set to 1, 
     When the parameter is read.
     */
    unsigned int mask;
    
    /// selection bitfield used to hide some fibers (known as `mask[1]`)
    /**
     `mask_bitfield` is a 32-bitfield that is compared to the signature of the object,
     itself a random bitfield. The Object is hidden if the result is non-zero.
     So if `mask_bitfield` has many 1s, fewer filaments will be visible.
     Note that `mask_bitfield' is set randomly if `mask` is given.
     */
    unsigned int mask_bitfield;
    
    
    /// conversion coefficient from tension to color, for `line_style==2`
    /**
     The values of `tension_scale` determines how longitudinal tensions are displayed:
     - tension_scale < 0 : compressive forces are highlighted,
     - tension_scale > 0 : tensile forces are highlighted.
     .
     A longitudinal tension equal to `tension_scale` will be displayed with a blue tint,
     while a value three times higher will be displayed red.
     Lower tension_scale values will yield brighter colors for the same force in the fiber.
     */
    real         tension_scale;
    
    /// ( if > 0 ) display the net forces FP acting on vertices
    /**
     A force F acting on a vertex is displayed as segments of length `force_scale * F`.
     A color can be specified as forces[1]
     */
    real         force_scale;
    
    /// this color is specified as forces[1]
    gle_color    force_color;

    
    /// the 'explosion' effect shift the fibers in space
    /**
     This can be useful to visualize dense regions,
     but is only implemented for style=2
     */
    int          explode;
    
    /// amount of lateral shift to separate fibers when display is exploded (known as `explode[1]`)
    real         explode_range;
    
    
    /// if true, display the average fiber
    /**
     The 'average fiber' is calculated from the centroid of the fiber tips,
     and the centroid of the polymer mass.
     It is useful to evaluate the amount of order in the network.
     */
    int          draw_average;

    /// @}

public:
    
    /// constructor
    FiberDisp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~FiberDisp() { }
    
    /// identifies the property
    std::string category() const { return "fiber:display"; }

    /// clear to default values
    void clear();
    
    /// set from glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new FiberDisp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
};


#endif

