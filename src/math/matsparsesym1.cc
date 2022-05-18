// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matsparsesym1.h"
#include "assert_macro.h"
#include "cblas.h"

#include <iomanip>
#include <sstream>

#ifdef __SSE3__
#  define MATRIX1_USES_SSE REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define MATRIX1_USES_SSE 0
#endif

#ifdef __AVX__
#  define MATRIX1_USES_AVX REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define MATRIX1_USES_AVX 0
#endif


MatrixSparseSymmetric1::MatrixSparseSymmetric1()
{
    allocated_ = 0;
    column_    = nullptr;
    col_size_  = nullptr;
    col_max_   = nullptr;
    
#if MATRIX1_OPTIMIZED_MULTIPLY
    nmax_      = 0;
    ija_       = nullptr;
    sa_        = nullptr;
#endif
#if MATRIX1_USES_COLNEXT
    next_  = new index_t[1];
    next_[0] = 0;
#endif
}


void MatrixSparseSymmetric1::allocate(size_t alc)
{
    if ( alc > allocated_ )
    {
        /*
         'chunk' can be increased to gain performance:
         more memory will be used, but reallocation will be less frequent
         */
        constexpr size_t chunk = 64;
        alc = ( alc + chunk - 1 ) & ~( chunk -1 );

        //fprintf(stderr, "MSS1 allocate matrix %u\n", alc);
        Element ** col_new      = new Element*[alc];
        unsigned * col_size_new = new unsigned[alc];
        size_t   * col_max_new  = new size_t[alc];
        
        index_t ii = 0;
        if ( column_ )
        {
            for ( ; ii < allocated_; ++ii )
            {
                col_new[ii]      = column_[ii];
                col_size_new[ii] = col_size_[ii];
                col_max_new[ii]  = col_max_[ii];
            }
            delete[] column_;
            delete[] col_size_;
            delete[] col_max_;
        }
        
        column_    = col_new;
        col_size_  = col_size_new;
        col_max_   = col_max_new;
        allocated_ = alc;

        for ( ; ii < alc; ++ii )
        {
            column_[ii]   = nullptr;
            col_size_[ii] = 0;
            col_max_[ii]  = 0;
        }
        
#if MATRIX1_USES_COLNEXT
        delete[] next_;
        next_ = new index_t[alc+1];
        for ( size_t n = 0; n <= alc; ++n )
            next_[n] = n;
#endif
    }
}


void MatrixSparseSymmetric1::deallocate()
{
    if ( column_ )
    {
        for ( size_t ii = 0; ii < allocated_; ++ii )
            delete[] column_[ii];
        delete[] column_;    column_   = nullptr;
        delete[] col_size_;  col_size_ = nullptr;
        delete[] col_max_;   col_max_  = nullptr;
#if MATRIX1_OPTIMIZED_MULTIPLY
        delete[] ija_;
        free_real(sa_);
        ija_ = nullptr;
        sa_ = nullptr;
#endif
#if MATRIX1_USES_COLNEXT
        delete[] next_;  next_ = nullptr;
#endif
    }
    allocated_ = 0;
}


/// copy `cnt` elements from `src` to `dst`
void copy(unsigned cnt, MatrixSparseSymmetric1::Element * src, MatrixSparseSymmetric1::Element * dst)
{
    for ( unsigned ii = 0; ii < cnt; ++ii )
        dst[ii] = src[ii];
}

/// move `cnt` elements to the next index, starting at vec[0]
void shift(unsigned cnt, MatrixSparseSymmetric1::Element * vec)
{
    for ( unsigned ii = cnt; ii > 0; --ii )
        vec[ii] = vec[ii-1];
}


void MatrixSparseSymmetric1::allocateColumn(const index_t jj, size_t alc)
{
    assert_true( jj < size_ );
    if ( alc > col_max_[jj] )
    {
        //fprintf(stderr, "MSS1 allocate column %i size %u\n", jj, alc);
        constexpr size_t chunk = 16;
        alc = ( alc + chunk - 1 ) & ~( chunk -1 );
        Element * col_new = new Element[alc];
        
        if ( column_[jj] )
        {
            //copy over previous column elements
            copy(col_size_[jj], column_[jj], col_new);
            
            //release old memory
            delete[] column_[jj];
        }
        column_[jj]  = col_new;
        col_max_[jj] = alc;
    }
}


