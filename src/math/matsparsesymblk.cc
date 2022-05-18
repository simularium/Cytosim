// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matsparsesymblk.h"
#include "assert_macro.h"
#include "vector2.h"
#include "vector3.h"
#include <sstream>

// Flag to enable AVX implementation
#ifdef __AVX__
#  define MATRIXSSB_USES_AVX REAL_IS_DOUBLE
#else
#  define MATRIXSSB_USES_AVX 0
#endif


MatrixSparseSymmetricBlock::MatrixSparseSymmetricBlock()
{
    allocated_ = 0;
    column_    = nullptr;
    
    next_  = new index_t[1];
    next_[0] = 0;
}


void MatrixSparseSymmetricBlock::allocate(size_t alc)
{
    if ( alc > allocated_ )
    {
        /*
         'chunk' can be increased to gain performance:
          more memory will be used, but reallocation will be less frequent
        */
        constexpr size_t chunk = 64;
        alc = ( alc + chunk - 1 ) & ~( chunk -1 );

        //fprintf(stderr, "MSSB allocates %u\n", alc);
        Column * col_new = new Column[alc];
       
        if ( column_ )
        {
            for (size_t  n = 0; n < allocated_; ++n )
                col_new[n] = column_[n];
            delete[] column_;
        }
        
        column_    = col_new;
        allocated_ = alc;
        
        delete[] next_;
        next_ = new index_t[alc+1];
        for ( size_t n = 0; n <= alc; ++n )
            next_[n] = n;
    }
}


void MatrixSparseSymmetricBlock::deallocate()
{
    delete[] column_;
    delete[] next_;
    column_ = nullptr;
    next_   = nullptr;
    allocated_ = 0;
}


void MatrixSparseSymmetricBlock::Column::allocate(size_t alc)
{
    if ( alc > allo_ )
    {
        //fprintf(stderr, "MSSB reallocates column %i for %u: %p\n", inx_[0], alc);
        //else fprintf(stderr, "MSSB allocates column for %u: %p\n", alc);
        /*
         'chunk' can be increased, to possibly gain performance:
         more memory will be used, but reallocation will be less frequent
         */
        constexpr size_t chunk = 16;
        alc = ( alc + chunk - 1 ) & ~( chunk - 1 );
        
        // use aligned memory:
        void * ptr = new_real(alc*sizeof(SquareBlock)/sizeof(real));
        SquareBlock * blk_new  = new(ptr) SquareBlock[alc];

        if ( posix_memalign(&ptr, 32, alc*sizeof(index_t)) )
            throw std::bad_alloc();
        index_t * inx_new = (index_t*)ptr;

        if ( inx_ )
        {
            for ( index_t n = 0; n < size_; ++n )
                inx_new[n] = inx_[n];
            free(inx_);
        }

        if ( blk_ )
        {
            for ( index_t n = 0; n < size_; ++n )
                blk_new[n] = blk_[n];
            free_real(blk_);
        }
        inx_  = inx_new;
        blk_  = blk_new;
        allo_ = alc;
        
        //std::clog << "Column " << this << "  " << alc << ": ";
        //std::clog << " alignment " << ((uintptr_t)elem_ & 63) << "\n";
    }
}


void MatrixSparseSymmetricBlock::Column::deallocate()
{
    //if ( inx_ ) fprintf(stderr, "MSSB deallocates column %i\n", inx_[0]);
    free(inx_);
    free_real(blk_);
    inx_ = nullptr;
    blk_ = nullptr;
}


void MatrixSparseSymmetricBlock::Column::operator =(MatrixSparseSymmetricBlock::Column & col)
{
    //if ( inx_ ) fprintf(stderr, "MSSB transfers column %u\n", inx_[0]);
    free(inx_);
    free_real(blk_);

    size_ = col.size_;
    allo_ = col.allo_;
    inx_ = col.inx_;
    blk_ = col.blk_;
    
    col.size_ = 0;
    col.allo_ = 0;
    col.inx_ = nullptr;
    col.blk_ = nullptr;
}

/**
 This allocate to be able to hold the matrix element if necessary
 */
SquareBlock& MatrixSparseSymmetricBlock::Column::block(index_t ii, index_t jj)
{
    assert_true( ii >= jj );
    if ( size_ > 0 )
    {
        if ( inx_[0] == ii )
            return blk_[0];
        /* This is a silly search that could be optimized */
        for ( index_t n = 1; n < size_; ++n )
            if ( inx_[n] == ii )
                return blk_[n];
    }
    else
    {
        allocate(2);
        // put diagonal term always first:
        inx_[0] = jj;
        blk_[0].reset();
        if ( ii == jj )
        {
            size_ = 1;
            return blk_[0];
        }
        //add the requested term:
        inx_[1] = ii;
        blk_[1].reset();
        size_ = 2;
        return blk_[1];
    }
    
    // add the requested term last:
    index_t n = size_;
    
    // allocate space for new Element if necessary:
    if ( n >= allo_ )
        allocate(n+1);
    
    assert_true( n < allo_ );
    inx_[n] = ii;
    blk_[n].reset();
    size_ = n + 1;
    
    //printColumn(jj);
    return blk_[n];
}


void MatrixSparseSymmetricBlock::Column::reset()
{
    size_ = 0;
}

SquareBlock& MatrixSparseSymmetricBlock::diag_block(index_t ii)
{
    assert_true( ii < size_ );
    Column & col = column_[ii];
    if ( col.size_ == 0 )
    {
        //fprintf(stderr, "new diagonal element for column %i\n", ii);
        col.allocate(1);
        col.size_ = 1;
        // put diagonal term always first:
        col.inx_[0] = ii;
        col.blk_[0].reset();
    }
    assert_true(col.inx_[0] == ii);
    return col.blk_[0];
}


