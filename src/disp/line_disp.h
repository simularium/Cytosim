// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef LINE_DISP_H
#define LINE_DISP_H


#include "vector.h"
#include "gle_color.h"

/// Display parameters for a Line
/**
 
 LineDisp holds the display attributes for a particular Fiber,
 and accordingly, there is one LineDisp for every Fiber.
 
 A user cannot set these attributes directly. Instead,
 the values are derived from the relevant FiberDisp automatically,
 when cytosim prepares the display.
 
 For example `end_color` will be set depending on the state of
 the ends of this fiber.

*/
class LineDisp
{
    
public:

    /// visibility flag
    int          visible;
    
    /// color of body
    gle_color    color;
    
    /// colors of PLUS_END and MINUS_END
    gle_color    end_color[2];

    /// amount of lateral displacement added during display
    real         explode_shift;
    
public:
    
    /// constructor
    LineDisp() { clear(); }
    
    /// destructor
    ~LineDisp() { }

    /// set to default values
    void clear();
    
};


#endif

