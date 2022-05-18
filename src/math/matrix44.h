// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// F. Nedelec, Strasbourg 12.06.2018

#ifndef MATRIX44
#define MATRIX44

#include "real.h"
#include "vector4.h"
#include <cstdio>
#include <iostream>

#ifdef __AVX__
#  define MATRIX44_USES_AVX REAL_IS_DOUBLE
#  include "simd.h"
#else
#  define MATRIX44_USES_AVX 0
#endif


/// 4x4 matrix class with 16 'real' elements
class alignas(32) Matrix44
{
    /// access to modifiable element by index
    real& operator[](int i)       { return val[i]; }
    
    /// access element value by index
    real  operator[](int i) const { return val[i]; }

public:

    /// values of the elements
    real val[4*4];
    
    Matrix44() {}
    
    /// copy constructor
    Matrix44(Matrix44 const& M)
    {
        for ( int u = 0; u < 16; ++u )
            val[u] = M.val[u];
    }
    
    /// construct Matrix from coordinates (column-major)
    Matrix44(real a, real b, real c, real d,
             real e, real f, real g, real h,
             real i, real j, real k, real l,
             real m, real n, real o, real p )
    {
        val[0x0] = a;
        val[0x1] = b;
        val[0x2] = c;
        val[0x3] = d;
        val[0x4] = e;
        val[0x5] = f;
        val[0x6] = g;
        val[0x7] = h;
        val[0x8] = i;
        val[0x9] = j;
        val[0xA] = k;
        val[0xB] = l;
        val[0xC] = m;
        val[0xD] = n;
        val[0xE] = o;
        val[0xF] = p;
    }

    /// construct Matrix with `d` on the diagonal and other values equal to `a`
    Matrix44(real z, real d)
    {
        val[0x0] = d;
        val[0x1] = z;
        val[0x2] = z;
        val[0x3] = z;
        val[0x4] = z;
        val[0x5] = d;
        val[0x6] = z;
        val[0x7] = z;
        val[0x8] = z;
        val[0x9] = z;
        val[0xA] = d;
        val[0xB] = z;
        val[0xC] = z;
        val[0xD] = z;
        val[0xE] = z;
        val[0xF] = d;
    }

    ~Matrix44() {}
    
    /// dimensionality
    static int dimension() { return 4; }
    
    /// human-readible identifier
    static std::string what() { return "4*4"; }

    /// set all elements to zero
    void reset()
    {
        for ( int u = 0; u < 16; ++u )
            val[u] = 0.0;
    }
    
    /// true if any value is different from 'zero'
    bool operator != (real zero) const
    {
        for ( int u = 0; u < 16; ++u )
            if ( val[u] != zero )
                return true;
        return false;
    }
    
    /// conversion to pointer of real
    operator real const*() const { return val; }

    /// conversion to array of 'real'
    real* data()             { return val; }
    real* addr(int i, int j) { return val + ( i + 4*j ); }

    /// access functions to element by line and column indices
    real& operator()(int i, int j)       { return val[i+4*j]; }
    real  operator()(int i, int j) const { return val[i+4*j]; }
    
    /// extract column vector at given index
    Vector4 column(const unsigned i) const
    {
        return Vector4(val+4*i);
    }
    
    /// extract line vector at given index
    Vector4 line(const unsigned i) const
    {
        return Vector4(val[i], val[4+i], val[8+i], val[12+i]);
    }
    
    /// extract diagonal
    Vector4 diagonal() const
    {
        return Vector4(val[0], val[5], val[10], val[15]);
    }
    
    /// sum of diagonal terms
    real trace() const
    {
        return ( val[0x0] + val[0x5] + val[0xA] + val[0xF] );
    }

