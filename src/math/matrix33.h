// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// F. Nedelec, Strasbourg 08.06.2018

#ifndef MATRIX33
#define MATRIX33

#include "real.h"
#include "vector3.h"

#include <cstdio>
#include <iostream>

/// BLD is the leading dimension of the matrix
/**
 The code works with BLD = 3 or 4, and typically memory storage is less with 3,
 but performance may be better with 4, as memory is aligned
 */

#ifdef __AVX__
#  define MATRIX33_USES_AVX REAL_IS_DOUBLE
#  include "simd.h"
#  define BLD 4
#else
#  define MATRIX33_USES_AVX 0
#  define BLD 3
#endif


/// 3x3 matrix class with 9 'real' elements
class alignas(32) Matrix33
{
private:
    
    /// values of the elements
    real val[BLD*3];

    /// access to modifiable element by index
    real& operator[](int i)       { return val[i]; }
    
    /// access element value by index
    real  operator[](int i) const { return val[i]; }
    
public:
    
    Matrix33() { clear_shadow(); }
    
    /// copy constructor
    Matrix33(Matrix33 const& M)
    {
        for ( int u = 0; u < BLD*3; ++u )
            val[u] = M.val[u];
    }

    /// construct Matrix from coordinates (given by columns)
    Matrix33(real a, real b, real c,
             real d, real e, real f,
             real g, real h, real i)
    {
        val[0] = a;
        val[1] = b;
        val[2] = c;
        val[0+BLD] = d;
        val[1+BLD] = e;
        val[2+BLD] = f;
        val[0+BLD*2] = g;
        val[1+BLD*2] = h;
        val[2+BLD*2] = i;
        clear_shadow();
    }

    /// construct Matrix with `d` on the diagonal and other values equal to `a`
    Matrix33(real z, real d)
    {
        val[0] = d;
        val[1] = z;
        val[2] = z;
        val[0+BLD] = z;
        val[1+BLD] = d;
        val[2+BLD] = z;
        val[0+BLD*2] = z;
        val[1+BLD*2] = z;
        val[2+BLD*2] = d;
        clear_shadow();
    }

    ~Matrix33() {}
    
    /// dimensionality
    static int dimension() { return 3; }
    
    /// human-readible identifier
#if ( BLD == 3 )
    static std::string what() { return "3*3"; }
#else
    static std::string what() { return "4*3"; }
#endif
    
    /// set all elements to zero
    void reset()
    {
        for ( int u = 0; u < BLD*3; ++u )
            val[u] = 0.0;
    }
    
    bool operator != (real zero) const
    {
        for ( int u = 0; u < BLD*3; ++u )
            if ( val[u] != zero )
                return true;
        return false;
    }
    
    /// conversion to pointer of real
    operator real const*() const { return val; }

    /// conversion to array of 'real'
    real* data()             { return val; }
    real* addr(int i, int j) { return val + ( i + BLD*j ); }
    
    /// access functions to element by line and column indices
    real& operator()(int i, int j)       { return val[i+BLD*j]; }
    real  operator()(int i, int j) const { return val[i+BLD*j]; }
    
    /// extract column vector at given index
    Vector3 column(const unsigned i) const
    {
        return Vector3(val+BLD*i);
    }
    
    /// extract line vector at given index
    Vector3 line(const unsigned i) const
    {
        return Vector3(val[i], val[BLD+i], val[BLD*2+i]);
    }
    
    /// extract diagonal
    Vector3 diagonal() const
    {
        return Vector3(val[0], val[BLD+1], val[BLD*2+2]);
    }
    
    /// sum of diagonal terms
    real trace() const
    {
        return ( val[0] + val[BLD+1] + val[BLD*2+2] );
    }

    /// set matrix by giving lines
    void setLines(Vector3 const& A, Vector3 const& B, Vector3 const& C)
    {
        val[0      ] = A.XX;
        val[1      ] = B.XX;
        val[2      ] = C.XX;
        val[0+BLD  ] = A.YY;
        val[1+BLD  ] = B.YY;
        val[2+BLD  ] = C.YY;
        val[0+BLD*2] = A.ZZ;
        val[1+BLD*2] = B.ZZ;
        val[2+BLD*2] = C.ZZ;
    }
    
    /// set matrix by giving columns
    void setColumns(Vector3 const& A, Vector3 const& B, Vector3 const& C)
    {
        val[0      ] = A.XX;
        val[1      ] = A.YY;
        val[2      ] = A.ZZ;
        val[0+BLD  ] = B.XX;
        val[1+BLD  ] = B.YY;
        val[2+BLD  ] = B.ZZ;
        val[0+BLD*2] = C.XX;
        val[1+BLD*2] = C.YY;
        val[2+BLD*2] = C.ZZ;
    }

