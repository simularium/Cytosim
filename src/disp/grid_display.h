// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created 09/03/2015 by Francois Nedelec

#ifndef GRID_DISPLAY_H
#define GRID_DISPLAY_H

#include "vector1.h"
#include "vector2.h"
#include "vector3.h"
#include "grid_base.h"
#include "grid.h"
#include "opengl.h"
#include "gle.h"


/// display the edges of a 1D grid using OpenGL
void drawEdges(GridBase<1> const& grid);


/// display the edges of a 2D grid using OpenGL
void drawEdges(GridBase<2> const& grid);


/// display the edges of a 3D grid using OpenGL
void drawEdges(GridBase<3> const& grid);

//------------------------------------------------------------------------------
#pragma mark -


/// display the values stored in the cells of a 1D grid using OpenGL
/**
 OpenGL color is to be specified by the provided function:
 bool set_color(void*, CELL const&, Vector2 const&);
 Each particular cell is displayed only if `set_color' returns true.
 */
template <typename CELL>
void drawValues(Grid<CELL, 1> const& grid,
                bool set_color(void*, CELL const&, Vector1 const&),
                void* arg)
{
    float d = grid.cellWidth(0);
    float e = 2;
    
    // paint all cells one by one
    for ( typename Grid<CELL, 1>::index_t c = 0; c < grid.breadth(0); ++c )
    {
        float x = grid.position(0, c);
        if ( set_color(arg, grid[c], Vector1(x)) )
        {
            glBegin(GL_TRIANGLE_STRIP);
            gle::gleVertex(x  , -e);
            gle::gleVertex(x+d, -e);
            gle::gleVertex(x  ,  e);
            gle::gleVertex(x+d,  e);
            glEnd();
        }
    }
}


/// display the values stored in the cells of a 2D grid using OpenGL
/**
 OpenGL color is to be specified by the provided function:
 bool set_color(void*, CELL const&, Vector2 const&);
 Each particular cell is displayed only if `set_color' returns true.
*/
template <typename CELL>
void drawValues(Grid<CELL, 2> const& grid,
                bool set_color(void*, CELL const&, Vector2 const&),
                void* arg)
{
    real d = 0.5 * grid.cellWidth(0);
    real e = 0.5 * grid.cellWidth(1);
    
    // paint all cells one by one
    for ( typename Grid<CELL, 2>::index_t c = 0; c < grid.nbCells(); ++c )
    {
        Vector2 w;
        grid.setPositionFromIndex(w, c, 0.5);
        if ( set_color(arg, grid[c], w) )
        {
            glBegin(GL_TRIANGLE_STRIP);
            gle::gleVertex(w.XX-d, w.YY-e);
            gle::gleVertex(w.XX+d, w.YY-e);
            gle::gleVertex(w.XX-d, w.YY+e);
            gle::gleVertex(w.XX+d, w.YY+e);
            glEnd();
        }
    }
}


/// display the slice of a 3D grid in a plane parallel to XY at `Z = z_pos`
/**
 OpenGL color is to be specified by the provided function:
 bool set_color(void*, CELL const&, Vector2 const&);
 Each particular cell is displayed only if `set_color' returns true.
 */
template <typename CELL>
void drawValues(Grid<CELL, 3> const& grid,
                bool set_color(void*, CELL const&, Vector3 const&),
                void* arg,
                real z_pos = 0)
{
    assert_true(grid.hasCells());
    
    real d = 0.5 * grid.cellWidth(0);
    real e = 0.5 * grid.cellWidth(1);
    
    typename Grid<CELL, 3>::index_t z = grid.index(2, z_pos);
    
    for ( typename Grid<CELL, 3>::index_t y = 0; y < grid.breadth(1); ++y )
    for ( typename Grid<CELL, 3>::index_t x = 0; x < grid.breadth(0); ++x )
    {
        Vector3 w(grid.position(0, x+0.5), grid.position(1, y+0.5), z_pos);
        if ( set_color(arg, grid.icell3D(x,y,z), w) )
        {
            glBegin(GL_TRIANGLE_STRIP);
            gle::gleVertex(w.XX-d, w.YY-e, z_pos);
            gle::gleVertex(w.XX+d, w.YY-e, z_pos);
            gle::gleVertex(w.XX-d, w.YY+e, z_pos);
            gle::gleVertex(w.XX+d, w.YY+e, z_pos);
            glEnd();
        }
    }
}