    /// output in human-friendly format
    void print(FILE * f) const
    {
        fprintf(f, " / %9.3f %+9.3f %+9.3f %+9.3f \\\n",  val[0x0], val[0x4], val[0x8], val[0xC]);
        fprintf(f, "(  %9.3f %+9.3f %+9.3f %+9.3f  )\n" , val[0x1], val[0x5], val[0x9], val[0xD]);
        fprintf(f, "(  %9.3f %+9.3f %+9.3f %+9.3f  )\n" , val[0x2], val[0x6], val[0xA], val[0xE]);
        fprintf(f, " \\ %9.3f %+9.3f %+9.3f %+9.3f /\n",  val[0x3], val[0x7], val[0xB], val[0xF]);
    }
    
    /// conversion to string
    std::string to_string(int w, int p) const
    {
        std::ostringstream os;
        os.precision(p);
        os << "[";
        for ( int i = 0; i < 4; ++i )
        {
            for ( int j = 0; j < 4; ++j )
                os << " " << std::fixed << std::setw(w) << (*this)(i,j);
            if ( i < 2 )
                os << ";";
            else
                os << " ]";
        }
        return os.str();
    }

    /// scale all elements
    void scale(const real alpha)
    {
        for ( int u = 0; u < 16; ++u )
            val[u] *= alpha;
    }
    
    /// scale matrix
    void operator *=(const real alpha)
    {
        scale(alpha);
    }
    
    /// return opposite matrix (i.e. -M)
    const Matrix44 operator -() const
    {
        Matrix44 M;
        for ( int u = 0; u < 16; ++u )
            M.val[u] = -val[u];
        return M;
    }
    
    /// returns alpha * M
    const Matrix44 operator *(const real alpha) const
    {
        Matrix44 M;
        for ( int u = 0; u < 16; ++u )
            M.val[u] = val[u] * alpha;
        return M;
    }
    
    /// multiplication by scalar
    friend const Matrix44 operator *(const real alpha, Matrix44 const& mat)
    {
        return mat * alpha;
    }

    /// return sum of two matrices
    const Matrix44 operator +(Matrix44 const& M) const
    {
        Matrix44 res;
        for ( int u = 0; u < 16; ++u )
            res.val[u] = val[u] + M.val[u];
        return res;
    }

    /// subtract given matrix
    void operator +=(Matrix44 const& M)
    {
#if MATRIX44_USES_AVX
        store4(val  , add4(load4(val  ), load4(M.val  )));
        store4(val+4, add4(load4(val+4), load4(M.val+4)));
        store4(val+8, add4(load4(val+8), load4(M.val+8)));
        store4(val+12, add4(load4(val+12), load4(M.val+12)));
#else
        for ( int u = 0; u < 16; ++u )
            val[u] += M.val[u];
#endif
    }

    /// add given matrix
    void operator -=(Matrix44 const& M)
    {
#if MATRIX44_USES_AVX
        store4(val  , sub4(load4(val  ), load4(M.val  )));
        store4(val+4, sub4(load4(val+4), load4(M.val+4)));
        store4(val+8, sub4(load4(val+8), load4(M.val+8)));
        store4(val+12, sub4(load4(val+12), load4(M.val+12)));
#else
        for ( int u = 0; u < 16; ++u )
            val[u] -= M.val[u];
#endif
    }
    
    /// transpose matrix in place
    void transpose()
    {
        std::swap(val[0x4], val[0x1]);
        std::swap(val[0x8], val[0x2]);
        std::swap(val[0xC], val[0x3]);
        std::swap(val[0x9], val[0x6]);
        std::swap(val[0xD], val[0x7]);
        std::swap(val[0xE], val[0xB]);
    }
    
