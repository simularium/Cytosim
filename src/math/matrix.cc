// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matrix.h"
#include "assert_macro.h"
#include "cblas.h"
#include <iomanip>

//------------------------------------------------------------------------------
real Matrix::value(const index_t x, const index_t y) const
{
    real* v = addr( x, y );
    if ( v == nullptr )
        return 0;
    else
        return *v;
}

real Matrix::norm_inf() const
{
    const index_t Z = size();
    real result = 0;
    for ( index_t ii = 0; ii < Z; ++ii )
    {
        for ( index_t jj = 0; jj < Z; ++jj )
        {
            real* v = addr( ii, jj );
            if ( v  &&  ( *v > result ) )
                result = *v;
        }
    }
    return result;
}

bool Matrix::nonZero() const
{
    const index_t Z = size();
    for ( index_t ii = 0; ii < Z; ++ii )
        for ( index_t jj = 0; jj < Z; ++jj )
            if ( 0 != value( ii, jj ) )
                return true;
    return false;
}

size_t Matrix::nbElements(index_t start, index_t stop) const
{
    assert_true( start <= stop );
    assert_true( stop <= size_ );
    
    size_t result = 0;
    for ( index_t jj = start; jj < stop; ++jj )
        for ( index_t ii = 0; ii < size_; ++ii )
            result += ( 0 != value( ii, jj ) );
    return result;
}

//------------------------------------------------------------------------------
void Matrix::copyBlock(real* mat, unsigned ldd, index_t sx, unsigned nx, index_t sy, unsigned ny) const
{
    assert_true( sx + nx < size() );
    assert_true( sy + ny < size() );
    
    for ( index_t ii = 0; ii < nx; ++ii )
    for ( index_t jj = 0; jj < ny; ++jj )
        mat[ii + ldd * jj] = value( sx + ii, sy + jj );
}


void Matrix::addDiagonalBlock(real* mat, const index_t ldd, const index_t si, const unsigned nb) const
{
    assert_true( si + nb < size() );

    for ( index_t jj = 0; jj < nb; ++jj )
    for ( index_t ii = 0; ii < nb; ++ii )
        mat[ ii + ldd * jj ] += value( si + ii, si + jj );
}


void Matrix::addTriangularBlock(real* mat, const index_t ldd, const index_t si, const unsigned nb, const unsigned dim) const
{
    assert_true( si + nb < size() );

    for ( index_t ii = 0; ii < nb; ++ii )
    for ( index_t jj = ii; jj < nb; ++jj )
        mat[ dim*ii + ldd * dim*jj ] += value( si + ii, si + jj );
}

//------------------------------------------------------------------------------
void Matrix::vecMul( const real* X, real* Y ) const
{
    zero_real(size(), Y);
    vecMulAdd( X, Y );
}


//------------------------------------------------------------------------------
void Matrix::printFull(std::ostream& os) const
{
    char str[32];
    const index_t Z = size();
    //printf("%i %i\n", size, size);
    for ( index_t ii = 0; ii < Z; ++ii )
    {
        for ( index_t jj = 0; jj < Z; ++jj )
        {
            real * a = addr(ii,jj);
            if ( a )
            {
                snprintf(str, sizeof(str), " %9.3f", *a);
                os << str;
            }
            else
                os << "       .  ";
        }
        std::endl(os);
    }
}

void Matrix::printSparse(std::ostream& os) const
{
    char str[256];
    const index_t Z = size();
    for ( index_t ii = 0; ii < Z; ++ii )
        for ( index_t jj = 0; jj < Z; ++jj )
        {
            real * v = addr(ii, jj);
            if ( v && *v != 0 )
            {
                snprintf(str, sizeof(str), "%6i %6i %16.6f\n", ii, jj, *v);
                os << str;
            }
        }
}