    /// print matrix in human readible format
    void print(FILE * f) const
    {
        fprintf(f, " / %9.3f %+9.3f %+9.3f \\\n",  val[0], val[0+BLD], val[0+BLD*2]);
        fprintf(f, "(  %9.3f %+9.3f %+9.3f  )\n" , val[1], val[1+BLD], val[1+BLD*2]);
        fprintf(f, " \\ %9.3f %+9.3f %+9.3f /\n",  val[2], val[2+BLD], val[2+BLD*2]);
    }
    
    /// conversion to string
    std::string to_string(int w, int p) const
    {
        std::ostringstream os;
        os.precision(p);
        os << "[";
        for ( int i = 0; i < 3; ++i )
        {
            for ( int j = 0; j < 3; ++j )
                os << " " << std::fixed << std::setw(w) << (*this)(i,j);
            if ( i < 2 )
                os << ";";
            else
                os << " ]";
        }
        return os.str();
    }

    /// clear values that do not represent matrix elements
    void clear_shadow()
    {
#if ( BLD == 4 )
        val[3      ] = 0.0;
        val[3+BLD  ] = 0.0;
        val[3+BLD*2] = 0.0;
#endif
    }
    
    /// scale all elements
    void scale(const real alpha)
    {
        for ( int u = 0; u < BLD*3; ++u )
            val[u] *= alpha;
    }

    /// scale matrix
    void operator *=(const real alpha)
    {
        scale(alpha);
    }
    
    /// return opposite matrix (i.e. -M)
    const Matrix33 operator -() const
    {
        Matrix33 M;
        for ( int u = 0; u < BLD*3; ++u )
            M.val[u] = -val[u];
        return M;
    }
    
    /// scaled matrix
    const Matrix33 operator *(const real alpha) const
    {
        Matrix33 res;
        for ( int u = 0; u < BLD*3; ++u )
            res.val[u] = val[u] * alpha;
        return res;
    }
    
    /// multiplication by scalar
    friend const Matrix33 operator *(const real alpha, Matrix33 const& mat)
    {
        return mat * alpha;
    }

    /// return sum of two matrices
    const Matrix33 operator +(Matrix33 const& M) const
    {
        Matrix33 res;
        for ( int u = 0; u < BLD*3; ++u )
            res.val[u] = val[u] + M.val[u];
        return res;
    }

    /// return sum of two matrices
    const Matrix33 operator -(Matrix33 const& M) const
    {
        Matrix33 res;
        for ( int u = 0; u < BLD*3; ++u )
            res.val[u] = val[u] - M.val[u];
        return res;
    }

    /// subtract given matrix
    void operator +=(Matrix33 const& M)
    {
#if MATRIX33_USES_AVX && ( BLD == 4 )
        store4(val  , add4(load4(val  ), load4(M.val  )));
        store4(val+4, add4(load4(val+4), load4(M.val+4)));
        store4(val+8, add4(load4(val+8), load4(M.val+8)));
#else
        for ( int u = 0; u < BLD*3; ++u )
            val[u] += M.val[u];
#endif
    }

    /// add given matrix
    void operator -=(Matrix33 const& M)
    {
#if MATRIX33_USES_AVX && ( BLD == 4 )
        store4(val  , sub4(load4(val  ), load4(M.val  )));
        store4(val+4, sub4(load4(val+4), load4(M.val+4)));
        store4(val+8, sub4(load4(val+8), load4(M.val+8)));
#else
        for ( int u = 0; u < BLD*3; ++u )
            val[u] -= M.val[u];
#endif
    }
    
    /// transpose matrix in place
    void transpose()
    {
        std::swap(val[1], val[0+BLD]);
        std::swap(val[2], val[0+BLD*2]);
        std::swap(val[2+BLD], val[1+BLD*2]);
    }
    
    /// return transposed matrix
    Matrix33 transposed() const
    {
        Matrix33 res;
        for ( int x = 0; x < 3; ++x )
        for ( int y = 0; y < 3; ++y )
            res[y+BLD*x] = val[x+BLD*y];
        return res;
    }
    
    /// maximum of all component's absolute values
    real norm_inf() const
    {
        real res = fabs(val[0]);
        for ( unsigned i = 1; i < 3*BLD; ++i )
            res = std::max(res, fabs(val[i]));
        return res;
    }

    /// determinant of matrix
    real determinant() const
    {
        return ( val[0]*val[BLD+1]*val[BLD*2+2] + val[2]*val[BLD  ]*val[BLD*2+1]
                +val[1]*val[BLD+2]*val[BLD*2  ] - val[2]*val[BLD+1]*val[BLD*2  ]
                -val[1]*val[BLD  ]*val[BLD*2+2] - val[0]*val[BLD+2]*val[BLD*2+1] );
    }
    