real& MatrixSparseSymmetricBlock::operator()(index_t iii, index_t jjj)
{
    // branchless code to address lower triangle
    index_t ii = std::max(iii, jjj);
    index_t jj = std::min(iii, jjj);
#if ( BLOCK_SIZE == 1 )
    return column_[jj].block(ii, jj).value();
#else
    index_t i = ii % BLOCK_SIZE;
    index_t j = jj % BLOCK_SIZE;
    return column_[jj-j].block(ii-i, jj-j)(i, j);
#endif
}


real* MatrixSparseSymmetricBlock::addr(index_t iii, index_t jjj) const
{
    // branchless code to address lower triangle
    index_t ii = std::max(iii, jjj);
    index_t jj = std::min(iii, jjj);
#if ( BLOCK_SIZE == 1 )
    return &column_[jj].block(ii, jj).value();
#else
    index_t i = ii % BLOCK_SIZE;
    index_t j = jj % BLOCK_SIZE;
    return column_[jj-j].block(ii-i, jj-j).addr(i, j);
#endif
}


//------------------------------------------------------------------------------
#pragma mark -

void MatrixSparseSymmetricBlock::reset()
{
    for ( index_t n = 0; n < size_; ++n )
        column_[n].reset();
}


bool MatrixSparseSymmetricBlock::nonZero() const
{
    //check for any non-zero sparse term:
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        Column & col = column_[jj];
        for ( unsigned n = 0 ; n < col.size_ ; ++n )
            if ( col[n] != 0.0 )
                return true;
    }
    //if here, the matrix is empty
    return false;
}


void MatrixSparseSymmetricBlock::scale(const real alpha)
{
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        Column & col = column_[jj];
        for ( unsigned n = 0 ; n < col.size_ ; ++n )
            col[n].scale(alpha);
    }
}


void MatrixSparseSymmetricBlock::addTriangularBlock(real* mat, const unsigned ldd,
                                                const index_t si,
                                                const unsigned nb,
                                                const unsigned dim) const
{
    if ( si % BLOCK_SIZE )  ABORT_NOW("index incompatible with matrix block size");
    if ( nb % BLOCK_SIZE )  ABORT_NOW("size incompatible with matrix block size");

    index_t up = si + nb;
    index_t off = si + ldd * si;
    assert_true( up <= size_ );
    
    for ( index_t jj = si; jj < up; ++jj )
    {
        Column & col = column_[jj];
        if ( col.size_ > 0 )
        {
            assert_true(col.inx_[0] == jj);
            col[0].addto_upper(mat + ( jj + ldd*jj ) - off, ldd);
            for ( index_t n = 1; n < col.size_; ++n )
            {
                index_t ii = col.inx_[n];
                // assuming lower triangle is stored:
                assert_true( ii > jj );
                if ( ii < up )
                {
                    //fprintf(stderr, "`B %4i %4i % .4f\n", ii, jj, a);
                    col[n].addto_trans(mat + ( jj + ldd*ii ) - off, ldd);
                }
            }
        }
    }
}


void MatrixSparseSymmetricBlock::addDiagonalBlock(real* mat, unsigned ldd,
                                              const index_t si,
                                              const unsigned nb) const
{
    if ( si % BLOCK_SIZE )  ABORT_NOW("index incompatible with matrix block size");
    if ( nb % BLOCK_SIZE )  ABORT_NOW("size incompatible with matrix block size");
    
    index_t up = si + nb;
    index_t off = si + ldd * si;
    assert_true( up <= size_ );
    
    for ( index_t jj = si; jj < up; ++jj )
    {
        Column & col = column_[jj];
        if ( col.size_ > 0 )
        {
            assert_true(col.inx_[0] == jj);
            col[0].addto_symm(mat+( jj + ldd*jj )-off, ldd);
            for ( index_t n = 1; n < col.size_; ++n )
            {
                index_t ii = col.inx_[n];
                // assuming lower triangle is stored:
                assert_true( ii > jj );
                if ( ii < up )
                {
                    //fprintf(stderr, "MSSB %4i %4i % .4f\n", ii, jj, a);
                    col[n].addto(mat + ( ii + ldd*jj ) - off, ldd);
                    col[n].addto_trans(mat + ( jj + ldd*ii ) - off, ldd);
                }
            }
        }
    }
}


int MatrixSparseSymmetricBlock::bad() const
{
    if ( size_ <= 0 ) return 1;
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        Column & col = column_[jj];
        for ( unsigned n = 0 ; n < col.size_ ; ++n )
        {
            if ( col.inx_[n] >= size_ ) return 2;
            if ( col.inx_[n] <= jj )    return 3;
        }
    }
    return 0;
}


size_t MatrixSparseSymmetricBlock::nbElements(index_t start, index_t end) const
{
    assert_true( start <= end );
    assert_true( end <= size_ );
    //all allocated elements are counted, even if zero
    size_t cnt = 0;
    for ( index_t jj = start; jj < end; ++jj )
        cnt += column_[jj].size_;
    return cnt;
}


//------------------------------------------------------------------------------
#pragma mark -


std::string MatrixSparseSymmetricBlock::what() const
{
    std::ostringstream msg;
#if MATRIXSSB_USES_AVX
    msg << "MSSBx " << SquareBlock::what() << "*" << nbElements();
#elif defined(__SSE3__) &&  REAL_IS_DOUBLE
    msg << "MSSBe " << SquareBlock::what() << "*" << nbElements();
#else
    msg << "MSSB " << SquareBlock::what() << "*" << nbElements();
#endif
    return msg.str();
}


