// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Francois Nedelec; Created 07/03/2015. nedelec@embl.de

#ifndef GRID_BASE_H
#define GRID_BASE_H

#include "assert_macro.h"
#include "exceptions.h"
#include <cstdio>
#include <cmath>
#include "real.h"

///\def compile switch to disable support for periodic boundaries
/**
 Periodic boundaries are normally supported using a if statement called repeatedly.
 However, if GRID_HAS_PERIODIC==0, this test is ommitted, which might be faster,
 but Periodic boundaries cannot be supported henceforth.
 */
#define GRID_HAS_PERIODIC 1


///Divides a rectangle of dimensionality ORD into regular voxels
/** 
Grid<int ORD> creates a regular lattice over a rectangular
region of space of dimensionality ORD, initialized by setDimensions().

Functions are provided to convert from the real space coordinates (of type real)
into an index usable to access a one-dimensional C-array.
The cells are ordered successively, the first dimension (X) varying the fastest
i.e. cell[ii+1] will in most cases be located on the right of cell[ii], although
if cell[ii] is on the right edge, then cell[ii+1] is on the symmetric edge. 

\par Access:

Cells can be accessed in three ways:
 - Position:      a set of real       operator()( real[] ), or operator(real, real, real)
 - Index:         one integer         operator[](int index)
 - Coordinates:   a set of integer    function cell(int[]), or cell(int,int,int)
.
Valid indices are [0...nbCells()-1], where nbCells() is calculated by setDimensions().
If a position lies outside the rectangular region where the grid is defined,
index(real[]) returns the index of the closest voxel.

Functions to convert between the three types are provided:
 - index()
 - pack()
 - setCoordinatesFromIndex(),
 - setCoordinatesFromPosition()
 - setPositionFromCoordinates()
 - setPositionFromIndex()
.

\par Indices:

The grid is initialized by setDimensions(inf, sup, nbCells), which calculates:
  - cWidth[d] = ( sup[d] - inf[d] ) / nbCells[d], for d in [0, ORD[

The coordinates of a cell at position pos[] are:
  - c[d] = int(  ( pos[d] - inf[d] ) / cWidth[d] )

and its index is
  - with ORD==1: index = c[0]
  - with ORD==2: index = c[0] + nbcells[0] * c[1]
  - with ORD==3: index = c[0] + nbcells[0] * ( c[1] + nbcells[1] * c[2] )
  - etc.
.
    
For a 4x4 2D grid, the index are like this:

    12  13  14  15
    8    9  10  11
    4    5   6   7
    0    1   2   3

\par Neighborhood:

The class also provides information on which cells surround each cell:
 - createSquareRegions(range) calculates square regions of size range
   ( range==1 gives nearest neighbors ).
 - createRoundRegions(range) calculates round regions of size range
 - createSideRegions(range)
.
After calling one of the above function, getRegion(offsets, index) will set 'offsets'
to point to an array of 'index offsets' for the cell referred by 'index'.
A zero offset value (0) is always first in the list and refers to self.
In the example above:
    - for index = 0 it would return { 0 1 4 5 }
    - for index = 5 it would return { 0 -1 1 -5 -4 -3 3 4 5 }
.
You obtain the cell-indices of the neighboring cells by adding offsets[n] to 'index':
Example:

    CELL * cell = & myGrid.icell(indx);
    nb_neighbors = myGrid.getRegion(region, indx);
    for ( int n = 1; n < nb_neighbors; ++n ) 
    {
        Cell & neighbor = cell[region[n]];
        ...
    }

*/

///\todo add Grid<> copy constructor and copy assignment

template <int ORD>
class GridBase
{
public:
    
    /// the type for indices
    typedef unsigned index_t;

    /// Disabled copy constructor
    GridBase<ORD>(GridBase<ORD> const&);
    
    /// Disabled copy assignment
    GridBase<ORD>& operator=(GridBase<ORD> const&);

protected:
    
    /// allocated size of array cells[]
    index_t gAllocated;
   
    /// Total number of cells in the map; size of cells[]
    size_t  nCells;
    
    /// The number of cells in each dimension
    index_t gDim[ORD];
    
    /// Offset between two consecutive cells along each dimension
    index_t gStride[ORD];
    
    /// The position of the inferior (min) edge in each dimension
    real    gInf[ORD];
    
    /// The position of the superior (max) edge in each dimension
    real    gSup[ORD];
    