    /// inverse in place
    void inverse()
    {
        real det = 1.0 / determinant();
        Vector3 X = column(0);
        Vector3 Y = column(1);
        Vector3 Z = column(2);
        setLines(cross(Y,Z)*det, cross(Z,X)*det, cross(X,Y)*det);
    }

    /// return inverse matrix
    Matrix33 inverted() const
    {
        Matrix33 res;
        real det = 1.0 / determinant();
        Vector3 X = column(0);
        Vector3 Y = column(1);
        Vector3 Z = column(2);
        res.setLines(cross(Y,Z)*det, cross(Z,X)*det, cross(X,Y)*det);
        //std::clog << " mat * inverse = " << mul(res).to_string(10, 3) << "\n";
        return res;
    }
    
    /// inversion of a symmetric matrix, addressing lower triangle
    /** This methods uses a L*D*L^t factorization with:
     L = ( 1 0 0; a 1 0; b c 1 )
     D = ( u 0 0; 0 v 0; 0 0 w )
     The result is a symetric matrix
     */
    int symmetricInverse()
    {
        /*
         // solving mat =  L * D * L^t:
         val[0+BLD*0] = u;
         val[1+BLD*0] = a * u;
         val[2+BLD*0] = b * u;
         val[0+BLD*1] = a * u;
         val[1+BLD*1] = a * a * u + v;
         val[2+BLD*1] = a * b * u + c * v;
         val[0+BLD*2] = b * u;
         val[1+BLD*2] = a * b * u + c * v;
         val[2+BLD*2] = b * b * u + c * c * v + w;
         */
        real u = val[0];
        real iu = 1.0 / u;
        real a = val[1] * iu;
        real b = val[2] * iu;
        real v = val[1+BLD] - a * val[1];
        real iv = 1.0 / v;
        real x = val[2+BLD] - a * val[2];
        real c = x * iv;
        real iw = 1.0 / ( val[2+BLD*2] - b * val[2] - c * x );
        // inverse triangular matrix U = inverse(L^t):
        b = -b + a * c;
        a = -a;
        c = -c;
        real aiv = a * iv;
        real biw = b * iw;
        real ciw = c * iw;
        // calculate U * inverse(D) * U^t:
        val[0+BLD*0] = iu + a * aiv + b * biw;
        val[1+BLD*0] = aiv + c * biw;
        val[2+BLD*0] = biw;
        val[0+BLD*1] = val[1+BLD*0];
        val[1+BLD*1] = iv + c * ciw;
        val[2+BLD*1] = ciw;
        val[0+BLD*2] = biw;
        val[1+BLD*2] = ciw;
        val[2+BLD*2] = iw;
        return 0;
    }

    /// copy values from lower triangle to upper triangle
    void copy_lower()
    {
        val[0+BLD  ] = val[1];
        val[0+BLD*2] = val[2];
        val[1+BLD*2] = val[2+BLD];
    }

    /// true if matrix is symmetric
    real asymmetry() const
    {
        return ( std::abs(val[BLD]-val[1])
                + std::abs(val[BLD*2]-val[2]) + std::abs(val[1+BLD*2]-val[2+BLD]) );
    }

#if MATRIX33_USES_AVX
    /// multiplication by a vector: this * V
    const vec4 vecmul3(double const* V) const
    {
        vec4 xyxy = broadcast2(V);
        vec4 xxxx = duplo4(xyxy); //broadcast1(V);
        vec4 yyyy = duphi4(xyxy); //broadcast1(V+1);
        vec4 zzzz = broadcast1(V+2);
#if ( BLD == 4 )
        xxxx = mul4(load4(val), xxxx);
        yyyy = mul4(load4(val+BLD), yyyy);
        return fmadd4(load4(val+BLD*2), zzzz, add4(xxxx,yyyy));
#else
        xxxx = mul4(load3(val), xxxx);
        yyyy = mul4(load3(val+BLD), yyyy);
        return fmadd4(load3(val+BLD*2), zzzz, add4(xxxx,yyyy));
#endif
    }
    
    /// multiplication by a vector: this * V
    const vec4 vecmul4(const vec4 xyzt) const
    {
        vec4 xyxy = permute2f128(xyzt, xyzt, 0x00);
        vec4 ztzt = permute2f128(xyzt, xyzt, 0x11);
        vec4 xxxx = duplo4(xyxy);
        vec4 yyyy = duphi4(xyxy);
        vec4 zzzz = duplo4(ztzt);
#if ( BLD == 4 )
        xxxx = mul4(load4(val), xxxx);
        yyyy = mul4(load4(val+BLD), yyyy);
        return fmadd4(load4(val+BLD*2), zzzz, add4(xxxx,yyyy));
#else
        xxxx = mul4(load3(val), xxxx);
        yyyy = mul4(load3(val+BLD), yyyy);
        return fmadd4(load3(val+BLD*2), zzzz, add4(xxxx,yyyy));
#endif
    }