MatrixSparseSymmetric1::Element * MatrixSparseSymmetric1::insertElement(const index_t jj, index_t inx)
{
    assert_true( jj < size_ );
    // allocate space for new Element if necessary:
    if ( col_size_[jj] >= col_max_[jj] )
    {
        constexpr size_t chunk = 16;
        size_t alc = ( col_size_[jj] + chunk ) & ~( chunk -1 );
        Element * col_new = new Element[alc];
        if ( column_[jj] )
        {
            copy(inx, column_[jj], col_new);
            copy(col_size_[jj]-inx, column_[jj]+inx, col_new+inx+1);
            delete[] column_[jj];
        }
        column_[jj]  = col_new;
        col_max_[jj] = alc;
    }
    else
    {
        shift(col_size_[jj]-inx, column_[jj]+inx);
    }
    column_[jj][inx].reset(-1);
    ++col_size_[jj];
    return column_[jj]+inx;
}


real& MatrixSparseSymmetric1::diagonal(index_t ix)
{
    assert_true( ix < size_ );
    
    Element * col;
    
    if ( col_size_[ix] == 0 )
    {
        allocateColumn(ix, 1);
        col = column_[ix];
        //diagonal term always first:
        col->reset(ix);
        col_size_[ix] = 1;
    }
    else
    {
        col = column_[ix];
        assert_true( col->inx == ix );
    }
    
    return col->val;
}

/**
 This allocate to be able to hold the matrix element if necessary
*/
real& MatrixSparseSymmetric1::operator()(index_t i, index_t j)
{
    assert_true( i < size_ );
    assert_true( j < size_ );
    //fprintf(stderr, "MSS1( %6i %6i )\n", i, j);
    
    Element * col;
    
    if ( i == j )
    {
        // return diagonal element
        if ( col_size_[j] <= 0 )
        {
            allocateColumn(j, 1);
            col = column_[j];
            // put diagonal term always first:
            col->reset(j);
            col_size_[j] = 1;
        }
        else
        {
            col = column_[j];
            assert_true( col->inx == j );
        }
        return col->val;
    }
 
    // swap to get ii > jj (address lower triangle)
    index_t ii = std::max(i, j);
    index_t jj = std::min(i, j);

    //check if the column is empty:
    if ( col_size_[jj] < 2 )
    {
        allocateColumn(jj, 2);
        col = column_[jj];
        if ( col_size_[jj] == 0 )
        {
            // put diagonal term always first:
            col->reset(jj);
        }
        //add the requested term:
        col[1].reset(ii);
        col_size_[jj] = 2;
        return col[1].val;
    }
    
    col = column_[jj];
    Element * e = col + 1;
    Element * lst = col + col_size_[jj] - 1;
    
    //search, knowing that elements are kept ordered in the column:
    while ( e->inx < ii )
    {
        if ( ++e > lst )
        {
            // add one element last
            unsigned n = col_size_[jj];
            if ( n >= col_max_[jj] )
            {
                allocateColumn(jj, n+1);
                col = column_[jj];
            }
            ++col_size_[jj];
            col[n].reset(ii);
            return col[n].val;
        }
    }
    
    if ( e->inx == ii )
        return e->val;
    
    index_t n = e - col;

    assert_true( col[n].inx > ii );
    col = insertElement(jj, n);
    assert_true( n < col_max_[jj] );

    // add the requested term
    col->reset(ii);

    //printColumn(std::clog, jj);
    return col->val;
}


real* MatrixSparseSymmetric1::addr(index_t i, index_t j) const
{
    // swap to get ii <= jj (address lower triangle)
    index_t ii = std::max(i, j);
    index_t jj = std::min(i, j);

    for ( unsigned kk = 0; kk < col_size_[jj]; ++kk )
        if ( column_[jj][kk].inx == ii )
            return &( column_[jj][kk].val );
    
    return nullptr;
}


