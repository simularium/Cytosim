// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <sys/time.h>

#define DIM 3

#include "real.h"
#include "timer.h"
#include "random.h"
#include "vecprint.h"
#include "cblas.h"
#include "simd.h"


const real scalar = 2.0;

/// number of segments:
const size_t NBS = 129;
const size_t NBR = DIM * ( NBS + 1 );
const size_t ALOC = NBR + 8;



#ifdef __AVX__

inline __m256d loadc4(double const* ptr)
{
#if ( 0 )
    // we can check memory alignment here:
    uintptr_t x = ((uintptr_t)ptr) & 31;
    if ( x )
    {
        fprintf(stderr, "loading unaligned memory %i\n", x);
        return _mm256_loadu_pd(ptr);
    }
#endif
    return _mm256_load_pd(ptr);
}

#endif


inline void print_alignment(real const* ptr, const char msg[])
{
    fprintf(stderr, "%s %p alignment %lu\n", msg, ptr, (uintptr_t)ptr&31);
}

//------------------------------------------------------------------------------


real *diff = nullptr, *pos = nullptr, *lagmul = nullptr, *force = nullptr;


void setFilament(int np, real * vec, real seg, real persistence_length)
{
    real sigma = sqrt(2.0*seg/persistence_length);
    
    real pX = 0, pY = 0;
    real a = RNG.sreal() * M_PI;
   
    vec[0] = pX;
    vec[1] = pY;

    for ( int p = 1 ; p < np; ++p )
    {
        real dX = seg * cos(a);
        real dY = seg * sin(a);
        diff[2*p-2] = dX;
        diff[2*p-1] = dY;
        pX += dX;
        pY += dY;
        vec[2*p  ] = pX;
        vec[2*p+1] = pY;

        a += sigma * RNG.gauss();
    }
}

void setRandom(int np, real * vec, real mag)
{
    for ( size_t p = 0 ; p < ALOC; ++p )
        vec[p] = mag * RNG.sreal();
}

void new_reals(real*& x, real*& y, real*& z, real mag)
{
    x = new_real(ALOC);
    y = new_real(ALOC);
    z = new_real(ALOC);
    
    for ( unsigned ii=0; ii<ALOC; ++ii )
    {
        x[ii] = mag * RNG.sreal();
        y[ii] = mag * RNG.sreal();
        z[ii] = mag * RNG.sreal();
    }
}

void free_real(real* x, real* y, real* z)
{
    free_real(x);
    free_real(y);
    free_real(z);
}


//------------------------------------------------------------------------------
#pragma mark - RIGIDITY

/*
 This is the simple implementation
 */
void add_rigidity0(const unsigned nbt, const real* X, const real rigid, real* Y)
{
#pragma vector unaligned
    for ( unsigned jj = 0; jj < nbt; ++jj )
    {
        real f = rigid * (( X[jj+DIM*2] - X[jj+DIM] ) - ( X[jj+DIM] - X[jj] ));
        Y[jj      ] -=   f;
        Y[jj+DIM  ] += 2*f;
        Y[jj+DIM*2] -=   f;
    }
}

/*
 This an implementation for 2D
 */
void add_rigidity2(const unsigned nbt, const real* vec, const real rigid, real* Y)
{
    real fx = 0;
    real fy = 0;
#pragma vector unaligned
    for ( unsigned jj = 0; jj < nbt; jj += 2 )
    {
        real const* X = vec + jj;
        real gx = rigid * ( X[4] - X[2] - X[2] + X[0] );
        real gy = rigid * ( X[5] - X[3] - X[3] + X[1] );
        Y[jj  ] += fx-gx;
        Y[jj+1] += fy-gy;
        Y[jj+2] -= fx-gx;
        Y[jj+3] -= fy-gy;
        fx = gx;
        fy = gy;
    }
    Y[nbt  ] += fx;
    Y[nbt+1] += fy;
    Y[nbt+2] -= fx;
    Y[nbt+3] -= fy;
}

/*
 In this version for 2D or 3D, the loop is unrolled, pointers are used,
 and ( a0 -2*a1 + a2 ) is replaced by (a2-a1)-(a1-a0).
 */
