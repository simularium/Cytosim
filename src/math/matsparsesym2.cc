// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matsparsesym2.h"
#include "assert_macro.h"
#include "cblas.h"

#include <iomanip>
#include <sstream>

#ifdef __SSE3__
#  define MATRIX2_USES_SSE REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define MATRIX2_USES_SSE 0
#endif


MatrixSparseSymmetric2::MatrixSparseSymmetric2()
{
    allocated_ = 0;
    col_       = nullptr;
    col_size_  = nullptr;
    col_max_   = nullptr;
    
#if MATRIX2_OPTIMIZED_MULTIPLY
    nmax_      = 0;
    ija_       = nullptr;
    sa_        = nullptr;
    next_  = new index_t[1];
    next_[0] = 0;
#endif
}


void MatrixSparseSymmetric2::allocate(size_t alc)
{
    if ( alc > allocated_ )
    {
        constexpr size_t chunk = 32;
        alc = ( alc + chunk - 1 ) & ~( chunk -1 );

        //fprintf(stderr, "MSS1 allocate matrix %u\n", alc);
        Element ** col_new      = new Element*[alc];
        unsigned * col_size_new = new unsigned[alc];
        size_t   * col_max_new  = new size_t[alc];
        
        index_t ii = 0;
        if ( col_ )
        {
            for ( ; ii < allocated_; ++ii )
            {
                col_new[ii]      = col_[ii];
                col_size_new[ii] = col_size_[ii];
                col_max_new[ii]  = col_max_[ii];
            }
            delete[] col_;
            delete[] col_size_;
            delete[] col_max_;
        }
        
        col_       = col_new;
        col_size_  = col_size_new;
        col_max_   = col_max_new;
        allocated_ = alc;

        for ( ; ii < alc; ++ii )
        {
            col_[ii]      = nullptr;
            col_size_[ii] = 0;
            col_max_[ii]  = 0;
        }
        
#if MATRIX2_OPTIMIZED_MULTIPLY
        delete[] next_;
        next_ = new index_t[alc+1];
        for ( size_t n = 0; n <= alc; ++n )
            next_[n] = n;
#endif
    }
}


void MatrixSparseSymmetric2::deallocate()
{
    if ( col_ )
    {
        for ( size_t ii = 0; ii < allocated_; ++ii )
            delete[] col_[ii];
        delete[] col_;       col_      = nullptr;
        delete[] col_size_;  col_size_ = nullptr;
        delete[] col_max_;   col_max_  = nullptr;
#if MATRIX2_OPTIMIZED_MULTIPLY
        delete[] next_;  next_ = nullptr;
        delete[] ija_;    ija_ = nullptr;
        free_real(sa_); sa_ = nullptr;
#endif
    }
    allocated_ = 0;
}


/// copy `cnt` elements from `src` to `dst`
void copy(unsigned cnt, MatrixSparseSymmetric2::Element * src, MatrixSparseSymmetric2::Element * dst)
{
    for ( unsigned ii = 0; ii < cnt; ++ii )
        dst[ii] = src[ii];
}

/// move `cnt` elements to next index, starting at vec[0]
void shift(unsigned cnt, MatrixSparseSymmetric2::Element * vec)
{
    for ( unsigned ii = cnt; ii > 0; --ii )
        vec[ii] = vec[ii-1];
}


void MatrixSparseSymmetric2::allocateColumn(const index_t jj, size_t alc)
{
    if ( jj >= size_ )
    {
        fprintf(stderr, "out of range index %i for matrix of size %u\n", jj, size_);
        exit(1);
    }

    if ( alc > col_max_[jj] )
    {
        //fprintf(stderr, "MSS1 allocate column %i size %u\n", jj, alc);
        constexpr size_t chunk = 32;
        alc = ( alc + chunk - 1 ) & ~( chunk -1 );
        Element * col_new = new Element[alc];
        
        if ( col_[jj] )
        {
            //copy over previous column elements
            copy(col_size_[jj], col_[jj], col_new);
            
            //release old memory
            delete[] col_[jj];
        }
        col_[jj]     = col_new;
        col_max_[jj] = alc;
    }
}