    /// true if Grid has periodic boundary conditions
    bool    gPeriodic[ORD];

    /// The size of a cell: cWidth[d] = ( gSup[d] - inf[d] ) / gDim[d]
    real    cWidth[ORD];
    
    /// cDelta[d] = 1.0 / cWidth[d]
    real    cDelta[ORD];
    
    /// gStart[d] = gInf[d] / cWidth[d]
    real    gStart[ORD];

    /// The volume occupied by one cell
    real    cVolume;
    
protected:
    
    /// return closest integer to `c` in the segment [ 0, s-1 ]
    static inline index_t imagei_periodic(index_t s, int c)
    {
        while ( c <  0 )  c += s;
        index_t u = (index_t) c;
        while ( u >= s )  u -= s;
        return u;
    }

    static inline index_t imagei_clamped(index_t s, int c)
    {
        return std::min((index_t)std::max(0, c), s-1);
        //return c <= 0 ? 0 : ( c >= s ? s-1 : c );
    }
    
    /// return closest integer to `c` in the segment [ 0, gDim[d]-1 ]
    inline index_t image(const int d, int c) const
    {
#if GRID_HAS_PERIODIC
        if ( gPeriodic[d] )
            return imagei_periodic(gDim[d], c);
        else
#endif
            return imagei_clamped(gDim[d], c);
    }


    /// return f modulo s in [ 0, s-1 ]
    static inline index_t imagef_periodic(index_t s, real f)
    {
        while ( f <  0 )  f += s;
        index_t u = (index_t) f;
        while ( u >= s )  u -= s;
        return u;
    }

    static inline index_t imagef_clamped(index_t s, real f)
    {
        if ( f > 0 )
        {
            index_t u = (index_t) f;
            //return ( u >= s ? s-1 : u );
            return std::min(u, s-1);
        }
        return 0;
    }
    
    
    /// returns  ( f - gInf[d] ) / cWidth[d]
    inline real map(const int d, real f) const
    {
        return f * cDelta[d] - gStart[d];
    }

    /// return closest integer to `c` in the segment [ 0, gDim[d]-1 ]
    inline index_t imagef(const int d, real f) const
    {
        real x = map(d, f);
#if GRID_HAS_PERIODIC
        if ( gPeriodic[d] )
            return imagef_periodic(gDim[d], x);
        else
#endif
            return imagef_clamped(gDim[d], x);
    }

    /// return closest integer to `c` in the segment [ 0, gDim[d]-1 ]
    inline index_t imagef(const int d, real f, real offset) const
    {
        real x = map(d, f) + offset;
#if GRID_HAS_PERIODIC
        if ( gPeriodic[d] )
            return imagef_periodic(gDim[d], x);
        else
#endif
            return imagef_clamped(gDim[d], x);
    }

//--------------------------------------------------------------------------
#pragma mark -
public:
    
    /// constructor
    GridBase() : gDim{0}, gInf{0}, gSup{0}, gPeriodic{false}, cWidth{0}, cDelta{0}, gStart{0}
    {
        gAllocated  = 0;
        nCells      = 0;
        regionsEdge = nullptr;
        regions     = nullptr;
        cVolume     = 0;
    }
    
    /// Free memory
    void destroy()
    {
        deleteRegions();
    }
    
    /// Destructor
    virtual ~GridBase()
    {
        destroy();
    }
    
    //--------------------------------------------------------------------------
    /// specifies the area covered by the Grid
    /**
     the edges of the area are specified in dimension `d` by 'infs[d]' and 'sups[d]',
     and the number of cells by 'nbcells[d]'.
     */
    void setDimensions(const real infs[ORD], real sups[ORD], const int nbcells[ORD])
    {
        nCells = 1;
        cVolume = 1;
        
        for ( unsigned d = 0; d < ORD; ++d )
        {
            if ( nbcells[d] <= 0 )
                throw InvalidParameter("Cannot build grid as nbcells[] is <= 0");
            
            if ( infs[d] > sups[d] )
            {
                if ( infs[d] > sups[d] + REAL_EPSILON )
                    throw InvalidParameter("Cannot build grid as sup[] < inf[]");
                sups[d] = infs[d];
            }
            
            if ( infs[d] == sups[d] )
                throw InvalidParameter("Cannot build grid as sup[] == inf[]");
            
            gStride[d] = nCells;
            nCells    *= nbcells[d];
            gDim[d]    = nbcells[d];
            gInf[d]    = infs[d];
            gSup[d]    = sups[d];
            cWidth[d]  = ( gSup[d] - gInf[d] ) / real( gDim[d] );
            // inverse of cell width:
            cDelta[d]  = real( gDim[d] ) / ( gSup[d] - gInf[d] );
            gStart[d]  = ( gInf[d] * gDim[d] ) / ( gSup[d] - gInf[d] );
            cVolume   *= cWidth[d];
        }
    }
    
