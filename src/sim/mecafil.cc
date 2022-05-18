// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "dim.h"
#include "sim.h"
#include "mecafil.h"
#include "cblas.h"
#include "clapack.h"
#include "matrix.h"
#include "matsparsesym1.h"
#include "random.h"
#include "vecprint.h"

//------------------------------------------------------------------------------
Mecafil::Mecafil()
{
    buildProjection();
    rfPointMobility = 0;
    rfRigidity = 0;
    rfDiff = nullptr;
    rfLag  = nullptr;
    rfLLG  = nullptr;
    rfVTP  = nullptr;
}


Mecafil::~Mecafil()
{
    destroyProjection();
    free_real(rfDiff);
    rfDiff = nullptr;
    rfLag  = nullptr;
    rfLLG  = nullptr;
    rfVTP  = nullptr;
}


//------------------------------------------------------------------------------
Mecafil::Mecafil(Mecafil const&)
{
    ABORT_NOW("unfinished: cannot copy a Fiber");
}


Mecafil& Mecafil::operator=(Mecafil const&)
{
    ABORT_NOW("unfinished: cannot copy a Fiber");
}


//------------------------------------------------------------------------------
size_t Mecafil::allocateMecable(const size_t nbp)
{
    size_t ms = Mecable::allocateMecable(nbp);
    /*
     if Mecable::allocateMecable() allocated memory, it will return the 
     size of the new array, and we allocate the same size for other arrays.
     */
    if ( ms )
    {
        //std::clog << "Mecafil::allocatePoints " << ms << std::endl;
        allocateProjection(ms);
        
        // allocate memory:
        free_real(rfDiff);
        
        rfDiff = new_real(ms*(2*DIM+1));
        rfLag  = rfDiff + ms*DIM;
        rfLLG  = rfLag + ms;
        
        // reset Lagrange multipliers
        zero_real(ms, rfLag);
    }
    return ms;
}