void add_rigidity3(const unsigned nbt, const real* X, const real rigid, real* Y)
{
    const real * xn = X + DIM;
    
    real x0 = xn[0];
    real x1 = xn[1];
#if ( DIM == 3 )
    real x2 = xn[2];
#endif
    
    real d0 = x0 - X[0];
    real d1 = x1 - X[1];
#if ( DIM == 3 )
    real d2 = x2 - X[2];
#endif
    
    real df0, of0 = 0, odf0 = 0;
    real df1, of1 = 0, odf1 = 0;
#if ( DIM == 3 )
    real df2, of2 = 0, odf2 = 0;
#endif
    
    xn += DIM;
    
    real * yp = Y;
    real *const end = Y + nbt;
    while ( yp < end )
    {
        real e0 = *xn - x0;
        x0 = *xn;
        ++xn;
        real f0 = rigid * ( e0 - d0 );
        d0      = e0;
        df0     = f0 - of0;
        of0     = f0;
        *yp    += odf0 - df0;
        odf0    = df0;
        ++yp;
        
        real e1 = *xn - x1;
        x1 = *xn;
        ++xn;
        real f1 = rigid * ( e1 - d1 );
        d1      = e1;
        df1     = f1 - of1;
        of1     = f1;
        *yp    += odf1 - df1;
        odf1    = df1;
        ++yp;
        
#if ( DIM == 3 )
        real e2 = *xn - x2;
        x2 = *xn;
        ++xn;
        real f2 = rigid * ( e2 - d2 );
        d2      = e2;
        df2     = f2 - of2;
        of2     = f2;
        *yp    += odf2 - df2;
        odf2    = df2;
        ++yp;
#endif
    }
    
    yp[0]   += df0 + of0;
    yp[1]   += df1 + of1;
#if ( DIM == 3 )
    yp[2]   += df2 + of2;
#endif
    
    yp += DIM;
    
    yp[0] -= of0;
    yp[1] -= of1;
#if ( DIM == 3 )
    yp[2] -= of2;
#endif
}

#ifdef __SSE3__

void add_rigidity_SSE(const unsigned nbt, const real* X, const real rigid, real* Y)
{
    vec2 R = set2(rigid);
    real *const end = Y + nbt;

    vec2 nn = load2(X+2);
    vec2 oo = mul2(R, sub2(nn, load2(X)));
    vec2 yy = load2(Y);
    vec2 zz = load2(Y+2);
    
    while ( Y < end )
    {
        vec2 mm = load2(X+4);
        X += 2;
        vec2 dd = mul2(R, sub2(mm, nn));
        vec2 ff = sub2(dd, oo);
        oo = dd;
        nn = mm;
        store2(Y, sub2(yy, ff));
        yy = add2(zz, add2(ff, ff));
        zz = sub2(load2(Y+4), ff);
        Y += 2;
    }
    store2(Y, yy);
    store2(Y+2, zz);
}

/// older implementation
void add_rigidity_SSO(const unsigned nbt, const real* X, const real rigid, real* Y)
{
    vec2 R = set2(rigid);
    real *const end = Y + nbt;

    vec2 xx  = load2(X+DIM);
    vec2 d   = sub2(xx, load2(X));
    vec2 df  = setzero2();
    vec2 of  = setzero2();
    vec2 yy  = load2(Y);
    
    X += 2*DIM;
    while ( Y < end )
    {
        vec2 nn = load2(X);
        X += DIM;
        vec2 e = sub2(nn, xx);
        xx = nn;
        
        vec2 f = mul2(R, sub2(e, d));
        d  = e;
        df = sub2(f, of);
        of = f;
        store2(Y, sub2(yy, df));
        yy = add2(load2(Y+DIM), df);
        Y += DIM;
    }
    store2(Y, add2(load2(Y), add2(df, of)));
    store2(Y+DIM, sub2(load2(Y+DIM), of));
}
#endif

#ifdef __AVX__