    ///true if setDimensions() was called
    bool hasDimensions() const
    {
        return nCells > 0;
    }
    
    /// true if dimension `d` has periodic boundary conditions
    bool isPeriodic(int d) const
    {
#if GRID_HAS_PERIODIC
        if ( d < ORD )
            return gPeriodic[d];
#endif
        return false;
    }
    
    /// change boundary conditions
    void setPeriodic(int d, bool p)
    {
#if GRID_HAS_PERIODIC
        if ( d < ORD )
            gPeriodic[d] = p;
#else
        if ( p )
            throw InvalidParameter("grid.h was compiled without PERIODIC_SUPPORT");
#endif
    }
    
    /// true if boundary conditions are periodic
    bool isPeriodic() const
    {
#if GRID_HAS_PERIODIC
        for ( int d = 0; d < ORD; ++d )
            if ( gPeriodic[d] )
                return true;
#endif
        return false;
    }

    //--------------------------------------------------------------------------
#pragma mark -

    /// total number of cells in the map
    size_t          nbCells()           const { return nCells; }

    /// number of cells in dimensionality `d`
    index_t         breadth(int d)      const { return gDim[d]; }
    
    /// offset to the next cell in the direction `d`
    index_t         stride(int d)       const { return gStride[d]; }
    
    /// position of the inferior (left/bottom/etc) edge
    const real*     inf()               const { return gInf;    }
    real            inf(int d)          const { return gInf[d]; }
    
    /// position of the superior (right/top/etc) edge
    const real*     sup()               const { return gSup;    }
    real            sup(int d)          const { return gSup[d]; }
    
    /// the widths of a cell
    const real*     delta()             const { return cDelta;    }
    real            delta(int d)        const { return cDelta[d]; }
    
    const real*     cellWidth()         const { return cWidth;    }
    real            cellWidth(int d)    const { return cWidth[d]; }

    /// position in dimension `d`, of the cell of index `c`
    real            position(int d, real c) const { return gInf[d] + c * cWidth[d]; }
    
    /// index in dimension `d` corresponding to position `w`
    int             index(int d, real w) const { return (int)map(d, w); }
    
    /// the volume of a cell
    real            cellVolume()        const { return cVolume; }

    /// the length of the diagonal of a cell = sqrt( sum(cWidth[d]^2) )
    real diagonalLength() const
    {
        real res = cWidth[0] * cWidth[0];
        for ( unsigned int d = 1; d < ORD; ++d )
            res += cWidth[d] * cWidth[d];
        return sqrt( res );
    }
    
    /// the smallest cell width, along dimensions that have more than `min_size` cells
    real minimumWidth(unsigned int min_size) const
    {
        real res = 0;
        for ( unsigned int d = 0; d < ORD; ++d )
            if ( gDim[d] > min_size )
                res = cWidth[d];
        for ( unsigned int d = 0; d < ORD; ++d )
            if ( gDim[d] > min_size  &&  cWidth[d] < res )
                res = cWidth[d];
        return res;
    }
    
    /// radius of the minimal sphere placed in (0,0,0) that entirely covers all cells
    real radius() const
    {
        real res = 0;
        for ( unsigned d = 0; d < ORD; ++d )
        {
            real m = std::max(gSup[d], -gInf[d]);
            res += m * m;
        }
        return sqrt(res);
    }

    //--------------------------------------------------------------------------
#pragma mark - Conversion