/**
 This allocate to be able to hold the matrix element if necessary
*/
real& MatrixSparseSymmetric2::operator()(index_t i, index_t j)
{
    assert_true( i < size_ );
    assert_true( j < size_ );
    //fprintf(stderr, "MSS( %6i %6i )\n", i, j);
    
    // swap to get ii > jj (address lower triangle)
    index_t ii = std::max(i, j);
    index_t jj = std::min(i, j);

    Element * col = col_[jj];
    
    if ( col_size_[jj] > 0 )
    {
        Element * e = col;
        Element * lst = col + col_size_[jj] - 1;
        
        //check all elements in the column:
        while ( e <= lst )
        {
            if ( e->inx == ii )
                return e->val;
            ++e;
        }
    }
    else
    {
        allocateColumn(jj, 2);
        col = col_[jj];
        // put diagonal term always first:
        col->reset(jj);
        if ( ii == jj )
        {
            col_size_[jj] = 1;
            return col[0].val;
        }
        //add the requested term:
        col[1].reset(ii);
        col_size_[jj] = 2;
        return col[1].val;
    }
    
    // add the requested term at the end:
    unsigned n = col_size_[jj];
    
    // allocate space for new Element if necessary:
    if ( n >= col_max_[jj] )
    {
        allocateColumn(jj, n+1);
        col = col_[jj];
    }
    
    assert_true( n < col_max_[jj] );
    col[n].reset(ii);
    ++col_size_[jj];
    
    //printColumn(jj);
    return col[n].val;
}


real* MatrixSparseSymmetric2::addr(index_t i, index_t j) const
{
    // swap to get ii > jj (address lower triangle)
    index_t ii = std::max(i, j);
    index_t jj = std::min(i, j);

    for ( unsigned kk = 0; kk < col_size_[jj]; ++kk )
        if ( col_[jj][kk].inx == ii )
            return &( col_[jj][kk].val );
    
    return nullptr;
}


//------------------------------------------------------------------------------
#pragma mark -

void MatrixSparseSymmetric2::reset()
{
    for ( index_t jj = 0; jj < size_; ++jj )
        col_size_[jj] = 0;
}


bool MatrixSparseSymmetric2::nonZero() const
{
    //check for any non-zero sparse term:
    for ( index_t jj = 0; jj < size_; ++jj )
        for ( unsigned kk = 0 ; kk < col_size_[jj] ; ++kk )
            if ( col_[jj][kk].val != 0 )
                return true;
    
    //if here, the matrix is empty
    return false;
}


void MatrixSparseSymmetric2::scale(const real alpha)
{
    for ( index_t jj = 0; jj < size_; ++jj )
        for ( unsigned n = 0; n < col_size_[jj]; ++n )
            col_[jj][n].val *= alpha;
}


void MatrixSparseSymmetric2::addTriangularBlock(real* mat, const unsigned ldd,
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
            index_t ii = col_[jj][n].inx;
            // assuming lower triangle is stored:
            assert_true( ii >= jj );
            if ( ii < up )
            {
                //printf("MSS1 %4i %4i % .4f\n", ii, jj, a);
                mat[dim*( jj-si + ldd * (ii-si) )] += col_[jj][n].val;
            }
        }
    }
}


void MatrixSparseSymmetric2::addDiagonalBlock(real* mat, unsigned ldd,
                                              const index_t si,
                                              const unsigned nb) const
{
    index_t up = si + nb;
    assert_true( up <= size_ );
    
    for ( index_t jj = si; jj < up; ++jj )
    {
        for ( unsigned n = 0; n < col_size_[jj]; ++n )
        {
            index_t ii = col_[jj][n].inx;
            // assuming lower triangle is stored:
            assert_true( ii >= jj );
            if ( ii < up )
            {
                //printf("MSS1 %4i %4i % .4f\n", ii, jj, a);
                mat[jj-si+ldd*(ii-si)] += col_[jj][n].val;
                if ( jj != ii )
                    mat[ii-si+ldd*(jj-si)] += col_[jj][n].val;
            }
        }
    }
}


int MatrixSparseSymmetric2::bad() const
{
    if ( size_ <= 0 ) return 1;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        for ( unsigned kk = 0 ; kk < col_size_[jj] ; ++kk )
        {
            if ( col_[jj][kk].inx >= size_ ) return 2;
            if ( col_[jj][kk].inx <= jj )   return 3;
        }
    }
    return 0;
}


size_t MatrixSparseSymmetric2::nbElements(index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );
    //all allocated elements are counted, even if zero
    size_t cnt = 0;
    for ( index_t jj = start; jj < stop; ++jj )
        cnt += col_size_[jj];
    return cnt;
}

//------------------------------------------------------------------------------
#pragma mark -

std::string MatrixSparseSymmetric2::what() const
{
    std::ostringstream msg;
#if MATRIX2_USES_SSE
    msg << "MSS2e " << nbElements();
#else
    msg << "MSS2 " << nbElements();
#endif
    return msg.str();
}