//------------------------------------------------------------------------------
#pragma mark -

void MatrixSparseSymmetric1::reset()
{
    for ( index_t jj = 0; jj < size_; ++jj )
        col_size_[jj] = 0;
}


bool MatrixSparseSymmetric1::nonZero() const
{
    //check for any non-zero sparse term:
    for ( index_t jj = 0; jj < size_; ++jj )
        for ( unsigned kk = 0 ; kk < col_size_[jj] ; ++kk )
            if ( column_[jj][kk].val != 0 )
                return true;
    
    //if here, the matrix is empty
    return false;
}


void MatrixSparseSymmetric1::scale(const real alpha)
{
    for ( index_t jj = 0; jj < size_; ++jj )
        for ( unsigned n = 0; n < col_size_[jj]; ++n )
            column_[jj][n].val *= alpha;
}


void MatrixSparseSymmetric1::addTriangularBlock(real* mat, const unsigned ldd,
                                                const index_t si,
                                                const unsigned nb,
                                                const unsigned dim) const
{
    index_t up = si + nb;
    assert_true( up <= size_ );
    
    for ( index_t jj = si; jj < up; ++jj )
    {
        for ( unsigned n = 0; n < col_size_[jj]; ++n )
        {
            index_t ii = column_[jj][n].inx;
            // assuming lower triangle is stored:
            assert_true( ii >= jj );
            if ( ii < up )
            {
                //printf("MSS1 %4i %4i % .4f\n", ii, jj, a);
                mat[dim*( jj-si + ldd * (ii-si) )] += column_[jj][n].val;
            }
        }
    }
}


void MatrixSparseSymmetric1::addDiagonalBlock(real* mat, unsigned ldd,
                                              const index_t si,
                                              const unsigned nb) const
{
    index_t up = si + nb;
    assert_true( up <= size_ );
    
    for ( index_t jj = si; jj < up; ++jj )
    {
        for ( unsigned n = 0; n < col_size_[jj]; ++n )
        {
            index_t ii = column_[jj][n].inx;
            // assuming lower triangle is stored:
            assert_true( ii >= jj );
            if ( ii < up )
            {
                //printf("MSS1 %4i %4i % .4f\n", ii, jj, a);
                mat[jj-si+ldd*(ii-si)] += column_[jj][n].val;
                if ( jj != ii )
                    mat[ii-si+ldd*(jj-si)] += column_[jj][n].val;
            }
        }
    }
}


int MatrixSparseSymmetric1::bad() const
{
    if ( size_ <= 0 ) return 1;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        for ( unsigned kk = 0 ; kk < col_size_[jj] ; ++kk )
        {
            if ( column_[jj][kk].inx >= size_ ) return 2;
            if ( column_[jj][kk].inx <= jj )   return 3;
        }
    }
    return 0;
}


size_t MatrixSparseSymmetric1::nbElements(index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );
    //all allocated elements are counted, even if zero
    size_t cnt = 0;
    for ( index_t jj = start; jj < stop; ++jj )
        cnt += col_size_[jj];
    return cnt;
}


std::string MatrixSparseSymmetric1::what() const
{
    std::ostringstream msg;
#if MATRIX1_USES_AVX
    msg << "MSS1x " << nbElements();
#elif MATRIX1_USES_SSE
    msg << "MSS1e " << nbElements();
#else
    msg << "MSS1 " << nbElements();
#endif
    return msg.str();
}


void MatrixSparseSymmetric1::printSparse(std::ostream& os) const
{
    char str[256];
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
            os << "% column " << jj << "\n";
        for ( unsigned n = 0 ; n < col_size_[jj] ; ++n )
        {
            real v = column_[jj][n].val;
            if ( v != 0 )
            {
                snprintf(str, sizeof(str), "%6i %6i %16.6f\n", column_[jj][n].inx, jj, v);
                os << str;
            }
        }
    }
}


