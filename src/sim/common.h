// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 @file
 @brief Common definitions: FiberEnd, etc.
 */

#ifndef COMMON_H
#define COMMON_H


/// Designates the tip of a Fiber, but also the origin and center points
enum FiberEnd
{
    NO_END      = 0,   ///< not an end
    PLUS_END    = 1,   ///< highest abscissa = last vertex
    MINUS_END   = 2,   ///< lowest abscissa = fist vertex at index 0
    BOTH_ENDS   = 3,   ///< used to designate any of the two ends
    ORIGIN      = 7,   ///< refers to the origin of abscissa
    CENTER      = 8    ///< the mid-point between the two ends
};


/// Possible dynamic states for the tip of a Fiber [dynamic instability]
/** 
 The naming is intentionally vague and does not refer to the nature of the states,
 since their actual interpretation may be be different in different types of Fiber.
 */
enum AssemblyState
{
    STATE_WHITE  = 0,   ///<  Used to indicate a non-dynamic end
    STATE_GREEN  = 1,   ///<  First dynamic state: usually growing
    STATE_YELLOW = 2,   ///<  Intermediate dynamic state
    STATE_ORANGE = 3,   ///<  Intermediate dynamic state
    STATE_RED    = 4,   ///<  Third dynamic state: usually shrinking
    STATE_BLACK  = 7    ///<  used by cutter to indicate deletion
};


// used as function argument to define an AssemblyState
/* This is needed as ENUM are treated as signed int */
typedef unsigned state_t;


/// Possible modes of confinements
enum Confinement
{
    CONFINE_OFF         = 0,   ///< not confined
    CONFINE_INSIDE      = 1,   ///< confine vertices inside the Space
    CONFINE_OUTSIDE     = 2,   ///< confine vertices outside the Space
    CONFINE_ON          = 3,   ///< confine on the surface of the Space
    CONFINE_ALL_INSIDE  = 4,   ///< confine the entire bead inside
    CONFINE_ALL_OUTSIDE = 5,   ///< confine the entire bead outside
    CONFINE_PLUS_END    = 10,  ///< confine the PLUS_END of fibers to the surface of the Space
    CONFINE_MINUS_END   = 11,  ///< confine the MINUS_END of fibers to the surface of the Space
    CONFINE_BOTH_ENDS   = 12,  ///< confine both ends of fibers to the surface of the Space
    CONFINE_PLUS_OUT    = 13,  ///< confine the PLUS_END outside
    CONFINE_POINT       = 14,  ///< confine Point 0 of a Solid
    CONFINE_RANGE       = 15   ///< confine a range of point on Fibers
};


#endif