void add_rigidity_AVX(const unsigned nbt, const real* X, const real rigid, real* Y)
{
    vec4 R = set4(rigid);
    vec4 two = set4(2.0);
    
    real *const end = Y + nbt - 8;
    
    vec4 xxx = load4(X);
    vec4 eee = setzero4();
#if 1
    // unrolled 2x2
    while ( Y < end )
    {
        vec4 nnn = load4(X+4);
        vec4 iii = permute2f128(xxx, nnn, 0x21);
        vec4 ddd = sub4(sub4(nnn, iii), sub4(iii, xxx));
        xxx = load4(X+8);
        X += 8;
        vec4 ppp = permute2f128(eee, ddd, 0x21);
        vec4 jjj = permute2f128(nnn, xxx, 0x21);
        store4(Y, fmadd4(R, fmsub4(two, ppp, add4(eee, ddd)), load4(Y)));
        eee = sub4(sub4(xxx, jjj), sub4(jjj, nnn));
        ppp = permute2f128(ddd, eee, 0x21);
        store4(Y+4, fmadd4(R, fmsub4(two, ppp, add4(ddd, eee)), load4(Y+4)));
        Y += 8;
    }
#endif
#if 1
    if ( Y < end+4 )
    {
        vec4 nnn = load4(X+4);
        X += 4;
        vec4 iii = loadu4(X-2); //permute2f128(xxx, nnn, 0x21);
        vec4 ddd = sub4(sub4(nnn, iii), sub4(iii, xxx));
        xxx = nnn;
        vec4 ppp = permute2f128(eee, ddd, 0x21);
        store4(Y, fmadd4(R, fmsub4(two, ppp, add4(eee, ddd)), load4(Y)));
        eee = ddd;
        Y += 4;
    }
#endif
    vec2 nn = gethi(xxx);
    vec2 oo = sub2(nn, getlo(xxx));
    vec2 ee = gethi(eee);
    vec2 yy = fmsub2(getlo(two), ee, getlo(eee));
    while ( Y < end+8 )
    {
        vec2 mm = load2(X+4);
        X += 2;
        vec2 ff = sub2(mm, nn);
        vec2 dd = sub2(ff, oo);
        nn = mm;
        oo = ff;
        store2(Y, fmadd2(getlo(R), sub2(yy, dd), load2(Y)));
        yy = fmsub2(getlo(two), dd, ee);
        ee = dd;
        Y += 2;
    }
    store2(Y  ,  fmadd2(getlo(R), yy, load2(Y  )));
    store2(Y+2, fnmadd2(getlo(R), ee, load2(Y+2)));
}

#endif

void add_rigidityE(const unsigned nbt, const real* X, const real R1, real* Y)
{
    real const* E = X + nbt + DIM;  //index to last point
    
    const real R2 = R1 * 2;
    
    if ( nbt == DIM )
    {
        for ( int d = 0; d < DIM; ++d )
        {
            real x = 2 * X[d+DIM] - ( X[d+DIM*2] + X[d] );
            Y[d      ] += R1 * x;
            Y[d+DIM  ] -= R2 * x;
            Y[d+DIM*2] += R1 * x;
        }
    }
    else
    {
        const real R4 = R1 * 4;
        const real R6 = R1 * 6;
        
        // this is where the bulk of the calculation takes place:
        const int end = nbt;
        #pragma ivdep
        for ( int i = DIM*2; i < end; ++i )
            Y[i] += R4 * (X[i-DIM]+X[i+DIM]) - R1 * (X[i-DIM*2]+X[i+DIM*2]) - R6 * X[i];
        
        for ( int d = 0; d < DIM; ++d )
        {
            Y[    d+DIM] -= R1 * (X[d+DIM]+X[d+DIM*3]) - R4 * (X[d+DIM*2]-X[d+DIM]) - R2 * X[d];
            Y[nbt+d    ] -= R1 * (E[d-DIM]+E[d-DIM*3]) - R4 * (E[d-DIM*2]-E[d-DIM]) - R2 * E[d];
            Y[    d    ] -= R1 * (X[d+DIM*2]+X[d]) - R2 * X[d+DIM];
            Y[nbt+d+DIM] -= R1 * (E[d-DIM*2]+E[d]) - R2 * E[d-DIM];
        }
    }
}


