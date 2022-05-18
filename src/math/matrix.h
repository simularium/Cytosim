// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATRIX_H
#define MATRIX_H

#include <string>
#include <iostream>
#include "real.h"


/// type of an index into a matrix (unsigned)
typedef unsigned index_t;


/// The interface for all the large matrices
class Matrix
{
public:
    
protected:

    /// size of matrix
    index_t   size_;
    
private:
    
    /// Disabled copy constructor (@todo: write copy constructor)
    Matrix(Matrix const&);
    
    /// Disabled copy assignment (@todo: write copy assignement)
    Matrix& operator=(Matrix const&);

public:
    
    /// constructor
    Matrix() { size_ = 0; }

    /// constructor
    Matrix(index_t s) { size_ = s; }
    
    /// return the size of the matrix
    index_t size() const { return size_; }
    
    /// change the size of the matrix
    void resize(index_t s) { allocate(s); size_=s; }

    //----------------------------------------------------------------------
    
    /// allocate the matrix to hold ( sz * sz ), all values may be lost
    virtual void allocate(size_t alc) = 0;
        
    /// returns the address of element at (x, y), no allocation is done
    virtual real*  addr(index_t x, index_t y) const = 0;
    
    /// returns the address of element at (x, y), allocating if necessary
    virtual real&  operator()(index_t x, index_t y) = 0;
    
    /// returns the value of element at (x, y) or zero if not allocated
    real value(index_t x, index_t y) const;
    
    //----------------------------------------------------------------------
    
    /// set all the elements to zero
    virtual void reset() = 0;
    
    /// scale the matrix by a scalar factor
    virtual void scale(real) = 0;
    
    /// copy the block ( x, y, x+sx, y+sy ) into `mat`
    void copyBlock(real* mat, unsigned ldd, index_t sx, unsigned nx, index_t sy, unsigned ny) const;
    
    /// add the block ( x, x, x+sx, x+sx ) from this matrix to `mat`
    virtual void addDiagonalBlock(real* mat, index_t ldd, index_t si, unsigned nb) const;
    
    /// add upper triangular half of ( idx, idx, idx+siz, idx+siz ) to `mat`
    virtual void addTriangularBlock(real* mat, index_t ldd, index_t si, unsigned nb, unsigned dim) const;
    
    //----------------------------------------------------------------------
    
    /// Optional optimization to accelerate multiplications below
    virtual void prepareForMultiply(int dim) {}
    
    /// Vector multiplication: Y <- Y + M * X, size(X) = size(Y) = size(M)
    virtual void vecMulAdd(const real* X, real* Y) const = 0;
    
    /// Vector multiplication: Y <- M * X, size(X) = size(Y) = size(M)
    virtual void vecMul(const real* X, real* Y) const;

    /// isotropic vector multiplication: Y = Y + M * X, size(X) = size(Y) = 2 * size(M)
    virtual void vecMulAddIso2D(const real*, real*) const = 0;
    
    /// isotropic vector multiplication: Y = Y + M * X, size(X) = size(Y) = 3 * size(M)
    virtual void vecMulAddIso3D(const real*, real*) const = 0;
    
    //----------------------------------------------------------------------
    
    /// maximum absolute value among all the elements
    virtual real norm_inf() const;
    
    /// true if the matrix is non-zero
    virtual bool nonZero() const;
    
    /// number of element which are not null
    virtual size_t nbElements(index_t start, index_t stop) const;
    
    /// number of blocks which are not null
    size_t nbElements() const { return nbElements(0, size_); }

    /// returns a string which a description of the type of matrix
    virtual std::string what() const = 0;
    
    /// printf debug function in sparse mode: i, j : value
    virtual void printSparse(std::ostream&) const;
    
    /// printf debug function in full lines, all columns
    virtual void printFull(std::ostream&) const;
    
};


#endif