void MatrixSparseSymmetricBlock::printSparse(std::ostream& os) const
{
    char str[256];
    std::streamsize p = os.precision();
    os.precision(8);
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        Column & col = column_[jj];
        if ( col.size_ > 0 )
            os << "% column " << jj << "\n";
        for ( unsigned n = 0 ; n < col.size_ ; ++n )
        {
            index_t ii = col.inx_[n];
            SquareBlock blk = col.blk_[n];
            int d = ( ii == jj );
            for ( int x = 0  ; x < BLOCK_SIZE; ++x )
            for ( int y = x*d; y < BLOCK_SIZE; ++y )
            {
                real v = blk(y, x);
                if ( v != 0 )
                {
                    snprintf(str, sizeof(str), "%6i %6i %16.6f\n", ii+y, jj+x, blk(y, x));
                    os << str;
                }
            }
        }
    }
    os.precision(p);
}


void MatrixSparseSymmetricBlock::printColumns(std::ostream& os)
{
    os << "MSSB size " << size_ << ":";
    for ( index_t jj = 0; jj < size_; ++jj )
    {
        os << "\n   " << jj << "   " << column_[jj].size_;
        os << " next " << next_[jj];
    }
    std::endl(os);
}


void MatrixSparseSymmetricBlock::Column::print(std::ostream& os) const
{
    for ( index_t n = 0; n < size_; ++n )
        os << "\n" << inx_[n] << " : " << blk_[n] << "\n";
    std::endl(os);
}


//------------------------------------------------------------------------------
#pragma mark - Vector Multiplication


/// A block element of the sparse matrix suitable for qsort()
class alignas(32) MatrixSparseSymmetricBlock::Element
{
public:
    /// index
    index_t inx;

    /// block element
    SquareBlock blk;
};


/// function for qsort, comparing line indices
int compareMSSBElement(const void * p, const void * q)
{
    int a = static_cast<MatrixSparseSymmetricBlock::Element const*>(p)->inx;
    int b = static_cast<MatrixSparseSymmetricBlock::Element const*>(q)->inx;

    return ( a > b ) - ( b > a );
}

/**
 This copies the data to the provided temporary array
 */
void MatrixSparseSymmetricBlock::Column::sort(Element*& tmp, size_t tmp_size)
{
    assert_true( size_ <= tmp_size );
    for ( unsigned i = 1; i < size_; ++i )
    {
        tmp[i].blk = blk_[i];
        tmp[i].inx = inx_[i];
    }
    
    //std::clog << "sizeof(Element) " << sizeof(Element) << "\n";
    qsort(tmp+1, size_-1, sizeof(Element), &compareMSSBElement);
    
    for ( unsigned i = 1; i < size_; ++i )
    {
         blk_[i] = tmp[i].blk;
         inx_[i] = tmp[i].inx;
    }
}


size_t newElements(MatrixSparseSymmetricBlock::Element*& ptr, size_t size)
{
    constexpr size_t chunk = 16;
    size_t all = ( size + chunk - 1 ) & ~( chunk - 1 );
    free(ptr);  // there is no destructor with Element
    void* tmp = nullptr;
    if ( size > 0 )
    {
        if ( posix_memalign(&tmp, 32, all*sizeof(MatrixSparseSymmetricBlock::Element)) )
            throw std::bad_alloc();
        ptr = new(tmp) MatrixSparseSymmetricBlock::Element[all];
    }
    else
        ptr = nullptr;
    return all;
}


void MatrixSparseSymmetricBlock::prepareForMultiply(int)
{
    next_[size_] = size_;
    
    if ( size_ > 0 )
    {
        index_t inx = size_;
        index_t nxt = size_;
        while ( --inx > 0 )
        {
            if ( column_[inx].size_ > 0 )
                nxt = inx;
            next_[inx] = nxt;
        }
        if ( column_[0].size_ > 0 )
            next_[0] = 0;
        else
            next_[0] = nxt;
    }
    
    //unsigned cnt = 0;
    size_t tmp_size = 0;
    Element * tmp = nullptr;

    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
        Column & col = column_[jj];
        //std::clog << "MSSB column " << jj << " has " << col.size_ << " elements\n";

        // order the elements within the column:
        if ( col.size_ > 2 )
        {
            if ( tmp_size < col.size_ )
                tmp_size = newElements(tmp, col.size_);
            col.sort(tmp, tmp_size);
        }
        
        //++cnt;
        assert_true( jj < size_ );
        assert_true( col.size_ > 0 );
        
        // diagonal element should be first:
        assert_true( col.inx_[0] == jj );
        col.blk_[0].copy_lower();
#ifndef NDEBUG
        for ( unsigned n = 1 ; n < col.size_ ; ++n )
        {
            const index_t ii = col.inx_[n];
            assert_true( ii < size_ );
            assert_true( ii != jj );
        }
#endif
    }
    
    newElements(tmp, 0);
    //std::clog << "MatrixSparseSymmetricBlock " << size_ << " with " << cnt << " non-empty columns\n";
}


//------------------------------------------------------------------------------
#pragma mark - Column Vector Multiplication


void MatrixSparseSymmetricBlock::Column::vecMulAdd1D(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 1 )
    assert_true( size_ > 0 );
    const real X0 = X[jj];
    real D = blk_[0].value();
    real Y0 = Y[jj] + D * X0;
    assert_true(inx_[0]==jj);
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        const real M = blk_[n].value();
        Y[ii] += M * X0;
        Y0 += M * X[ii];
    }
    Y[jj] = Y0;
#endif
}


