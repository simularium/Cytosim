// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "exceptions.h"
#include "vecprint.h"


void Mecafil::buildProjection()
{
    //reset all variables for the projections:
    mtJJt        = nullptr;
    mtJJtiJforce = nullptr;
}


void Mecafil::allocateProjection(const size_t ms)
{
    //std::clog << reference() << "allocateProjection(" << nbp << ")\n";
    free_real(mtJJt);
    real * mem = new_real(4*ms);
    //zero_real(4*ms, mem);
    mtJJt        = mem;
    mtJJtU       = mem + ms * 2;
    mtJJtiJforce = mem + ms * 3;
}


void Mecafil::destroyProjection()
{
    //std::clog << reference() << "destroyProjection\n";
    free_real(mtJJt);
    mtJJt        = nullptr;
    mtJJtU       = nullptr;
    mtJJtiJforce = nullptr;
}



/// standard version with isotropic drag coefficient
void Mecafil::makeProjection()
{
    assert_true( nbPoints() >= 2 );

    //set the diagonal and off-diagonal of J*J'
    const unsigned nbu = nbPoints() - 2;
    const real*const dif = rfDiff;

    for ( unsigned jj = 0; jj < nbu; ++jj )
    {
        const real* X = dif + DIM * jj;
#if ( DIM == 2 )
        real xn = X[0]*X[2] + X[1]*X[3];
#else
        real xn = X[0]*X[3] + X[1]*X[4] + X[2]*X[5];
#endif
        
#if ( DIM == 2 )
        mtJJt[jj] = 2.0 * ( X[0]*X[0] + X[1]*X[1] );
#else
        mtJJt[jj] = 2.0 * ( X[0]*X[0] + X[1]*X[1] + X[2]*X[2] );
#endif
        // the diagonal term should be nearly equal to 2, since dif[] vectors are normalized
        //mtJJt[jj]  = 2.0;
        mtJJtU[jj] = -xn;
    }
    
    const real* X = dif + DIM*nbu;
#if ( DIM == 2 )
    mtJJt[nbu] = 2.0 * ( X[0]*X[0] + X[1]*X[1] );
#else
    mtJJt[nbu] = 2.0 * ( X[0]*X[0] + X[1]*X[1] + X[2]*X[2] );
#endif
    // the diagonal term should be nearly equal to 2, since dif[] vectors are normalized
    //mtJJt[nbu] = 2.0;

    int info = 0;
    lapack::xpttrf(nbu+1, mtJJt, mtJJtU, &info);

    if ( 0 )
    {
        std::clog << "D "; VecPrint::print(std::clog, nbu+1, mtJJt, 3);
        std::clog << "E "; VecPrint::print(std::clog, nbu, mtJJtU, 3);
        //std::clog << "X="; VecPrint::print(std::clog, DIM*(nbu+2), pPos);
    }

    if ( info )
    {
        std::clog << "Mecafil::makeProjection failed (" << info << ")\n";
        throw Exception("could not build Fiber's projection matrix");
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Perform first calculation needed by projectForces:
 tmp <- J * X
 */
void projectForcesU_(unsigned nbs, const real* dif, const real* vec, real* mul)
{
    #pragma vector unaligned
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        const real * X = vec + DIM * jj;
        const real * d = dif + DIM * jj;
        mul[jj] = d[0] * ( X[DIM  ] - X[0] )
                + d[1] * ( X[DIM+1] - X[1] )
#if ( DIM > 2 )
                + d[2] * ( X[DIM+2] - X[2] )
#endif
        ;
    }
}

/**
 Perform second calculation needed by projectForces:
 Y <- s * ( X + Jt * tmp )
 */
void projectForcesD_(unsigned nbs, const real* dif, const real* X, const real* mul, real* Y)
{
    for ( unsigned d = 0, e = DIM*nbs; d < DIM; ++d, ++e )
    {
        Y[d] = X[d] + dif[d    ] * mul[    0];
        Y[e] = X[e] - dif[e-DIM] * mul[nbs-1];
    }
    
    for ( unsigned jj = 1; jj < nbs; ++jj )
    {
        const unsigned kk = DIM*jj;
        Y[kk  ] = X[kk  ] + dif[kk  ] * mul[jj] - dif[kk-DIM  ] * mul[jj-1];
        Y[kk+1] = X[kk+1] + dif[kk+1] * mul[jj] - dif[kk-DIM+1] * mul[jj-1];
#if ( DIM > 2 )
        Y[kk+2] = X[kk+2] + dif[kk+2] * mul[jj] - dif[kk-DIM+2] * mul[jj-1];
#endif
    }
}


/**
 Perform second calculation needed by projectForces:
 */
void projectForcesD__(unsigned nbs, const real* dif,
                      const real* X, const real* mul, real* Y)
{
    real a0 = dif[0] * mul[0];
    real a1 = dif[1] * mul[0];
#if ( DIM > 2 )
    real a2 = dif[2] * mul[0];
#endif
    
    Y[0] = X[0] + a0;
    Y[1] = X[1] + a1;
#if ( DIM > 2 )
    Y[2] = X[2] + a2;
#endif
    
    for ( unsigned jj = 1; jj < nbs; ++jj )
    {
        const unsigned kk = DIM * jj;
        real b0 = dif[kk  ] * mul[jj];
        Y[kk  ] = X[kk  ] + b0 - a0;
        a0 = b0;
        
        real b1 = dif[kk+1] * mul[jj];
        Y[kk+1] = X[kk+1] + b1 - a1;
        a1 = b1;
        
#if ( DIM > 2 )
        real b2 = dif[kk+2] * mul[jj];
        Y[kk+2] = X[kk+2] + b2 - a2;
        a2 = b2;
#endif
    }
    
    const unsigned ee = DIM * nbs;
    Y[ee  ] = X[ee  ] - a0;
    Y[ee+1] = X[ee+1] - a1;
#if ( DIM > 2 )
    Y[ee+2] = X[ee+2] - a2;
#endif
}


#if ( DIM == 2 ) && defined(__SSE3__) && REAL_IS_DOUBLE

#include "simd.h"

/**
 Perform first calculation needed by projectForces:
 */
inline void projectForcesU_SSE(unsigned nbs, const real* dif, const real* X, real* mul)
{
    const real* pD = dif;
    const real* pX = X;
    real const*const end = mul + nbs - 2;
    real* pT = mul;
    
    vec2 y, x = load2(pX);
    while ( pT <= end )
    {
        y = load2(pX+2);
        pX += 4;
        vec2 a = mul2(sub2(y, x), load2(pD));
        x = load2(pX);
        vec2 b = mul2(sub2(x, y), load2(pD+2));
        pD += 4;
        storeu2(pT, hadd2(a, b));
        pT += 2;
    }
    
    if ( pT < end+2 )
    {
        y = load2(pX+2);
        vec2 a = mul2(sub2(y, x), load2(pD));
        storelo(pT, hadd2(a, a));
    }
}

/**
 Perform second calculation needed by projectForces:
 */
inline void projectForcesD_SSE(unsigned nbs, const real* dif,
                               const real* X, const real* mul, real* Y)
{
    real *pY = Y;
    const real* pX = X;
    const real* pD = dif;
    
    vec2 cc = load2(X);
    
    real const* pM = mul;
    real const*const end = mul + nbs;
    while ( pM < end )
    {
        pX += DIM;
        vec2 d = mul2(load2(pD), loaddup2(pM));
        ++pM;
        pD += DIM;
        store2(pY, add2(cc, d));
        pY += DIM;
        cc = sub2(load2(pX), d);
    }
    store2(pY, cc);
}

#endif

#if ( DIM == 2 ) && defined(__AVX__) && REAL_IS_DOUBLE

#include "simd.h"

/**
 Perform first calculation needed by projectForces:
 tmp <- J * X
 F. Nedelec, 9.12.2016, 6.9.2018
 */
inline void projectForcesU_AVX(unsigned nbs, const real* dif, const real* X, real* mul)
{
    const real* pD = dif;
    const real* pX = X;
    real const*const end = mul + nbs - 4;
    real* pT = mul;

    while ( pT <= end )
    {
        vec4 a = mul4(sub4(loadu4(pX+2), loadu4(pX  )), load4(pD  ));
        vec4 b = mul4(sub4(loadu4(pX+6), loadu4(pX+4)), load4(pD+4));
        pD += 8;
        pX += 8;
        //store4(pT, hadd4(permute2f128(a,b,0x20), permute2f128(a,b,0x31)));
        vec4 p = permute2f128(a,b,0x20), q = permute2f128(a,b,0x31);
        store4(pT, add4(unpacklo4(p, q), unpackhi4(p, q)));
        pT += 4;
    }
    
    while ( pT <= end+2 )
    {
        vec4 d = mul4(sub4(loadu4(pX+2), loadu4(pX)), load4(pD));
        pX += 4;
        pD += 4;
        vec2 h = gethi(d);
        storeu2(pT, add2(unpacklo2(getlo(d),h), unpackhi2(getlo(d),h)));
        pT += 2;
    }
    
    if ( pT < end+4 )
    {
        vec2 a = mul2(sub2(load2(pX+2), load2(pX)), load2(pD));
        storelo(pT, hadd2(a, a));
    }
}


/**
 Perform second calculation needed by projectForces:
 Y <- s * ( X + Jt * tmp )
 ATTENTION: memory X and Y are not necessarily aligned since they are chunck from
 an array containing contiguous coordinates
 F. Nedelec, 9.12.2016, 23.03.2018
 */
inline void projectForcesD_AVX(unsigned nbs, const real* dif,
                               const real* X, const real* mul, real* Y)
{
    real *pY = Y;
    const real* pX = X;
    const real* pD = dif;
    
    vec4 cc = setzero4();
    
    const bool odd = nbs & 1;
    real const* pM = mul;
    real const*const end = mul + nbs - odd;
    
    while ( pM < end )
    {
        vec4 t = broadcast2(pM);
        vec4 x = loadu4(pX);
        pM += 2;
        vec4 m = permute4(t, 0b1100);
        vec4 d = mul4(m, load4(pD));
        pD += 4;
        vec4 n = permute2f128(cc,d,0x21);
        cc = d;
        vec4 z = add4(x, sub4(d, n));
        pX += 4;
        storeu4(pY, z);
        pY += 4;
    }
    
    vec2 c = gethi(cc);
    
    if ( odd )
    {
        assert( pM + 1 == mul + nbs );
        vec2 m = loaddup2(pM);
        vec2 x = mul2(m, load2(pD));
        vec2 z = add2(load2(pX), sub2(x, c));
        storeu2(pY, z);
        c = x;
        pY += 2;
        pX += 2;
    }
    
    vec2 z = sub2(load2(pX), c);
    storeu2(pY, z);
    assert( pY == Y + DIM * nbs );
    assert( pX == X + DIM * nbs );
}

#endif


/**
 Perform first calculation needed by projectForces:
 */
inline void projectForcesU_PTR(unsigned nbs, const real* dif, const real* X, real* mul)
{
    const real * pX = X + DIM;
    const real * pM = dif;
    real x3, x0 = X[0];
    real x4, x1 = X[1];
#if ( DIM >= 3 )
    real x5, x2 = X[2];
#endif
    real *const end = mul + nbs;
    
    //normally optimized version
    for ( real* pT = mul; pT < end; ++pT )
    {
        x3 = pX[0];
        x4 = pX[1];
#if ( DIM == 2 )
        pT[0] = pM[0] * (x3 - x0) + pM[1] * (x4 - x1);
#elif ( DIM >= 3 )
        x5 = pX[2];
        pT[0] = pM[0] * (x3 - x0) + pM[1] * (x4 - x1) + pM[2] * (x5 - x2);
        x2 = x5;
#endif
        pX += DIM;
        pM += DIM;
        x0 = x3;
        x1 = x4;
    }
}


/**
 Perform first calculation needed by projectForces:
 */
inline void projectForcesU_PTR2(unsigned nbs, const real* dif, const real* X, real* mul)
{
    const real * pX = X + DIM;
    const real * pM = dif;
    real x3, x0 = X[0];
    real x4, x1 = X[1];
#if ( DIM >= 3 )
    real x5, x2 = X[2];
#endif
    real *const end = mul + nbs;
    
    //further optimization with manual loop-unrolling
    real* pT = mul;
    if ( nbs & 1 )
    {
        x3 = pX[0];
        x4 = pX[1];
#if ( DIM == 2 )
        pT[0] = pM[0] * (x3 - x0) + pM[1] * (x4 - x1);
#elif ( DIM >= 3 )
        x5 = pX[2];
        pT[0] = pM[0] * (x3 - x0) + pM[1] * (x4 - x1) + pM[2] * (x5 - x2);
        x2 = x5;
#endif
        ++pT;
        pX += DIM;
        pM += DIM;
        x0 = x3;
        x1 = x4;
    }
    
    while ( pT < end )
    {
        x3 = pX[0];
        x4 = pX[1];
#if ( DIM == 2 )
        pT[0] = pM[0] * (x3 - x0) + pM[1] * (x4 - x1);
#elif ( DIM >= 3 )
        x5 = pX[2];
        pT[0] = pM[0] * (x3 - x0) + pM[1] * (x4 - x1) + pM[2] * (x5 - x2);
#endif
        
#if ( DIM == 2 )
        x0 = pX[2];
        x1 = pX[3];
        pT[1] = pM[2] * (x0 - x3) + pM[3] * (x1 - x4);
#elif ( DIM >= 3 )
        x0 = pX[3];
        x1 = pX[4];
        x2 = pX[5];
        pT[1] = pM[3] * (x0 - x3) + pM[4] * (x1 - x4) + pM[5] * (x2 - x5);
#endif
        
        pT += 2;
        pX += 2*DIM;
        pM += 2*DIM;
    }
    assert_true( pT == end );
}

/**
 Perform second calculation needed by projectForces:
 */
void projectForcesD_PTR(unsigned nbs, const real* dif,
                        const real* X, const real* mul, real* Y)
{
    // Y <- X + Jt * tmp :
    real x0 = X[0];
    real x1 = X[1];
#if ( DIM > 2 )
    real x2 = X[2];
#endif
    
    const real* pX = X+DIM;
    const real* pM = dif;
    real *pY = Y;
    real const*const end = mul+nbs;
    for ( real const* pT = mul; pT < end; ++pT )
    {
        real y0 = *pT * pM[0];
        real y1 = *pT * pM[1];
#if ( DIM > 2 )
        real y2 = *pT * pM[2];
#endif
        pM  += DIM;
        pY[0]  = x0 + y0;
        pY[1]  = x1 + y1;
#if ( DIM > 2 )
        pY[2]  = x2 + y2;
#endif
        pY  += DIM;
        x0     = pX[0] - y0;
        x1     = pX[1] - y1;
#if ( DIM > 2 )
        x2     = pX[2] - y2;
#endif
        pX  += DIM;
    }
    pY[0] = x0;
    pY[1] = x1;
#if ( DIM > 2 )
    pY[2] = x2;
#endif
}


//------------------------------------------------------------------------------
#pragma mark -


#if ( DIM == 2 ) && REAL_IS_DOUBLE
#  if defined(__AVX__)
#    warning "Using AVX implementation"
#    define projectForcesU projectForcesU_AVX
#    define projectForcesD projectForcesD_AVX
#  elif defined(__SSE3__)
#    warning "Using SSE3 implementation"
#    define projectForcesU projectForcesU_SSE
#    define projectForcesD projectForcesD_SSE
#  else
#    define projectForcesU projectForcesU_
#    define projectForcesD projectForcesD_
#  endif
#else
#  define projectForcesU projectForcesU_
#  define projectForcesD projectForcesD_
#endif


void Mecafil::projectForces(const real* X, real* Y) const
{
    const unsigned nbs = nbSegments();
    //printf("X  "); VecPrint::print(std::clog, DIM*nbPoints(), X);

    // calculate `lag` without modifying `X`
    projectForcesU(nbs, rfDiff, X, rfLLG);

    // rfLLG <- inv( J * Jt ) * rfLLG to find the Lagrange multipliers
    lapack::xptts2(nbs, 1, mtJJt, mtJJtU, rfLLG, nbs);
    
    // set Y, using values in X and rfLLG
    projectForcesD(nbs, rfDiff, X, rfLLG, Y);

    //printf("Y  "); VecPrint::print(std::clog, DIM*nbPoints(), Y);
}


void Mecafil::computeTensions(const real* force)
{
    const unsigned nbs = nbSegments();
    
    projectForcesU(nbs, rfDiff, force, rfLag);
    
    // tmp <- inv( J * Jt ) * tmp to find the multipliers
    lapack::xptts2(nbs, 1, mtJJt, mtJJtU, rfLag, nbs);
}


void Mecafil::printProjection(std::ostream& os) const
{
    const unsigned nbv = DIM * nbPoints();
    real * res = new_real(nbv*nbv);
    real * src = new_real(nbv);
    real * dst = new_real(nbv);
    zero_real(nbv, src);
    zero_real(nbv, dst);
    for ( unsigned i = 0; i < nbv; ++i )
    {
        src[i] = 1.0;
        projectForces(src, dst);
        copy_real(nbv, dst, res+nbv*i);
        src[i] = 0.0;
    }
    free_real(dst);
    free_real(src);
    os << "Mecafil:Projection  " << reference() << '\n';
    VecPrint::print(os, nbv, nbv, res, nbv);
    free_real(res);
}



//------------------------------------------------------------------------------
#pragma mark - Projection DIFF
//#include "cytoblas.h"

void Mecafil::makeProjectionDiff(const real* force)
{
    const unsigned nbs = nbSegments();
    assert_true( nbs > 0 );
    
#if 0
    // Check here that iLLG[] contains the correct Lagrange multipliers
    // compute Lagrange multipliers corresponding to 'force' in iLag:
    computeTensions(force);
    real n = blas::max_diff(nbs, rfLLG, rfLag);
    if ( n > 1e-6 )
    {
        fprintf(stderr, "\n|iLag - iLLG| = %e", n);
        fprintf(stderr, "\niLag "); VecPrint::print(std::clog, std::min(20LU,nbs), rfLag);
        fprintf(stderr, "\niLLG "); VecPrint::print(std::clog, std::min(20LU,nbs), rfLLG);
    }
#endif
    
    // use Lagrange multipliers computed from the last projectForces() in iLLG

    // remove compressive forces ( negative Lagrange-multipliers )
    useProjectionDiff = false;
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        if ( rfLLG[jj] > 0 )
        {
            useProjectionDiff = true;
            break;
        }
    }
    
    if ( useProjectionDiff )
    {
        const real th = 0.0;
        const real sc = 1.0 / segmentation();
        #pragma vector unaligned
        for ( size_t jj = 0; jj < nbs; ++jj )
            mtJJtiJforce[jj] = std::max(th, rfLLG[jj] * sc);
        
        //std::clog << "projectionDiff: " << blas::nrm2(nbs, mtJJtiJforce) << std::endl;
        //std::clog << "projectionDiff:"; VecPrint::print(std::clog, std::min(20u,nbs), mtJJtiJforce);
    }
}