void Mecafil::release()
{
    free_real(rfDiff);
    rfDiff = nullptr;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 The argument should be: sc = kT / dt;
 */
real Mecafil::addBrownianForces(real const* rnd, real sc, real* rhs) const
{
    real b = sqrt( 2 * sc / rfPointMobility );

    for ( unsigned jj = 0; jj < DIM*nPoints; ++jj )
        rhs[jj] += b * rnd[jj];
    
    return b * rfPointMobility;
}


//------------------------------------------------------------------------------

/**
 Calculate the normalized difference between successive vertices of the fiber:

     for ( int n = 0; n < DIM*lastPoint(); ++n )
         rfDiff[n] = ( pPos[n+DIM] - pPos[n] ) / segmentation();

 */

void Mecafil::storeDirections()
{
#if ( 1 )
    /*
     assume here that successive points are correctly separated, which is usally
     not the case, but the error is usually small
     */
    const real sc  = 1.0 / segmentation();
    const unsigned end = DIM * lastPoint();
    for ( unsigned p = 0; p < end; ++p )
        rfDiff[p] = sc * ( pPos[p+DIM] - pPos[p] );
#else
    for ( unsigned p = 0; p < lastPoint(); ++p )
        normalize(diffPoints(p)).store(rfDiff+DIM*p);
#endif
    
}



//------------------------------------------------------------------------------
#pragma mark - Project

#if ( DIM > 1 )
#     include "mecafil_project.cc"
#else

void Mecafil::buildProjection()   {}  //DIM == 1
void Mecafil::makeProjection()    {}  //DIM == 1
void Mecafil::destroyProjection() {}  //DIM == 1
void Mecafil::allocateProjection(size_t) {}  //DIM == 1

void Mecafil::projectForces(const real* X, real* Y) const
{
    real sum = X[0];
    for ( unsigned int ii = 1; ii < nPoints; ++ii )
        sum += X[ii];
    
    sum = sum / nPoints;
    for ( unsigned int ii = 0; ii < nPoints; ++ii )
        Y[ii] = sum;
}

void Mecafil::computeTensions(const real*) {} //DIM == 1
void Mecafil::makeProjectionDiff(const real*) {} //DIM == 1
void Mecafil::addProjectionDiff(const real*, real*) const {} //DIM == 1

#endif


void Mecafil::printTensions(FILE * out, char c) const
{
    fprintf(out, "\n%c%s ", c, reference().c_str());
    VecPrint::print(out, nbSegments(), rfLag, 2);
    fprintf(out, "  fM"); netForceEndM().print(out);
    fprintf(out, "  fP"); netForceEndP().print(out);
}


void Mecafil::getForces(const real* ptr)
{
    Mecable::getForces(ptr);
    //fprintf(stderr, "\nF "); VecPrint::print(stderr, DIM*nbPoints(), ptr, 2, DIM);
    computeTensions(ptr);
    //printTensions(stderr);
}

//-----------------------------------------------------------------------
#pragma mark -


/**
 Set elements of matrix `mat` corresponding to the elastic terms of the Fiber.
 The array `mat` must be square of dimension `dim * this->nPoints`
 Only terms above the diagonal and corresponding to the first subspace are set
 */
void add_rigidity_upper(unsigned cnt, real* mat, unsigned ldd, const real R1)
{
    const real R2 = R1 * 2;
    const real R4 = R1 * 4;
    const real R5 = R1 * 5;
    const real R6 = R1 * 6;

    constexpr unsigned D = DIM, T = DIM*2, V = DIM*3;
    const unsigned e = DIM * ( cnt - 2 );
    const unsigned f = DIM * ( cnt - 1 );
    
    mat[0      ] -= R1;
    mat[  ldd*D] += R2;
    mat[  ldd*T] -= R1;
    
    mat[e+ldd*f] += R2;
    mat[f+ldd*f] -= R1;
    
    if ( 3 < cnt )
    {
        mat[D+ldd*D] -= R5;
        mat[D+ldd*T] += R4;
        mat[D+ldd*V] -= R1;
        mat[e+ldd*e] -= R5;
    }
    else
    {
        mat[D+ldd*D] -= R4;
    }
    
    for ( unsigned n = T; n < e; n += D )
    {
        mat[n+ldd* n   ] -= R6;
        mat[n+ldd*(n+D)] += R4;
        mat[n+ldd*(n+T)] -= R1;
    }
}


void Mecafil::addRigidityUpper(real * mat, unsigned ldd) const
{
    if ( nPoints > 2 )
    {
        add_rigidity_upper(nPoints, mat, ldd, rfRigidity);
    }
}

//------------------------------------------------------------------------------

/*
 This is the reference implementation
 */
void add_rigidity0(const unsigned nbt, const real* X, const real rigid, real* Y)
{
    assert_true( X != Y );
    for ( unsigned jj = 0; jj < nbt; ++jj )
    {
        real f = rigid * (( X[jj+DIM*2] - X[jj+DIM] ) - ( X[jj+DIM] - X[jj] ));
        Y[jj      ] -=   f;
        Y[jj+DIM  ] += 2*f;
        Y[jj+DIM*2] -=   f;
    }
}

/*
 This is an optimized implementation
 */
void add_rigidityF(const unsigned nbt, const real* X, const real R1, real* Y)
{
    assert_true(nbt > DIM);
    const real R2 = R1 * 2;
    const real R4 = R1 * 4;
    const real R6 = R1 * 6;
    
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

/**
 Add rigidity terms between three points {A, B, C}
 Done with Serge DMITRIEFF, 2015
 */
void add_rigidity(unsigned A, unsigned B, unsigned C, const real* X, const real R1, real* Y)
{
    for ( unsigned d = 0; d < DIM; ++ d )
    {
        real x = 2 * X[B*DIM+d] - ( X[A*DIM+d] + X[C*DIM+d] );
        Y[A*DIM+d] += x * R1;
        Y[B*DIM+d] -= x * (R1+R1);
        Y[C*DIM+d] += x * R1;
    }
}

//------------------------------------------------------------------------------

/**
 calculates the second-derivative of point's coordinates,
 scale by the rigidity term, and add to vector Y
*/
void Mecafil::addRigidity(const real* X, real* Y) const
{
    if ( nPoints > 3 )
    {
        add_rigidityF(DIM*(nPoints-2), X, rfRigidity, Y);
    
#if NEW_FIBER_LOOP
        if ( rfRigidityLoop )
        {
            /*
             Done with Serge DMITRIEFF, 2015
             Link first and last point in the same way as all other points,
             making the fiber mechanically homogeneous and all points equivalent
             */
            const unsigned L = lastPoint();
            add_rigidity(L,   0, 1, X, rfRigidity, Y);
            add_rigidity(L-1, L, 0, X, rfRigidity, Y);
        }
#endif
    }
    else if ( nPoints > 2 )
    {
        add_rigidity(0, 1, 2, X, rfRigidity, Y);
    }
}