/// display the slice of a 3D grid in a plane parallel to Y: `Y=pos`
/**
 OpenGL color is to be specified by the provided function:
 bool set_color(void*, CELL const&, Vector2 const&);
 Each particular cell is displayed only if `set_color' returns true.
*/
template <typename CELL>
void drawValuesXZ(Grid<CELL, 3> const& grid,
                  bool set_color(void*, CELL const&, Vector3 const&),
                  void* arg,
                  real pos)
{
    assert_true(grid.hasCells());
    
    real d = 0.5 * grid.cellWidth(0);
    real e = 0.5 * grid.cellWidth(2);
    
    typename Grid<CELL, 3>::index_t y = grid.index(1, pos);
    
    for ( typename Grid<CELL, 3>::index_t z = 0; z < grid.breadth(2); ++z )
    for ( typename Grid<CELL, 3>::index_t x = 0; x < grid.breadth(0); ++x )
    {
        Vector3 w(grid.position(0, x+0.5), pos, grid.position(2, y+0.5));
        if ( set_color(arg, grid.icell3D(x,y,z), w) )
        {
            glBegin(GL_TRIANGLE_STRIP);
            gle::gleVertex(w.XX-d, pos, w.ZZ-e);
            gle::gleVertex(w.XX+d, pos, w.ZZ-e);
            gle::gleVertex(w.XX-d, pos, w.ZZ+e);
            gle::gleVertex(w.XX+d, pos, w.ZZ+e);
            glEnd();
        }
    }
}


// display the slice of a 3D grid in a plane parallel to X: `X=pos`
/**
 OpenGL color is to be specified by the provided function:
 bool set_color(void*, CELL const&, Vector2 const&);
 Each particular cell is displayed only if `set_color' returns true.
 */
template <typename CELL>
void drawValuesYZ(Grid<CELL, 3> const& grid,
                  bool set_color(void*, CELL const&, Vector3 const&),
                  void* arg,
                  real pos)
{
    assert_true(grid.hasCells());
    
    real d = 0.5 * grid.cellWidth(0);
    real e = 0.5 * grid.cellWidth(2);
    
    typename Grid<CELL, 3>::index_t x = grid.index(0, pos);
    
    for ( typename Grid<CELL, 3>::index_t z = 0; z < grid.breadth(2); ++z )
    for ( typename Grid<CELL, 3>::index_t y = 0; y < grid.breadth(1); ++y )
    {
        Vector3 w(pos, grid.position(1, y+0.5), grid.position(2, z+0.5));
        if ( set_color(arg, grid.icell3D(x,y,z), w) )
        {
            glBegin(GL_TRIANGLE_STRIP);
            gle::gleVertex(pos, w.YY-d, w.ZZ-e);
            gle::gleVertex(pos, w.YY+d, w.ZZ-e);
            gle::gleVertex(pos, w.YY-d, w.ZZ+e);
            gle::gleVertex(pos, w.YY+d, w.ZZ+e);
            glEnd();
        }
    }
}


/// display a slice of the field in a plane perpendicular to 'dir'
/**
 OpenGL color is to be specified by the function `set_color`:
 bool set_color(void*, CELL const&, Vector2 const&);
 The return value of this function is ignored.
 */
template <typename CELL>
void drawValues(Grid<CELL, 3> const& grid,
                bool set_color(void*, CELL const&, Vector3 const&),
                void* arg,
                Vector3 const& dir,
                real z_pos)
{
    assert_true(grid.hasCells());
    
    // this defines the finesse of the triangular mesh:
    real n = 0.2 * grid.minimumWidth(1);
    int m = (int)( grid.radius() / n );
    
    Vector3 dx, dy;
    dir.orthonormal(dx, dy);
    dx *= n;
    dy *= n;
    
    Vector3 dh = dy * cos(M_PI/6);
    Vector3 w, a;
    
    for ( int y = -m; y <= m; y+=2 )
    {
        a = y * dh + z_pos * dir;
        glBegin(GL_TRIANGLE_STRIP);
        for ( int x = -m; x <= m; ++x )
        {
            w = a + x * dx;
            set_color(arg, grid.interpolate3D(w), w);
            gle::gleVertex(w);
            
            w = a + ( x + 0.5 ) * dx + dh;
            set_color(arg, grid.interpolate3D(w), w);
            gle::gleVertex(w);
        }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for ( int x = -m; x <= m; ++x )
        {
            w = a + x * dx + dh + dh;
            set_color(arg, grid.interpolate3D(w), w);
            gle::gleVertex(w);
            
            w = a + ( x + 0.5 ) * dx + dh;
            set_color(arg, grid.interpolate3D(w), w);
            gle::gleVertex(w);
        }
        glEnd();
   }
}


#endif