    /// multiplication by a vector: transpose(M) * V
    const vec4 trans_vecmul3(double const* V) const
    {
#if ( BLD == 4 )
        vec4 vec = loadu4(V); // { x, y, z, garbage }
#else
        vec4 vec = load3(V); // { x, y, z, 0 }
#endif
        vec4 s0 = mul4(load4(val      ), vec);
        vec4 s1 = mul4(load4(val+BLD  ), vec);
        vec4 s2 = mul4(load4(val+BLD*2), vec);
        vec4 s3 = setzero4();
        s0 = add4(unpacklo4(s0, s1), unpackhi4(s0, s1));
        s1 = add4(unpacklo4(s2, s3), unpackhi4(s2, s3));
        return add4(permute2f128(s0, s1, 0x20), permute2f128(s0, s1, 0x31));
    }
#endif
    
    /// multiplication by a vector: this * V
    const Vector3 vecmul0(Vector3 const& V) const
    {
        return Vector3(val[0] * V.XX + val[  BLD] * V.YY + val[  BLD*2] * V.ZZ,
                       val[1] * V.XX + val[1+BLD] * V.YY + val[1+BLD*2] * V.ZZ,
                       val[2] * V.XX + val[2+BLD] * V.YY + val[2+BLD*2] * V.ZZ);
    }
    
    /// multiplication by a vector: this * V
    const Vector3 vecmul0(real const* ptr) const
    {
        return Vector3(val[0] * ptr[0] + val[  BLD] * ptr[1] + val[  BLD*2] * ptr[2],
                       val[1] * ptr[0] + val[1+BLD] * ptr[1] + val[1+BLD*2] * ptr[2],
                       val[2] * ptr[0] + val[2+BLD] * ptr[1] + val[2+BLD*2] * ptr[2]);
    }

    /// multiplication by a vector: this * V
    inline const Vector3 vecmul(Vector3 const& vec) const
    {
#if MATRIX33_USES_AVX
        return vecmul3(vec);
#else
        return vecmul0(vec);
#endif
    }
    
    /// multiplication by a vector: this * { ptr[0], ptr[1] }
    const Vector3 vecmul(real const* ptr) const
    {
#if MATRIX33_USES_AVX
        return vecmul3(ptr);
#else
        return vecmul0(ptr);
#endif
    }

    /// multiplication with a vector: M * V
    friend Vector3 operator * (Matrix33 const& mat, Vector3 const& vec)
    {
        return mat.vecmul(vec);
    }

    /// multiplication by a vector: transpose(M) * V
    const Vector3 trans_vecmul0(Vector3 const& V) const
    {
        return Vector3(val[0    ] * V.XX + val[1      ] * V.YY + val[2      ] * V.ZZ,
                       val[BLD  ] * V.XX + val[1+BLD  ] * V.YY + val[2+BLD  ] * V.ZZ,
                       val[BLD*2] * V.XX + val[1+BLD*2] * V.YY + val[2+BLD*2] * V.ZZ);
    }

    /// multiplication by a vector: transpose(M) * V
    const Vector3 trans_vecmul0(real const* V) const
    {
        return Vector3(val[0    ] * V[0] + val[1      ] * V[1] + val[2      ] * V[2],
                       val[BLD  ] * V[0] + val[1+BLD  ] * V[1] + val[2+BLD  ] * V[2],
                       val[BLD*2] * V[0] + val[1+BLD*2] * V[1] + val[2+BLD*2] * V[2]);
    }

    /// multiplication by a vector: transpose(M) * V
    inline const Vector3 trans_vecmul(real const* V) const
    {
#if MATRIX33_USES_AVX
        return trans_vecmul3(V);
#else
        return trans_vecmul0(V);
#endif
    }

    /// multiplication by another matrix: @returns this * M
    const Matrix33 mul(Matrix33 const& M) const
    {
        Matrix33 res;
        res[0] = val[0] * M[0] + val[0+BLD] * M[1] + val[0+BLD*2] * M[2];
        res[1] = val[1] * M[0] + val[1+BLD] * M[1] + val[1+BLD*2] * M[2];
        res[2] = val[2] * M[0] + val[2+BLD] * M[1] + val[2+BLD*2] * M[2];
        
        res[0+BLD] = val[0] * M[BLD] + val[0+BLD] * M[1+BLD] + val[0+BLD*2] * M[2+BLD];
        res[1+BLD] = val[1] * M[BLD] + val[1+BLD] * M[1+BLD] + val[1+BLD*2] * M[2+BLD];
        res[2+BLD] = val[2] * M[BLD] + val[2+BLD] * M[1+BLD] + val[2+BLD*2] * M[2+BLD];

        res[0+BLD*2] = val[0] * M[BLD*2] + val[0+BLD] * M[1+BLD*2] + val[0+BLD*2] * M[2+BLD*2];
        res[1+BLD*2] = val[1] * M[BLD*2] + val[1+BLD] * M[1+BLD*2] + val[1+BLD*2] * M[2+BLD*2];
        res[2+BLD*2] = val[2] * M[BLD*2] + val[2+BLD] * M[1+BLD*2] + val[2+BLD*2] * M[2+BLD*2];
        return res;
    }
    