void MatrixSparseSymmetric1::printColumns(std::ostream& os)
{
    os << "MSS1 size " << size_ << ":";
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        os << "\n   " << jj << "   " << col_size_[jj];
#if MATRIX1_USES_COLNEXT
        os << " next " << next_[jj];
#endif
    }
    std::endl(os);
}


void MatrixSparseSymmetric1::printColumn(std::ostream& os, const index_t jj)
{
    Element const* col = column_[jj];
    os << "MSS1 col " << jj << ":";
    for ( unsigned n = 0; n < col_size_[jj]; ++n )
    {
        os << "\n" << col[n].inx << " :";
        os << " " << col[n].val;
    }
    std::endl(os);
}


void MatrixSparseSymmetric1::printSparseArray(std::ostream& os) const
{
#if MATRIX1_OPTIMIZED_MULTIPLY
    unsigned end = ija_[size_];
    
    os << "ija ";
    for ( index_t n = 0; n < end; ++n )
        os << " " << std::setw(6) << ija_[n];
    os << "\n";
    
    std::streamsize p = os.precision();
    os.precision(2);
    os << "sa  ";
    for ( index_t n = 0; n < end; ++n )
        os << " " << std::setw(6) << sa_[n];
    os << "\n";
    os.precision(p);
#else
    os << "optimized sparse matrix storage unavailable\n";
#endif
}


//------------------------------------------------------------------------------
#pragma mark - Vector Multiplication

#if !MATRIX1_OPTIMIZED_MULTIPLY

void MatrixSparseSymmetric1::prepareForMultiply(int)
{
}

/*
 Using the primary storage in 'col_' and 'col_size_'
 */
void MatrixSparseSymmetric1::vecMulAdd(const real* X, real* Y, index_t jj, Element col[], size_t size) const
{
    const real X0 = X[jj];
    real Y0 = Y[jj] + col[0].val * X0;
    for ( unsigned n = 1 ; n < size ; ++n )
    {
        const index_t ii = col[n].inx;
        const real a = col[n].val;
        Y[ii] += a * X0;
        assert_true( ii > jj );
        Y0 += a * X[ii];
    }
    Y[jj] = Y0;
}

/*
 Using the primary storage in 'col_' and 'col_size_'
 */
void MatrixSparseSymmetric1::vecMulAdd(const real* X, real* Y) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            assert_true( col_[jj][0].inx == jj );
            vecMulAddCol1(X, Y, jj, col_[jj], col_size_[jj]);
        }
    }
}

/*
 Using the primary storage in 'col_' and 'col_size_'
 */
void MatrixSparseSymmetric1::vecMulAddIso2D(const real* X, real* Y, index_t jj, Element col[], size_t size) const
{
    const real X0 = X[jj  ];
    const real X1 = X[jj+1];
    real Y0 = Y[jj  ] + col[0].val * X0;
    real Y1 = Y[jj+1] + col[0].val * X1;
    for ( unsigned n = 1 ; n < size ; ++n )
    {
        const index_t ii = 2 * col[n].inx;
        const real  a = col[n].val;
        Y[ii  ] += a * X0;
        Y[ii+1] += a * X1;
        assert_true( ii > jj );
        Y0 += a * X[ii  ];
        Y1 += a * X[ii+1];
    }
    Y[jj  ] = Y0;
    Y[jj+1] = Y1;
}


/*
 Using the primary storage in 'col_' and 'col_size_'
 */
void MatrixSparseSymmetric1::vecMulAddIso2D(const real* X, real* Y) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            assert_true( col_[jj][0].inx == jj );
            vecMulAddCol2(X, Y, 2*jj, col_[jj], col_size_[jj]);
        }
    }
}


/*
 Using the primary storage in 'col_' and 'col_size_'
 */
