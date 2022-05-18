// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// F. Nedelec, Strasbourg 08.06.2018

#ifndef MATRIX11
#define MATRIX11

#include "real.h"
#include "vector1.h"
#include <cstdio>
#include <iostream>

/// 1x1 matrix class with 1 'real' elements
/**
 This matrix can use AVX instructions if 'real == double'
 */
class Matrix11
{
private:
    
    real val_;
    
    /// access to modifiable element by index
    real& operator[](int i)       { return val_; }
    
    /// access element value by index
    real  operator[](int i) const { return val_; }

public:
    
    Matrix11() {}
    
    /// construct Matrix with value `v`
    Matrix11(real v)
    {
        val_ = v;
    }

    /// construct Matrix with `d` on the diagonal and other values equal to `a`
    Matrix11(real, real d)
    {
        val_ = d;
    }

    ~Matrix11() {}
    
    /// dimensionality
    static int dimension() { return 1; }
    
    /// human-readible identifier
    static std::string what() { return "1"; }
    
    /// set all elements to zero
    void reset()
    {
        val_ = 0.;
    }
    
    /// true if element is different from 'zero'
    bool operator != (real zero) const
    {
        return ( val_ != zero );
    }
    
    /// conversion to real
    //operator real() const { return val_; }

    /// copy values from lower triangle to upper triangle
    void copy_lower() { }

    /// direct access to 'unique' scalar
    real& value()            { return val_; }
    real  value() const      { return val_; }
    
    /// conversion to array of 'real'
    real* data()             { return &val_; }
    real* addr(int i, int j) { return &val_; }
    
    /// access functions to element by line and column indices
    real& operator()(int i, int j)       { return val_; }
    real  operator()(int i, int j) const { return val_; }
    
    /// extract column vector at given index
    Vector1 column(const unsigned) const
    {
        return Vector1(val_);
    }
    
    /// extract line vector at given index
    Vector1 line(const unsigned) const
    {
        return Vector1(val_);
    }

    /// extract diagonal
    Vector1 diagonal() const
    {
        return Vector1(val_);
    }

    /// sum of diagonal terms
    real trace() const
    {
        return val_;
    }

    /// human-friendly output
    void print(FILE * f) const
    {
        fprintf(f, "[ %9.3f ]\n", val_);
    }

    /// conversion to string
    std::string to_string(int w, int p) const
    {
        std::ostringstream os;
        os.precision(p);
        os << "[ " << value() << " ]";
        return os.str();
    }

    /// always zero
    real asymmetry() const
    {
        return 0;
    }
    
    /// scale all elements
    void scale(const real alpha)
    {
        val_ *= alpha;
    }
    
    /// scale matrix
    void operator *=(const real alpha)
    {
        scale(alpha);
    }
    
    /// return opposite matrix (i.e. -M)
    const Matrix11 operator -() const
    {
        return Matrix11(-val_);
    }
    
    /// scaled matrix
    const Matrix11 operator *(const real alpha) const
    {
        return Matrix11( val_ * alpha );
    }

    /// return sum of two matrices
    const Matrix11 operator +(Matrix11 const& M) const
    {
        return Matrix11( val_ + M.val_ );
    }

    /// return difference of two matrices
    const Matrix11 operator -(Matrix11 const& M) const
    {
        return Matrix11( val_ - M.val_ );
    }

    /// subtract given matrix
    void operator +=(Matrix11 const& M)
    {
        val_ += M.val_;
    }

    /// add given matrix
    void operator -=(Matrix11 const& M)
    {
        val_ -= M.val_;
    }
    
    /// transpose matrix in place
    void transpose()
    {
    }
    
    /// return transposed matrix
    Matrix11 transposed() const
    {
        return Matrix11(val_);
    }
    
    /// maximum of all component's absolute values
    real norm_inf() const
    {
        return fabs(val_);
    }

    /// multiplication by a vector: this * V
    const Vector1 vecmul(Vector1 const& V) const
    {
        return Vector1(val_ * V.XX);
    }
    
    /// multiplication by a vector: this * V
    const Vector1 vecmul(real const* ptr) const
    {
        return Vector1(val_ * ptr[0]);
    }

    /// matrix-vector multiplication
    friend Vector1 operator * (Matrix11 const& mat, Vector1 const& vec)
    {
        return mat.vecmul(vec);
    }

    /// multiplication by a vector: transpose(M) * V
    const Vector1 trans_vecmul(real const* V) const
    {
        return Vector1(val_ * V[0]);
    }

