// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATSPARSESYMBLK_H
#define MATSPARSESYMBLK_H

#include "matrix.h"
#include <cstdio>
#include "assert_macro.h"

/**
 The block size 'BLOCK_SIZE' can be defined on the command line during compilation,
 and is otherwise set here, depending on the dimensionality of the simulation
 */

#ifndef BLOCK_SIZE
#   include "dim.h"
#   if ( DIM == 1 )
#      define BLOCK_SIZE 1
#   elif ( DIM == 2 )
#      define BLOCK_SIZE 2
#   else
#      define BLOCK_SIZE 3
#   endif
#endif


#if ( BLOCK_SIZE == 1 )
#   include "matrix11.h"
typedef Matrix11 SquareBlock;
#elif ( BLOCK_SIZE == 2 )
#   include "matrix22.h"
typedef Matrix22 SquareBlock;
#elif ( BLOCK_SIZE == 3 )
#   include "matrix33.h"
typedef Matrix33 SquareBlock;
#else
#   include "matrix44.h"
typedef Matrix44 SquareBlock;
#endif


///real symmetric sparse Matrix
/**
 MatrixSparseSymmetricBlock uses a sparse storage, with arrays of elements for each column.
 Each element is a full square block of size DIM x DIM.
 
 Elements are stored in random order in the column.
 The lower triangle of the matrix is stored.
 
 F. Nedelec, 17--27 March 2017, revised entirely June 2018
 */
class MatrixSparseSymmetricBlock
{
public:
    
    /// accessory class
    class Element;

private:
    
    /// size of matrix
    index_t  size_;
    
    /// amount of memory which has been allocated
    size_t   allocated_;

    /// A column of the sparse matrix
    class Column
    {
        friend class MatrixSparseSymmetricBlock;

        size_t   allo_;
        unsigned size_;
        //Element* elem_;
        index_t     * inx_;
        SquareBlock * blk_;
        
    public:
        
        /// constructor
        Column() { size_ = 0; allo_ = 0; inx_ = nullptr; blk_ = nullptr; }
        
        /// the assignment operator will transfer memory
        void operator =(Column&);
        
        /// destructor
        ~Column() { deallocate(); }
        
        /// allocate to hold 'nb' elements
        void allocate(size_t nb);
        
        /// deallocate memory
        void deallocate();

        /// set as zero
        void reset();
        
        /// sort element by increasing indices, using provided temporary array
        void sort(Element*&, size_t);
        
        /// print
        void print(std::ostream&) const;

        /// return n-th block (not necessarily, located at line inx_[n]
        SquareBlock& operator[](int n) { return blk_[n]; }

        /// return block located at line 'i' and column 'j'
        SquareBlock& block(index_t i, index_t j);
        
        /// multiplication of a vector: Y <- Y + M * X, block_size = 1
        void vecMulAdd1D(const real* X, real* Y, index_t j) const;
        
        /// multiplication of a vector: Y <- Y + M * X, block_size = 2
        void vecMulAdd2D(const real* X, real* Y, index_t j) const;
        
        /// multiplication of a vector: Y <- Y + M * X, block_size = 3
        void vecMulAdd3D(const real* X, real* Y, index_t j) const;
        
        /// multiplication of a vector: Y <- Y + M * X, block_size = 4
        void vecMulAdd4D(const real* X, real* Y, index_t j) const;

        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 2
        void vecMulAdd2D_SSE(const real* X, real* Y, index_t j) const;
        
        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 2
        void vecMulAdd2D_AVX(const real* X, real* Y, index_t j) const;
        
        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 2
        void vecMulAdd2D_AVXU(const real* X, real* Y, index_t j) const;
        
        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 2
        void vecMulAdd2D_AVXU4(const real* X, real* Y, index_t j) const;

        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 3
        void vecMulAdd3D_AVX(const real* X, real* Y, index_t j) const;

        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 3
        void vecMulAdd3D_AVXU(const real* X, real* Y, index_t j) const;

        /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(M), block_size = 4
        void vecMulAdd4D_AVX(const real* X, real* Y, index_t j) const;
    };
    
private:
    
    /// array col_[c][] holds Elements of column 'c'
    Column *  column_;
    
    /// next_[ii] is the index of the first non-empty column of index >= ii
    index_t * next_;

public:
    
    /// return the size of the matrix
    index_t size() const { return size_; }
    
    /// change the size of the matrix
    void resize(index_t s) { allocate(s); size_=s; }

    /// base for destructor
    void deallocate();
    
    /// default constructor
    MatrixSparseSymmetricBlock();
    
    /// default destructor
    virtual ~MatrixSparseSymmetricBlock()  { deallocate(); }
    
    /// set all the element to zero
    void reset();
    
    /// allocate the matrix to hold ( sz * sz )
    void allocate(size_t alc);
    
    /// returns element stored at line ii and column jj, if ( ii > jj )
    SquareBlock& block(const index_t ii, const index_t jj)
    {
        assert_true( ii < size_ );
        assert_true( jj < size_ );
        assert_true( ii % BLOCK_SIZE == 0 );
        assert_true( jj % BLOCK_SIZE == 0 );
#if ( 1 )
        // safe swap, with branchless code:
        index_t i = std::max(ii, jj);
        index_t j = std::min(ii, jj);
        return column_[j].block(i, j);
#else
        assert_true( ii > jj );
        return column_[jj].block(ii, jj);
#endif
    }
    
    /// returns element at (i, i)
    SquareBlock& diag_block(index_t i);
    
    /// returns the address of element at (x, y), no allocation is done
    real* addr(index_t x, index_t y) const;
    
    /// returns the address of element at (x, y), allocating if necessary
    real& operator()(index_t x, index_t y);
    
    /// scale the matrix by a scalar factor
    void scale(real);
    
    /// add the diagonal block ( x, x, x+sx, x+sx ) from this matrix to M
    void addDiagonalBlock(real* mat, unsigned ldd, index_t si, unsigned nb) const;
    
    /// add upper triangular half of 'this' block ( idx, idx, idx+siz, idx+siz ) to `mat`
    void addTriangularBlock(real* mat, index_t ldd, index_t si, unsigned nb, unsigned dim) const;
    
    
    ///optional optimization that may accelerate multiplications by a vector
    void prepareForMultiply(int dim);

    /// multiplication of a vector, for columns within [start, end[
    void vecMulAdd(const real*, real* Y, index_t start, index_t end) const;

    /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(Y) = dim(M)
    void vecMulAdd(const real* X, real* Y) const { vecMulAdd(X, Y, 0, size_); }

    /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(Y) = dim(M)
    void vecMulAdd_TIME(const real* X, real* Y) const;

    /// multiplication of a vector: Y <- Y + M * X with dim(X) = dim(Y) = dim(M)
    void vecMulAdd_ALT(const real* X, real* Y) const;

    /// 2D isotropic multiplication (not implemented)
    void vecMulAddIso2D(const real* X, real* Y) const {};
    
    /// 3D isotropic multiplication (not implemented)
    void vecMulAddIso3D(const real*, real*) const {};

    /// true if matrix is non-zero
    bool nonZero() const;
    
    /// number of blocks in columns within [start, end[
    size_t nbElements(index_t start, index_t end) const;
    
    /// number of blocks which are not null
    size_t nbElements() const { return nbElements(0, size_); }

    /// returns a string which a description of the type of matrix
    std::string what() const;
    
    /// printf debug function in sparse mode: i, j : value
    void printSparse(std::ostream&) const;

    /// print content of one column
    void printColumns(std::ostream&);
    
    /// debug function
    int bad() const;
};


#endif