    /// checks if coordinates are inside the box
    bool inside(const int coord[ORD]) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
        {
            if ( coord[d] < 0 || (index_t)coord[d] >= gDim[d] )
                return false;
        }
        return true;
    }
    
    /// checks if point is inside the box
    bool inside(const real w[ORD]) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
        {
            if ( w[d] < gInf[d] || w[d] >= gSup[d] )
                return false;
        }
        return true;
    }
    
    /// periodic image
    void bringInside(int coord[ORD]) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
            coord[d] = image(d, coord[d]);
    }
    
    /// conversion from index to coordinates
    void setCoordinatesFromIndex(int coord[ORD], index_t indx) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
        {
            coord[d] = indx % gDim[d];
            indx    /= gDim[d];
        }
    }
    
    /// conversion from Position to coordinates (offset should be in [0,1])
    void setCoordinatesFromPosition(int coord[ORD], const real w[ORD], const real offset=0) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
            coord[d] = imagef(d, w[d], offset);
    }

    /// conversion from Index to Position (offset should be in [0,1])
    void setPositionFromIndex(real w[ORD], index_t indx, real offset) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
        {
            w[d] = gInf[d] + cWidth[d] * ( offset + indx % gDim[d] );
            indx /= gDim[d];
        }
    }
    
    /// conversion from Coordinates to Position (offset should be in [0,1])
    void setPositionFromCoordinates(real w[ORD], const int coord[ORD], real offset=0) const
    {
        for ( unsigned int d = 0; d < ORD; ++d )
            w[d] = gInf[d] + cWidth[d] * ( offset + coord[d] );
    }

    /// conversion from coordinates to index
    index_t pack(const int coord[ORD]) const
    {
        index_t inx = image(ORD-1, coord[ORD-1]);
        
        for ( int d = ORD-2; d >= 0; --d )
            inx = gDim[d] * inx + image(d, coord[d]);
        
        return inx;
    }
    
    
    /// returns the index of the cell whose center is closest to the point w[]
    index_t index(const real w[ORD]) const
    {
        index_t inx = imagef(ORD-1, w[ORD-1]);
        
        for ( int d = ORD-2; d >= 0; --d )
            inx = gDim[d] * inx + imagef(d, w[d]);
        
        return inx;
    }

    
    /// returns the index of the cell whose center is closest to the point w[]
    index_t index(const real w[ORD], const real offset) const
    {
        index_t inx = imagef(ORD-1, w[ORD-1], offset);
        
        for ( int d = ORD-2; d >= 0; --d )
            inx = gDim[d] * inx + imagef(d, w[d], offset);
        
        return inx;
    }

    
    /// return cell that is next to `c` in the direction `d`
    index_t next(index_t c, int dd) const
    {
        int coord[ORD];
        for ( unsigned int d = 0; d < ORD; ++d )
        {
            coord[d] = c % gDim[d];
            c       /= gDim[d];
        }

        coord[dd] = image(dd, coord[dd]+1);

        index_t inx = coord[ORD-1];
        
        for ( int d = ORD-2; d >= 0; --d )
            inx = gDim[d] * inx + coord[d];
        
        return inx;
    }
    
    /// convert coordinate to array index, if ORD==1
    index_t pack1D(const int x) const
    {
        return image(0, x);
    }
    
    /// convert coordinate to array index, if ORD==2
    index_t pack2D(const int x, const int y) const
    {
        return image(0, x) + gDim[0]*image(1, y);
    }
    
    /// convert coordinate to array index, if ORD==3
    index_t pack3D(const int x, const int y, const int z) const
    {
        return image(0, x) + gDim[0]*( image(1, y) + gDim[1]*image(2, z) );
    }

    
    /// return index of cell corresponding to position (x), if ORD==1
    index_t index1D(const real x) const
    {
        return image(0, map(0, x));
    }
    
    /// return index of cell corresponding to position (x, y), if ORD==2
    index_t index2D(const real x, const real y) const
    {
        return image(0, map(0, x))
               + gDim[0] * image(1, map(1, y));
    }
    
    /// return index of cell corresponding to position (x, y, z), if ORD==3
    index_t index3D(const real x, const real y, const real z) const
    {
        return image(0, map(0, x))
               + gDim[0]*( image(1, map(1, y))
               + gDim[1]*  image(2, map(2, z)) );
    }

    //--------------------------------------------------------------------------

#pragma mark - Regions

    /** For any cell, we can find the adjacent cells by adding 'index offsets'
    However, the valid offsets depends on wether the cell is on a border or not.
    For each 'edge', a list of offsets and its gDim are stored.*/
    
private:
    
    /// array of index offset to neighbors, for each edge type
    int  * regionsEdge;
    
    /// pointers to regionsEdge[], as a function of cell index
    int ** regions;
    
