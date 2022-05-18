// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Francois Nedelec; Last updated Jan 2008. nedelec@embl.de

#ifndef GRID_H
#define GRID_H

#include "grid_base.h"


/// A GridBase with an instantiation of class CELL in each voxel
/** 
Grid<int ORD, typename CELL> creates a regular lattice over a rectangular
region of space of dimensionality ORD.
The grid is initialized by setDimensions() and createCells() allocates a one-dimensional
array of CELL, with one value for each lattice point of the grid.

Functions are provided to convert from the real space coordinates (of type real)
into an index usable to access the one-dimensional array of CELL.
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

    CELL * cell = & myGrid.cell(indx);
    nb_neighbors = myGrid.getRegion(region, indx);
    for ( int n = 1; n < nb_neighbors; ++n ) 
    {
        Cell & neighbor = cell[region[n]];
        ...
    }

*/

///\todo Derive GridNumeric specialized for numerical values
///\todo add Grid<> copy constructor and copy assignment

template <typename CELL, int ORD>
class Grid : public GridBase<ORD>
{
    
    /// Disabled copy constructor
    Grid<CELL, ORD>(Grid<CELL, ORD> const&);
    
    /// Disabled copy assignment
    Grid<CELL, ORD>& operator=(Grid<CELL, ORD> const&);

protected:
    
    /// The array of pointers to cells
    CELL *  gCell;
    
public:
    
    /// Type of the parent class
    typedef GridBase<ORD> GRID;
    
    /// index
    typedef typename GRID::index_t index_t;

    /// The type of cells (=CELL)
    typedef CELL value_type;
    
    /// constructor
    Grid()
    {
        gCell = nullptr;
    }
    
    /// Free memory
    void destroy()
    {
        deleteCells();
        GRID::destroy();
    }
    
    /// Destructor
    virtual ~Grid()
    {
        destroy();
    }
    
    /// allocate the array of cells
    void createCells()
    {
        if ( GRID::nCells == 0 )
            printf("nCells==0 in createCells() : call setDimensions() first\n");
        
        delete[] gCell;
        
        gCell = new CELL[GRID::nCells];
        GRID::gAllocated = GRID::nCells;
    }
    
    /// returns true if cells have been allocated
    size_t hasCells() const
    {
        if ( gCell )
            return GRID::gAllocated;
        return 0;
    }
    
    /// deallocate array of cells
    void deleteCells()
    {
        delete[] gCell;
        gCell = nullptr;
        GRID::gAllocated = 0;
    }
    
    /// call function clear() for all cells
    void clear()
    {
        if ( !gCell )
            return;
        
        CELL * c = gCell;
        const CELL * last = gCell + GRID::nCells;
#if ( 0 )
        for ( ; c < last; ++c )
            c->clear();
#else
        //we unroll the loop to go faster
        const CELL * stop = gCell + (GRID::nCells % 8);
        for ( ; c < stop; ++c )
            c->clear();
        for ( ; c < last ; c += 8 )
        {
            c[0].clear();
            c[1].clear();
            c[2].clear();
            c[3].clear();
            c[4].clear();
            c[5].clear();
            c[6].clear();
            c[7].clear();
        }
#endif
    }

    //--------------------------------------------------------------------------

    /// address of the cell array ( equivalent to &cell(0) )
    CELL * data() const
    {
        return gCell;
    }
    
    /// return cell at index 'indx'
    CELL & icell(const index_t indx) const
    {
        assert_true( indx < GRID::gAllocated );
        assert_true( indx < GRID::nCells );
        return gCell[ indx ];
    }
    
    /// reference to CELL whose center is closest to w[]
    CELL & cell(const real w[ORD]) const
    {
        index_t inx = GRID::index(w);
        assert_true( inx < GRID::nCells );
        return gCell[ inx ];
    }
    