void MatrixSparseSymmetric1::vecMulAddIso3D(const real* X, real* Y, index_t jj, Element col[], size_t size) const
{
    const real X0 = X[jj  ];
    const real X1 = X[jj+1];
    const real X2 = X[jj+2];
    real Y0 = Y[jj  ] + col[0].val * X0;
    real Y1 = Y[jj+1] + col[0].val * X1;
    real Y2 = Y[jj+2] + col[0].val * X2;
    for ( unsigned n = 1 ; n < size ; ++n )
    {
        const index_t ii = 3 * col[n].inx;
        const real  a = col[n].val;
        Y[ii  ] += a * X0;
        Y[ii+1] += a * X1;
        Y[ii+2] += a * X2;
        assert_true( ii > jj );
        Y0 += a * X[ii  ];
        Y1 += a * X[ii+1];
        Y2 += a * X[ii+2];
    }
    Y[jj  ] = Y0;
    Y[jj+1] = Y1;
    Y[jj+2] = Y2;
}


/*
 Using the primary storage in 'col_' and 'col_size_'
 */
void MatrixSparseSymmetric1::vecMulAddIso3D(const real* X, real* Y) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            assert_true( col_[jj][0].inx == jj );
            vecMulAddCol3(X, Y, 3*jj, col_[jj], col_size_[jj]);
        }
    }
}


#else  // MATRIX1_OPTIMIZED_MULTIPLY enabled below


#if MATRIX1_USES_COLNEXT
void MatrixSparseSymmetric1::setNextColumn()
{
    next_[size_] = size_;

    if ( size_ > 0 )
    {
        index_t inx = size_;
        index_t nxt = size_;
        while ( --inx > 0 )
        {
            if ( col_size_[inx] > 0 )
                nxt = inx;
            next_[inx] = nxt;
        }
        if ( col_size_[0] > 0 )
            next_[0] = 0;
        else
            next_[0] = nxt;
    }
}
#endif


void MatrixSparseSymmetric1::prepareForMultiply(int dim)
{
    assert_true( size_ <= allocated_ );
    
#if MATRIX1_USES_COLNEXT
    setNextColumn();
#endif
    
#if ( 0 )
    unsigned cnt = 0;
    for ( index_t jj = 0; jj < size_; ++jj )
        cnt += ( col_size_[jj] == 0 );
    std::clog << "MatrixSparseSymmetric1 has " << cnt << " / " << size_ << " empty columns\n";
#endif

    //count number of non-zero elements, including diagonal
    unsigned nbe = 1;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
            nbe += col_size_[jj];
        else
            nbe ++;
    }
    
    //allocate classical sparse matrix storage (Numerical Recipes)
    if ( nbe > nmax_ )
    {
        delete[] ija_;
        free_real(sa_);

        nmax_  = nbe + size_;
        ija_   = new index_t[nmax_];
        sa_    = new_real(nmax_);
    }
    
    /*
     Create the compressed sparse format described in Numerical Recipe,
     Chapter 2.7 Sparse Linear Systems - Indexed Storage of Sparse Matrices
     indices however start here at zero, and everything is shifted by one index,
     compared to numerical recipe's code.
     */
    ija_[0] = size_+1;
    sa_[size_] = 42; // this is the arbitrary value
    index_t inx = size_;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            // diagonal term first:
            assert_true( column_[jj][0].inx == jj );
            sa_[jj] = column_[jj][0].val;
            // other non-zero elements:
            for ( unsigned cc = 1; cc < col_size_[jj]; ++cc )
            {
                ++inx;
                assert_true( inx < nbe );
                sa_[inx]  = column_[jj][cc].val;
                ija_[inx] = dim * column_[jj][cc].inx;
            }
        }
        else {
            sa_[jj] = 0.0;
        }
        ija_[jj+1] = inx+1;
    }
    if ( inx+1 != nbe ) ABORT_NOW("internal error");

    //printSparse(std::clog);
    //printSparseArray(std::clog);
}


void MatrixSparseSymmetric1::vecMulAdd(const real* X, real* Y, index_t jj,
                                       real const* dia, index_t start, index_t stop) const
{
    real X0 = X[jj];
    real Y0 = Y[jj] + dia[0] * X0;
    for ( index_t n = start; n < stop; ++n )
    {
        real a = sa_[n];
        index_t ii = ija_[n];
        Y[ii] += a * X0;
        Y0    += a * X[ii];
    }
    Y[jj] = Y0;
}