//------------------------------------------------------------------------------

///straightforward implementation:
inline void add_projectiondiff(const unsigned nbs, const real* mul, const real* X, real* Y)
{
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        for ( unsigned d = 0; d < DIM; ++d )
        {
            const real w = mul[jj] * ( X[DIM*jj+DIM+d] - X[DIM*jj+d] );
            Y[DIM*jj    +d] += w;
            Y[DIM*jj+DIM+d] -= w;
        }
    }
}

///expanded implementation:
inline void add_projectiondiffR(const unsigned nbs, const real* mul, const real* X, real* Y)
{
    // this loop cannot be unrolled as there is an OUTPUT dependency in Y
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        const real x = mul[jj] * ( X[DIM*jj+DIM  ] - X[DIM*jj  ] );
        Y[DIM*jj      ] += x;
        Y[DIM*jj+DIM  ] -= x;
#if ( DIM > 1 )
        const real y = mul[jj] * ( X[DIM*jj+DIM+1] - X[DIM*jj+1] );
        Y[DIM*jj    +1] += y;
        Y[DIM*jj+DIM+1] -= y;
#endif
#if ( DIM > 2 )
        const real z = mul[jj] * ( X[DIM*jj+DIM+2] - X[DIM*jj+2] );
        Y[DIM*jj    +2] += z;
        Y[DIM*jj+DIM+2] -= z;
#endif
    }
}