void add_rigidityF(const unsigned nbt, const real* X, const real R1, real* Y)
{
    const real R6 = R1 * 6;
    const real R4 = R1 * 4;
    const real R2 = R1 * 2;

    const int end = nbt;
    #pragma ivdep
    for ( int i = DIM*2; i < end; ++i )
        Y[i] += R4 * (X[i-DIM]+X[i+DIM]) - R1 * (X[i-DIM*2]+X[i+DIM*2]) - R6 * X[i];
    
    // special cases near the edges:
    real      * Z = Y + nbt + DIM;
    real const* E = X + nbt + DIM;
    #pragma ivdep
    for ( int d = 0; d < DIM; ++d )
    {
        Y[d    ] -= R1 * (X[d+DIM*2]+X[d]) - R2 * X[d+DIM];
        Y[d+DIM] -= R1 * (X[d+DIM]+X[d+DIM*3]) + R4 * (X[d+DIM]-X[d+DIM*2]) - R2 * X[d];
        Z[d-DIM] -= R1 * (E[d-DIM]+E[d-DIM*3]) + R4 * (E[d-DIM]-E[d-DIM*2]) - R2 * E[d];
        Z[d    ] -= R1 * (E[d-DIM*2]+E[d]) - R2 * E[d-DIM];
    }
}


inline void testRigidity(unsigned cnt, void (*func)(const unsigned, const real*, real, real*), char const* str)
{
    real * x = nullptr, * y = nullptr, * z = nullptr;
    new_reals(x, y, z, 1.0);
    
    unsigned nbt = DIM * ( NBS - 1 );
    tic();
    for ( unsigned int i=0; i<cnt; ++i )
    {
        func(nbt, y, scalar, x);
        func(nbt, x, scalar, z);
        func(nbt, z, scalar, y);
    }
    printf("  %10s %5.2f", str, toc(cnt*NBS));
    zero_real(ALOC, x);
    func(nbt, pos, scalar, x);
    VecPrint::print(std::cout, std::min(16ul,NBR), x);
    
    zero_real(ALOC, y);
    add_rigidity0(nbt, pos, scalar, y);
    real err = blas::max_diff(nbt+2*DIM, x, y);
    printf("  -> %e\n", err);
    
    free_real(x, y, z);
}


void testRigidity(unsigned cnt)
{
    std::cout << "addRigidity " << DIM << "D " << NBS << "\n";
    testRigidity(cnt, add_rigidity0,    "0  ");
#if ( DIM == 2 )
    testRigidity(cnt, add_rigidity2,    "2  ");
#endif
    testRigidity(cnt, add_rigidity3,    "3  ");
    testRigidity(cnt, add_rigidityE,    "E  ");
    testRigidity(cnt, add_rigidityF,    "F  ");
    testRigidity(cnt, add_rigidityE,    "E  ");
#if defined __SSE__ & ( DIM == 2 )
    testRigidity(cnt, add_rigidity_SSO, "SSO");
    testRigidity(cnt, add_rigidity_SSE, "SSE");
#endif
#if defined __AVX__ & ( DIM == 2 )
    testRigidity(cnt, add_rigidity_AVX, "AVX");
#endif
}


//------------------------------------------------------------------------------
#pragma mark - PROJECT UP

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


#ifdef __SSE3__

/**
 Perform first calculation needed by projectForces:
 tmp <- J * X
 */
inline void projectForcesU_SSE(unsigned nbs, const real* dif, const real* X, real* mul)
{
    const real* pM = dif;
    const real* pX = X;
    real const*const end = mul + nbs;
    real* pT = mul;

    vec2 y, x = load2(pX);
    while ( pT+2 <= end )
    {
        y = load2(pX+2);
        pX += 4;
        vec2 a = mul2(sub2(y, x), load2(pM));
        x = load2(pX);
        vec2 b = mul2(sub2(x, y), load2(pM+2));
        pM += 4;
        storeu2(pT, hadd2(a, b));
        //storeu2(pT, add2(unpacklo2(a, b), unpackhi2(a, b)));
        pT += 2;
    }
    
    if ( pT < end )
    {
        y = load2(pX+2);
        vec2 a = mul2(sub2(y, x), load2(pM));
        storelo(pT, hadd2(a, a));
    }
}

#endif

#ifdef __AVX__

__m256i make_mask(long i)
{
    vec4 v{0.5, 1.5, 2.5, 3.5};
    return _mm256_castpd_si256(cmp4(v, set4(i), _CMP_LT_OQ));
}