    /// reference to CELL of coordinates c[]
    CELL & cell(const int c[ORD]) const
    {
        assert_true( GRID::pack(c) < GRID::nCells );
        return gCell[ GRID::pack(c) ];
    }
   
    /// operator access to a cell by index
    CELL & operator[](const index_t indx) const
    {
        assert_true( indx < GRID::nCells );
        return gCell[ indx ];
    }

    /// short-hand access to a cell by coordinates
    CELL & operator()(const int c[ORD]) const
    {
        assert_true( GRID::pack(c) < GRID::nCells );
        return gCell[ GRID::pack(c) ];
    }
    
    /// operator access to a cell by position
    CELL & operator()(const real w[ORD]) const
    {
        assert_true( GRID::index(w) < GRID::nCells );
        return gCell[ GRID::index(w) ];
    }
    
    
    //--------------------------------------------------------------
#pragma mark -
    
    /// create a 1D-map
    void create1D(real i, real s, int d)
    {
        assert_true( ORD == 1 );
        GRID::setDimensions(&i, &s, &d);
        createCells();
    }

    /// access to cell for ORD==1
    CELL & icell1D(const int x) const
    {
        assert_true( GRID::pack1D(x) < GRID::nCells );
        return gCell[GRID::pack1D(x)];
    }
    
    /// access to cell for ORD==2
    CELL & icell2D(const int x, const int y) const
    {
        assert_true( GRID::pack2D(x,y) < GRID::nCells );
        return gCell[GRID::pack2D(x,y)];
    }
    
    /// access to cell for ORD==3
    CELL & icell3D(const int x, const int y, const int z) const
    {
        assert_true( GRID::pack3D(x,y,z) < GRID::nCells );
        return gCell[GRID::pack3D(x,y,z)];
    }
    
    /// access to cell for ORD==1
    CELL & cell1D(const real x) const
    {
        assert_true( GRID::index1D(x) < GRID::nCells );
        return gCell[GRID::index1D(x)];
    }
    
    /// access to cell for ORD==2
    CELL & cell2D(const real x, const real y) const
    {
        assert_true( GRID::index2D(x,y) < GRID::nCells );
        return gCell[GRID::index2D(x,y)];
    }
    
    /// access to cell for ORD==3
    CELL & cell3D(const real x, const real y, const real z) const
    {
        assert_true( GRID::index3D(x,y,z) < GRID::nCells );
        return gCell[GRID::index3D(x,y,z)];
    }

    //-----------------------------------------------------------------------
#pragma mark - Interpolate
    