    /// multiplication with a matrix
    friend Matrix33 operator * (Matrix33 const& mat, Matrix33 const& mut)
    {
        return mat.mul(mut);
    }

    /// multiplication by another matrix: @returns transpose(this) * M
    const Matrix33 trans_mul(Matrix33 const& M) const
    {
        Matrix33 res;
        res[0] = val[BLD*0] * M[0] + val[1+BLD*0] * M[1] + val[2+BLD*0] * M[2];
        res[1] = val[BLD*1] * M[0] + val[1+BLD*1] * M[1] + val[2+BLD*1] * M[2];
        res[2] = val[BLD*2] * M[0] + val[1+BLD*2] * M[1] + val[2+BLD*2] * M[2];
        
        res[0+BLD] = val[BLD*0] * M[BLD] + val[1+BLD*0] * M[1+BLD] + val[2+BLD*0] * M[2+BLD];
        res[1+BLD] = val[BLD*1] * M[BLD] + val[1+BLD*1] * M[1+BLD] + val[2+BLD*1] * M[2+BLD];
        res[2+BLD] = val[BLD*2] * M[BLD] + val[1+BLD*2] * M[1+BLD] + val[2+BLD*2] * M[2+BLD];
        
        res[0+BLD*2] = val[BLD*0] * M[BLD*2] + val[1+BLD*0] * M[1+BLD*2] + val[2+BLD*0] * M[2+BLD*2];
        res[1+BLD*2] = val[BLD*1] * M[BLD*2] + val[1+BLD*1] * M[1+BLD*2] + val[2+BLD*1] * M[2+BLD*2];
        res[2+BLD*2] = val[BLD*2] * M[BLD*2] + val[1+BLD*2] * M[1+BLD*2] + val[2+BLD*2] * M[2+BLD*2];
        return res;
    }
    
    
    /// add full matrix: this <- this + M
    void add_full(Matrix33 const& M)
    {
        real const* src = M.val;
#if MATRIX33_USES_AVX  && ( BLD == 4 )
        store4(val  , add4(load4(val  ), load4(src  )));
        store4(val+4, add4(load4(val+4), load4(src+4)));
        store4(val+8, add4(load4(val+8), load4(src+8)));
#else
        for ( int u = 0; u < BLD*3; ++u )
            val[u] += src[u];
#endif
    }
    
    /// add full matrix: this <- this + alpha * M
    void add_full(const real alpha, Matrix33 const& M)
    {
        real const* src = M.val;
#if MATRIX33_USES_AVX  && ( BLD == 4 )
        vec4 a = set4(alpha);
        store4(val  , fmadd4(a, load4(src  ), load4(val  )));
        store4(val+4, fmadd4(a, load4(src+4), load4(val+4)));
        store4(val+8, fmadd4(a, load4(src+8), load4(val+8)));
#else
        for ( int u = 0; u < BLD*3; ++u )
            val[u] += alpha * src[u];
#endif
    }
    
    /// sub full matrix: this <- this - M
    void sub_full(Matrix33 const& M)
    {
        real const* src = M.val;
#if MATRIX33_USES_AVX  && ( BLD == 4 )
        store4(val  , sub4(load4(val  ), load4(src  )));
        store4(val+4, sub4(load4(val+4), load4(src+4)));
        store4(val+8, sub4(load4(val+8), load4(src+8)));
#else
        for ( int u = 0; u < BLD*3; ++u )
            val[u] -= src[u];
#endif
    }

    /// subtract transposed matrix: this <- this - transposed(M)
    void sub_trans(Matrix33 const& M)
    {
        real const* src = M.val;
        for ( int x = 0; x < 3; ++x )
        for ( int y = 0; y < 3; ++y )
            val[y+BLD*x] -= src[x+BLD*y];
    }
    
    /// add transposed matrix: this <- this + alpha * transposed(M)
    void add_trans(Matrix33 const& M)
    {
        real const* src = M.val;
        for ( int x = 0; x < 3; ++x )
        for ( int y = 0; y < 3; ++y )
            val[y+BLD*x] += src[x+BLD*y];
    }
    
    /// add transposed matrix: this <- this + alpha * transposed(M)
    void add_trans(const real alpha, Matrix33 const& M)
    {
        real const* src = M.val;
        for ( int x = 0; x < 3; ++x )
            for ( int y = 0; y < 3; ++y )
                val[y+BLD*x] += alpha * src[x+BLD*y];
    }