/**
 Perform first calculation needed by projectForces:
 tmp <- J * X
 F. Nedelec, 9.12.2016
 */
inline void projectForcesU_AVX(unsigned nbs, const real* dif, const real* X, real* mul)
{
    const real* pM = dif;
    const real* pX = X;
    real const*const end = mul + nbs - 4;
    real* pT = mul;
    
#if ( 0 )
    while ( pT+4 <= end )
    {
        vec4 a = mul4(sub4(loadu4(pX+2 ), loadu4(pX   )), loadc4(pM   ));
        vec4 b = mul4(sub4(loadu4(pX+6 ), loadu4(pX+4 )), loadc4(pM+4 ));
        vec4 c = mul4(sub4(loadu4(pX+10), loadu4(pX+8 )), loadc4(pM+8 ));
        vec4 d = mul4(sub4(loadu4(pX+14), loadu4(pX+12)), loadc4(pM+12));
        pX += 16;
        pM += 16;
        store4(pT  , hadd4(permute2f128(a,b,0x20), permute2f128(a,b,0x31)));
        store4(pT+4, hadd4(permute2f128(c,d,0x20), permute2f128(c,d,0x31)));
        pT += 8;
    }

    while ( pT <= end )
    {
        vec4 a = mul4(sub4(loadu4(pX+2), loadu4(pX  )), loadc4(pM  ));
        vec4 b = mul4(sub4(loadu4(pX+6), loadu4(pX+4)), loadc4(pM+4));
        pM += 8;
        pX += 8;
        //store4(pT, hadd4(permute2f128(a,b,0x20), permute2f128(a,b,0x31)));
        vec4 p = permute2f128(a,b,0x20), q = permute2f128(a,b,0x31);
        store4(pT, add4(unpacklo4(p, q), unpackhi4(p, q)));
        pT += 4;
    }
    #endif

    while ( pT <= end+2 )
    {
        //mul[jj] = dif[0] * ( X[DIM] - X[0] ) + dif[1] * ( X[DIM+1] - X[1] )
        vec4 d = mul4(sub4(loadu4(pX+2), loadu4(pX)), load4(pM));
        pX += 4;
        pM += 4;
        vec2 h = gethi(d);
        storeu2(pT, add2(unpacklo2(getlo(d),h), unpackhi2(getlo(d),h)));
        pT += 2;
    }
    
    if ( pT < end+4 )
    {
        vec2 a = mul2(sub2(load2(pX+2), load2(pX)), load2(pM));
        storelo(pT, add2(a, permute2(a,1)));
    }
}


/**
 Attetion: this does not check the boundaries and will write beyond the
 nbs-th point of tmp, which should be allocated accordingly.
 F. Nedelec, 11.01.2018
 */
inline void projectForcesU_AVY(unsigned nbs, const real* dif, const real* X, real* tmp)
{
    const real* pM = dif;
    const real* pX = X;
    
    real *pT = tmp;
    real *const end = tmp + nbs;
    
    // calculate the terms 8 by 8
    while ( pT < end )
    {
        vec4 a = mul4(sub4(loadu4(pX+2), loadu4(pX  )), loadc4(pM  ));
        vec4 b = mul4(sub4(loadu4(pX+6), loadu4(pX+4)), loadc4(pM+4));
        pM += 8;
        pX += 8;
        //store4(pT, hadd4(permute2f128(a,b,0x20), permute2f128(a,b,0x31)));
        vec4 p = permute2f128(a, b, 0x20);
        a = permute2f128(a, b, 0x31);
        store4(pT, add4(unpacklo4(p, a), unpackhi4(p, a)));
        pT += 4;
    }
}


#endif


//------------------------------------------------------------------------------
#pragma mark - PROJECT DOWN

/**
 Perform second calculation needed by projectForces:
 Y <- X + Jt * tmp
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


/**
 Perform second calculation needed by projectForces:
 */