void MatrixSparseSymmetric2::printSparse(std::ostream& os) const
{
    std::streamsize p = os.precision();
    os.precision(8);
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        for ( unsigned n = 0 ; n < col_size_[jj] ; ++n )
        {
            os << col_[jj][n].inx << " " << jj << " ";
            os << col_[jj][n].val << "\n";
        }
    }
    os.precision(p);
}


void MatrixSparseSymmetric2::printColumns(std::ostream& os)
{
    os << "MSS2 size " << size_ << ":";
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        os << "\n   " << jj << "   " << col_size_[jj];
#if MATRIX2_OPTIMIZED_MULTIPLY
        os << " next " << next_[jj];
#endif
    }
    std::endl(os);
}


void MatrixSparseSymmetric2::printColumn(std::ostream& os, const index_t jj)
{
    Element const* col = col_[jj];
    os << "MSS2 col " << jj << ":";
    for ( unsigned n = 0; n < col_size_[jj]; ++n )
    {
        os << "\n" << col[n].inx << " :";
        os << " " << col[n].val;
    }
    std::endl(os);
}

//------------------------------------------------------------------------------
#pragma mark -

#if !MATRIX2_OPTIMIZED_MULTIPLY

void MatrixSparseSymmetric2::prepareForMultiply(int)
{
}


void MatrixSparseSymmetric2::vecMulAdd(const real* X, real* Y) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            const real X0 = X[jj];
            assert_true( col_[jj][0].inx == jj );
            real Y0 = Y[jj] + col_[jj][0].val * X0;
            for ( unsigned kk = 1 ; kk < col_size_[jj] ; ++kk )
            {
                const index_t ii = col_[jj][kk].inx;
                const real a = col_[jj][kk].val;
                Y[ii] += a * X0;
                Y0 += a * X[ii];
            }
            Y[jj] = Y0;
        }
    }
}


void MatrixSparseSymmetric2::vecMulAddIso2D(const real* X, real* Y) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            const index_t Djj = 2 * jj;
            const real X0 = X[Djj  ];
            const real X1 = X[Djj+1];
            assert_true( col_[jj][0].inx == jj );
            real Y0 = Y[Djj  ] + col_[jj][0].val * X0;
            real Y1 = Y[Djj+1] + col_[jj][0].val * X1;
            for ( unsigned kk = 1 ; kk < col_size_[jj] ; ++kk )
            {
                const index_t Dii = 2 * col_[jj][kk].inx;
                const real  a = col_[jj][kk].val;
                Y[Dii  ] += a * X0;
                Y[Dii+1] += a * X1;
                assert_true( Dii != Djj );
                Y0 += a * X[Dii  ];
                Y1 += a * X[Dii+1];
            }
            Y[Djj  ] = Y0;
            Y[Djj+1] = Y1;
        }
    }
}


void MatrixSparseSymmetric2::vecMulAddIso3D(const real* X, real* Y) const
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        if ( col_size_[jj] > 0 )
        {
            const index_t Djj = 3 * jj;
            const real X0 = X[Djj  ];
            const real X1 = X[Djj+1];
            const real X2 = X[Djj+2];
            assert_true( col_[jj][0].inx == jj );
            real Y0 = Y[Djj  ] + col_[jj][0].val * X0;
            real Y1 = Y[Djj+1] + col_[jj][0].val * X1;
            real Y2 = Y[Djj+2] + col_[jj][0].val * X2;
            for ( unsigned kk = 1 ; kk < col_size_[jj] ; ++kk )
            {
                const index_t Dii = 3 * col_[jj][kk].inx;
                const real  a = col_[jj][kk].val;
                Y[Dii  ] += a * X0;
                Y[Dii+1] += a * X1;
                Y[Dii+2] += a * X2;
                assert_true( Dii != Djj );
                Y0 += a * X[Dii  ];
                Y1 += a * X[Dii+1];
                Y2 += a * X[Dii+2];
            }
            Y[Djj  ] = Y0;
            Y[Djj+1] = Y1;
            Y[Djj+2] = Y2;
        }
    }
}


#else  // MATRIX2_OPTIMIZED_MULTIPLY


void MatrixSparseSymmetric2::setNextColumn()
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