void MatrixSparseSymmetricBlock::Column::vecMulAdd2D(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 2 )
    assert_true( size_ > 0 );
    const Vector2 xx(X+jj);
    assert_true(inx_[0]==jj);
    assert_small(blk_[0].asymmetry());
    Vector2 yy = blk_[0].vecmul(xx);
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        SquareBlock const& M = blk_[n];
        M.vecmul(xx).add_to(Y+ii);
        yy += M.trans_vecmul(X+ii);
    }
    yy.add_to(Y+jj);
#endif
}

void MatrixSparseSymmetricBlock::Column::vecMulAdd3D(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 3 )
    assert_true( size_ > 0 );
    const Vector3 xxx(X+jj);
    assert_true(inx_[0]==jj);
    assert_small(blk_[0].asymmetry());
    Vector3 yyy = blk_[0].vecmul(xxx);
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        SquareBlock const& M = blk_[n];
        M.vecmul(xxx).add_to(Y+ii);
        yyy += M.trans_vecmul(X+ii);
    }
    yyy.add_to(Y+jj);
#endif
}


void MatrixSparseSymmetricBlock::Column::vecMulAdd4D(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 4 )
    assert_true( size_ > 0 );
    const vec4 xxxx = load4(X+jj);
    assert_true(inx_[0]==jj);
    assert_small(blk_[0].asymmetry());
    vec4 yyyy = blk_[0].vecmul4(xxxx);
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        SquareBlock const& M = blk_[n];
        store4(Y+ii, add4(load4(Y+ii), M.vecmul4(xxxx)));
        yyyy += M.trans_vecmul(X+ii);
    }
    store4(Y+jj, add4(yyyy, load4(Y+jj)));
#endif
}

//------------------------------------------------------------------------------
#pragma mark - Manually Optimized Vector Multiplication

#include "simd.h"

void MatrixSparseSymmetricBlock::Column::vecMulAdd2D_SSE(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 2 ) && defined(__SSE3__) && REAL_IS_DOUBLE
    vec2 x0, x1;
    vec2 yy = load2(Y+jj);
    {
        //const real X0 = X[jj  ];
        //const real X1 = X[jj+1];
        vec2 xx = load2(X+jj);
        x0 = unpacklo2(xx, xx);
        x1 = unpackhi2(xx, xx);
        
        // load 2x2 matrix element into 2 vectors:
        real const* M = blk_[0];
        //assume the block is already symmetrized:
        //real Y0 = Y[jj  ] + M[0] * X0 + M[1] * X1;
        //real Y1 = Y[jj+1] + M[1] * X0 + M[3] * X1;
        xx = add2(mul2(load2(M  ), x0), yy);
        yy = add2(mul2(load2(M+2), x1), xx);
    }
    
    // while x0 and x1 are constant, there is a dependency in the loop for 'yy'.
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        vec2 xx = load2(X+ii);
        
        // load 2x2 matrix element into 2 vectors:
        real const* M = blk_[n];
        vec2 m01 = load2(M);
        vec2 m23 = load2(M+2);
        
        // multiply with the full block:
        //Y[ii  ] += M[0] * X0 + M[2] * X1;
        //Y[ii+1] += M[1] * X0 + M[3] * X1;
        vec2 mx0 = add2(mul2(m01, x0), load2(Y+ii));
        mx0 = add2(mul2(m23, x1), mx0);
        store2(Y+ii, mx0);

        // multiply with the transposed block:
        //Y0 += M[0] * X[ii] + M[1] * X[ii+1];
        //Y1 += M[2] * X[ii] + M[3] * X[ii+1];
        vec2 mxx = mul2(m01, xx);
        vec2 myy = mul2(m23, xx);
        yy = add2(add2(unpacklo2(mxx, myy), unpackhi2(mxx, myy)), yy);
    }
    //Y[jj  ] = Y0;
    //Y[jj+1] = Y1;
    store2(Y+jj, yy);
#endif
}

void MatrixSparseSymmetricBlock::Column::vecMulAdd2D_AVX(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 2 ) && MATRIXSSB_USES_AVX
    // xy = { X0 X1 X0 X1 }
    vec4 xy = broadcast2(X+jj);
    //multiply with full block, assuming it was symmetrized:
    //real Y0 = M[0] * X0 + M[1] * X1;
    //real Y1 = M[1] * X0 + M[3] * X1;
    
    // yyyy = { Y0 Y0 Y1 Y1 }
    // load 2x2 matrix element into 2 vectors:
    vec4 ss = mul4(load4(blk_[0]), xy);

    //const real X0 = X[jj  ];
    //const real X1 = X[jj+1];
    // xxyy = { X0 X0 X1 X1 }
    const vec4 xxyy = permute4(xy, 0b1100);

    // while x0 and x1 are constant, there is a dependency in the loop for 'yy'.
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t& ii = inx_[n];
        vec4 mat = load4(blk_[n]);      // load 2x2 matrix
        vec4 yy = load2Z(Y+ii);         // yy = { Y0 Y1 0 0 }
        vec4 xx = broadcast2(X+ii);     // xx = { X0 X1 X0 X1 }

        // multiply with the full block:
        //Y[ii  ] += M[0] * X0 + M[2] * X1;
        //Y[ii+1] += M[1] * X0 + M[3] * X1;
        vec4 u = fmadd4(mat, xxyy, yy);
        store2(Y+ii, add2(getlo(u), gethi(u)));
        
        // multiply with the transposed block:
        //Y0 += M[0] * X[ii] + M[1] * X[ii+1];
        //Y1 += M[2] * X[ii] + M[3] * X[ii+1];
        ss = fmadd4(mat, xx, ss);
    }
    // need to collapse yyyy = { S0 S0 S1 S1 }
    // Y[jj  ] += yyyy[0] + yyyy[1];
    // Y[jj+1] += yyyy[2] + yyyy[3];
    vec2 yy = load2(Y+jj);
    vec2 h = gethi(ss);
    store2(Y+jj, add2(yy, add2(unpacklo2(getlo(ss), h), unpackhi2(getlo(ss), h))));