void MatrixSparseSymmetric1::vecMulAddIso2D(const real* X, real* Y, index_t jj,
                                            real const* dia, index_t start, index_t stop) const
{    
    real X0 = X[jj  ];
    real X1 = X[jj+1];
    real Y0 = Y[jj  ] + dia[0] * X0;
    real Y1 = Y[jj+1] + dia[0] * X1;
    for ( index_t n = start; n < stop; ++n )
    {
        index_t ii = ija_[n];
        assert_true( ii > jj );
        real a = sa_[n];
        Y0      += a * X[ii  ];
        Y1      += a * X[ii+1];
        Y[ii  ] += a * X0;
        Y[ii+1] += a * X1;
    }
    Y[jj  ] = Y0;
    Y[jj+1] = Y1;
}


void MatrixSparseSymmetric1::vecMulAddIso3D(const real* X, real* Y, index_t jj,
                                            real const* dia, index_t start, index_t stop) const
{
    real X0 = X[jj  ];
    real X1 = X[jj+1];
    real X2 = X[jj+2];
    real Y0 = Y[jj  ] + dia[0] * X0;
    real Y1 = Y[jj+1] + dia[0] * X1;
    real Y2 = Y[jj+2] + dia[0] * X2;
    for ( index_t n = start; n < stop; ++n )
    {
        index_t ii = ija_[n];
        assert_true( ii > jj );
        real a = sa_[n];
        Y0      += a * X[ii  ];
        Y1      += a * X[ii+1];
        Y2      += a * X[ii+2];
        Y[ii  ] += a * X0;
        Y[ii+1] += a * X1;
        Y[ii+2] += a * X2;
    }
    Y[jj  ] = Y0;
    Y[jj+1] = Y1;
    Y[jj+2] = Y2;
}


//------------------------------------------------------------------------------
#pragma mark - SIMD

#if MATRIX1_USES_SSE

inline void multiply2(const real* X, real* Y, index_t ii,
                      const real* val, vec2 const& xx, vec2& ss)
{
    vec2 aa = loaddup2(val);
    ss = fmadd2(load2(X+ii), aa, ss);
    store2(Y+ii, fmadd2(xx, aa, load2(Y+ii)));
}


void MatrixSparseSymmetric1::vecMulAddIso2D_SSE(const real* X, real* Y, index_t jj,
                                                real const* dia, index_t start, index_t stop) const
{
    const vec2 xx = load2(X+jj);
    vec2 ss = fmadd2(loaddup2(dia), xx, load2(Y+jj));
    // there is a dependence here for 'ss'
    for ( index_t n = start; n < stop; ++n )
        multiply2(X, Y, ija_[n], sa_+n, xx, ss);
    store2(Y+jj, ss);
}


