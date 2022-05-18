// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "real.h"
#include "matsparse.h"
#include "assert_macro.h"
#include "cblas.h"
#include <iomanip>
#include <sstream>

#define MATRIX_OPTIMIZED_MULTIPLY

const int AVAILABLE_CELL = -1;
const int LAST_IN_COLUMN = -2;


MatrixSparse::MatrixSparse()
{
    size_      = 0;
    allocated_ = 0;
    mxCol      = nullptr;
    mxRow      = nullptr;
}


void MatrixSparse::allocate(size_t sz)
{
    size_ = sz;
    if ( size_ > allocated_ )
    {
        real ** mxCol_new = new real*[size_];
        int **  mxRow_new = new  int*[size_];
        
        unsigned int ii = 0;
        if ( mxCol )
        {
            for ( ; ii < allocated_; ++ii )
            {
                mxCol_new[ii] =  mxCol[ii];
                mxRow_new[ii] =  mxRow[ii];
            }
            delete[] mxCol;
            delete[] mxRow;
        }
        
        for ( ; ii < size_; ++ii )
        {
            mxCol_new[ii] = nullptr;
            mxRow_new[ii] = nullptr;
        }
        
        mxCol = mxCol_new;
        mxRow = mxRow_new;
        allocated_ = size_;
    }
}


void MatrixSparse::deallocate()
{
    if ( mxCol )
    {
        for ( size_t ii = 0; ii < allocated_; ++ii )
            if ( mxCol[ii] )
            {
                delete[] mxCol[ii];
                delete[] mxRow[ii];
            };
        delete[] mxCol;
        delete[] mxRow;
        mxCol = nullptr;
        mxRow = nullptr;
    }
    allocated_ = 0;
}


void MatrixSparse::allocateColumn( const index_t jj, size_t sz )
{
    assert_true( jj < size_ );
    assert_true( sz > 0 );
    //printf("new S-COL %i %i\n", jj, sz );
    
    constexpr size_t chunk = 16;
    sz = ( sz + chunk - 1 ) & ~( chunk -1 );

    real* mxCol_new  = new real[sz];
    int*  mxRow_new  = new int[sz];
    
    unsigned int ii = 0;
    if ( mxCol[jj] )
    {
        for ( ; mxRow[jj][ii] != LAST_IN_COLUMN ; ++ii )
        {
            mxCol_new[ii] =  mxCol[jj][ii];
            mxRow_new[ii] =  mxRow[jj][ii];
        }
        
        delete[] mxCol[jj];
        delete[] mxRow[jj];
    }
    for ( ; ii < sz-1 ; ++ii )
        mxRow_new[ii] = AVAILABLE_CELL;
    mxRow_new[sz-1] = LAST_IN_COLUMN;
    
    mxCol[jj]  = mxCol_new;
    mxRow[jj]  = mxRow_new;
}


//allocate the position if necessary:
real& MatrixSparse::operator()( index_t x, index_t y )
{
    assert_true( x < size_ );
    assert_true( y < size_ );
    
    if ( mxRow[y] )
    {
        index_t ii = 0;
        for ( ; mxRow[y][ii] >= 0; ++ii )
            if ( mxRow[y][ii] == (int)x )
                return mxCol[y][ii];
        
        if ( mxRow[y][ii] == LAST_IN_COLUMN )
            allocateColumn( y, ii + 1 );
        assert_true( mxRow[y][ii] == AVAILABLE_CELL );
        mxRow[y][ii] = x;
        mxCol[y][ii] = 0;
        //printf("allo. %3i %3i\n", x, y );
        return mxCol[y][ii];
    }
    
    allocateColumn( y, 1 );
    //printf("allo. %3i %3i\n", nx, ny );
    assert_true( mxRow[y][0] == AVAILABLE_CELL );
    
    //put the diagonal term first:
    mxRow[y][0] = y;
    mxCol[y][0] = 0;
    if ( x == y )
        return mxCol[y][0];
    
    mxRow[y][1] = x;
    mxCol[y][1] = 0;
    return mxCol[y][1];
}


//does not allocate the position:
real* MatrixSparse::addr( index_t x, index_t y) const
{
    int * row = mxRow[y];
    if ( row )
    {
        for ( ; *row >= 0; ++row )
            if ( *row == (int)x )
                return & mxCol[y][ row - mxRow[y] ];
    }
    return nullptr;
}


void MatrixSparse::reset()
{
    for ( index_t ii = 0; ii < size_; ++ii )
        if ( mxRow[ii] )
            for ( int jj = 0; mxRow[ii][jj] >= 0; ++jj )
                mxRow[ii][jj] = AVAILABLE_CELL;
}


void MatrixSparse::scale( real a )
{
    for ( index_t ii = 0; ii < size_; ++ii )
        if ( mxRow[ii] )
            for ( int jj = 0; mxRow[ii][jj] >= 0; ++jj )
                mxCol[ii][jj] *= a;
}


