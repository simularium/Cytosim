// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATSPARSE_H
#define MATSPARSE_H

#include "real.h"
#include "matrix.h"
#include <cstdio>
#include <string>

/// a real (non-symmetric) sparse Matrix
/**
 This class is not used currently in Cytosim
 */
class MatrixSparse
{
private:
    
    /// size of matrix
    index_t size_;

    /// size of memory which has been allocated
    size_t  allocated_;

    // array [ size ][ ? ] holding the values for each column
    real ** mxCol;
    
    // array [ size ][ ? ] holding the line index for each column
    int  ** mxRow;
    
    // allocate column to hold nb values
    void allocateColumn(index_t column_index, size_t nb_values);
    
public:
    
    /// return the size of the matrix
    index_t size() const { return size_; }
    
    /// change the size of the matrix
    void resize(index_t s) { allocate(s); size_=s; }

    /// base for destructor
    void deallocate();
    
    /// default constructor
    MatrixSparse();
    
    /// default destructor
    virtual ~MatrixSparse()  { deallocate(); }
    
    /// set all the element to zero
    void reset();
    
    /// allocate the matrix to hold ( sz * sz )
    void allocate(size_t sz);
        
    /// returns the address of element at (x, y), no allocation is done
    real* addr( index_t x, index_t y ) const;
    
    /// returns the address of element at (x, y), allocating if necessary
    real& operator()( index_t x, index_t y );
    
    /// scale the matrix by a scalar factor
    void scale( real a );
    
    /// add the diagonal block ( x, x, x+sx, x+sx ) from this matrix to M
    void addDiagonalBlock(real* mat, unsigned ldd, index_t si, unsigned nb) const;
    
    /// add this' data block ( idx, idx, idx+siz, idx+siz ) to upper triangular half of `mat`
    void addTriangularBlock(real* mat, index_t ldd, index_t si, unsigned nb, unsigned dim) const;
    
    /// multiplication of a vector: Y = Y + M * X, dim(X) = dim(M)
    void vecMulAdd( const real* X, real* Y ) const;
    
    /// 2D isotropic multiplication of a vector: Y = Y + M * X
    void vecMulAddIso2D( const real* X, real* Y ) const;
    
    /// 3D isotropic multiplication of a vector: Y = Y + M * X
    void vecMulAddIso3D( const real* X, real* Y ) const;
    
    /// true if matrix is non-zero
    bool nonZero() const;
    
    /// number of element which are non-zero
    size_t nbElements(index_t start, index_t stop) const;
    
    /// number of blocks which are not null
    size_t nbElements() const { return nbElements(0, size_); }

    /// returns a string which a description of the type of matrix
    std::string what() const;
    
    /// printf debug function in sparse mode: i, j : value
    void printSparse(std::ostream&) const;
    
    /// debug function
    int bad() const;
};


#endif