/**
 Create the sparse representation, described in numerical-recipe,
 however indices start at zero, unlike in numerical recipe
*/
void MatrixSparseSymmetric2::prepareForMultiply(int)
{
    assert_true( size_ <= allocated_ );
    
    setNextColumn();
    
    //count number of non-zero elements, including diagonal
    unsigned nbe = 1+size_;
    for ( index_t jj = 0; jj < size_; ++jj )
        nbe += col_size_[jj];
    
    //allocate classical sparse matrix storage (Numerical Recipes)
    if ( nbe > nmax_ )
    {
        delete[] ija_;
        free_real(sa_);
        
        nmax_  = nbe + size_;
        ija_   = new index_t[nmax_];
        sa_    = new_real(nmax_);
    }
    
    ija_[0] = size_+1;
    index_t kk = size_;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        sa_[jj] = 0.0;
        for ( unsigned cc = 0; cc < col_size_[jj]; ++cc )
        {
            if ( col_[jj][cc].inx == jj )
                sa_[jj] = col_[jj][cc].val;
            else
            {
                ++kk;
                assert_true( kk < nbe );
                sa_[kk]  = col_[jj][cc].val;
                ija_[kk] = col_[jj][cc].inx;
            }
        }
        ija_[jj+1] = kk+1;
    }
    if ( kk >= nbe )
        ABORT_NOW("internal out of range error, memory was corrupted");
}


void MatrixSparseSymmetric2::vecMulAdd(const real* X, real* Y) const
{
    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
        assert_true( col_size_[jj] > 0 );
        real X0 = X[jj];
        real Y0 = Y[jj] + sa_[jj] * X0;
        const index_t end = ija_[jj+1];
        for ( index_t kk = ija_[jj]; kk < end; ++kk )
        {
            real a = sa_[kk];
            index_t ii = ija_[kk];
            Y[ii] += a * X0;
            Y0    += a * X[ii];
        }
        Y[jj] = Y0;
    }
}

//------------------------------------------------------------------------------

#if MATRIX2_USES_SSE

void MatrixSparseSymmetric2::vecMulAddIso2D(const real* X, real* Y) const
{
    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
        assert_true( col_size_[jj] > 0 );
        assert_true( col_[jj][0].inx == jj );
        vec2 xx = load2(X+2*jj);
        vec2 aa = loaddup2(sa_+jj);
        vec2 yy = add2(load2(Y+2*jj), mul2(aa, xx));
        const index_t end = ija_[jj+1];
        for ( index_t kk = ija_[jj]; kk < end; ++kk )
        {
            vec2 tt;
            aa = loaddup2(sa_+kk);
            tt = add2(load2(Y+2*ija_[kk]), mul2(xx, aa));
            yy = add2(yy, mul2(load2(X+2*ija_[kk]), aa));
            store2(Y+2*ija_[kk], tt);
        }
        store2(Y+2*jj, yy);
    }
}

#else

void MatrixSparseSymmetric2::vecMulAddIso2D(const real* X, real* Y) const
{    
    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
        assert_true( col_size_[jj] > 0 );
        assert_true( col_[jj][0].inx == jj );
        index_t Djj = 2 * jj;
        real X0 = X[Djj  ];
        real X1 = X[Djj+1];
        real Y0 = Y[Djj  ] + sa_[jj] * X0;
        real Y1 = Y[Djj+1] + sa_[jj] * X1;
        const index_t end = ija_[jj+1];
        for ( index_t kk = ija_[jj]; kk < end; ++kk )
        {
            index_t Dii = 2 * ija_[kk];
            assert_true( Djj != Dii );
            real a = sa_[kk];
            Y0       += a * X[Dii  ];
            Y1       += a * X[Dii+1];
            Y[Dii  ] += a * X0;
            Y[Dii+1] += a * X1;
        }
        Y[Djj  ] = Y0;
        Y[Djj+1] = Y1;
    }
}

#endif


void MatrixSparseSymmetric2::vecMulAddIso3D(const real* X, real* Y) const
{
    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
        assert_true( col_size_[jj] > 0 );
        assert_true( col_[jj][0].inx == jj );
        index_t Djj = 3 * jj;
        real X0 = X[Djj  ];
        real X1 = X[Djj+1];
        real X2 = X[Djj+2];
        real Y0 = Y[Djj  ] + sa_[jj] * X0;
        real Y1 = Y[Djj+1] + sa_[jj] * X1;
        real Y2 = Y[Djj+2] + sa_[jj] * X2;
        const index_t next = ija_[jj+1];
        for ( index_t kk = ija_[jj]; kk < next; ++kk )
        {
            index_t Dii = 3 * ija_[kk];
            assert_true( Djj != Dii );
            real a = sa_[kk];
            Y0       += a * X[Dii  ];
            Y1       += a * X[Dii+1];
            Y2       += a * X[Dii+2];
            Y[Dii  ] += a * X0;
            Y[Dii+1] += a * X1;
            Y[Dii+2] += a * X2;
        }
        Y[Djj  ] = Y0;
        Y[Djj+1] = Y1;
        Y[Djj+2] = Y2;
    }
}

#endif