#endif
}


#if ( BLOCK_SIZE == 2 ) && MATRIXSSB_USES_AVX
inline void multiply2D(real const* X, real* Y, unsigned ii, vec4 const& mat, vec4 const& xxxx, vec4& ss)
{
    vec4 xx = broadcast2(X+ii);
    vec4 u = fmadd4(mat, xxxx, load2Z(Y+ii));
    store2(Y+ii, add2(getlo(u), gethi(u)));
    ss = fmadd4(mat, xx, ss);
}
#endif

void MatrixSparseSymmetricBlock::Column::vecMulAdd2D_AVXU(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 2 ) && MATRIXSSB_USES_AVX
    vec4 xyxy = broadcast2(X+jj);
    vec4 ss = mul4(load4(blk_[0]), xyxy);
    const vec4 xxyy = permute4(xyxy, 0b1100);
    vec4 s1 = setzero4();

    unsigned n = 1;
    const unsigned stop = 1 + 2 * ( ( size_ - 1 ) / 2 );
    // process 4 by 4:
    for ( ; n < stop; n += 2 )
    {
#if ( 0 )
        /*
         Since all the indices are different, the blocks can be processed in
         parallel, and micro-operations can be interleaved to avoid latency.
         The compiler however cannot assume this, because the indices of the
         blocks are not known at compile time.
         */
        multiply2D(X, Y, inx_[n  ], load4(blk_[n  ]), xxyy, ss);
        multiply2D(X, Y, inx_[n+1], load4(blk_[n+1]), xxyy, s1);
#else
        /* we remove here the apparent dependency on the values of Y[],
         which are read and written, but at different indices.
         The compiler can reorder instructions to avoid lattencies */
        const index_t i0 = inx_[n  ];
        const index_t i1 = inx_[n+1];
        vec4 mat0 = load4(blk_[n  ]);
        vec4 mat1 = load4(blk_[n+1]);
        vec4 u0 = fmadd4(mat0, xxyy, load2Z(Y+i0));
        vec4 u1 = fmadd4(mat1, xxyy, load2Z(Y+i1));
        ss = fmadd4(mat0, broadcast2(X+i0), ss);
        s1 = fmadd4(mat1, broadcast2(X+i1), s1);
        store2(Y+i0, add2(getlo(u0), gethi(u0)));
        store2(Y+i1, add2(getlo(u1), gethi(u1)));
#endif
    }
    // collapse 'ss'
    ss = add4(ss, s1);
    // process remaining blocks:
    for ( ; n < size_; ++n )
        multiply2D(X, Y, inx_[n], load4(blk_[n]), xxyy, ss);
    /* finally horizontally sum ss = { SX SX SY SY } */
    vec2 h = gethi(ss);
    h = add2(unpacklo2(getlo(ss), h), unpackhi2(getlo(ss), h));
    store2(Y+jj, add2(load2(Y+jj), h));
#endif
}


void MatrixSparseSymmetricBlock::Column::vecMulAdd2D_AVXU4(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 2 ) && MATRIXSSB_USES_AVX
    vec4 xyxy = broadcast2(X+jj);
    vec4 ss = mul4(load4(blk_[0]), xyxy);
    const vec4 xxyy = permute4(xyxy, 0b1100);
    vec4 s1 = setzero4();
    vec4 s2 = setzero4();
    vec4 s3 = setzero4();

    unsigned n = 1;
    const unsigned stop = 1 + 4 * ( ( size_ - 1 ) / 4 );
    // process 4 by 4:
    for ( ; n < stop; n += 4 )
    {
#if ( 0 )
        /*
         Since all the indices are different, the blocks can be processed in
         parallel, and micro-operations can be interleaved to avoid latency.
         The compiler however cannot assume this, because the indices of the
         blocks are not known at compile time.
         */
        multiply2D(X, Y, inx_[n  ], load4(blk_[n  ]), xxyy, ss);
        multiply2D(X, Y, inx_[n+1], load4(blk_[n+1]), xxyy, s1);
        multiply2D(X, Y, inx_[n+2], load4(blk_[n+2]), xxyy, s2);
        multiply2D(X, Y, inx_[n+3], load4(blk_[n+3]), xxyy, s3);
#else
        /* we remove here the apparent dependency on the values of Y[],
         which are read and written, but at different indices.
         The compiler can reorder instructions to avoid lattencies */
        const index_t i0 = inx_[n  ];
        const index_t i1 = inx_[n+1];
        const index_t i2 = inx_[n+2];
        const index_t i3 = inx_[n+3];
        vec4 mat0 = load4(blk_[n  ]);
        vec4 mat1 = load4(blk_[n+1]);
        vec4 mat2 = load4(blk_[n+2]);
        vec4 mat3 = load4(blk_[n+3]);
        vec4 u0 = fmadd4(mat0, xxyy, load2Z(Y+i0));
        vec4 u1 = fmadd4(mat1, xxyy, load2Z(Y+i1));
        vec4 u2 = fmadd4(mat2, xxyy, load2Z(Y+i2));
        vec4 u3 = fmadd4(mat3, xxyy, load2Z(Y+i3));
        ss = fmadd4(mat0, broadcast2(X+i0), ss);
        s1 = fmadd4(mat1, broadcast2(X+i1), s1);
        s2 = fmadd4(mat2, broadcast2(X+i2), s2);
        s3 = fmadd4(mat3, broadcast2(X+i3), s3);
        store2(Y+i0, add2(getlo(u0), gethi(u0)));
        store2(Y+i1, add2(getlo(u1), gethi(u1)));
        store2(Y+i2, add2(getlo(u2), gethi(u2)));
        store2(Y+i3, add2(getlo(u3), gethi(u3)));
#endif
    }
    // collapse 'ss'
    ss = add4(add4(ss,s1), add4(s2,s3));
    // process remaining blocks:
    for ( ; n < size_; ++n )
        multiply2D(X, Y, inx_[n], load4(blk_[n]), xxyy, ss);
    /* finally sum ss = { S0 S0 S1 S1 } */
    vec2 h = gethi(ss);
    h = add2(unpacklo2(getlo(ss), h), unpackhi2(getlo(ss), h));
    store2(Y+jj, add2(load2(Y+jj), h));
