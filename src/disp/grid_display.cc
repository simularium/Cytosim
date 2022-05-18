// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "grid_display.h"


/**
 This uses the current OpenGL color and line width.
 */
void drawEdges(GridBase<1> const& grid)
{
    const real u =  0.5;
    const real d = -0.5;
    glBegin(GL_LINES);
    for ( real ix = 0; ix <= grid.breadth(0); ++ix )
    {
        real x = grid.position(0, ix);
        gle::gleVertex(x, d);
        gle::gleVertex(x, u);
    }
    glEnd();
}


/**
 This uses the current OpenGL color and line width.
 */
void drawEdges(GridBase<2> const& grid)
{
    real i = grid.inf(0);
    real s = grid.sup(0);
    glBegin(GL_LINES);
    for ( float iy = 0; iy <= grid.breadth(1); ++iy )
    {
        real y = grid.position(1, iy);
        gle::gleVertex(i, y);
        gle::gleVertex(s, y);
    }
    glEnd();
    
    i = grid.inf(1);
    s = grid.sup(1);
    glBegin(GL_LINES);
    for ( float ix = 0; ix <= grid.breadth(0); ++ix )
    {
        real x = grid.position(0, ix);
        gle::gleVertex(x, i);
        gle::gleVertex(x, s);
    }
    glEnd();
}


/**
 This uses the current OpenGL color and line width.
 */
void drawEdges(GridBase<3> const& grid)
{
    real i = grid.inf(0);
    real s = grid.sup(0);
    glBegin(GL_LINES);
    for ( float iy = 0; iy <= grid.breadth(1); ++iy )
    for ( float iz = 0; iz <= grid.breadth(2); ++iz )
    {
        real y = grid.position(1, iy);
        real z = grid.position(2, iz);
        gle::gleVertex(i, y, z);
        gle::gleVertex(s, y, z);
    }
    glEnd();
    
    i = grid.inf(1);
    s = grid.sup(1);
    glBegin(GL_LINES);
    for ( float ix = 0; ix <= grid.breadth(0); ++ix )
    for ( float iz = 0; iz <= grid.breadth(2); ++iz )
    {
        real x = grid.position(0, ix);
        real z = grid.position(2, iz);
        gle::gleVertex(x, i, z);
        gle::gleVertex(x, s, z);
    }
    glEnd();
    
    i = grid.inf(2);
    s = grid.sup(2);
    glBegin(GL_LINES);
    for ( float ix = 0; ix <= grid.breadth(0); ++ix )
    for ( float iy = 0; iy <= grid.breadth(1); ++iy )
    {
        real x = grid.position(0, ix);
        real y = grid.position(1, iy);
        gle::gleVertex(x, y, i);
        gle::gleVertex(x, y, s);
    }
    glEnd();
}