    /// return transposed matrix
    Matrix44 transposed() const
    {
        Matrix44 res;
#if MATRIX44_USES_AVX
        vec4 v0 = load4(val);
        vec4 v1 = load4(val+4);
        vec4 v2 = load4(val+8);
        vec4 v3 = load4(val+12);
        vec4 t0 = unpacklo4(v0, v1);
        vec4 t1 = unpackhi4(v0, v1);
        vec4 t2 = unpacklo4(v2, v3);
        vec4 t3 = unpackhi4(v2, v3);
        store4(res.val   , permute2f128(t0, t2, 0x20));
        store4(res.val+4 , permute2f128(t1, t3, 0x20));
        store4(res.val+8 , permute2f128(t0, t2, 0x31));
        store4(res.val+12, permute2f128(t1, t3, 0x31));
#else
        for ( int x = 0; x < 4; ++x )
        for ( int y = 0; y < 4; ++y )
            res.val[y+4*x] = val[x+4*y];
#endif
        return res;
    }
    
    /// maximum of all component's absolute values
    real norm_inf() const
    {
        real res = fabs(val[0]);
        for ( unsigned i = 1; i < 16; ++i )
            res = std::max(res, fabs(val[i]));
        return res;
    }

    /// copy values from lower triangle to upper triangle
    void copy_lower()
    {
        val[0x4] = val[0x1];
        val[0x8] = val[0x2];
        val[0xC] = val[0x3];
        val[0x9] = val[0x6];
        val[0xD] = val[0x7];
        val[0xE] = val[0xB];
    }

    /// true if matrix is symmetric
    real asymmetry() const
    {
        return ( std::fabs(val[0x4]-val[0x1])
                + std::fabs(val[0x8]-val[0x2])
                + std::fabs(val[0xC]-val[0x3])
                + std::fabs(val[0x9]-val[0x6])
                + std::fabs(val[0xD]-val[0x7])
                + std::fabs(val[0xE]-val[0xB]) );
    }

#if MATRIX44_USES_AVX
    /// multiplication by a vector: this * V
    const vec4 vecmul4(vec4 const& vec) const
    {
        vec4 p = permute2f128(vec, vec, 0x01);
        vec4 l = blend4(vec, p, 0b1100);
        vec4 u = blend4(vec, p, 0b0011);
        vec4 x = mul4(load4(val   ), duplo4(l));
        vec4 y = mul4(load4(val+4 ), duphi4(l));
        vec4 z = mul4(load4(val+8 ), duplo4(u));
        vec4 t = mul4(load4(val+12), duphi4(u));
        return add4(add4(x, y), add4(z, t));
    }
    
    /// multiplication by a vector: transpose(this) * V
    const vec4 trans_vecmul4(vec4 const& vec) const
    {
        vec4 s0 = mul4(load4(val   ), vec);
        vec4 s1 = mul4(load4(val+4 ), vec);
        vec4 s2 = mul4(load4(val+8 ), vec);
        vec4 s3 = mul4(load4(val+12), vec);
        s0 = add4(unpacklo4(s0, s1), unpackhi4(s0, s1));
        s1 = add4(unpacklo4(s2, s3), unpackhi4(s2, s3));
        return add4(permute2f128(s0, s1, 0x20), permute2f128(s0, s1, 0x31));
    }

    /// multiplication by a vector: this * V
    const vec4 vecmul(real const* ptr) const
    {
#if 1
        return vecmul4(load4(ptr));
#else
        return vec4{val[0x0] * ptr[0] + val[0x4] * ptr[1] + val[0x8] * ptr[2] + val[0xC] * ptr[3],
                    val[0x1] * ptr[0] + val[0x5] * ptr[1] + val[0x9] * ptr[2] + val[0xD] * ptr[3],
                    val[0x2] * ptr[0] + val[0x6] * ptr[1] + val[0xA] * ptr[2] + val[0xE] * ptr[3],
                    val[0x3] * ptr[0] + val[0x7] * ptr[1] + val[0xB] * ptr[2] + val[0xF] * ptr[3]};
#endif
    }