    /// add lower triangle of matrix including diagonal: this <- this + M
    void add_half(Matrix33 const& M)
    {
        real const* src = M.val;
#if MATRIX33_USES_AVX  && ( BLD == 4 )
        store4(val  , add4(load4(val  ), load4(src  )));
        store4(val+4, add4(load4(val+4), load4(src+4)));
        store4(val+8, add4(load4(val+8), load4(src+8)));
#elif ( 1 )
        for ( int u = 0; u < BLD*3; ++u )
            val[u] += src[u];
#else
        for ( int x = 0; x < 3; ++x )
        for ( int y = x; y < 3; ++y )
            val[y+BLD*x] += src[y+BLD*x];
#endif
    }
    
    /// add lower triangle of matrix including diagonal: this <- this + alpha * M
    void add_half(const real alpha, Matrix33 const& M)
    {
        real const* src = M.val;
        //std::clog << "matrix alignment " << ((uintptr_t)src & 63) << "\n";
#if MATRIX33_USES_AVX  && ( BLD == 4 )
        vec4 a = set4(alpha);
        store4(val  , fmadd4(a, load4(src  ), load4(val  )));
        store4(val+4, fmadd4(a, load4(src+4), load4(val+4)));
        store4(val+8, fmadd4(a, load4(src+8), load4(val+8)));
#elif ( 1 )
        for ( int u = 0; u < BLD*3; ++u )
            val[u] += alpha * src[u];
#else
        for ( int x = 0; x < 3; ++x )
        for ( int y = x; y < 3; ++y )
            val[y+BLD*x] += alpha * src[y+BLD*x];
#endif
    }
    
    /// subtract lower triangle of matrix including diagonal: this <- this - M
    void sub_half(Matrix33 const& M)
    {
        real const* src = M.val;
#if MATRIX33_USES_AVX  && ( BLD == 4 )
        store4(val  , sub4(load4(val  ), load4(src  )));
        store4(val+4, sub4(load4(val+4), load4(src+4)));
        store4(val+8, sub4(load4(val+8), load4(src+8)));
#elif ( 1 )
        for ( int u = 0; u < BLD*3; ++u )
            val[u] -= src[u];
#else
        for ( int x = 0; x < 3; ++x )
        for ( int y = x; y < 3; ++y )
            val[y+BLD*x] -= src[y+BLD*x];
#endif
    }
    
    /// add alpha to diagonal
    void add_diag(real alpha)
    {
        val[0]       += alpha;
        val[1+BLD]   += alpha;
        val[2+BLD*2] += alpha;
    }
    
    /// add -alpha to diagonal
    void sub_diag(real alpha)
    {
        val[0]       -= alpha;
        val[1+BLD]   -= alpha;
        val[2+BLD*2] -= alpha;
    }

    /// add all elements of block 'S' to array 'M'
    void addto(real * M, unsigned ldd) const
    {
        M[0      ] += val[0];
        M[1      ] += val[1      ];
        M[2      ] += val[2      ];
        M[  ldd  ] += val[0+BLD  ];
        M[1+ldd  ] += val[1+BLD  ];
        M[2+ldd  ] += val[2+BLD  ];
        M[  ldd*2] += val[0+BLD*2];
        M[1+ldd*2] += val[1+BLD*2];
        M[2+ldd*2] += val[2+BLD*2];
    }
    
    /// add lower elements of this block to upper triangle of 'M'
    void addto_upper(real * M, unsigned ldd) const
    {
        M[0      ] += val[0];
        M[  ldd  ] += val[1      ];
        M[  ldd*2] += val[2      ];
        M[1+ldd  ] += val[1+BLD  ];
        M[1+ldd*2] += val[2+BLD  ];
        M[2+ldd*2] += val[2+BLD*2];
    }
    
    /// add lower elements of this block to both upper and lower triangles of 'M'
    void addto_symm(real * M, unsigned ldd) const
    {
        M[0      ] += val[0];
        M[1      ] += val[1      ];
        M[2      ] += val[2      ];
        M[  ldd  ] += val[1      ];
        M[1+ldd  ] += val[1+BLD  ];
        M[2+ldd  ] += val[2+BLD  ];
        M[  ldd*2] += val[2      ];
        M[1+ldd*2] += val[2+BLD  ];
        M[2+ldd*2] += val[2+BLD*2];
    }
    
    /// add all elements of this block to 'M', with transposition
    void addto_trans(real * M, unsigned ldd) const
    {
        M[0      ] += val[0];
        M[1      ] += val[0+BLD  ];
        M[2      ] += val[0+BLD*2];
        M[  ldd  ] += val[1      ];
        M[1+ldd  ] += val[1+BLD  ];
        M[2+ldd  ] += val[1+BLD*2];
        M[  ldd*2] += val[2      ];
        M[1+ldd*2] += val[2+BLD  ];
        M[2+ldd*2] += val[2+BLD*2];
    }
    
    
    /// return symmetric Matrix from coordinates (column-major, lower triangle)
    static Matrix33 symmetric(real a, real b, real c,
                              real d, real e, real f)
    {
        return Matrix33(a, b, c, b, d, e, c, e, f);
    }