private:
    
    /// calculate the edge-characteristic from the size `s`, coordinate `c` and range `r`
    static index_t edge_signature(const index_t s, const int r, const int c)
    {
        if ( c < r )
            return r - c;
        else if ( c + r + 1 > (int)s )
            return 2 * r + c - s + 1;
        else
            return 0;
    }
    
    /// caculate the edge characteristic from the coordinates of a cell and the range vector
    index_t edgeFromCoordinates(const int coord[ORD], const int range[ORD]) const
    {
        index_t e = 0;
        for ( int d = ORD-1; d >= 0; --d )
        {
            e *= 2 * range[d] + 1;
            e += edge_signature(gDim[d], range[d], coord[d]);
        }
        return e;
    }
    
    
    int * makeRectangularGrid(int& cmx, const int range[ORD])
    {
        cmx = 1;
        for ( unsigned int d = 0; d < ORD; ++d )
            cmx *= ( 2 * range[d] + 1 );
        int * ccc = new int[ORD*cmx];
        
        int nb = 1;
        for ( unsigned int d = 0; d < ORD; ++d )
        {
            int h = 0;
            for ( ; h < nb && h < cmx; ++h )
                ccc[ORD*h+d] = 0;
            for ( int s = -range[d]; s <= range[d]; ++s )
            {
                if ( s != 0 )
                {
                    for ( int n = 0; n < nb && h < cmx; ++n, ++h )
                    {
                        for ( unsigned int e = 0; e < d; ++e )
                            ccc[ORD*h+e] = ccc[ORD*n+e];
                        ccc[ORD*h+d] = s;
                    }
                }
            }
            nb = h;
        }
        assert_true(nb==cmx);
        return ccc;
    }
    
    
    /// calculate cell index offsets between 'ori' and 'ori+shift'
    int calculateOffsets(int offsets[], int shift[], int cnt, int ori[], bool positive)
    {
        int nb = 0;
        int cc[ORD];
        int ori_indx = (int)pack(ori);
        for ( int ii = 0; ii < cnt; ++ii )
        {
            for ( unsigned int d = 0; d < ORD; ++d )
                cc[d] = ori[d] + shift[ORD*ii+d];
            int off = (int)pack(cc) - ori_indx;
            
            bool add = ( positive ? off >= 0 : true );
            if ( isPeriodic() )
            {
                //check that cell is not already included:
                for ( int n = 0; n < nb; ++n )
                    if ( offsets[n] == off )
                    {
                        add = false;
                        break;
                    }
            }
            else 
                add &= inside(cc);
            
            if ( add )
                offsets[nb++] = off;
        }
        return nb;
    }
    
   
    /// create regions in the offsets buffer
    /**
     Note: the range is taken specified in units of cells: 1 = 1 cell
     @todo: specify range in calculateRegion() as real distance!
     */
    void createRegions(int * ccc, const int regMax, const int range[ORD], bool positive)
    {
        index_t edgeMax = 0;
        for ( int d = ORD-1; d >= 0; --d )
        {
            edgeMax *= 2 * range[d] + 1;
            edgeMax += 2 * range[d];
        }
        ++edgeMax;
        
        //allocate and reset arrays:
        deleteRegions();
        
        regions     = new int*[nCells];
        regionsEdge = new int[edgeMax*(regMax+1)];
        for ( index_t e = 0; e < edgeMax*(regMax+1); ++e )
            regionsEdge[e] = 0;
        
        int ori[ORD];
        for ( size_t indx = 0; indx < nCells; ++indx )
        {
            setCoordinatesFromIndex(ori, indx);
            index_t e = edgeFromCoordinates(ori, range);
            assert_true( e < edgeMax );
            int * reg = regionsEdge + e * ( regMax + 1 );
            if ( reg[0] == 0 )
            {
                // calculate the region for this new edge-characteristic
                reg[0] = calculateOffsets(reg+1, ccc, regMax, ori, positive);
                //printf("edge %i has region of %i cells\n", e, reg[0]);
            }
            else if ( 0 )
            {
                // compare result for a different cell of the same edge-characteristic
                int * rig = new int[regMax+1];
                rig[0] = calculateOffsets(rig+1, ccc, regMax, ori, positive);
                if ( rig[0] != reg[0] )
                    ABORT_NOW("inconsistent region size");
                for ( int s = 1; s < rig[0]+1; ++s )
                    if ( rig[s] != reg[s] )
                        ABORT_NOW("inconsistent region offsets");
                delete[] rig;
            }
            regions[indx] = reg;
        }
    }
    
    /// accept within a certain diameter
    bool reject_disc(const int c[ORD], real radius)
    {
        real dsq = 0;
        for ( int d = 0; d < ORD; ++d ) 
            dsq += cWidth[d] * c[d] * cWidth[d] * c[d];
        return ( dsq > radius * radius );
    }
    
    /// accept within a certain diameter
    bool reject_square(const int c[ORD], real radius)
    {
        for ( int d = 0; d < ORD; ++d ) 
            if ( fabs( cWidth[d] * c[d] ) > radius )
                return true;
        return false;
    }
    
    /// used to remove ORD coordinates in array `ccc`
    void remove_entry(int * ccc, int& cmx, const int s)
    {
        --cmx;
        for ( int x = ORD*s; x < ORD*cmx; ++x )
            ccc[x] = ccc[x+ORD];
    }        
    