    /// multiplication by a vector: transpose(M) * V
    const vec4 trans_vecmul(real const* V) const
    {
#if 1
        return trans_vecmul4(load4(V));
#else
        return vec4{val[0x0] * V[0] + val[0x1] * V[1] + val[0x2] * V[2] + val[0x3] * V[3],
                    val[0x4] * V[0] + val[0x5] * V[1] + val[0x6] * V[2] + val[0x7] * V[3],
                    val[0x8] * V[0] + val[0x9] * V[1] + val[0xA] * V[2] + val[0xB] * V[3],
                    val[0xC] * V[0] + val[0xD] * V[1] + val[0xE] * V[2] + val[0xF] * V[3]};
#endif
    }
#endif
    
    
    /// multiplication by a vector: this * V
    const Vector4 vecmul0(Vector4 const& V) const
    {
        return Vector4(val[0x0] * V.XX + val[0x4] * V.YY + val[0x8] * V.ZZ + val[0xC] * V.TT,
                       val[0x1] * V.XX + val[0x5] * V.YY + val[0x9] * V.ZZ + val[0xD] * V.TT,
                       val[0x2] * V.XX + val[0x6] * V.YY + val[0xA] * V.ZZ + val[0xE] * V.TT,
                       val[0x3] * V.XX + val[0x7] * V.YY + val[0xB] * V.ZZ + val[0xF] * V.TT);
    }

    /// multiplication by a vector: transpose(this) * V
    const Vector4 vecmul0(real const* ptr) const
    {
        return Vector4(val[0x0] * ptr[0] + val[0x4] * ptr[1] + val[0x8] * ptr[2] + val[0xC] * ptr[3],
                       val[0x1] * ptr[0] + val[0x5] * ptr[1] + val[0x9] * ptr[2] + val[0xD] * ptr[3],
                       val[0x2] * ptr[0] + val[0x6] * ptr[1] + val[0xA] * ptr[2] + val[0xE] * ptr[3],
                       val[0x3] * ptr[0] + val[0x7] * ptr[1] + val[0xB] * ptr[2] + val[0xF] * ptr[3]);
    }

    /// vector multiplication
    friend Vector4 operator * (Matrix44 const& mat, Vector4 const& ptr)
    {
        return mat.vecmul0(ptr);
    }

    /// multiplication by a vector: transpose(M) * V
    const Vector4 trans_vecmul0(Vector4 const& V) const
    {
        return Vector4(val[0x0] * V.XX + val[0x1] * V.YY + val[0x2] * V.ZZ + val[0x3] * V.TT,
                       val[0x4] * V.XX + val[0x5] * V.YY + val[0x6] * V.ZZ + val[0x7] * V.TT,
                       val[0x8] * V.XX + val[0x9] * V.YY + val[0xA] * V.ZZ + val[0xB] * V.TT,
                       val[0xC] * V.XX + val[0xD] * V.YY + val[0xE] * V.ZZ + val[0xF] * V.TT);
    }

    /// multiplication by a vector: transpose(M) * V
    const Vector4 trans_vecmul0(real const* ptr) const
    {
        return Vector4(val[0x0] * ptr[0] + val[0x1] * ptr[1] + val[0x2] * ptr[2] + val[0x3] * ptr[3],
                       val[0x4] * ptr[0] + val[0x5] * ptr[1] + val[0x6] * ptr[2] + val[0x7] * ptr[3],
                       val[0x8] * ptr[0] + val[0x9] * ptr[1] + val[0xA] * ptr[2] + val[0xB] * ptr[3],
                       val[0xC] * ptr[0] + val[0xD] * ptr[1] + val[0xE] * ptr[2] + val[0xF] * ptr[3]);
    }
    