///scalar implementation
inline void add_projectiondiffF(const unsigned nbs, const real* mul, const real* X, real* Y)
{
    real px0 = X[0];
    real px1 = X[1];
    real pw0 = 0;
    real pw1 = 0;
#if ( DIM >= 3 )
    real px2 = X[2];
    real pw2 = 0;
#endif
    
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        const real m = mul[jj];
        real x0 = X[DIM*jj+DIM  ];
        real x1 = X[DIM*jj+DIM+1];
#if ( DIM >= 3 )
        real x2 = X[DIM*jj+DIM+2];
#endif
        real w0 = m * ( x0 - px0 );
        real w1 = m * ( x1 - px1 );
#if ( DIM >= 3 )
        real w2 = m * ( x2 - px2 );
#endif
        px0 = x0;
        px1 = x1;
#if ( DIM >= 3 )
        px2 = x2;
#endif
        Y[DIM*jj  ] += w0 - pw0;
        Y[DIM*jj+1] += w1 - pw1;
#if ( DIM >= 3 )
        Y[DIM*jj+2] += w2 - pw2;
#endif
        pw0 = w0;
        pw1 = w1;
#if ( DIM >= 3 )
        pw2 = w2;
#endif
    }
    Y[DIM*nbs  ] -= pw0;
    Y[DIM*nbs+1] -= pw1;