#endif
}


void MatrixSparseSymmetricBlock::Column::vecMulAdd3D_AVX(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 3 ) && MATRIXSSB_USES_AVX
    // load 3x3 matrix element into 3 vectors:
    real const* D = blk_[0];
    
    //multiply with the symmetrized block, assuming it has been symmetrized:
    //real Y0 = Y[jj  ] + M[0] * X0 + M[1] * X1 + M[2] * X2;
    //real Y1 = Y[jj+1] + M[1] * X0 + M[4] * X1 + M[5] * X2;
    //real Y2 = Y[jj+2] + M[2] * X0 + M[5] * X1 + M[8] * X2;
    /* vec4 s0, s1, s2 add lines of the transposed-matrix multiplied by 'xyz' */
#if ( BLD == 4 )
    vec4 tt = loadu4(X+jj);
    vec4 s0 = mul4(load4(D  ), tt);
    vec4 s1 = mul4(load4(D+4), tt);
    vec4 s2 = mul4(load4(D+8), tt);
#else
    vec4 tt = loadu4(X+jj);
    vec4 s0 = mul4(load3(D      ), tt);
    vec4 s1 = mul4(load3(D+BLD  ), tt);
    vec4 s2 = mul4(load3(D+BLD*2), tt);
#endif
    // sum non-diagonal elements:
#if ( 0 )
    const vec4 x0 = broadcast1(X+jj);
    const vec4 x1 = broadcast1(X+jj+1);
    const vec4 x2 = broadcast1(X+jj+2);
#else
    vec4 p = permute2f128(tt, tt, 0x01);
    vec4 l = blend4(tt, p, 0b1100);
    vec4 u = blend4(tt, p, 0b0011);
    const vec4 x0 = duplo4(l);
    const vec4 x1 = duphi4(l);
    const vec4 x2 = duplo4(u);
#endif
    // There is a dependency in the loop for 's0', 's1' and 's2'.
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        real const* M = blk_[n];
#if ( BLD == 4 )
        const vec4 m012 = load4(M  );
        const vec4 m345 = load4(M+4);
        const vec4 m678 = load4(M+8);
#else
        const vec4 m012 = load3(M      );
        const vec4 m345 = load3(M+BLD  );
        const vec4 m678 = load3(M+BLD*2);
#endif
        // multiply with the full block:
        //Y[ii  ] +=  M[0] * X0 + M[3] * X1 + M[6] * X2;
        //Y[ii+1] +=  M[1] * X0 + M[4] * X1 + M[7] * X2;
        //Y[ii+2] +=  M[2] * X0 + M[5] * X1 + M[8] * X2;
        vec4 z = fmadd4(m012, x0, loadu4(Y+ii));
        z = fmadd4(m345, x1, z);
        z = fmadd4(m678, x2, z);
        storeu4(Y+ii, z);
        
        // multiply with the transposed block:
        //Y0 += M[0] * X[ii] + M[1] * X[ii+1] + M[2] * X[ii+2];
        //Y1 += M[3] * X[ii] + M[4] * X[ii+1] + M[5] * X[ii+2];
        //Y2 += M[6] * X[ii] + M[7] * X[ii+1] + M[8] * X[ii+2];
        vec4 xyz = loadu4(X+ii);  // xyz = { X0 X1 X2 - }
        s0 = fmadd4(m012, xyz, s0);
        s1 = fmadd4(m345, xyz, s1);
        s2 = fmadd4(m678, xyz, s2);
    }
    // finally sum s0 = { Y0 Y0 Y0 - }, s1 = { Y1 Y1 Y1 - }, s2 = { Y2 Y2 Y2 - }
#if ( 0 )
    Y[jj  ] += s0[0] + s0[1] + s0[2];
    Y[jj+1] += s1[0] + s1[1] + s1[2];
    Y[jj+2] += s2[0] + s2[1] + s2[2];
#else
    vec4 s3 = setzero4();
    vec4 xy = add4(unpacklo4(s0, s1), unpackhi4(s0, s1));
    vec4 zt = add4(unpacklo4(s2, s3), unpackhi4(s2, s3));
    s3 = add4(permute2f128(xy, zt, 0x20), permute2f128(xy, zt, 0x31));
    storeu4(Y+jj, add4(loadu4(Y+jj), s3));
#endif
#endif
}