    /// return diagonal Matrix from diagonal terms
    static Matrix33 diagonal(real a, real b, real c)
    {
        return Matrix33(a, 0, 0, 0, b, 0, 0, 0, c);
    }
    
    /// identity matrix
    static Matrix33 identity()
    {
        return Matrix33(0, 1);
    }

    /// return a symmetric matrix: [ dir (x) transpose(dir) ]
    static Matrix33 outerProduct(Vector3 const& dir)
    {
        return symmetric(dir.XX*dir.XX, dir.YY*dir.XX, dir.ZZ*dir.XX,
                         dir.YY*dir.YY, dir.YY*dir.ZZ,
                         dir.ZZ*dir.ZZ );
    }

    /// return a symmetric matrix: alpha * [ dir (x) transpose(dir) ]
    static Matrix33 outerProduct(Vector3 const& dir, real alpha)
    {
#if MATRIX33_USES_AVX && ( BLD == 4 )
        Matrix33 res;
        vec4 p = permute2f128(dir, dir, 0x01);
        vec4 l = blend4(dir, p, 0b1100);
        vec4 u = blend4(dir, p, 0b0011);
        vec4 d = mul4(dir, set4(alpha));
        store4(res.val  , mul4(d, duplo4(l)));
        store4(res.val+4, mul4(d, duphi4(l)));
        store4(res.val+8, mul4(d, duplo4(u)));
        return res;
#else
        real XX = dir.XX * alpha;
        real YY = dir.YY * alpha;
        real ZZ = dir.ZZ * alpha;
        return symmetric(dir.XX*XX, dir.YY*XX, dir.ZZ*XX,
                         dir.YY*YY, dir.YY*ZZ,
                         dir.ZZ*ZZ );
#endif
    }
    
    /// return outer product: [ dir (x) transpose(vec) ]
    static Matrix33 outerProduct(Vector3 const& dir, Vector3 const& vec)
    {
#if MATRIX33_USES_AVX && ( BLD == 4 )
        Matrix33 res;
        vec4 p = permute2f128(vec, vec, 0x01);
        vec4 l = blend4(vec, p, 0b1100);
        vec4 u = blend4(vec, p, 0b0011);
        store4(res.val  , mul4(dir, duplo4(l)));
        store4(res.val+4, mul4(dir, duphi4(l)));
        store4(res.val+8, mul4(dir, duplo4(u)));
        return res;
#else
        return Matrix33(dir.XX*vec.XX, dir.YY*vec.XX, dir.ZZ*vec.XX,
                        dir.XX*vec.YY, dir.YY*vec.YY, dir.ZZ*vec.YY,
                        dir.XX*vec.ZZ, dir.YY*vec.ZZ, dir.ZZ*vec.ZZ );
#endif
    }
    
    /// return outer product: [ dir (x) transpose(vec) ]
    static Matrix33 outerProduct(real const* dir, real const* vec)
    {
#if MATRIX33_USES_AVX && ( BLD == 4 )
        Matrix33 res;
        vec4 d = load3(dir);
        store4(res.val  , mul4(d, broadcast1(vec  )));
        store4(res.val+4, mul4(d, broadcast1(vec+1)));
        store4(res.val+8, mul4(d, broadcast1(vec+2)));
        return res;
#else
        return Matrix33(dir[0]*vec[0], dir[1]*vec[0], dir[2]*vec[0],
                        dir[0]*vec[1], dir[1]*vec[1], dir[2]*vec[1],
                        dir[0]*vec[2], dir[1]*vec[2], dir[2]*vec[2] );
#endif
    }
    
    /// add outer product: [ dir (x) transpose(vec) ]
    void addOuterProduct(real const* dir, real const* vec)
    {
#if MATRIX33_USES_AVX && ( BLD == 4 )
        vec4 d = load3(dir);
        store4(val  , fmadd4(d, broadcast1(vec  ), load4(val  )));
        store4(val+4, fmadd4(d, broadcast1(vec+1), load4(val+4)));
        store4(val+8, fmadd4(d, broadcast1(vec+2), load4(val+8)));
#else
        val[0      ] += dir[0]*vec[0];
        val[1      ] += dir[1]*vec[0];
        val[2      ] += dir[2]*vec[0];
        val[0+BLD  ] += dir[0]*vec[1];
        val[1+BLD  ] += dir[1]*vec[1];
        val[2+BLD  ] += dir[2]*vec[1];
        val[0+BLD*2] += dir[0]*vec[2];
        val[1+BLD*2] += dir[1]*vec[2];
        val[2+BLD*2] += dir[2]*vec[2];
#endif
    }