#if ( DIM >= 3 )
    Y[DIM*nbs+2] -= pw2;
#endif
}

#if ( DIM == 2 ) && defined(__SSE3__) && REAL_IS_DOUBLE

#include "simd.h"

inline void add_projectiondiffSSE(const unsigned nbs, const real* mul, const real* X, real* Y)
{
    vec2 px = load2(X);
    vec2 pw = setzero2();
    
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        vec2 m = loaddup2(mul+jj);
        vec2 x = load2(X+DIM*jj+DIM);
        vec2 y = load2(Y+DIM*jj);
        vec2 w = mul2(m, sub2(x, px));
        px = x;
        store2(Y+DIM*jj, add2(y, sub2(w, pw)));
        pw = w;
    }
    store2(Y+DIM*nbs, sub2(load2(Y+DIM*nbs), pw));
}

#endif

#if ( DIM == 2 ) && defined(__AVX__) && REAL_IS_DOUBLE

#include "simd.h"

inline void add_projectiondiffAVX(const unsigned nbs, const real* mul, const real* X, real* Y)
{
    real * pY = Y;
    real const* pX = X;
    real const* pM = mul;
    
    if ( nbs & 1 )
    {
        vec2 m = loaddup2(pM);
        ++pM;
        vec2 s = mul2(sub2(load2(pX+DIM), load2(pX)), m);
        pX += DIM;
        storeu2(pY    , add2(load2(pY    ), s));
        storeu2(pY+DIM, sub2(load2(pY+DIM), s));
        pY += DIM;
    }
    
    real const*const end = mul + nbs;
    while ( pM < end )
    {
        vec4 a = broadcast2(pM);
        vec4 m = permute4(a, 0b1100);

        pM += DIM;
        vec4 s = mul4(m, sub4(loadu4(pX+2), loadu4(pX)));
        pX += 2*DIM;
        
        // this will not be fast, since the two vector are not independent:
        storeu4(pY  , add4(loadu4(pY  ), s));
        storeu4(pY+2, sub4(loadu4(pY+2), s));
        pY += 2*DIM;
    }
    assert_true(pM==end);
}