    /// multiplication by another matrix: @returns this * M
    const Matrix44 mul(Matrix44 const& M) const
    {
        Matrix44 res;
        res[0x0] = val[0x0] * M[0x0] + val[0x4] * M[0x1] + val[0x8] * M[0x2] + val[0xC] * M[0x3];
        res[0x1] = val[0x1] * M[0x0] + val[0x5] * M[0x1] + val[0x9] * M[0x2] + val[0xD] * M[0x3];
        res[0x2] = val[0x2] * M[0x0] + val[0x6] * M[0x1] + val[0xA] * M[0x2] + val[0xE] * M[0x3];
        res[0x3] = val[0x3] * M[0x0] + val[0x7] * M[0x1] + val[0xB] * M[0x2] + val[0xF] * M[0x3];

        res[0x4] = val[0x0] * M[0x4] + val[0x4] * M[0x5] + val[0x8] * M[0x6] + val[0xC] * M[0x7];
        res[0x5] = val[0x1] * M[0x4] + val[0x5] * M[0x5] + val[0x9] * M[0x6] + val[0xD] * M[0x7];
        res[0x6] = val[0x2] * M[0x4] + val[0x6] * M[0x5] + val[0xA] * M[0x6] + val[0xE] * M[0x7];
        res[0x7] = val[0x3] * M[0x4] + val[0x7] * M[0x5] + val[0xB] * M[0x6] + val[0xF] * M[0x7];

        res[0x8] = val[0x0] * M[0x8] + val[0x4] * M[0x9] + val[0x8] * M[0xA] + val[0xC] * M[0xB];
        res[0x9] = val[0x1] * M[0x8] + val[0x5] * M[0x9] + val[0x9] * M[0xA] + val[0xD] * M[0xB];
        res[0xA] = val[0x2] * M[0x8] + val[0x6] * M[0x9] + val[0xA] * M[0xA] + val[0xE] * M[0xB];
        res[0xB] = val[0x3] * M[0x8] + val[0x7] * M[0x9] + val[0xB] * M[0xA] + val[0xF] * M[0xB];

        res[0xC] = val[0x0] * M[0xC] + val[0x4] * M[0xD] + val[0x8] * M[0xE] + val[0xC] * M[0xF];
        res[0xD] = val[0x1] * M[0xC] + val[0x5] * M[0xD] + val[0x9] * M[0xE] + val[0xD] * M[0xF];
        res[0xE] = val[0x2] * M[0xC] + val[0x6] * M[0xD] + val[0xA] * M[0xE] + val[0xE] * M[0xF];
        res[0xF] = val[0x3] * M[0xC] + val[0x7] * M[0xD] + val[0xB] * M[0xE] + val[0xF] * M[0xF];
        return res;
    }
    
    /// multiplication by matrix
    friend Matrix44 operator * (Matrix44 const& mat, Matrix44 const& mut)
    {
        return mat.mul(mut);
    }

    /// multiplication by another matrix: @returns transpose(this) * M
    const Matrix44 trans_mul(Matrix44 const& M) const
    {
        ABORT_NOW("unfinished");
    }
    
    /// add full matrix: this <- this + M
    void add_full(Matrix44 const& M)
    {
        real const* src = M.val;
        for ( int u = 0; u < 16; ++u )
            val[u] += src[u];
    }
    
    /// add full matrix: this <- this + alpha * M
    void add_full(const real alpha, Matrix44 const& M)
    {
        real const* src = M.val;
        for ( int u = 0; u < 16; ++u )
            val[u] += alpha * src[u];
    }
    
    /// sub full matrix: this <- this - M
    void sub_full(Matrix44 const& M)
    {
        real const* src = M.val;
        for ( int u = 0; u < 16; ++u )
            val[u] -= src[u];
    }

    /// add lower triangle of matrix including diagonal: this <- this + M
    void add_half(Matrix44 const& M)
    {
        real const* src = M.val;
#if ( 1 )
        for ( int u = 0; u < 16; ++u )
            val[u] += src[u];
#else
        for ( int x = 0; x < 4; ++x )
        for ( int y = x; y < 4; ++y )
            val[y+4*x] += src[y+4*x];
#endif
    }
    