    /// return [ dir (x) transpose(vec) + vec (x) transpose(dir) ]
    static Matrix33 symmetricOuterProduct(Vector3 const& dir, Vector3 const& vec)
    {
        real xx = dir.XX * vec.XX;
        real yy = dir.YY * vec.YY;
        real zz = dir.ZZ * vec.ZZ;
        return symmetric(xx+xx, dir.YY*vec.XX + dir.XX*vec.YY, dir.ZZ*vec.XX + dir.XX*vec.ZZ,
                         yy+yy, dir.ZZ*vec.YY + dir.YY*vec.ZZ,
                         zz+zz);
    }
 
    /// return symmetric matrix block :  dia * I + [ dir (x) dir ] * len
    static Matrix33 offsetOuterProduct(const real dia, Vector3 const& dir, const real len)
    {
        real xl = dir.XX * len;
        real yl = dir.YY * len;
        real zl = dir.ZZ * len;
        return symmetric(xl * dir.XX + dia, yl * dir.XX, zl * dir.XX,
                         yl * dir.YY + dia, zl * dir.YY,
                         zl * dir.ZZ + dia);
    }
    
    // build the rotation matrix `M = 2 V * V' - 1` of angle 180
    static Matrix33 householder(const Vector3& axis)
    {        
        real X = axis.XX, Y = axis.YY, Z = axis.ZZ;
        real X2 = X + X, Y2 = Y + Y, Z2 = Z + Z;
        
        return symmetric(X * X2 - 1.0, X * Y2, X * Z2,
                         Y * Y2 - 1.0, Y * Z2,
                         Z * Z2 - 1.0);
    }

    /// rotation around `axis` (of norm 1) with angle set by cosinus and sinus values
    /** The values of 'c' and 's' can be scaled to obtain a matrix where the
     rotation components is also scaling. Vectors along the axis remain unchanged */
    static Matrix33 rotationAroundAxis(const Vector3& axis, const real c, const real s)
    {
        /*
         This is using Rodrigues's formula:
             I + sinus * K + ( 1 - cosinus ) * K^2
             K = -1 (x) axis
        Attention: this is correct only if norm(axis)==1
         */
        const real  X = axis.XX  ,  Y = axis.YY  ,  Z = axis.ZZ;
        const real dX = X - c * X, dY = Y - c * Y, dZ = Z - c * Z;
        const real sX = s * X    , sY = s * Y    , sZ = s * Z;

        return Matrix33(dX * X + c , dY * X + sZ, dZ * X - sY,
                        dX * Y - sZ, dY * Y + c , dZ * Y + sX,
                        dX * Z + sY, dY * Z - sX, dZ * Z + c );
    }

    /// rotation axis
    Vector3 rotationAxis() const;

    /// rotation angle
    real rotationAngle() const;

    /// calculate rotation angle and Euler angle of axis
    void getEulerAngles(real& angle, real&, real&) const;

    ///
    static Matrix33 rotationAroundAxisEuler(const real a[3]);

    /// return rotation of angle a, around axis of azimuth b and elevation c
    static Matrix33 rotationFromAngles(const real a[3]);

    
    /// return a rotation that transforms (1,0,0) into (-1,0,0)
    static Matrix33 rotation180();

    /// a rotation around the X axis of specified angle
    static Matrix33 rotationAroundX(real angle);
    
    /// a rotation around the Y axis of specified angle
    static Matrix33 rotationAroundY(real angle);
    
    /// a rotation around the Z axis of specified angle
    static Matrix33 rotationAroundZ(real angle);
    
    /// a rotation around one the axis X if `x==0`, Y if `x==1` or Z if `x==2`
    static Matrix33 rotationAroundPrincipalAxis(unsigned x, real angle);

    /// return a rotation that transforms (1,0,0) into `vec` ( norm(vec) should be > 0 )
    static Matrix33 rotationToVector(const Vector3&);
    
    /// return a random rotation that transforms (1,0,0) into `vec` ( norm(vec) should be > 0 )
    /**
     In 3D, this rotation is chosen uniformly among all the rotation transforming (1,0,0) into `vec`.
     The function will fail if ( vec == 0 ).
     */
    static Matrix33 randomRotationToVector(const Vector3&);
    
    /// a random rotation chosen uniformly
    static Matrix33 randomRotation();
    
    /// a rotation of angle 'angle' around an axis chosen randomly
    static Matrix33 randomRotation(real angle);
};


/// output a Matrix33
inline std::ostream& operator << (std::ostream& os, Matrix33 const& mat)
{
    int w = (int)os.width();
    os.width(1);
    os << "[";
    for ( int i = 0; i < 3; ++i )
    {
        for ( int j = 0; j < 3; ++j )
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