#endif


void Mecafil::addProjectionDiff(const real* X, real* Y) const
{
    assert_true(useProjectionDiff);
#if ( 0 )
    // debug code to compare with default implementation
    unsigned nbp = nbPoints()*DIM;
    real * vec = new_real(nbp);
    copy_real(nbp, Y, vec);
    add_projectiondiff(nbSegments(), mtJJtiJforce, X, vec);
#endif

#if ( DIM == 2 ) && defined(__SSE3__) && REAL_IS_DOUBLE
    add_projectiondiffSSE(nbSegments(), mtJJtiJforce, X, Y);
    //add_projectiondiff(nbSegments(), mtJJtiJforce, X, Y);
    //add_projectiondiffAVX(nbSegments(), mtJJtiJforce, X, Y);
#else
    add_projectiondiffF(nbSegments(), mtJJtiJforce, X, Y);
#endif
    
    
#if ( 0 )
    // debug code to compare with default implementation
    real n = blas::max_diff(nbp, Y, vec);
    if ( n > 1e-6 )
    {
        std::clog << "proj_diff error " << n << " (" << nbp << ")\n";
        VecPrint::print(std::clog, std::min(20u,nbp), vec);
        VecPrint::print(std::clog, std::min(20u,nbp), Y);
    }
    free_real(vec);
#endif
}