void MatrixSparseSymmetricBlock::Column::vecMulAdd3D_AVXU(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 3 ) && MATRIXSSB_USES_AVX
    vec4 sa, sb, sc;
    vec4 xa, xb, xc;
    vec4 ta = setzero4();
    vec4 tb = setzero4();
    vec4 tc = setzero4();
    // load 3x3 matrix element into 3 vectors:
    {
        real const* D = blk_[0];
        vec4 tt = loadu4(X+jj);
        // multiply by diagonal elements:
        sa = mul4(load4(D  ), tt);
        sb = mul4(load4(D+4), tt);
        sc = mul4(load4(D+8), tt);
        // prepare broadcasted vectors:
        vec4 p = permute2f128(tt, tt, 0x01);
        vec4 l = blend4(tt, p, 0b1100);
        vec4 u = blend4(tt, p, 0b0011);
        xa = duplo4(l);
        xb = duphi4(l);
        xc = duplo4(u);
    }
    // There is a dependency in the loop for 's0', 's1' and 's2'.
    unsigned n = 1;
    const unsigned stop = 1 + 2 * ( ( size_ - 1 ) / 2 );
    /*
     Unrolling will reduce the dependency chain, which is here limiting the overall
     throughput. However the number of registers in the CPU limits the unrolling at
     most by a factor 2, as this already uses all the 16 registers of AVX CPU
     */
    //process 2 by 2:
    for ( ; n < stop; n += 2 )
    {
        const index_t i0 = inx_[n  ];
        const index_t i1 = inx_[n+1];
        //printf("--- %4i %4i\n", i0, i1);
        real const* M0 = blk_[n  ];
        real const* M1 = blk_[n+1];
        vec4 ma0 = load4(M0);
        vec4 ma1 = load4(M1);
        vec4 z0 = fmadd4(ma0, xa, loadu4(Y+i0));
        vec4 z1 = fmadd4(ma1, xa, loadu4(Y+i1));
        vec4 xyz0 = loadu4(X+i0);
        vec4 xyz1 = loadu4(X+i1);
        sa = fmadd4(ma0, xyz0, sa);
        ta = fmadd4(ma1, xyz1, ta);
        // multiply with the full block:
        vec4 mb0 = load4(M0+4);
        vec4 mb1 = load4(M1+4);
        z0 = fmadd4(mb0, xb, z0);
        z1 = fmadd4(mb1, xb, z1);
        sb = fmadd4(mb0, xyz0, sb);
        tb = fmadd4(mb1, xyz1, tb);
        vec4 mc0 = load4(M0+8);
        vec4 mc1 = load4(M1+8);
        z0 = fmadd4(mc0, xc, z0);
        z1 = fmadd4(mc1, xc, z1);
        sc = fmadd4(mc0, xyz0, sc);
        tc = fmadd4(mc1, xyz1, tc);
        /*
         Attention: the 4th elements of the vectors z0 and z1 would be correct,
         because only zero was added to the value loaded from 'Y'. However, in the
         case where the indices i0 and i1 are consecutive and reverted (i1 < i0),
         the value stored in z0 would not have been updated giving a wrong results.
         The solution is to either use a 'store3(Y+i1, z1)', or to make sure that
         indices are non-consecutive or ordered in the column in increasing order.
         This affects performance since 'store3' is slower than 'storeu4'
         */
        storeu4(Y+i0, z0);
        storeu4(Y+i1, z1);
    }
    sa = add4(sa, ta);
    sb = add4(sb, tb);
    sc = add4(sc, tc);
    
    // process remaining blocks:
    for ( ; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        //printf("--- %4i\n", ii);
        real const* M = blk_[n];
        vec4 ma = load4(M);
        vec4 z = fmadd4(ma, xa, loadu4(Y+ii));
        vec4 xyz = loadu4(X+ii);
        sa = fmadd4(ma, xyz, sa);
        
        vec4 mb = load4(M+4);
        z = fmadd4(mb, xb, z);
        sb = fmadd4(mb, xyz, sb);
        
        vec4 mc = load4(M+8);
        z = fmadd4(mc, xc, z);
        sc = fmadd4(mc, xyz, sc);
        storeu4(Y+ii, z);
    }
    // finally sum s0 = { Y0 Y0 Y0 0 }, s1 = { Y1 Y1 Y1 0 }, s2 = { Y2 Y2 Y2 0 }
    xa = setzero4();
    xb = add4(unpacklo4(sa, sb), unpackhi4(sa, sb));
    xc = add4(unpacklo4(sc, xa), unpackhi4(sc, xa));
    sa = add4(permute2f128(xb, xc, 0x20), permute2f128(xb, xc, 0x31));
    storeu4(Y+jj, add4(loadu4(Y+jj), sa));
#endif
}


