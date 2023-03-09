// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "smath.h"
#include "assert_macro.h"
#include "chain.h"
#include "iowrapper.h"
#include "messages.h"
#include "mecapoint.h"
#include "interpolation.h"
#include "fiber_site.h"
#include "exceptions.h"
#include "glossary.h"
#include "clapack.h"
#include "modulo.h"
#include "vector3.h"
#include "simul.h"
#include "vecprint.h"


extern Modulo const* modulo;

/**
 This returns N+1, where N is the integer that minimizes
     fabs( length / N - segmentation ),
 */
unsigned Chain::bestNumberOfPoints(const real ratio)
{
    unsigned n = (int)ratio;
    
    if ( (2*n+1)*ratio >= 2*n*(n+1) )
        return n+2;
    
    return n+1;
}


real Chain::contourLength(const real* pts, unsigned n_pts)
{
    real len = 0;
    Vector a(pts), b;
    for ( unsigned n = 1; n < n_pts; ++n )
    {
        b.load(pts+DIM*n);
        len += (b-a).norm();
        a = b;
    }
    return len;
}


Chain::Chain()
{
    fnCut          = 0;
    fnSegmentation = 0;
    fnAbscissaM    = 0;
    fnAbscissaP    = 0;
#if FIBER_HAS_NORMAL
    fnNormal.set(0, 0, 1);
#endif
    fnBirthTime    = 0;
    needUpdate     = false;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
This does not change length or segmentation
*/
void Chain::setStraight(Vector const& pos, Vector const& dir)
{
    assert_true( dir.norm() > 0.1 );
    // 'dir' is normalized for safety:
    Vector dpts = dir * ( fnCut / dir.norm() );
    //
    for ( unsigned p = 0 ; p < nPoints; ++p )
        setPoint(p, pos + p * dpts);
}


void Chain::setStraight(Vector const& pos, Vector const& dir, real len)
{
    assert_true( fnSegmentation > REAL_EPSILON );

    if ( len <= 0 )
        throw InvalidParameter("fiber:length must be > 0");

    unsigned np = bestNumberOfPoints(len/fnSegmentation);
    
    setNbPoints(np);
    setSegmentation(len/(np-1));
    fnAbscissaP = fnAbscissaM + len;
    setStraight(pos, dir);
    updateFiber();
}


void Chain::placeEnd(const FiberEnd ref)
{
    switch( ref )
    {
        case NO_END:
            flipChainPolarity();
            break;
        
        case MINUS_END:
            translate(posMiddle()-posEndM());
            break;
            
        case PLUS_END:
            translate(posMiddle()-posEndM());
            flipChainPolarity();
            break;
            
        case CENTER:
            break;
            
        default:
            ABORT_NOW("invalid argument to Chain::placeEnd()");
    }
}


/**
 This will set the Fiber with `n_pts` points unless `n_pts == 0`, in which case
 the number of points will be set automatically from `fnSegmentation`.
 `pts[]` should provide `DIM * n_pts` coordinates.

 The given set of points do not need to be equally distributed.
 The MINUS_END and PLUS_END will be set to the first and last points in `pts[]`,
 and intermediate points will be interpolated at regular intervals on `pts[]`.
 
 The length of the resulting fiber will be roughly equal to the sum of all segment lengths.
 However, the length of the segments will only be approximately equal to each other,
 and reshape() should be called to equalize them if necessary.
 */
void Chain::setShape(const real pts[], unsigned n_pts, unsigned np)
{
    assert_true(n_pts > 1);
    Vector a(pts), b;
    
    //calculate the total length
    real len = contourLength(pts, n_pts);
    
    if ( np == 0 )
    {
        assert_true( fnSegmentation > REAL_EPSILON );
        np = bestNumberOfPoints(len/fnSegmentation);
    }
    setNbPoints(np);
    setSegmentation(len/(np-1));
    fnAbscissaP = fnAbscissaM + len;
    
    a.load(pts);
    b.load(pts+DIM);
    setPoint(0, a);
    
    len = (b-a).norm();
    real h = 0;
    unsigned p = 1;
    --np;
    
    for ( unsigned n = 1; n < np; ++n )
    {
        h += fnCut;

        while ( h > len )
        {
            h -= len;
            a = b;
            ++p;
            assert_true(p<n_pts);
            b.load(pts+DIM*p);
            len = (b-a).norm();
        }
        
        setPoint(n, a+(h/len)*(b-a));
    }
    b.load(pts+DIM*n_pts-DIM);
    setPoint(np, b);
    updateFiber();
    reshape();
}

/**
 The filament is set as a random walk with given persistence length

 This return a filament in a random direction, with the center of gravity at zero
 and the average orientation aligned with (1, 0, 0)
 */
void Chain::setEquilibrated(real len, real persistence_length)
{
    unsigned np = bestNumberOfPoints(len/fnSegmentation);
    assert_true( np > 1 );
    
    setNbPoints(np);
    setSegmentation(len/(np-1));
    fnAbscissaP = fnAbscissaM + len;
    
    real sigma = sqrt(2*fnCut/persistence_length);
    
    Vector dir(1,0,0);
    Vector vec(0,0,0);
    setPoint(0, vec);
    
    for ( unsigned p = 1 ; p < np; ++p )
    {
        vec += fnCut * dir;
        setPoint(p, vec);
        //rotate dir in a random direction:
        real a = sigma * RNG.gauss();
        real c = cos(a), s = sin(a);
        dir = c * dir + s * dir.randOrthoU(1);
    }
    
    // cancel out mean orientation and position:
    translate(-0.5*vec);
    if ( vec.normSqr() > 0.01 * fnCut )
    {
        Rotation rot = Rotation::rotationToVector(vec).transposed();
        rotate(rot);
    }
    updateFiber();
}


/**
 This adjusts the current `normal` or makes a new one if necessary
 (used for display)
 */
Vector3 Chain::adjustedNormal(Vector3 const& d) const
{
#if FIBER_HAS_NORMAL
    if ( fnNormal.normSqr() < 0.8 || dot(fnNormal, d) > 0.5 )
        fnNormal = d.orthogonal(1.0);
    else
        fnNormal = d.orthogonal(fnNormal, 1.0);
    return fnNormal;
#else
    LOG_ONCE("WARNING: Cytosim was compiled without FIBER_HAS_NORMAL\n");
    return d.orthogonal();
#endif
}


real Chain::age() const
{
    return simul().time() - fnBirthTime;
}


//===================================================================
#pragma mark -

/*
 This deals with Fiber having one segment only,
 for which the procedure is trivial
 */
void Chain::reshape_two(const real* src, real* dst, real cut)
{
    real X = src[  DIM] - src[0];
#if ( DIM == 1 )
    real s = 0.5 - 0.5 * (cut/fabs(X));
#elif ( DIM == 2 )
    real Y = src[1+DIM] - src[1];
    real n = sqrt( X * X + Y * Y );
    real s = 0.5 - 0.5 * (cut/n);
#else
    real Y = src[1+DIM] - src[1];
    real Z = src[2+DIM] - src[2];
    real n = sqrt( X * X + Y * Y + Z * Z );
    real s = 0.5 - 0.5 * (cut/n);
#endif
    
    dst[0    ] = src[0    ] + s * X;
    dst[  DIM] = src[  DIM] - s * X;
#if ( DIM > 1 )
    dst[1    ] = src[1    ] + s * Y;
    dst[1+DIM] = src[1+DIM] - s * Y;
#endif
#if ( DIM > 2 )
    dst[2    ] = src[2    ] + s * Z;
    dst[2+DIM] = src[2+DIM] - s * Z;
#endif
}


/**
 Shorten segments to restore their length to 'cut'.
 We use a multidimensional Newton's method, to find iteratively the scalar
 coefficients that define the amount of displacement of each point.
 
     X[i] = vector of position
 
 We note 'dif' the differences between consecutive points:  dif[i] = X[i+1] - X[i]
 Given one scalar per segment: A[i], the point is displaced as:
 
     Y[i] = X[i] + A[i] * dif[i] - A[i-1] * dif[i-1]
 
 except for the first and last points, for which there is only one term:
 
     Y[0] = X[0] + A[  0] * dif[  0]
     Y[L] = X[L] - A[L-1] * dif[L-1]
 
 We want 'A[]' to restore the length of segments:
 
     ( Y[i+1] - Y[i] )^2 = cut^2
 
 i.e. 'A[]' should fulfill a set of equalities F[i] = 0, with:
 
     F[i] = ( Y[i+1] - Y[i] )^2 - cut^2
 
 Note that:
 
     Y[i+1] - Y[i] = A[i+1] * dif[i+1] + (1-2*A[i]) * dif[i] + A[i-1] * dif[i-1]
 
 Method: use all zeros as first guess for 'sca', and apply a multidimensional
 Newton's method to iteratively refine the guess.
 
 In practice, we calculate `A_next` from `A` using the relationship:
 
     J(A) * ( A_next - A ) = -F(A)
 
 Where J is the Jacobian matrix: J[i,j] = dF[i] / dA[j]
 
 For this problem, J is square and tri-diagonal but not symmetric,
 and must be recalculated at each iteration. A factor 2 can be factorized:

     A_next = A - 1/2 inv(K).F(A)
 
 Where J = 2 * K

 FJN, Strasbourg, 22.02.2015 & Cambridge, 10.05.2019 -- 13.05.2019
 */
int Chain::reshape_calculate(const unsigned ns, real cutSqr, Vector const* dif,
                                real* mem, size_t chk)
{
    real * sca = mem;
    real * val = mem+chk;
    real * dia = mem+chk*2;
    real * low = mem+chk*3;
    real * upe = mem+chk*4;

    /*
     Perform here the first iteration of Newton's method
     the formula is the same as below, with all `sca` equal to zero,
     and thus 'vec == dif'
     The system is symmetric, and we can use a faster factorization
     */
    real err0 = 0;
    for ( unsigned i = 0; i < ns; ++i )
    {
        real n = dif[i].normSqr();
        sca[i] = n - cutSqr;
        err0 += fabs(sca[i]);
        dia[i] = n * 4;
        low[i] = dot(dif[i], dif[i+1]) * (-2);  //using undefined value
    }
    
    int info = 0;
    lapack::xpttrf(ns, dia, low, &info);
    if ( info ) {
        std::cerr << " reshape_local lapack::xpttrf failed " << info << std::endl;
        return 1;
    }
    lapack::xptts2(ns, 1, dia, low, sca, ns);

    //printf("\n ----err %20.16f", err0);
    //printf("\n     sca "); VecPrint::print(std::cout, ns, sca, 3);

    unsigned cnt = 0;
    while ( ++cnt < 16 )
    {
        assert_true( ns > 1 );
        // set the matrix elements and RHS of system,
        Vector vec = (1-2*sca[0])*dif[0] + sca[1]*dif[1];
        val[0] = vec.normSqr() - cutSqr;
        dia[0] = dot(vec, dif[0]) * ( -2 );
        upe[0] = dot(vec, dif[1]);
        real err = fabs(val[0]);
        for( unsigned i = 1; i+1 < ns; ++i )
        {
            vec = sca[i-1]*dif[i-1] + (1-2*sca[i])*dif[i] + sca[i+1]*dif[i+1];
            val[i] = vec.normSqr() - cutSqr;
            err += fabs(val[i]);
            low[i] = dot(vec, dif[i-1]);
            dia[i] = dot(vec, dif[i  ]) * ( -2 );
            upe[i] = dot(vec, dif[i+1]);
        }
        vec = sca[ns-2]*dif[ns-2] + (1-2*sca[ns-1])*dif[ns-1];
        val[ns-1] = vec.normSqr() - cutSqr;
        low[ns-1] = dot(vec, dif[ns-2]);
        dia[ns-1] = dot(vec, dif[ns-1]) * ( -2 );
        err += fabs(val[ns-1]);
#if ( 0 )
        printf("\n %3i err %20.16f norm(val) %8.5f", cnt, err, blas::nrm2(ns, val));
        //printf("\n     val "); VecPrint::print(std::cout, ns, val, 3);
        printf("\n     sca "); VecPrint::print(std::cout, ns, sca, 3);
#endif
        if ( err < 1e-10 )
            return 0;
        if ( err > err0 )
            return 3;
        err0 = err;
#if ( 0 )
        printf("\n     dia "); VecPrint::print(std::cout, ns, dia, 3);
        printf("\n     upe "); VecPrint::print(std::cout, ns-1, upe, 3);
        printf("\n     low "); VecPrint::print(std::cout, ns-1, low+1, 3);
#endif
#if ( 0 )
        real asy = 0, sup = 0;
        for ( unsigned i = 0; i < ns-1; ++i )
        {
            sup = std::max(sup, fabs(low[i+1]+upe[i]));
            asy += fabs(low[i+1]-upe[i]);
        }
        printf("\n %3i diff(low-upe) %12.6f", cnt, 2*asy/sup);
#endif
        lapack::xgtsv(ns, 1, low+1, dia, upe, val, ns, &info);
        if ( info )
        {
            std::cerr << " LAPACK dgtsv failed " << info << std::endl;
            return 2;
        }

        // update `sca`
        for ( unsigned u = 0; u < ns; ++u )
            sca[u] -= 0.5 * val[u];

        //printf("\n   ->sca "); VecPrint::print(std::cout, ns, sca, 3);
    }
    //printf("\n   >>err %20.16f", err);
    //printf("\n   >>sca "); VecPrint::print(std::cout, ns, sca, 3);
    return 4;
}


void Chain::reshape_apply(const unsigned ns, const real* src, real* dst,
                             const real * sca)
{
    assert_true( ns > 1 );
    Vector A(src);
    Vector B(src+DIM);
    Vector old = sca[0] * ( B - A );
    (A+old).store(dst);
    
    for ( size_t i = 1; i < ns; ++i )
    {
        Vector C(src+DIM*i+DIM);
        Vector vec = sca[i] * ( C - B );
        (B+(vec-old)).store(dst+DIM*i);
        old = vec;
        B = C;
    }
    
    (B-old).store(dst+DIM*ns);
}


/*
 Atempts to re-establish the length of the segments, by moving points along
 the directions of the flanking segments
 */
int Chain::reshape_local(const unsigned ns, const real* src, real* dst, real cut, real* tmp, size_t tmp_size)
{
    int res;
    assert_true( ns > 1 );

    real * mem = tmp + tmp_size*4;
    Vector * dif = reinterpret_cast<Vector*>(tmp);

    // calculate differences:
    for ( unsigned p = 0; p < ns; ++p )
        dif[p] = diffPoints(src, p);
    dif[ns].reset();
    
    res = reshape_calculate(ns, cut*cut, dif, mem, tmp_size);

    if ( res == 0 )
        reshape_apply(ns, src, dst, mem);

    return res;
}


/**
 The response of this method to a sudden perpendicular force is not ideal:
 For example, a force applied to the bottom of a vertical fibers leads
 to a 'L' configuration after one step of `solve()`.
 reshape() reduces the bottom leg of the 'L', by translating the entire vertical portion
 of the fiber, irrespective of the length of this section.
 */

#if ( 1 )   // 1 = optimized version of Chain::reshape_global()

/**
 Move the vertices relative to each other, such that when this is done,
 all segments have the same distance `fnCut` ( =segmentation() ).
 This is operation does not change the center of gravity of the fiber.

 
 NOTE: if two consecutive points overlap, there is no unique way to
 restore the constraints! We do nothing in that case, because most 
 likely, the Brownian motion will push the points appart soon.
 */

void Chain::reshape_global(const unsigned ns, const real* src, real* dst, real cut)
{
    Vector inc(0,0,0), sum(0,0,0);
    Vector seg = diffPoints(src, 0);
    real   dis = seg.norm();
    
    // translation needed to restore first segment
    if ( dis > REAL_EPSILON )
        inc = ( cut/dis - 1.0 ) * seg;
    
    Vector(src).store(dst);

    for ( size_t i = 1; i < ns; ++i )
    {
        seg = diffPoints(src, i);
        dis = seg.norm();
        
        //move the left point by off:
        (inc+Vector(src+DIM*i)).store(dst+DIM*i);
        //update the uniform motion of the points:
        sum += inc;
        
        //add to the translation needed to restore this segment
        if ( dis > REAL_EPSILON )
            inc += ( cut/dis - 1.0 ) * seg;
    }
    
    // move the last point by dp:
    (inc+Vector(src+DIM*ns)).store(dst+DIM*ns);
    
    // calculate uniform motion needed to conserve the center of gravity:
    sum = ( sum + inc ) / ( ns + 1 );
    
    // translate entire fiber uniformly:
    for ( unsigned i = 0; i <= ns; ++i )
        sum.sub_to(dst+DIM*i);
}

#else

// ------------  old ( less optimal ) version:
/**
 Move the vertices relative to each other, such that when this is done,
 all segments have the same distance segmentation() = fnCut.
 This is operation does not change the center of gravity of the fiber.
 */

void Chain::reshape_global(const unsigned ns, const real* src, real* dst, real cut)
{
    Vector off, sum(0,0,0);

    copy_real(DIM*(ns+1), src, dst);
    for ( unsigned pp = 1; pp <= ns; ++pp )
    {
        off      = diffPoints(dst, pp-1);
        real dis = off.norm();
        if ( dis > REAL_EPSILON )
        {
            off  *= ( cut/dis - 1.0 );
            for ( unsigned qq = pp; qq <= ns; ++qq )
                off.add_to(dst+DIM*qq);
            sum += ( 1 + ns - pp ) * off;
        }
    }
    
    sum /= ns + 1;
    for ( unsigned i = 0; i <= ns; ++i )
        sum.sub_to(dst+DIM*i);
}

#endif

/**
 Replace coordinates by ones provided in `ptr`
 A reshape operation is done
 */
void Chain::getPoints(real const* ptr)
{
    // allocate memory
    real* mem = new_real(9*allocated());

#if ( DIM > 1 )
    if ( nPoints == 2 )
        reshape_two(ptr, pPos, fnCut);
    else if ( reshape_local(nbSegments(), ptr, pPos, fnCut, mem, allocated()) )
#endif
    {
        std::string doc = document(ptr);
        reshape_global(nbSegments(), ptr, pPos, fnCut);
        Cytosim::warn << "crude motion was applied to " << doc << '\n';
        //copy_real(DIM*nbPoints(), ptr, pPos);
    }

    free_real(mem);
}


/**
 Flip all the points, such that minus_end becomes plus_end and vice-versa.
 This does not affects Abscissa and the abscissa of center thus stays as it is.
*/
void Chain::flipChainPolarity()
{
    unsigned ii = 0;
    unsigned jj = lastPoint();
    
    while ( ii < jj )
    {
        Vector P(pPos+DIM*ii);
        Vector Q(pPos+DIM*jj);
        Q.store(pPos+DIM*ii);
        P.store(pPos+DIM*jj);
        ++ii;
        --jj;
    }
}


//========================================================================
//=====================GROWING/SHRINKING==================================
//========================================================================
#pragma mark -

/**
 The argument 'delta' can be positive or negative:
 - delta > 0 : elongation,
 - delta < 0 : shortening
 .
 
 Note 1: This works nicely if `delta` is small compared to segmentation().
 For large decrease in length, use cutM().
 
 Note 2: Unless the chain is straight, the length of the segments after this
 will not exactly match `segmentation()`.
*/
void Chain::growM(const real delta)
{
    assert_true( length() + delta > 0 );
    real a = -delta / length();
    const size_t ns = nbSegments();
    
    if ( delta > 0 )
    {
        size_t p = 0, n = ns;
        Vector dp0 = diffPoints(0), dp1;
        movePoint(p, ( a * n ) * dp0);
        ++p;
        --n;
        
        if ( n > 0  &&  ( n & 1 ) )
        {
            dp1 = diffPoints(p);
            movePoint(p, ( a * n ) * dp0);
            dp0 = dp1;
            ++p;
            --n;
        }
        
        while ( n > 1 )
        {
            //assert_true( 0 == (p & 1) );
            dp1 = diffPoints(p);
            movePoint(p, ( a * n ) * dp0);
            ++p; --n;
            //assert_true( 1 == (p & 1) );
            dp0 = diffPoints(p);
            movePoint(p, ( a * n ) * dp1);
            ++p; --n;
        }
    }
    else if ( delta < 0 )
    {
        for ( size_t p = 0, n = ns; n > 0; ++p, --n )
            movePoint(p, ( a * n ) * diffPoints(p));
    }
    
    fnAbscissaM -= delta;
    setSegmentation(length()/ns);
    postUpdate();
}

/**
 This extends the fiber by adding one segment at the MINUS_END.
 Thus `segmentation()` is not changed, and the existing points are not displaced.
 */
void Chain::addSegmentM()
{
    unsigned pp = 1+nPoints;
    setNbPoints(pp);
    
    pp *= DIM;
    while ( --pp >= DIM )
        pPos[pp] = pPos[pp-DIM];
    
    for ( pp = 0; pp < DIM; ++pp )
        pPos[pp] += pPos[pp] - pPos[pp+2*DIM];
    
    fnAbscissaM -= fnCut;
    postUpdate();
}


/**
 The Fiber length is reduced by `delta` ( which must be >= 0 ).
 The portion of size `delta` near the MINUS_END is removed,
 the (fewer) vertices are recalculated.
 
 Note: after cutM(), the distance between the points is not exactly
 equal to segmentation(). This is true only if the fiber is straight.
 */
void Chain::cutM(const real delta)
{
    real len = length();
    assert_true( 0 <= delta );
    assert_true( delta < len );
    
    const size_t ns = bestNumberOfPoints((len-delta)/fnSegmentation) - 1;
    const real cut = (len-delta) / ns;
    real* tmp = new_real(DIM*(ns+1));

    // calculate intermediate points:
    for ( size_t i=0; i < ns; ++i )
    {
        Vector w = interpolateM(delta+i*cut).pos();
        w.store(tmp+DIM*i);
    }

    // copy the position of plus-end:
    copy_real(DIM, pPos+DIM*lastPoint(), tmp+DIM*ns);
    
    setNbPoints(ns+1);
    fnAbscissaM += delta;
    setSegmentation(cut);
    getPoints(tmp);
    free_real(tmp);
    postUpdate();
}


/**
 The argument 'delta' can be positive or negative:
 - delta > 0 : elongation,
 - delta < 0 : shortening
 .
 
 Note 1: This works nicely if `delta` is small compared to segmentation().
 For large decrease in length, use cutP().

 Note 2: Unless the chain is straight, the length of the segments after this
 will not exactly match `segmentation()`.
 */
void Chain::growP(const real delta)
{
    assert_true( length() + delta > 0 );
    real a = delta / length();
    const size_t ns = nbSegments();
    
    if ( delta > 0 )
    {
        Vector dp0 = diffPoints(ns-1), dp1;
        movePoint(ns, ( a * ns ) * dp0);
        size_t p = ns-1;
        
        if ( p > 0  &&  ( p & 1 ) )
        {
            dp1 = diffPoints(p-1);
            movePoint(p, ( a * p ) * dp0);
            dp0 = dp1;
            --p;
        }
        
        while ( p > 1 )
        {
            //assert_true( 0 == (p & 1) );
            dp1 = diffPoints(p-1);
            movePoint(p, ( a * p ) * dp0);
            --p;
            //assert_true( 1 == (p & 1) );
            dp0 = diffPoints(p-1);
            movePoint(p, ( a * p ) * dp1);
            --p;
        }
    }
    else if ( delta < 0 )
    {
        for ( size_t p = ns ; p > 0 ; --p )
            movePoint(p, ( a * p ) * diffPoints(p-1));
    }
    
    fnAbscissaP += delta;
    setSegmentation(length()/ns);
    postUpdate();
}


/**
 This extends the fiber by adding one segment at the PLUS_END.
 Thus `segmentation()` is not changed, and the existing points are not displaced.
 */
void Chain::addSegmentP()
{
    unsigned pp = nPoints;
    setNbPoints(pp+1);
    
    real * psp = pPos + pp * DIM;
    for ( unsigned int dd = 0; dd < DIM; ++dd )
        psp[dd] = 2 * psp[dd-DIM] - psp[dd-2*DIM];
    
    fnAbscissaP += fnCut;
    postUpdate();
}


/**
 The Fiber length is reduced by `delta` ( which must be >= 0 ).
 The portion of size `delta` near the PLUS_END is removed,
 and the fewer vertices are recalculated.

 Note: after cutP(), the distance between the points is not exactly
 equal to segmentation(). This is true only if the fiber is straight.
*/
void Chain::cutP(const real delta)
{
    real len = length();
    assert_true( 0 <= delta );
    assert_true( delta < len );
    
    const unsigned np = bestNumberOfPoints((len-delta)/fnSegmentation);
    const real cut = (len-delta) / (np-1);
    real* tmp = new_real(DIM*np);

    // copy minus end:
    copy_real(DIM, pPos, tmp);

    // calculate intermediate points:
    for ( unsigned i = 1; i < np; ++i )
    {
        Vector w = interpolateM(i*cut).pos();
        w.store(tmp+DIM*i);
    }

    setNbPoints(np);
    setSegmentation(cut);
    fnAbscissaP -= delta;
    getPoints(tmp);
    free_real(tmp);
    postUpdate();
}

//------------------------------------------------------------------------------

void Chain::grow(FiberEnd end, const real delta)
{
    if ( end == PLUS_END )
        growP(delta);
    else if ( end == MINUS_END )
        growM(delta);
}


void Chain::adjustLength(real len, FiberEnd ref)
{
    assert_true( len > 0 );
    
    if ( ref == PLUS_END )
    {
        if ( len < length() )
            cutP(length()-len);
        else
            growP(len-length());
    }
    else if ( ref == MINUS_END )
    {
        if ( len < length() )
            cutM(length()-len);
        else
            growM(len-length());
    }
}


void Chain::truncateM(unsigned p)
{
    Mecable::truncateM(p);
    fnAbscissaM = abscissaPoint(p);
    postUpdate();
}


void Chain::truncateP(unsigned p)
{
    Mecable::truncateP(p);
    fnAbscissaP = abscissaPoint(p);
    postUpdate();
}


/**
 `fib` is attached at the PLUS_END of `*this`
 
 The vertex are reinterpolated linearly, and the length of the
 segments will not fullfil the constraints of segmentation.
 If this is a problem, Chain::reshape() should be called.
 
 `fib` should usually be destroyed afterward.
 */
void Chain::join(Chain const* fib)
{
    const real len = length();
    const real lenT = len + fib->length();
    const size_t ns = bestNumberOfPoints(lenT/fnSegmentation) - 1;
    const real cut = lenT / real(ns);
    
    real* tmp = new_real(DIM*(ns+1));

    // calculate new points into tmp[]:
    for ( size_t i = 1; i < ns; ++i )
    {
        Vector w;
        if ( i*cut < len )
            w = interpolateM(i*cut).pos();
        else
            w = fib->interpolateM(i*cut-len).pos();
        w.store(tmp+DIM*i);
    }
    
    // copy position of PLUS_END:
    fib->posEndP().store(tmp+DIM*ns);

    setNbPoints(ns+1);
    setSegmentation(cut);
    fnAbscissaP = fnAbscissaM + cut * ns;
    getPoints(tmp);
    free_real(tmp);
    updateFiber();
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Returns the minimum and maximum distance between consecutive points
 */
/**
 Returns the minimum and maximum distance between consecutive points
 */
void Chain::segmentationMinMax(real const* ptr, real& in, real& ax) const
{
    real x = sMath::distanceSqr<DIM>(ptr, ptr+DIM);
    in = x;
    ax = x;
    for ( unsigned i = 1; i < lastPoint(); ++i )
    {
        x = sMath::distanceSqr<DIM>(ptr+DIM*i, ptr+DIM*(i+1));
        in = std::min(in, x);
        ax = std::max(ax, x);
    }
    in = std::sqrt(in);
    ax = std::sqrt(ax);
}

/**
 Returns the average and variances of segment length
 */
void Chain::segmentationVariance(real& avg, real& var) const
{
    avg = 0;
    var = 0;
    unsigned cnt = nbSegments();
    for ( unsigned n = 0; n < cnt; ++n )
    {
        real r = diffPoints(n).norm();
        avg += r;
        var += r*r;
    }
    avg /= cnt;
    var = var / cnt - avg * avg;
}

/**
 Calculate Menger curvature:
 the inverse of the radius of the circle that passes through A, B and C
 
 1/R = 4 * Area(triangle) / ( |AB|*|BC|*|AC| )
 
 curvature is ZERO if A, B and C are aligned
 
 Thank you, Serge to point this out!
 */
real curvature3(Vector const& A, Vector const& B, Vector const& C, real seg)
{
    real H = norm( A + C - 2 * B );  // 2 * height_of_triangle
    return H / ( seg * seg );
}

/**
 This returns
 */
real Chain::curvature(unsigned p) const
{
    if (( 0 < p ) & ( p < lastPoint() ))
        return curvature3(posP(p-1), posP(p), posP(p+1), fnCut);
    return 0;
}


/**
 The normalized bending energy is an integral over the curvilinear abscissa `s`:
 
     1/2 * sum( curvature(s)^2 ds )
 
 The curvature is calculated from the positions of the vertices:
 Given theta = angle between two consecutive segments,

     curvature = 1/R = 2 * sin(angle/2) / segmentation
 
 and since
 
     sin^2(angle/2) = ( 1 - cos(angle) ) / 2
 
 hence:
 
     1/2 * curvature^2 = ( 1 - cos(angle) ) / ( segmentation^2 )
 
 and finaly:
 
     1/2 * sum( curvature^2 * ds ) = sum( 1 - cos(angle) ) / segmentation
 
 */
real Chain::bendingEnergy0() const
{
    real e = 0;
    
    const unsigned lsp = nPoints - 2;
    if ( lsp > 0 )
    {
        for ( unsigned p = 0; p < lsp ; ++p )
        {
            Vector A = posP(p);
            Vector B = posP(p+1);
            Vector C = posP(p+2);
            e += dot(B - A, C - B);  // e += cos(angle) * segmentation^2
        }
        // e <- sum( 1 - cos(angle) )
        e = lsp - e / ( fnCut * fnCut );
        
        /*
         We correct the result, because we only considered (nPoints-2) junctions,
         and thus covered only a fraction of the total length of the filament
         */
        e *= ( lsp + 1 ) / ( fnCut * lsp );
    }
    
    return e;
}


real Chain::minCosinus() const
{
    real result;
    Vector dir1, dir2;
    
    unsigned ps = nbSegments() % 2;
    if ( ps )
    {
        dir1   = diffPoints(0);
        result = fnCut * fnCut;
    }
    else
    {
        dir1   = diffPoints(1);
        result = dot(diffPoints(0), dir1);
        ps = 2;
    }
    
    for ( ; ps < nbSegments(); ps+=2 )
    {
        dir2 = diffPoints(ps);
        real s = dot(dir1, dir2);
        if ( s < result ) result = s;
        dir1 = diffPoints(ps+1);
        real t = dot(dir1, dir2);
        if ( t < result ) result = t;
    }
    
    return result / ( fnCut * fnCut );
}


/**
 Returns the minimum and maximum distance between consecutive points
 */
unsigned Chain::nbKinks(real threshold) const
{
    threshold *= fnCut * fnCut;
    unsigned res = 0;
    Vector d = diffPoints(0);
    
    for ( unsigned n = 1; n < lastPoint(); ++n )
    {
        Vector r = diffPoints(n);
        if ( dot(d, r) < threshold )
            ++res;
        d = r;
    }
    return res;
}

/**
 This calculates the intersection between the support line of segment `s`,
 and the plane defined by <em> n.pos + a = 0 </em>

 @return scalar `x` specifying the intersection with the support line:
 - `x = 0` if intersection occurs at point 's'
 - `x in ]0, 1[` for intersections that are within the segment boundaries
 - `x = 1` if intersection occurs at point 's+1'
 - `x = INFINITY` if the segment is parallel to the plane
 .
 
 The abscissa of the intersection is `abscissaPoint(s+a)`.
 The position of the cut is `interpolatePoints(s, s+1, a)`
 */

real Chain::planarIntersect(unsigned s, Vector const& n, const real a) const
{
    assert_true( s < nbSegments() );
    
    real sca = dot(diffPoints(s), n);
    
    // if segment is parallel to plane, there is no intersection:
    if ( -REAL_EPSILON < sca  &&  sca < REAL_EPSILON )
        return INFINITY;
    
    Vector pos = posP(s);
    
    if ( modulo )
    {
        // calculate image that is closest to plane:
        Vector cen = n * ( -a / n.normSqr() );
        modulo->fold(pos, cen);
    }

    return - ( dot(pos, n) + a ) / sca;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Recalculate the vertices for 'ns' segments.
 
 @todo 2d-order interpolation in Chain::resegment()
 
 Note: Unless the chain is straight, the length of the segments after
 an interpolation will not exactly match `segmentation()`, and we therefore
 call reshape() here to correct for the problem.
 */
void Chain::resegment(unsigned ns)
{
    assert_true( ns > 0 );
    real cut = length() / ns;
    
    // calculate new intermediate points in tmp[]:
    Vector a = posP(0), b = posP(1);
    
    real h = 0;
    unsigned p = 1;
    real* tmp = new_real(DIM*(ns+1));
    
    a.store(tmp);
    for ( unsigned n = 1; n < ns; ++n )
    {
        h += cut;
        
        while ( h > fnCut )
        {
            h -= fnCut;
            a = b;
            ++p;
            assert_true(p<nbPoints());
            b.load(pPos+DIM*p);
        }
        
        Vector w = a + ( h / fnCut ) * ( b - a );
        w.store(tmp+DIM*n);
    }
    
    // copy coordinates of last point:
    a.load(pPos+DIM*lastPoint());
    a.store(tmp+DIM*ns);

    // resize filament:
    setNbPoints(ns+1);
    setSegmentation(cut);
    getPoints(tmp);
    free_real(tmp);
}


/**
 A fiber is segmented as a function of its length.
 The number of segments `NS` is the one that minimizes the absolute value:

     | length / NS - segmentation |
 
 Where `segmentation` is the parameter. NS is such that:

     length / NS < 4/3 * segmentation
     length / NS > 2/3 * segmentation

 */
void Chain::adjustSegmentation()
{
    assert_true( fnSegmentation > REAL_EPSILON );
    
    unsigned best = bestNumberOfPoints(nbSegments()*fnCut/fnSegmentation);
    
    if ( best != nPoints )
    {
        //std::clog << reference() << " resegment " << nPoints << " -> " << best << "\n";
#if ( 1 )
        resegment(best-1);
#else
        // copy current points in temporary array:
        real* tmp = new_real(DIM*nPoints);
        copy_real(DIM*nPoints, pPos, tmp);
        // re-interpolate:
        setShape(tmp, nPoints, best);
        free_real(tmp);
#endif
    }
}

void Chain::adjustSegmentation(real arg)
{
    if ( fnSegmentation != arg )
    {
        std::clog << "Resegmenting " << reference() << " -> " << arg << "\n";
        fnSegmentation = arg;
        adjustSegmentation();
        updateFiber();
        reshape();
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 return the abscissa with respect to the ORIGIN.
 */
real Chain::abscissaEnd(const FiberEnd end) const
{
    switch( end )
    {
        case ORIGIN:    return 0;
        case PLUS_END:  return abscissaP();
        case MINUS_END: return abscissaM();
        case CENTER:    return abscissaC();
        default:        ABORT_NOW("invalid argument value"); return 0;
    }
}


/**
 returns the abscissa (from the ORIGIN) of a point that is specified
 by a distance from the given reference.
 */
real Chain::abscissaFrom(const real dis, const FiberEnd ref) const
{
    switch( ref )
    {
        case ORIGIN:     return dis;
        case PLUS_END:   return abscissaP() - dis;
        case MINUS_END:  return dis + abscissaM();
        case CENTER:     return dis + abscissaC();
        default:         ABORT_NOW("invalid argument value"); return 0;
    }
}

/**
 This uses values at [1], [2] and [3] of opt[key] to define an abscissa
 on the Fiber:
 
     attach = FIBER, ABSCISSA, REFERENCE, MODIFIER
 
 with
 
     ABSCISSA = REAL
     REFERENCE = { 'plus_end', 'minus_end', 'center' }  (default = 'origin')
     MODIFIER = { 'none', 'uniform', 'exponential' }  (default = 'none')
 
 All these parameters are optional.
 The abscissa is counted from the reference and towards the other end.
 The MODIFIER introduces a random component to the position.
 If ABSCISSA is not specified, this return a random abscissa.
 
 Example:
 
     new filament
     {
         attach1 = simplex, 0.0, minus_end
         attach2 = simplex, 0.0, plus_end
     }

*/
real Chain::someAbscissa(real dis, FiberEnd ref, int mod, real alpha) const
{
    const real len = length();
    real a = dis;
    
    switch ( mod )
    {
        case 0:
            break;
        case 1:  // random
            do {
                a = dis * RNG.preal();
            } while ( a > len );
            break;
        case 2:  // exponential
            do {
                a = dis * RNG.exponential();
            } while ( a > len );
            break;
        case 3:  // regular
            a *= alpha;
            break;
        case 7:
            return RNG.real_uniform(abscissaM(), abscissaP());
    }
    
    dis = abscissaFrom(a, ref);

    if ( !betweenMP(dis) )
        throw InvalidParameter("hand::abscissa is out of range");
        
    return dis;
}

/**
 The Fiber is partitionned by this function in three regions:
 - a MINUS_END part of length `lambda`
 - a PLUS_END part, also of length `lambda`
 - and a NO_END section in between
 .
 Note that a Fiber shorter than `2*lambda` does not have a central region,
 and is composed of PLUS_END and MINUS_END parts of equal size.
 */    
FiberEnd Chain::whichEndDomain(const real ab, const real lambda) const
{
    const real abM = ab - fnAbscissaM;
    const real abP = fnAbscissaP - ab;
    
    if ( abM > abP )
    {
        if ( abP <= lambda )
            return PLUS_END;
    }
    else
    {
        if ( abM <= lambda )
            return MINUS_END;
    }
    return NO_END;
}


//------------------------------------------------------------------------------
#pragma mark -


Mecapoint Chain::exactEnd(const FiberEnd end) const
{
    if ( end == MINUS_END )
        return Mecapoint(this, 0);
    else
    {
        assert_true( end == PLUS_END );
        return Mecapoint(this, lastPoint());
    }
}


Interpolation Chain::interpolateEnd(const FiberEnd end) const
{
    if ( end == MINUS_END )
        return interpolateEndM();
    else
    {
        assert_true( end == PLUS_END );
        return interpolateEndP();
    }
}


Interpolation Chain::interpolateCenter() const
{
    unsigned int n = lastPoint() / 2;
    if ( 2*n == lastPoint() )
        return Interpolation(this, n, n+1, 0);
    else
        return Interpolation(this, n, n+1, 0.5);
}


/**
 return Interpolation corresponding to a distance `ab` from the MINUS_END
 The interpolation describes a position:
       X = P(r) * (1-a) + P(r+1) * a
 where
 - `r` is an integer: 0 <= r < lastPoint(),
 - `a` is a positive real coefficient: 0 <= a <= 1
 .
 When `ab` is above the PLUS_END, an interpolation of the last point is returned.
 
 */
Interpolation Chain::interpolateM(const real ab) const
{
    real a = std::max(ab, 0.0) / fnCut;
    //beyond the last point, we interpolate the PLUS_END
    unsigned s = std::min((unsigned)a, nPoints-2);
    return Interpolation(this, s, s+1, std::min(a-s, 1.0));
}


Interpolation Chain::interpolate(const real ab, const FiberEnd end) const
{
    switch( end )
    {
        case ORIGIN:
            return interpolate(ab);
            
        case MINUS_END:
            return interpolateM(ab);
            
        case CENTER:
            return interpolateM(ab + 0.5*length());
            
        case PLUS_END:  //this is counted from the plus towards the minus end
            return interpolateM(fnCut*nbSegments() - ab);
        
        default:
            ABORT_NOW("invalid argument value");
    }
    return interpolate(0);
}

//------------------------------------------------------------------------------
#pragma mark -

#if ( DIM > 1 )
Vector Chain::posM(const real ab) const
{
    // return MINUS_END
    if ( ab <= 0 )
        return posP(0);
    
    real a = ab / fnCut;
    unsigned s = (unsigned)a;
    
    // check if PLUS_END is reached:
    if ( s+1 < nPoints )
        return interpolatePoints(s, s+1, a-s);
    else
        return posP(lastPoint());
}

Vector Chain::dirM(const real ab) const
{
    // at MINUS_END
    if ( ab <= 0 )
        return dirSegment(0);
    
    real a = ab / fnCut;
    unsigned s = (unsigned)a;
    
    // check if PLUS_END is reached
    if ( s+1 < nPoints )
        return dirSegment(s);
    else
        return dirSegment(lastSegment());
}
#endif


Vector Chain::posEnd(FiberEnd end) const
{
    if ( end == MINUS_END )
        return posEndM();
    else if ( end == PLUS_END )
        return posEndP();
    else
        return posM(abscissaFrom(0, end));
}


Vector Chain::dirEnd(const FiberEnd end) const
{
    if ( end == MINUS_END )
        return dirSegment(0);
    else if ( end == PLUS_END )
        return dirSegment(lastSegment());
    else
        return dirM(abscissaFrom(0, end));
}


/// force on the PLUS_END projected on the direction of elongation
real Chain::projectedForceEndM() const
{
    return -dot(netForce(0), dirSegment(0));
}

/// force on the PLUS_END projected on the direction of elongation
real Chain::projectedForceEndP() const
{
    unsigned p = lastSegment();
    return dot(netForce(p+1), dirSegment(p));
}


/**
 The returned value is negative when the force antagonizes elongation,
 and this is true at both ends. 
 */
real Chain::projectedForceEnd(const FiberEnd end) const
{
    if ( end == PLUS_END )
        return projectedForceEndP();
    else
    {
        assert_true( end == MINUS_END );
        return projectedForceEndM();
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Chain::briefdoc(std::ostream& os, real len, real con, real mn, real mx) const
{
    std::streamsize p = os.precision();
    os.precision(3);
    os << "chain " << reference();
    os << "( seg " << segmentation() << ": " << mn << " +" << mx-mn;
    os << " len " << len << " " << std::showpos << con-len << std::noshowpos << " )";
    os.precision(p);
}


/**
 Prints info on the length of Segments, which can be useful for debugging
 */
void Chain::document(std::ostream& os, real len, real con, real mn, real mx) const
{
    os << "chain " << std::setw(7) << reference() << '\n';
    os << "{\n";
    os << "    segmentation = " << segmentation() << '\n';
    os << "    segment_min = " << mn << '\n';
    os << "    segment_max = " << mx << '\n';
    os << "    length  = " << len << '\n';
    os << "    contour = " << con << '\n';
    os << "}" << std::endl;
}


int Chain::checkLength(real const* ptr, std::ostream& os, real len) const
{
    real mn, mx;
    segmentationMinMax(ptr, mn, mx);
    real dev = ( mx - mn ) / segmentation();
    real con = contourLength(ptr, nPoints);
    real err = abs_real( con - len ) / ( con + len );
    int res = ( dev > 0.05 ) + ( err > 0.05 );
    if ( res )
        document(os, len, con, mn, mx);
    return res;
}


/**
 Prints info on the length of Segments, which can be useful for debugging
 */
void Chain::document(real const* ptr, std::ostream& os) const
{
    real mn, mx;
    real len = length();
    segmentationMinMax(ptr, mn, mx);
    real con = contourLength(ptr, nPoints);
    briefdoc(os, len, con, mn, mx);
}


std::string Chain::document(real const* ptr) const
{
    std::ostringstream ss;
    document(ptr, ss);
    return ss.str();
}


//------------------------------------------------------------------------------
#pragma mark - I/O

void Chain::write(Outputter& out) const
{
    assert_small( length1() - length() );
    out.writeUInt32(signature());
    out.writeFloat(length());
    out.writeFloat(fnSegmentation);
    out.writeFloat(fnAbscissaM);
    out.writeFloat(fnBirthTime);
    Mecable::write(out);
}


/**
 The fiber will be re-segmented if its current desired segmentation 
 does not match the one stored in the file.
 */
void Chain::read(Inputter& in, Simul& sim, ObjectTag tag)
{
    //Cytosim::log << "  reading Chain at " << in.pos() << '\n';
    
    ObjectSignature s = in.readUInt32();
    if ( s ) signature(s);
    
    real len    = in.readFloat();
    real seg    = in.readFloat();
    fnAbscissaM = in.readFloat();
    
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() > 49 ) // 12.12.2018 moved birthTime
#endif
        fnBirthTime = in.readFloat();

    if ( len <= 0 )
        throw InvalidIO("invalid (negative) fiber length");

    if ( len > 1e6 )
        throw InvalidIO("excessive fiber length");
    
    if ( seg <= 1e-6 || seg > 1e6 )
        throw InvalidIO("invalid fiber segmentation");

    Mecable::read(in, sim, tag);
    
    if ( nPoints < 2 )
        throw InvalidIO("invalid fiber with 0 or 1 point");

#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() <= 37 )
    {
        setSegmentation(len);
        len *= nbSegments();
    }
    else
#endif
        setSegmentation(len/nbSegments());

    fnAbscissaP = fnAbscissaM + len;
    fnSegmentation = seg;
    //Mecable::write(std::cerr);
    
    // verify the length and segmentation:
    if ( in.vectorSize() == DIM )
        checkLength(pPos, std::clog, len);
}