public:
    
    /// create regions which contains cells at a distance 'range' or less
    /**
     Note: the range is specified in real units.
     the region will cover an area of space that is approximately square.
     */
    void createSquareRegions(const real radius)
    {
        int range[ORD];
        for ( int d = 0; d < ORD; ++d )
            range[d] = ceil( radius / cWidth[d] );
        int cmx = 0;
        int * ccc = makeRectangularGrid(cmx, range);
        
        for ( int s = cmx-1; s >= 0 ; --s )
            if ( reject_square(ccc+ORD*s, radius) )
                remove_entry(ccc, cmx, s);
        
        createRegions(ccc, cmx, range, false);
        delete[] ccc;
    }
    
    /// create regions which contains cells at a distance 'range' or less
    /**
     Note: the range is specified in real units.
     The region covers an area of space that is approximately circular.
     */
    void createRoundRegions(const real radius)
    {
        int range[ORD];
        for ( int d = 0; d < ORD; ++d )
        {
            assert_true( cWidth[d] > 0 );
            range[d] = ceil( radius / cWidth[d] );
        }
        int cmx = 0;
        int * ccc = makeRectangularGrid(cmx, range);
       
        for ( int s = cmx-1; s >= 0 ; --s )
            if ( reject_disc(ccc+ORD*s, radius) )
                remove_entry(ccc, cmx, s);
        
        createRegions(ccc, cmx, range, false);
        delete[] ccc;
    }

    /// regions that only contain cells of greater index.
    /**
     This is suitable for pair-wise interaction of particles, since it can
     be used to go through the cells one by one such that at the end, all
     pairs of cells have been considered only once.

     Note: the radius is taken specified in units of cells: 1 = 1 cell
     */
    void createSideRegions(const int radius)
    {
        int range[ORD];
        for ( unsigned int d = 0; d < ORD; ++d )
            range[d] = radius;
        int cmx = 0;
        int * ccc = makeRectangularGrid(cmx, range);
        createRegions(ccc, cmx, range, true);
        delete[] ccc;
    }
    
    /// true if createRegions() or createRoundRegions() was called
    bool hasRegions() const
    {
        return ( regions && regionsEdge );
    }
    
    /// set region array 'offsets' for given cell index
    /**
     A zero offset is always first in the list.
     //\returns the size of the list.
     
     \par Example:

         CELL * cell = & myGrid.icell(indx);
         n_offset = myGrid.getRegion(offset, indx);
         for ( int n = 1; n < n_offset; ++n )
         {
             Cell & neighbor = cell[offset[n]];
             ...
         }
     
     Note: createRegions() must be called first
    */
    int getRegion(int*& offsets, const index_t indx) const
    {
        assert_true( hasRegions() );
        offsets = regions[indx]+1;
        assert_true( offsets[0] == 0 );
        return regions[indx][0];
    }
    
    /// free memory occupied by the regions
    void deleteRegions()
    {
        delete[] regions;
        regions = nullptr;
        
        delete[] regionsEdge;
        regionsEdge = nullptr;
    }

#pragma mark -

    /// write some info on the grid
    void printSummary(std::ostream& os, std::string str)
    {
        os << str << " of dim " << ORD << " has " << gAllocated << " cells:" << std::endl;
        for ( unsigned d = 0; d < ORD; ++d )
            os << "     [ " << gInf[d] << " " << gSup[d] << " ] / " << gDim[d] << " = " << cWidth[d] << std::endl;
    }
};


#endif