void MatrixSparseSymmetricBlock::Column::vecMulAdd4D_AVX(const real* X, real* Y, index_t jj) const
{
#if ( BLOCK_SIZE == 4 ) && MATRIXSSB_USES_AVX
    real const* D = blk_[0];
    //multiply with the symmetrized block, assuming it has been symmetrized:
    /* vec4 s0, s1, s2 add lines of the transposed-matrix multiplied by 'xyz' */
    vec4 tt = load4(X+jj);
    vec4 s0 = mul4(load4(D   ), tt);
    vec4 s1 = mul4(load4(D+4 ), tt);
    vec4 s2 = mul4(load4(D+8 ), tt);
    vec4 s3 = mul4(load4(D+12), tt);
    // sum non-diagonal elements:
#if ( 0 )
    const vec4 x0 = broadcast1(X+jj);
    const vec4 x1 = broadcast1(X+jj+1);
    const vec4 x2 = broadcast1(X+jj+2);
    const vec4 x3 = broadcast1(X+jj+3);
#else
    vec4 l = permute2f128(tt, tt, 0x00);
    vec4 u = permute2f128(tt, tt, 0x11);
    const vec4 x0 = duplo4(l);
    const vec4 x1 = duphi4(l);
    const vec4 x2 = duplo4(u);
    const vec4 x3 = duphi4(u);
#endif
    // There is a dependency in the loop for 's0', 's1' and 's2'.
    for ( index_t n = 1; n < size_; ++n )
    {
        const index_t ii = inx_[n];
        real const* M = blk_[n];
        const vec4 yy = load4(Y+ii);
        const vec4 xyzt = load4(X+ii);  // xyzt = { X0 X1 X2 X3 }
        const vec4 m0 = load4(M);
        vec4 z = fmadd4(m0, x0, yy);
        s0 = fmadd4(m0, xyzt, s0);
        
        const vec4 m1 = load4(M+4);
        z  = fmadd4(m1, x1, z);
        s1 = fmadd4(m1, xyzt, s1);

        const vec4 m2 = load4(M+8);
        z  = fmadd4(m2, x2, z);
        s2 = fmadd4(m2, xyzt, s2);

        const vec4 m3 = load4(M+12);
        z  = fmadd4(m3, x3, z);
        s3 = fmadd4(m3, xyzt, s3);
        store4(Y+ii, z);
    }
    // finally sum s0 = { Y0 Y0 Y0 Y0 }, s1 = { Y1 Y1 Y1 Y1 }, s2 = { Y2 Y2 Y2 Y2 }
    vec4 xy = add4(unpacklo4(s0, s1), unpackhi4(s0, s1));
    vec4 zt = add4(unpacklo4(s2, s3), unpackhi4(s2, s3));
    s0 = add4(permute2f128(xy, zt, 0x20), permute2f128(xy, zt, 0x31));
    store4(Y+jj, add4(load4(Y+jj), s0));
#endif
}


//------------------------------------------------------------------------------
#pragma mark - Vector Multiplication

#if MATRIXSSB_USES_AVX
#   define VECMULADD2D vecMulAdd2D_AVXU
#   define VECMULADD3D vecMulAdd3D_AVXU
#   define VECMULADD4D vecMulAdd4D_AVX
#elif defined(__SSE3__) && REAL_IS_DOUBLE
#   define VECMULADD2D vecMulAdd2D_SSE
#   define VECMULADD3D vecMulAdd3D
#   define VECMULADD4D vecMulAdd4D
#else
#   define VECMULADD2D vecMulAdd2D
#   define VECMULADD3D vecMulAdd3D
#   define VECMULADD4D vecMulAdd4D
#endif


// multiplication of a vector: Y = Y + M * X
void MatrixSparseSymmetricBlock::vecMulAdd(const real* X, real* Y, index_t start, index_t end) const
{
    assert_true( start <= end );
    assert_true( end <= size_ );
#if ( 1 )
    for ( index_t jj = next_[start]; jj < end; jj = next_[jj+1] )
#else
    for ( index_t jj = start; jj < end; jj += BLOCK_SIZE )
        if ( column_[jj].size_ > 0 )
#endif
            {
                //std::clog << "MatrixSparseSymmetricBlock column " << jj << "  " << size_ << " \n";
#if ( BLOCK_SIZE == 1 )
                column_[jj].vecMulAdd1D(X, Y, jj);
#elif ( BLOCK_SIZE == 2 )
                column_[jj].VECMULADD2D(X, Y, jj);
#elif ( BLOCK_SIZE == 3 )
                column_[jj].VECMULADD3D(X, Y, jj);
#elif ( BLOCK_SIZE == 4 )
                column_[jj].VECMULADD4D(X, Y, jj);
#endif
            }
}


// multiplication of a vector: Y = Y + M * X
void MatrixSparseSymmetricBlock::vecMulAdd_ALT(const real* X, real* Y) const
{
    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
        //std::clog << "MatrixSparseSymmetricBlock column " << jj << "  " << size_ << " \n";
#if ( BLOCK_SIZE == 1 )
        column_[jj].vecMulAdd1D(X, Y, jj);
#elif ( BLOCK_SIZE == 2 )
        column_[jj].vecMulAdd2D(X, Y, jj);
#elif ( BLOCK_SIZE == 3 )
        column_[jj].vecMulAdd3D(X, Y, jj);
#elif ( BLOCK_SIZE == 4 )
        column_[jj].vecMulAdd4D(X, Y, jj);
#endif
    }
}

#define TIME_PRINTOUT 0

// multiplication of a vector: Y = Y + M * X
void MatrixSparseSymmetricBlock::vecMulAdd_TIME(const real* X, real* Y) const
{
#if TIME_PRINTOUT
    unsigned long cnt = 0, col = 0;
    unsigned long long time = __rdtsc();
#endif
    for ( index_t jj = next_[0]; jj < size_; jj = next_[jj+1] )
    {
#if TIME_PRINTOUT
        col++;
        cnt += column_[jj].size_;
#endif
        //std::clog << "MatrixSparseSymmetricBlock column " << jj << "  " << size_ << " \n";
#if ( BLOCK_SIZE == 1 )
        column_[jj].vecMulAdd1D(X, Y, jj);
#elif ( BLOCK_SIZE == 2 )
        column_[jj].vecMulAdd2D_AVXU(X, Y, jj);
#elif ( BLOCK_SIZE == 3 )
        column_[jj].vecMulAdd3D_AVXU(X, Y, jj);
#elif ( BLOCK_SIZE == 4 )
        column_[jj].vecMulAdd4D_AVX(X, Y, jj);
#endif
    }
#if TIME_PRINTOUT
    if ( cnt > 0 )
        fprintf(stderr, "MSSB %6u with %6lu blocks/column  cycles/block: %6llu\n", size_, cnt/col, (__rdtsc()-time)/cnt);
#endif
}