    /// add lower triangle of matrix including diagonal: this <- this + alpha * M
    void add_half(const real alpha, Matrix44 const& M)
    {
        real const* src = M.val;
#if ( 1 )
        for ( int u = 0; u < 16; ++u )
            val[u] += alpha * src[u];
#else
        for ( int x = 0; x < 4; ++x )
        for ( int y = x; y < 4; ++y )
            val[y+4*x] += alpha * src[y+4*x];
#endif
    }
    
    /// add alpha to diagonal
    void add_diag(real alpha)
    {
        val[0x0] += alpha;
        val[0x5] += alpha;
        val[0xA] += alpha;
        val[0xF] += alpha;
    }
    
    /// add -alpha to diagonal
    void sub_diag(real alpha)
    {
        val[0x0] -= alpha;
        val[0x5] -= alpha;
        val[0xA] -= alpha;
        val[0xF] -= alpha;
    }

    /// subtract lower triangle of matrix including diagonal: this <- this - M
    void sub_half(Matrix44 const& M)
    {
        real const* src = M.val;
#if ( 1 )
        for ( int u = 0; u < 16; ++u )
            val[u] -= src[u];
#else
        for ( int x = 0; x < 4; ++x )
        for ( int y = x; y < 4; ++y )
            val[y+4*x] -= src[y+4*x];
#endif
    }

    
    /// add all elements of block 'S' to array 'M'
    void addto(real * M, unsigned ldd) const
    {
        for ( int x = 0; x < 4; ++x )
        for ( int y = 0; y < 4; ++y )
            M[y+ldd*x] = val[y+4*x];
    }
    
    /// add lower elements of this block to upper triangle of 'M'
    void addto_upper(real * M, unsigned ldd) const
    {
        for ( int x = 0; x < 4; ++x )
        for ( int y = x; y < 4; ++y )
            M[y+ldd*x] = val[y+4*x];
    }
    
    /// add all elements of this block to 'M', with transposition
    void addto_trans(real * M, unsigned ldd) const
    {
        for ( int x = 0; x < 4; ++x )
        for ( int y = 0; y < 4; ++y )
            M[x+ldd*y] = val[y+4*x];
    }
    
    /// add lower elements of this block to both upper and lower triangles of 'M'
    void addto_symm(real * M, unsigned ldd) const
    {
        for ( int x = 0; x < 4; ++x )
        {
            M[x+ldd*x] = val[x+4*x];
            for ( int y = x+1; y < 4; ++y )
            {
                M[y+ldd*x] = val[y+4*x];
                M[x+ldd*y] = val[y+4*x];
            }
        }
    }


    /// return diagonal Matrix from diagonal terms
    static Matrix44 diagonal(real a, real b, real c, real d)
    {
        return Matrix44(a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, 0, 0, 0, d);
    }
    
    /// identity matrix
    static Matrix44 identity()
    {
        return Matrix44(0, 1);
    }

    /// construct Matrix from coordinates (column-major)
    static Matrix44 symmetric(real a, real b, real c, real d,
                              real e, real f, real g, real h,
                              real i, real j )
    {
        return Matrix44(a, b, c, d, b, e, f, g, c, f, h, i, d, g, i, j);
    }

    /// return a symmetric matrix: [ dir (x) transpose(dir) ]
    static Matrix44 outerProduct(Vector4 const& V)
    {
        return symmetric(V[0]*V[0], V[1]*V[0], V[2]*V[0], V[3]*V[0],
                         V[1]*V[1], V[2]*V[1], V[3]*V[1],
                         V[2]*V[2], V[3]*V[2],
                         V[3]*V[3] );
    }
    