    /// multiplication by another matrix: @returns this * B
    const Matrix11 mul(Matrix11 const& B) const
    {
        return Matrix11(val_ * B.val_);
    }
    
    /// matrix-matrix multiplication
    friend Matrix11 operator * (Matrix11 const& mat, Matrix11 const& mut)
    {
        return mat.mul(mut);
    }

    /// multiplication by another matrix: @returns transpose(this) * B
    const Matrix11 trans_mul(Matrix11 const& B) const
    {
        return Matrix11(val_ * B.val_);
    }

    /// add full matrix: this <- this + M
    void add_full(Matrix11 const& M)
    {
        val_ += M.val_;
    }
    
    /// add full matrix: this <- this + alpha * M
    void add_full(const real alpha, Matrix11 const& M)
    {
        val_ += alpha * M.val_;
    }
    
    /// add lower triangle of matrix including diagonal: this <- this + M
    void add_half(Matrix11 const& M)
    {
        val_ += M.val_;
    }
    
    /// add lower triangle of matrix including diagonal: this <- this + alpha * M
    void add_half(const real alpha, Matrix11 const& M)
    {
        val_ += alpha * M.val_;
    }
    
    /// subtract lower triangle of matrix including diagonal: this <- this - M
    void sub_half(Matrix11 const& M)
    {
        val_ -= M.val_;
    }
    
    /// add alpha to diagonal
    void add_diag(real alpha)
    {
        val_ += alpha;
    }
    
    /// add -alpha to diagonal
    void sub_diag(real alpha)
    {
        val_ -= alpha;
    }
    
    /// add all elements of block 'S' to array 'M'
    void addto(real * M, unsigned ldd) const
    {
        M[0] += val_;
    }
    
    /// add lower elements of this block to upper triangle of 'M'
    void addto_upper(real * M, unsigned ldd) const
    {
        M[0] += val_;
    }
    
    /// add lower elements of this block to both upper and lower triangles of 'M'
    void addto_symm(real * M, unsigned ldd) const
    {
        M[0] += val_;
    }
    
    /// add all elements of this block to 'M', with transposition
    void addto_trans(real * M, unsigned ldd) const
    {
        M[0] += val_;
    }
    
    /// return `a * Identity`
    static Matrix11 diagonal(real a)
    {
        return Matrix11(a);
    }
    
    /// identity matrix
    static Matrix11 identity()
    {
        return Matrix11(1);
    }

    /// set a symmetric matrix: [ dir (x) transpose(dir) ]
    static Matrix11 outerProduct(Vector1 const& dir)
    {
        return Matrix11(dir.XX * dir.XX);
    }

    /// set a symmetric matrix: alpha * [ dir (x) transpose(dir) ]
    static Matrix11 outerProduct(Vector1 const& dir, real alpha)
    {
        return Matrix11(dir.XX * dir.XX * alpha);
    }
    
    /// return outer product: [ dir (x) transpose(vec) ]
    static Matrix11 outerProduct(Vector1 const& dir, Vector1 const& vec)
    {
        return Matrix11(dir.XX * vec.XX);
    }
    
    /// return symmetric matrix block :  dia * I + [ dir (x) dir ] * len
    static Matrix11 offsetOuterProduct(const real dia, Vector1 const& dir, const real len)
    {
        return Matrix11(len * dir.XX * dir.XX + dia);
    }
    
    /// return rotation matrix of angle defined by cosinus and sinus
    static Matrix11 rotation(real c, real s)
    {
        return Matrix11(std::copysign(1, c));
    }

    /// rotation angle
    real rotationAngle() const;

    /// return a rotation that transforms (1,0,0) into (-1,0,0)
    static Matrix11 rotation180();

    /// return a rotation that transforms (1,0,0) into `vec` ( norm(vec) should be > 0 )
    static Matrix11 rotationToVector(const Vector1& vec);
    
    /// return a random rotation that transforms (1,0,0) into `vec` ( norm(vec) should be > 0 )
    static Matrix11 randomRotationToVector(const Vector1& vec)
    {
        return rotationToVector(vec);
    }
    
    /// a random rotation chosen uniformly
    static Matrix11 randomRotation();
    
    /// a random rotation of given angle
    static Matrix11 randomRotation(real);

};


/// output a Matrix11
inline std::ostream& operator << (std::ostream& os, Matrix11 const& mat)
{
    os << "[ " << mat.value() << " ]";
    return os;
}

#endif