    /// return linear interpolation of values stored at the center of each cell
    CELL interpolate( const real w[ORD] ) const
    {
        //we have 2^ORD corner cells
        const int sz = 1 << ORD;
        index_t inx[sz];   //incides of the corner cells
        real    alp[sz];   //coefficients of interpolation
        
        int nb = 0;
        for ( int d = ORD-1; d >= 0; --d )
        {
            real a = GRID::map(d, w[d]) + 0.5;
            int ia = std::floor(a);
            a     -= ia;
            int  l = GRID::image(d, ia-1);
            int  u = GRID::image(d, ia  );
            
            if ( nb == 0 )
            {
                //initialize the edges ( l and u ) and appropriate coefficients
                inx[1] = u;  alp[1] = a;
                inx[0] = l;  alp[0] = 1-a;
                nb = 2;
            }
            else
            {
                //double the amount of edges at each round,
                //with the indices and coefficients for lower and upper bounds
                for ( int c = 0; c < nb; ++c )
                {
                    inx[c+nb] = GRID::breadth(d) * inx[c] + u;
                    alp[c+nb] = alp[c] * a;
                    inx[c   ] = GRID::breadth(d) * inx[c] + l;
                    alp[c   ] = alp[c] * (1-a);
                }
                nb *= 2;
            }
        }
        assert_true( nb == sz );
        
        //sum weighted cells to interpolate
        CELL res = 0;
        for ( int c = 0; c < sz; ++c ) 
            res += alp[c] * gCell[inx[c]];
        return res;
    }
    
    
    /// return linear interpolation of values stored at the center of each cell, if ORD==1
    CELL interpolate1D( const real xx ) const
    {
        assert_true( ORD == 1 );
        
        real  ax = 0.5 + GRID::map(0, xx);
        
#if GRID_HAS_PERIODIC
        int   ix = std::floor(ax);
#else
        int   ix = (int)ax;
#endif
        
        ax -= ix;
        
        index_t lx = GRID::image(0, ix-1);
        index_t ux = GRID::image(0, ix  );
        
        return gCell[lx] + ax * ( gCell[ux] - gCell[lx] );
    }

    
    /// return linear interpolation of values stored at the center of each cell, if ORD==2
    CELL interpolate2D( const real w[ORD] ) const
    {
        assert_true( ORD == 2 );
        
        real  ax = 0.5 + GRID::map(0, w[0]);
        real  ay = 0.5 + GRID::map(1, w[1]);
        
#if GRID_HAS_PERIODIC
        int   ix = std::floor(ax);
        int   iy = std::floor(ay);
#else
        int   ix = (int)ax;
        int   iy = (int)ay;
#endif
        
        ax -= ix;
        ay -= iy;
        
        index_t lx = GRID::image(0, ix-1);
        index_t ux = GRID::image(0, ix  );
        
        index_t ly = GRID::image(1, iy-1) * GRID::breadth(0);
        index_t uy = GRID::image(1, iy  ) * GRID::breadth(0);
        
        //sum weighted cells to get interpolation
        CELL  rl = gCell[lx+ly] + ay * ( gCell[lx+uy] - gCell[lx+ly] );
        CELL  ru = gCell[ux+ly] + ay * ( gCell[ux+uy] - gCell[ux+ly] );

        return rl + ax * ( ru - rl );
    }

    
    /// return linear interpolation of values stored at the center of each cell, if ORD==3
    CELL interpolate3D( const real w[ORD] ) const
    {
        assert_true( ORD == 3 );
        
        real  ax = 0.5 + GRID::map(0, w[0]);
        real  ay = 0.5 + GRID::map(1, w[1]);
        real  az = 0.5 + GRID::map(2, w[2]);
        
#if GRID_HAS_PERIODIC
        int   ix = std::floor(ax);
        int   iy = std::floor(ay);
        int   iz = std::floor(az);
#else
        int   ix = (int)ax;
        int   iy = (int)ay;
        int   iz = (int)az;
#endif
        
        ax -= ix;
        ay -= iy;
        az -= iz;

        index_t lx = GRID::image(0, ix-1);
        index_t ux = GRID::image(0, ix  );
        
        index_t ly = GRID::image(1, iy-1) * GRID::breadth(0);
        index_t uy = GRID::image(1, iy  ) * GRID::breadth(0);
        
        index_t lz = GRID::image(2, iz-1) * GRID::breadth(1) * GRID::breadth(0);
        index_t uz = GRID::image(2, iz  ) * GRID::breadth(1) * GRID::breadth(0);

        CELL * cul = gCell + (uy+lz), rul = cul[lx] + ax * ( cul[ux] - cul[lx] );
        CELL * cuu = gCell + (uy+uz), ruu = cuu[lx] + ax * ( cuu[ux] - cuu[lx] );
        CELL * cll = gCell + (ly+lz), rll = cll[lx] + ax * ( cll[ux] - cll[lx] );
        CELL * clu = gCell + (ly+uz), rlu = clu[lx] + ax * ( clu[ux] - clu[lx] );

        CELL x = rll + az * ( rlu - rll );
        return x + ay * ( rul + az * ( ruu - rul ) - x );
        //return (1-ay) * ( rll + az * (rlu-rll) ) + ay * ( rul + az * (ruu-rul) );
    }


    //--------------------------------------------------------------------------
#pragma mark - For Numerical Cells