void MatrixSparse::addTriangularBlock(real* mat, index_t ldd, index_t si, unsigned nb, unsigned dim) const
{
    assert_true( si + nb <= size_ );
    
    for ( unsigned jj = 0; jj < nb; ++jj )
    {
        int* row = mxRow[jj + si];
        if ( row != nullptr )
        {
            real* col = mxCol[jj + si];
            for ( ; *row >= 0 ; ++row, ++col )
            {
                if ( *row > (int)si )
                {
                    index_t ii = *row - si;
                    if ( ii < nb )
                    {
                        assert_true( ii <= jj );
                        mat[dim*( ii + ldd * jj )] += *col;
                        //printf("Sp %4i %4i % .4f\n", ii, jj, a );
                    }
                }
            }
        }
    }
}


void MatrixSparse::addDiagonalBlock(real* mat, unsigned ldd, index_t si, unsigned nb) const
{
    assert_true( si + nb <= size_ );
    
    for ( index_t jj = 0; jj < nb; ++jj )
    {
        int * row = mxRow[jj + si];
        if ( row )
        {
            real* col = mxCol[jj + si];
            for ( ; *row >= 0 ; ++row, ++col )
            {
                if ( *row > (int)si )
                {
                    index_t ii = *row - si;
                    if ( ii < nb )
                    {
                        //printf("Sp %4i %4i % .4f\n", ii, jj, a );
                        mat[ii+ldd*jj] += *col;
                    }
                }
            }
        }
    }
}


int MatrixSparse::bad() const
{
    if ( size_ <= 0 ) return 1;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                if ( mxRow[jj][ii] < 0     ) return 2;
                if ( mxRow[jj][ii] >= (int)size_ ) return 3;
            }
    }
    return 0;
}


void MatrixSparse::printSparse(std::ostream& os) const
{
    os.precision(8);
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                os << mxRow[jj][ii] << " " << jj << " ";
                os << std::setw(16) << mxCol[jj][ii] << std::endl;
            }
    }
}


bool MatrixSparse::nonZero() const
{
    for ( index_t jj = 0; jj < size_; ++jj )
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
                if ( mxCol[jj][ii] != 0 )
                    return true;
    return false;
}


size_t MatrixSparse::nbElements(index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );
    //all allocated elements are counted, even if the value is zero
    size_t cnt = 0;
    for ( index_t jj = start; jj < stop; ++jj )
        if ( mxRow[jj] )
            for ( index_t ii = 0; mxRow[jj][ii] >= 0; ++ii )
                ++cnt;
    return cnt;
}


std::string MatrixSparse::what() const
{
    std::ostringstream msg;
    msg << "mS " << nbElements();
    return msg.str();
}


void MatrixSparse::vecMulAdd( const real* X, real* Y ) const
{
    int kk;
    
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( mxRow[jj] )
            for ( index_t ii = 0; ( kk = mxRow[jj][ii] ) >= 0; ++ii )
            {
                Y[kk] += mxCol[jj][ii] * X[jj];
            }
    }
}

//------------------------------------------------------------------------------
#ifndef MATRIX_OPTIMIZED_MULTIPLY


void MatrixSparse::vecMulAddIso2D( const real* X, real* Y ) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( mxRow[jj] )
            for ( index_t ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                const index_t kk = 2 * mxRow[jj][ii];
                const real a = mxCol[jj][ii];
                Y[kk  ] += a * X[kk  ];
                Y[kk+1] += a * X[kk+1];
            }
    }
}


void MatrixSparse::vecMulAddIso3D( const real* X, real* Y ) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( mxRow[jj] )
            for ( index_t ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                const index_t kk = 3 * mxRow[jj][ii];
                const real a = mxCol[jj][ii];
                Y[kk  ] += a * X[kk  ];
                Y[kk+1] += a * X[kk+1];
                Y[kk+2] += a * X[kk+2];
            }
    }
}


#else  //MATRIX_OPTIMIZED_MULTIPLY


void MatrixSparse::vecMulAddIso2D( const real* X, real* Y ) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        int* row = mxRow[jj];
        if ( row != nullptr )
        {
            real* col = mxCol[jj];
            index_t ll = 2 * jj;
            
            real X1 = X[ll  ];
            real X2 = X[ll+1];
            
            while ( *row >= 0 )
            {
                index_t kk = 2 * ( *row );
                Y[kk  ] += (*col) * X1;
                Y[kk+1] += (*col) * X2;
                
                ++row;
                ++col;
            }
        }
    }
}


void MatrixSparse::vecMulAddIso3D( const real* X, real* Y ) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        int* row = mxRow[jj];
        if ( row != nullptr )
        {
            real* col = mxCol[jj];
            index_t ll = 3 * jj;
            
            real X1 = X[ll  ];
            real X2 = X[ll+1];
            real X3 = X[ll+2];
            
            while ( * row >= 0 )
            {
                int kk = 3 * ( *row );
                Y[kk  ] += (*col) * X1;
                Y[kk+1] += (*col) * X2;
                Y[kk+2] += (*col) * X3;
                
                ++row;
                ++col;
            }
        }
    }
}

#endif