void MatrixSparseSymmetric1::vecMulAddIso2D_SSEU(const real* X, real* Y, index_t jj,
                                                 real const* dia, index_t start, index_t stop) const
{
    const vec2 xx = load2(X+jj);
    vec2 s0 = mul2(loaddup2(dia), xx);
    vec2 s1 = load2(Y+jj);
    vec2 s2 = setzero2();
    vec2 s3 = setzero2();
    
    index_t n = start;
    index_t end = n + 4 * ( ( stop - n ) / 4 );
    // process 4 by 4:
#pragma nounroll
    for ( ; n < end; n += 4 )
    {
#if ( 0 )
        /*
         Since all the indices are different, the blocks can be processed in
         parallel, and micro-operations can be interleaved to avoid latency.
         The compiler however cannot assume this, because the indices of the
         blocks are not known at compile time.
         */
        multiply2(X, Y, ija_[n  ], sa_+n  , xx, s0);
        multiply2(X, Y, ija_[n+1], sa_+n+1, xx, s1);
        multiply2(X, Y, ija_[n+2], sa_+n+2, xx, s2);
        multiply2(X, Y, ija_[n+3], sa_+n+3, xx, s3);
#else
        /* we remove here the apparent dependency on the values of Y[],
         which are read and written, but at different indices.
         The compiler can reorder instructions to avoid lattencies */
        const index_t i0 = ija_[n  ];
        const index_t i1 = ija_[n+1];
        const index_t i2 = ija_[n+2];
        const index_t i3 = ija_[n+3];
        vec2 y0 = load2(Y+i0);
        vec2 y1 = load2(Y+i1);
        vec2 y2 = load2(Y+i2);
        vec2 y3 = load2(Y+i3);
        vec2 a0 = loaddup2(sa_+n);
        vec2 a1 = loaddup2(sa_+n+1);
        vec2 a2 = loaddup2(sa_+n+2);
        vec2 a3 = loaddup2(sa_+n+3);
        s0 = fmadd2(load2(X+i0), a0, s0);
        s1 = fmadd2(load2(X+i1), a1, s1);
        s2 = fmadd2(load2(X+i2), a2, s2);
        s3 = fmadd2(load2(X+i3), a3, s3);
        store2(Y+i0, fmadd2(xx, a0, y0));
        store2(Y+i1, fmadd2(xx, a1, y1));
        store2(Y+i2, fmadd2(xx, a2, y2));
        store2(Y+i3, fmadd2(xx, a3, y3));
#endif
    }
    // collapse 's0'
    s0 = add2(add2(s0,s1), add2(s2,s3));
    // process remaining blocks:
#pragma nounroll
    for ( ; n < stop; ++n )
        multiply2(X, Y, ija_[n], sa_+n, xx, s0);
    store2(Y+jj, s0);
}

#endif

#if MATRIX1_USES_AVX

/*
Accumulation is done here in the higher part of 'ss'
The high position of 'xx' is not used
The low position of 'ss' is used locally
*/
inline void multiply4(const real* X, real* Y, index_t ii,
                      const real* val, vec4 const& xx, vec4& ss)
{
    vec4 x = blend4(xx, broadcast2(X+ii), 0b1100);  // hi <- X , lo <- xx
    ss = blend4(load2crap(Y+ii), ss, 0b1100);    // hi <- ss, lo <- Y
    ss = fmadd4(broadcast1(val), x, ss);
    store2(Y+ii, getlo(ss));
}


void MatrixSparseSymmetric1::vecMulAddIso2D_AVX(const real* X, real* Y, index_t jj,
                                                real const* dia, index_t start, index_t stop) const
{
    const vec4 xx = broadcast2(X+jj);  // hi position
    vec4 ss = fmadd4(broadcast1(dia), xx, broadcast2(Y+jj));
    // there is a dependence here for 'ss'
    for ( index_t n = start; n < stop; ++n )
        multiply4(X, Y, ija_[n], sa_+n, xx, ss);
    store2(Y+jj, gethi(ss));
}