    /// return a symmetric matrix: alpha * [ dir (x) transpose(dir) ]
    static Matrix44 outerProduct(Vector4 const& V, real alpha)
    {
        real X = V[0] * alpha;
        real Y = V[1] * alpha;
        real Z = V[2] * alpha;
        real T = V[3] * alpha;
        return symmetric(V[0]*X, V[1]*X, V[2]*X, V[3]*X,
                         V[1]*Y, V[2]*Y, V[3]*Y,
                         V[2]*Z, V[3]*Z,
                         V[3]*T );
    }
    
    /// return outer product: [ dir (x) transpose(vec) ]
    static Matrix44 outerProduct(const real D[], const real V[])
    {
#if MATRIX44_USES_AVX
        Matrix44 res;
        vec4 s = load4(V);
        vec4 p = permute2f128(s, s, 0x01);
        vec4 l = blend4(s, p, 0b1100);
        vec4 u = blend4(s, p, 0b0011);
        vec4 d = load4(D);
        store4(res.val   , mul4(d, duplo4(l)));
        store4(res.val+4 , mul4(d, duphi4(l)));
        store4(res.val+8 , mul4(d, duplo4(u)));
        store4(res.val+12, mul4(d, duphi4(u)));
        return res;
#else
        return Matrix44(D[0]*V[0], D[1]*V[0], D[2]*V[0], D[3]*V[0],
                        D[0]*V[1], D[1]*V[1], D[2]*V[1], D[3]*V[1],
                        D[0]*V[2], D[1]*V[2], D[2]*V[2], D[3]*V[2],
                        D[0]*V[3], D[1]*V[3], D[2]*V[3], D[3]*V[3] );
#endif
    }
    
    /// return [ dir (x) transpose(vec) + vec (x) transpose(dir) ]
    static Matrix44 symmetricOuterProduct(const real D[], const real V[])
    {
        real xx = D[0] * V[0];
        real yy = D[1] * V[1];
        real zz = D[2] * V[2];
        real tt = D[3] * V[3];
        return symmetric(xx+xx, D[1]*V[0] + D[0]*V[1], D[2]*V[0] + D[0]*V[2], D[3]*V[0] + D[0]*V[3],
                         yy+yy, D[2]*V[1] + D[1]*V[2], D[3]*V[1] + D[1]*V[3],
                         zz+zz, D[3]*V[2] + D[2]*V[3],
                         tt+tt);
    }
 
    /// return symmetric matrix block :  dia * I + [ dir (x) dir ] * len
    static Matrix44 offsetOuterProduct(const real dia, Vector4 const& dir, const real len)
    {
        real xl = dir.XX * len;
        real yl = dir.YY * len;
        real zl = dir.ZZ * len;
        real tl = dir.TT * len;
        return symmetric(xl * dir.XX + dia, yl * dir.XX, zl * dir.XX, tl * dir.XX,
                         yl * dir.YY + dia, zl * dir.YY, tl * dir.YY,
                         zl * dir.ZZ + dia, tl * dir.ZZ,
                         tl * dir.TT + dia);
    }
    
    /// return symmetric matrix block :  dia * I + [ dir (x) dir ] * len
    static Matrix44 offsetOuterProduct(const real dia, const real D[], const real len)
    {
        real xl = D[0] * len;
        real yl = D[1] * len;
        real zl = D[2] * len;
        real tl = D[3] * len;
        return symmetric(xl * D[0] + dia, yl * D[0], zl * D[0], tl * D[0],
                         yl * D[1] + dia, zl * D[1], tl * D[1],
                         zl * D[2] + dia, tl * D[2],
                         tl * D[3] + dia);
    }
};


/// output a Matrix44
inline std::ostream& operator << (std::ostream& os, Matrix44 const& mat)
{
    int w = (int)os.width();
    os.width(1);
    os << "[";
    for ( int i = 0; i < 4; ++i )
    {
        for ( int j = 0; j < 4; ++j )
            os << " " << std::fixed << std::setw(w) << mat(i,j);
        if ( i < 2 )
            os << ";";
        else
            os << " ]";
    }
    os.width(w);
    return os;
}

#endif