    /// set all cells to `val`
    void setValues(const CELL val)
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
            gCell[ii] = val;
    }
    
    /// multiply all cells by `val`
    void scaleValues(const CELL val)
    {
        assert_true ( GRID::nCells <= GRID::gAllocated );
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
            gCell[ii] *= val;
    }
    
    /// get sum, minimum and maximum value over all cells
    void infoValues(CELL& sum, CELL& mn, CELL& mx) const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        sum = 0;
        mn = gCell[0];
        mx = gCell[0];
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
        {
            if ( gCell[ii] < mn ) mn = gCell[ii];
            if ( gCell[ii] > mx ) mx = gCell[ii];
            sum += gCell[ii];
        }
    }

    /// sum of all values, if CELL supports the addition
    CELL sumValues() const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        CELL result = 0;
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
            result += gCell[ii];
        return result;
    }
    
    /// maximum value over all cells
    CELL maxValue() const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        CELL res = gCell[0];
        for ( index_t ii = 1; ii < GRID::nCells; ++ii )
        {
            if ( res < gCell[ii] )
                res = gCell[ii];
        }
        return res;
    }
    
    /// minimum value over all cells
    CELL minValue() const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        CELL res = gCell[0];
        for ( index_t ii = 1; ii < GRID::nCells; ++ii )
        {
            if ( res > gCell[ii] )
                res = gCell[ii];
        }
        return res;
    }
    
    /// true if any( cells[] < 0 )
    bool hasNegativeValue() const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
            if ( gCell[ii] < 0 )
                return true;
        return false;
    }
    
#pragma mark -
    
    /// the sum of the values in the region around cell referred by 'indx'
    CELL sumValuesInRegion(const index_t indx) const
    {
        CELL result = 0;
        int * offsets = nullptr;
        const CELL * ce = gCell + indx;
        int nb = GRID::getRegion(offsets, indx);
        for ( int c = 0; c < nb; ++c )
            result += ce[ offsets[c] ];
        return result;
    }
    
    /// the sum of the values in the region around cell referred by 'indx'
    CELL avgValueInRegion(const index_t indx) const
    {
        CELL result = 0;
        int * offsets = nullptr;
        const CELL * ce = gCell + indx;
        int nb = GRID::getRegion(offsets, indx);
        for ( int c = 0; c < nb; ++c )
            result += ce[ offsets[c] ];
        return result / (real)nb;
    }
    
    /// the maximum of the values in the region around cell referred by 'indx'
    CELL maxValueInRegion(const index_t indx) const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        CELL result = gCell[indx];
        int * offsets = nullptr;
        const CELL * ce = gCell + indx;
        int nb = GRID::getRegion(offsets, indx);
        for ( int c = 0; c < nb; ++c )
            if ( result < ce[ offsets[c] ] )
                result = ce[ offsets[c] ];
        return result;
    }
    
#pragma mark -
    
    /// write values to a file, with the position for each cell (file can be stdout)
    void printValues(FILE* file, const real offset) const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        real w[ORD];
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
        {
            GRID::setPositionFromIndex(w, ii, offset);
            for ( int d=0; d < ORD; ++d )
                fprintf(file, "%7.2f ", w[d]);
            fprintf(file,"  %f\n", gCell[ii]);
        }
    }
    
    /// write values to a file, with the range for each cell (file can be stdout)
    void printValuesWithRange(FILE* file) const
    {
        assert_true( GRID::nCells <= GRID::gAllocated );
        real l[ORD], r[ORD];
        for ( index_t ii = 0; ii < GRID::nCells; ++ii )
        {
            GRID::setPositionFromIndex(l, ii, 0.0);
            GRID::setPositionFromIndex(r, ii, 1.0);
            for ( int d=0; d < ORD; ++d )
                fprintf(file, "%7.2f %7.2f  ", l[d], r[d]);
            fprintf(file,"  %f\n", gCell[ii]);
        }
    }
};

#endif