void MatrixSparseSymmetric1::vecMulAddIso2D_AVXU(const real* X, real* Y, index_t jj,
                                                 real const* dia, index_t start, index_t stop) const
{
    const vec4 xx = broadcast2(X+jj);  // hi and lo position
    vec4 s0 = mul4(broadcast1(dia), xx);
    vec4 s1 = broadcast2(Y+jj);
    vec4 s2 = setzero4();
    vec4 s3 = setzero4();
    
    index_t * inx = ija_ + start;
    const real * val = sa_ + start;
    const real * end = val + 4 * ((stop-start)/4);
    // process 4 by 4:
#pragma nounroll
    for ( ; val < end; val += 4 )
    {
#if ( 0 )
        /*
         Since all the indices are different, the blocks can be processed in
         parallel, and micro-operations can be interleaved to avoid latency.
         The compiler however cannot assume this, because the indices of the
         blocks are not known at compile time.
         */
        multiply4(X, Y, inx[0], val  , xx, s0);
        multiply4(X, Y, inx[1], val+1, xx, s1);
        multiply4(X, Y, inx[2], val+2, xx, s2);
        multiply4(X, Y, inx[3], val+3, xx, s3);
#else
        /* we remove here the apparent dependency on the values of Y[],
         which are read and written, but at different indices.
         The compiler can reorder instructions to avoid lattencies */
        //__m128i ii = _mm_slli_epi32(_mm_loadu_si128((__m128i*)(ija_+n)), 0x1);
        //printi(ii, "indx");
        const index_t i0 = inx[0];
        const index_t i1 = inx[1];
        const index_t i2 = inx[2];
        const index_t i3 = inx[3];
        s0 = blend4(load2crap(Y+i0),s0,0b1100);    // lo = Y
        s1 = blend4(load2crap(Y+i1),s1,0b1100);    // lo = Y
        s2 = blend4(load2crap(Y+i2),s2,0b1100);    // lo = Y
        s3 = blend4(load2crap(Y+i3),s3,0b1100);    // lo = Y
        vec4 x0 = blend4(xx,broadcast2(X+i0),0b1100);   // hi = X , lo <- xx
        vec4 x1 = blend4(xx,broadcast2(X+i1),0b1100);   // hi = X , lo <- xx
        vec4 x2 = blend4(xx,broadcast2(X+i2),0b1100);   // hi = X , lo <- xx
        vec4 x3 = blend4(xx,broadcast2(X+i3),0b1100);   // hi = X , lo <- xx
        s0 = fmadd4(broadcast1(val  ), x0, s0);
        s1 = fmadd4(broadcast1(val+1), x1, s1);
        s2 = fmadd4(broadcast1(val+2), x2, s2);
        s3 = fmadd4(broadcast1(val+3), x3, s3);
        store2(Y+i0, getlo(s0));
        store2(Y+i1, getlo(s1));
        store2(Y+i2, getlo(s2));
        store2(Y+i3, getlo(s3));
#endif
        inx += 4;
    }
    // collapse into 's0'
    s0 = add4(add4(s0,s1), add4(s2,s3));
    // process remaining values:
    end = sa_ + stop;
#pragma nounroll
    for ( ; val < end; ++val, ++inx )
        multiply4(X, Y, inx[0], val, xx, s0);
    store2(Y+jj, gethi(s0));
}

#endif

//------------------------------------------------------------------------------
#pragma mark - Matrix-Vector multiplication


void MatrixSparseSymmetric1::vecMulAdd(const real* X, real* Y, index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );

#if MATRIX1_USES_COLNEXT
    for ( index_t jj = next_[start]; jj < stop; jj = next_[jj+1] )
#else
    for ( index_t jj = start; jj < stop; ++jj )
#endif
    {
        vecMulAdd(X, Y, jj, sa_+jj, ija_[jj], ija_[jj+1]);
    }
}


void MatrixSparseSymmetric1::vecMulAddIso2D(const real* X, real* Y, index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );

#if MATRIX1_USES_COLNEXT
    for ( index_t jj = next_[start]; jj < stop; jj = next_[jj+1] )
#else
    for ( index_t jj = start; jj < stop; ++jj )
#endif
    {
#if MATRIX1_USES_AVX
        vecMulAddIso2D_AVXU(X, Y, 2*jj, sa_+jj, ija_[jj], ija_[jj+1]);
#elif MATRIX1_USES_SSE
        vecMulAddIso2D_SSEU(X, Y, 2*jj, sa_+jj, ija_[jj], ija_[jj+1]);
#else
        vecMulAddIso2D(X, Y, 2*jj, sa_+jj, ija_[jj], ija_[jj+1]);
#endif
    }
}


void MatrixSparseSymmetric1::vecMulAddIso3D(const real* X, real* Y, index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );

#if MATRIX1_USES_COLNEXT
    for ( index_t jj = next_[start]; jj < stop; jj = next_[jj+1] )
#else
    for ( index_t jj = start; jj < stop; ++jj )
#endif
    {
        vecMulAddIso3D(X, Y, 3*jj, sa_+jj, ija_[jj], ija_[jj+1]);
    }
}

#endif