void projectForcesD___(unsigned nbs, const real* dif, const real* X, const real* lag, real* Y)
{
    real a0 = X[0];
    real a1 = X[1];
#if ( DIM > 2 )
    real a2 = X[2];
#endif
    
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        const unsigned kk = DIM * jj;
        real b0 = dif[kk  ] * lag[jj];
        real b1 = dif[kk+1] * lag[jj];
#if ( DIM > 2 )
        real b2 = dif[kk+2] * lag[jj];
#endif

        Y[kk  ] = a0 + b0;
        Y[kk+1] = a1 + b1;
#if ( DIM > 2 )
        Y[kk+2] = a2 + b2;
#endif
        
        a0 = X[DIM+kk  ] - b0;
        a1 = X[DIM+kk+1] - b1;
#if ( DIM > 2 )
        a2 = X[DIM+kk+2] - b2;
#endif
    }
    
    const unsigned ee = DIM * nbs;
    Y[ee  ] = a0;
    Y[ee+1] = a1;
#if ( DIM > 2 )
    Y[ee+2] = a2;
#endif
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

#if defined __SSE3__ && ( DIM == 2 )

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

#if defined __AVX__ && ( DIM == 2 )


/**
 Perform second calculation needed by projectForces:
 Y <- X + Jt * tmp
 F. Nedelec, 9.12.2016
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


//------------------------------------------------------------------------------
#pragma mark - Test


inline void testU(unsigned cnt, void (*func)(unsigned, const real*, const real*, real*), char const* str)
{
    real *x = nullptr, *y = nullptr, *z = nullptr;
    new_reals(x, y, z, 1.0);

    tic();
    for ( unsigned ii=0; ii<cnt; ++ii )
    {
        func(NBS, diff, y, z);
        // check the code with unaligned memory:
        func(NBS, diff, x+2, y);
        func(NBS, diff, z+4, y);
    }
    printf("  %10s %5.2f", str, toc(cnt*NBS));

    zero_real(ALOC, lagmul);
    func(NBS, diff, force, lagmul);
    VecPrint::print(std::cout, std::min(20UL,NBS+1), lagmul) << std::endl;
    
    free_real(x,y,z);
}


inline void testD(unsigned cnt, void (*func)(unsigned, const real*, const real*, const real*, real*), char const* str)
{
    real *x = nullptr, *y = nullptr, *z = nullptr;
    new_reals(x, y, z, 1.0);

    tic();
    for ( unsigned ii=0; ii<cnt; ++ii )
    {
        func(NBS, diff, x, y, z);
        // check the code with unaligned memory:
        func(NBS, diff, y+2, z, x+2);
        func(NBS, diff, z+4, x, y+4);
    }
    printf("  %10s %5.2f", str, toc(cnt*NBS));

    zero_real(ALOC, x);
    func(NBS, diff, pos, lagmul, x);
    VecPrint::print(std::cout, std::min(20UL,NBR+2), x) << std::endl;
    
    free_real(x,y,z);
}


void testProjectionU(unsigned cnt)
{
    std::cout << "ProjectForces\n";
    testU(cnt, projectForcesU_,    "U_   ");
#if defined __SSE__ & ( DIM == 2 )
    testU(cnt, projectForcesU_SSE, "U_SSE");
#endif
#if defined __AVX__ && ( DIM == 2 )
    testU(cnt, projectForcesU_AVX, "U_AVX");
    testU(cnt, projectForcesU_AVY, "U_AVY");
#endif
}

void testProjectionD(unsigned cnt)
{
    std::cout << "ProjectForces\n";
    testD(cnt, projectForcesD_,    "D_   ");
    testD(cnt, projectForcesD__,   "D__  ");
    testD(cnt, projectForcesD___,  "D___ ");
    //testD(cnt, projectForcesD_PTR, "D_PTR");
#if defined __SSE__ & ( DIM == 2 )
    testD(cnt, projectForcesD_SSE, "D_SSE");
#endif
#if defined __AVX__ & ( DIM == 2 )
    testD(cnt, projectForcesD_AVX, "D_AVX");
#endif
}


int main(int argc, char* argv[])
{
    //re-seed the random number generator:
    RNG.seed();

    pos = new_real(ALOC);
    new_reals(force, lagmul, diff, 0.0);
    
    setFilament(NBS+1, pos, 1.0, 2.0);
    setRandom(NBS+1, force, 1.0);

    //testRigidity(1<<18);
    testProjectionU(1<<20);
    testProjectionD(1<<20);

    free_real(pos);
    free_real(diff, lagmul, force);
    
    return EXIT_SUCCESS;
}
