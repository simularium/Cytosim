// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 This contains C front-ends to some functions of BLAS
 see http://www.netlib.org/blas
  
 Functions are renamed : 
 
 xcopy calls scopy if ( real is float ), or dcopy if ( real is double ).
*/

#ifndef CBLAS_H 
#define CBLAS_H

#include "real.h"
#include "simd.h"
#include <cstdio>


#ifdef __cplusplus
namespace blas {
extern "C" {
#endif
    
#undef FORTRAN
    
#if REAL_IS_DOUBLE
#   define FORTRAN(x) d##x##_
#   define iFORTRAN(x) id##x##_
#else
#   define FORTRAN(x) s##x##_
#   define iFORTRAN(x) is##x##_
#endif

/*
 * ===========================================================================
 * Prototypes for level 1 BLAS routines
 * ===========================================================================
 */
#pragma mark -

float  sdot_(int*, const float*, int*, const float*, int*);
double ddot_(int*, const double*, int*, const double*, int*);
double dsdot_(int*, const float*, int*, const float*, int*);


/**
 We always use double precision to accumulate the dot product of two vectors:
 */
inline double xdot(int N, const real* X, int incX, const real* Y, int incY)
{
#if REAL_IS_DOUBLE
    return ddot_(&N, X, &incX, Y, &incY);
#else
    return dsdot_(&N, X, &incX, Y, &incY);
#endif
}

inline double dot(int N, const real* X, const real* Y)
{
    int one = 1;
#if REAL_IS_DOUBLE
    return ddot_(&N, X, &one, Y, &one);
#else
    return dsdot_(&N, X, &one, Y, &one);
#endif
}

/// this is the standard Euclidian norm
inline double nrm2(int N, const real* X)
{
    //using double precision to accumulate:
    return sqrt(blas::xdot(N, X, 1, X, 1));
}
    
inline real ddot(int N, const double* X, int incX, const double* Y, int incY)
{
    return ddot_(&N, X, &incX, Y, &incY);
}

inline double dsdot(int N, const float* X, int incX, const float* Y, int incY)
{
    return dsdot_(&N, X, &incX, Y, &incY);
}

double sdsdot_(int*, const float* s, const float*, int*, const float*, int*);
inline double sdsdot(int N, float SB, const float* X, int incX, const float* Y, int incY)
{
    return sdsdot_(&N, &SB, X, &incX, Y, &incY);
}


// use 'blas::nrm2' defined above if applicable
real FORTRAN(nrm2)(int*, const real*, int*);
inline real xnrm2(int N, const real*X, int incX)
{
    return FORTRAN(nrm2)(&N, X, &incX);
}
    
real FORTRAN(asum)(int*, const real*, int*);
inline real xasum(int N, const real*X, int incX)
{
    return FORTRAN(asum)(&N, X, &incX);
}

real FORTRAN(sum)(int*, const real*, int*);
inline real xsum(int N, const real*X, int incX)
{
    return FORTRAN(sum)(&N, X, &incX);
}

int iFORTRAN(amax)(int*, const real*, int*);
inline int ixamax(int N, const real*X, int incX)
{
    return (iFORTRAN(amax)(&N, X, &incX) - 1);
}

int iFORTRAN(max)(int*, const real*, int*);
inline int ixmax(int N, const real*X, int incX)
{
    return (iFORTRAN(max)(&N, X, &incX) - 1);
}

int iFORTRAN(amin)(int*, const real*, int*);
inline int ixamin(int N, const real*X, int incX)
{
    return (iFORTRAN(amin)(&N, X, &incX) - 1);
}

int iFORTRAN(min)(int*, const real*, int*);
inline int ixmin(int N, const real*X, int incX)
{
    return (iFORTRAN(min)(&N, X, &incX) - 1);
}

void FORTRAN(swap)(int*, real*, int*, real*, int*);
inline void xswap(int N, real*X, int incX, real*Y, int incY)
{
    FORTRAN(swap)(&N, X, &incX, Y, &incY);
}

void FORTRAN(copy)(int*, const real*, int*, real*, int*);
inline void xcopy(int N, const real*X, int incX, real*Y, int incY)
{
    FORTRAN(copy)(&N, X, &incX, Y, &incY);
}

inline void copy(int N, const real* X, real* Y)
{
    //copy_real(N, X, Y);
    blas::xcopy(N, X, 1, Y, 1);
}

void FORTRAN(axpy)(int*, real*, const real*, int*, real*, int*);
inline void xaxpy(int N, real alpha, const real*X, int incX, real*Y, int incY)
{
    FORTRAN(axpy)(&N, &alpha, X, &incX, Y, &incY);
}
    
void FORTRAN(rotg)(real*, real*, real*, real*);
inline void xrotg(real*a, real*b, real*c, real*s)
{
    FORTRAN(rotg)(a, b, c, s);
}

void FORTRAN(rotmg)(const real*, const real*, const real*, real*, real*);
inline void xrotmg(const real*d1, const real*d2, const real*b1, real b2, real*P)
{
    FORTRAN(rotmg)(d1, d2, b1, &b2, P);
}

void FORTRAN(rot)(int*, real*, int*, real*, int*, real*, real*);
inline void xrot( int N, real*X, int incX, real*Y, int incY, real c, real s)
{
    FORTRAN(rot)(&N, X, &incX, Y, &incY, &c, &s);
}

void FORTRAN(rotm)(int*, real*, int*, real*, int*, real*);
inline void xrotm( int N, real*X, int incX, real*Y, int incY, real*P)
{
    FORTRAN(rotm)(&N, X, &incX, Y, &incY, P);
}

void FORTRAN(scal)(int*, real*, real*, int*);
inline void xscal(int N, real alpha, real*X, int incX)
{
    FORTRAN(scal)( &N, &alpha, X, &incX);
}
    
#ifdef __INTEL_MKL__
    /**
     axpby() performs Y <- alpha * X + beta * Y
     This routine is not part of BLAS, but is provided by Intel Math Kernel Library
     */
    void FORTRAN(axpby)(int*, real*, const real*, int*, real*, real*, int*);
    inline void xaxpby(int N, real alpha, const real*X, int incX, real beta, real*Y, int incY)
    {
        FORTRAN(axpby)(&N, &alpha, X, &incX, &beta, Y, &incY);
    }
#else
    inline void xaxpby(int N, real alpha, const real* X, int incX, real beta, real* Y, int incY)
    {
        if ( incX == 1  &&  incY == 1 )
        {
            for ( int i = 0; i < N; ++i )
                Y[i] = alpha * X[i] + beta * Y[i];
        }
        else
        {
            for ( int i = 0; i < N; ++i )
                Y[i*incY] = alpha * X[i*incX] + beta * Y[i*incY];
        }
    }
#endif


/// calculates Y <- alpha * Y + X
inline void xpay(int N, const real* X, real alpha, real* Y)
{
    #pragma ivdep
    #pragma vector always
    for ( int i = 0; i < N; ++i )
        Y[i] = alpha * Y[i] + X[i];
}


/// addition Y[] <- Y[] + X[], for array of size N
inline void add(int N, const real* X, real* Y)
{
    //xaxpy(N, 1.0, X, 1, Y, 1);
    #pragma ivdep
    #pragma vector always
    for ( int i = 0; i < N; ++i )
        Y[i] = Y[i] + X[i];
}
    
/// subtraction Y[] <- Y[] - X[], for array of size N
inline void sub(int N, const real* X, real* Y)
{
    //xaxpy(N, -1.0, X, 1, Y, 1);
    #pragma ivdep
    #pragma vector always
    for ( int i = 0; i < N; ++i )
        Y[i] = Y[i] - X[i];
}

/*
 * ===========================================================================
 * Prototypes for level 2 BLAS
 * ===========================================================================
 */
#pragma mark -


void FORTRAN(gemv)(char*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void xgemv(char TransA, int M, int N, real alpha, const real*A, int lda,
                       const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(gemv)(&TransA, &M, &N, &alpha, A, &lda, X, &incX, &beta, Y, &incY);
}

void FORTRAN(trmv)( char*, char*, char*, int*, const real*, int*, real*, int*);
inline void xtrmv( char Uplo, char TransA, char Diag, int N, const real*A, int lda, real*X, int incX)
{
    FORTRAN(trmv)(&Uplo, &TransA, &Diag, &N, A, &lda, X, &incX);
    
}

inline void xtrsv(char Uplo, char TransA, char Diag, int N, const real*A, int lda, real*X, int incX);

inline void xgbmv(char TransA, int M, int N, int Kl,  int Ku, real alpha, const real*A, int lda, const real*X, int incX, real beta, real*Y, int incY);

inline void xtbmv(char Uplo, char TransA, char Diag, int N, int K, const real*A, int lda, real*X, int incX);

inline void xtbsv(char Uplo, char TransA, char Diag, int N, int K, const real*A, int lda, real*X, int incX);

inline void xtpsv(char Uplo, char TransA, char Diag, int N, const real*Ap, real*X, int incX);

inline void xtpmv(char Uplo, char TransA, char Diag, int N, const real*Ap, real*X, int incX);

void FORTRAN(ger)(int*, int*, real* alpha, const real*, int*, const real*, int*, real*, int*);
inline void xger(int M, int N, real alpha, const real*X, int incX, const real*Y, int incY, real*A, int lda)
{
    FORTRAN(ger)(&M, &N, &alpha, X, &incX, Y, &incY, A, &lda);
}


void FORTRAN(symv)(char*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void xsymv(char Uplo, int N, real alpha,  const real*A, int lda, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(symv)(&Uplo,&N,&alpha,A,&lda,X,&incX,&beta,Y,&incY);
}

void FORTRAN(sbmv)(char*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void xsbmv(char Uplo, int N, int K, real alpha, const real*A, int lda, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(sbmv)(&Uplo,&N,&K,&alpha,A,&lda,X,&incX,&beta,Y,&incY);
}

void FORTRAN(spmv)(char*, int*, real*, const real*, const real*, int*, real*, real*, int*);
inline void xspmv(char Uplo, int N, real alpha, const real*A, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(spmv)(&Uplo,&N,&alpha,A,X,&incX,&beta,Y,&incY);
}

void FORTRAN(syr)(char*, int*, real*, const real*, int*, real*, int*);
inline void xsyr(char Uplo, int N, real alpha, const real*X, int incX, real*A, int lda)
{
    FORTRAN(syr)(&Uplo, &N, &alpha, X, &incX, A, &lda);
}

void FORTRAN(syr2)(char*, int*, real*, const real*, int*, const real*, int*, real*, int*);
inline void xsyr2(char Uplo, int N, real alpha, const real*X, int incX, const real*Y, int incY, real* A, int lda)
{
    FORTRAN(syr2)(&Uplo, &N, &alpha, X, &incX, Y, &incY, A, &lda);
}

    
void FORTRAN(spr)(char*, int*, real*, const real*, int*, real*);
inline void xspr(char Uplo, int N, real alpha, const real*X, int incX, real*Ap)
{
    FORTRAN(spr)(&Uplo, &N, &alpha, X, &incX, Ap);
}

inline void xspr2(char Uplo, int N, real alpha, const real*X, int incX, const real*Y, int incY, real*A);

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */
#pragma mark -


void FORTRAN(gemm)(char*, char*, int*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void xgemm(char TransA, char TransB, int M, int N, int K, real alpha, const real*A,
                       int lda, const real*B, int ldb, real beta, real*C, int ldc)
{
    FORTRAN(gemm)(&TransA,&TransB,&M,&N,&K,&alpha,A,&lda,B,&ldb,&beta,C,&ldc);
}

void FORTRAN(symm)(char*, char*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void xsymm(char Side, char Uplo, int M, int N, real alpha, const real*A, int lda,
                       const real*B, int ldb, real beta, real*C, int ldc)
{
    FORTRAN(symm)(&Side,&Uplo,&M,&N,&alpha,A,&lda,B,&ldb,&beta,C,&ldc);
}


void FORTRAN(syrk)(char*, char*, int*, int*, real*, const real*, int*, real*, real*, int*);
inline void xsyrk(char Uplo, char Trans, int N, int K, real alpha, const real*A, int lda, real beta, real*C, int ldc)
{
    FORTRAN(syrk)(&Uplo,&Trans,&N,&K,&alpha,A,&lda,&beta,C,&ldc);
}

inline void xsyr2k(char Uplo, char Trans, int N, int K, real alpha, const real*A, int lda, const real*B, int ldb, real beta, real*C, int ldc);

inline void xtrmm(char Uplo, char TransA, char Diag, int M, int N, real alpha, const real*A, int lda, real*B, int ldb);

void FORTRAN(trsm)(char*, char*, char*, char*, int*, int*, real*, const real*, int*, real*, int*);
inline void xtrsm(char side, char uplo, char transA, char diag, int M, int N, real alpha, const real*A, int lda, real*B, int ldb)
{
    FORTRAN(trsm)(&side, &uplo, &transA, &diag, &M, &N, &alpha, A, &lda, B, &ldb);
}
    
    
#ifdef __cplusplus
}}
#endif

/*
 * ===========================================================================
 * Below are non-standard additions by Francois Nedelec
 * ===========================================================================
 */

#include <algorithm>

namespace blas
{

/**
 return the infinite norm of the vector

     int inx = ixamax(N, X, inc);
     return fabs(X[inx]);
 
 */
inline real nrm8(const int N, const real* X, int inc)
{
    if ( N == 0 )
        return 0;
    real u = std::abs(X[0]);
    for ( int i = 1; i < N; ++i )
        u = std::max(u, std::abs(X[i*inc]));
    return u;
}


inline real nrm8(const int siz, const real* X)
{
    real res = std::abs(X[0]);
#pragma ivdep
#pragma vector always
    for ( int i = 1; i < siz; ++i )
        res = std::max(res, std::abs(X[i]));
    return res;
}


/**
 return the infinite norm of the difference between two vectors
 */
inline real max_diff(const int N, const real* X, const real* Y)
{
    if ( N == 0 )
        return 0;
    real u = std::abs(X[0] - Y[0]);
    for ( int i = 1; i < N; ++i )
        u = std::max(u, std::abs(X[i] - Y[i]));
    return u;
}


/**
Set N values of `X` to value `alpha`
 */
inline void xfill(const int N, real alpha, real* X)
{
    for ( int u = 0; u < N; ++u )
        X[u] = alpha;
}

/**
 Set N values of `X` to value `alpha`
*/
inline void xfill(const int N, real alpha, real* X, const int inc)
{
    for ( int u = 0; u < N; ++u )
        X[u*inc] = alpha;
}

}

#endif
