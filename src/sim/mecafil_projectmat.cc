// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 projections performed with explicit matrices
 This is a slow method, but it can be useful to compare with other methods
*/
#include "vecprint.h"


void Mecafil::buildProjection()
{
    //reset all variables for the projections:
    mtProj       = nullptr;
    mtDiffP      = nullptr;
    mtJJtiJ      = nullptr;
    mtJJtiJforce = nullptr;
}


void Mecafil::allocateProjection(const size_t ms)
{
    //std::clog << reference() << "allocateProjection(" << nbp << ")\n";
    free_real(mtProj);
    free_real(mtDiffP);
    free_real(mtJJtiJ);
    const size_t N = DIM * ms;
    mtProj       = new_real(N*N);
    mtDiffP      = new_real(N*N);
    mtJJtiJ      = new_real(N*ms);
    mtJJtiJforce = new_real(ms);
}


void Mecafil::destroyProjection()
{
    //std::clog << reference() << "destroyProjection\n";
    free_real(mtProj);
    free_real(mtDiffP);
    free_real(mtJJtiJforce);
    free_real(mtJJtiJ);
    mtProj       = nullptr;
    mtDiffP      = nullptr;
    mtJJtiJ      = nullptr;
    mtJJtiJforce = nullptr;
}


#pragma mark -

/*
 Computes the projection matrix

     P = I - J' ( J J' )^-1 J

 that is associated with the length constraints:

     | point(p+1) - point(p) |^2 = lambda^2

 */
void Mecafil::makeProjection()
{
    const size_t nbc = nbSegments();             //number of constraints
    const size_t nbv = DIM * nbPoints();         //number of variables
    assert_true( nbc > 0 );
    
    //----- allocate needed temporaries:
    real* J    = new_real(nbv*nbc);
    real* JJt0 = new_real(nbc);
    real* JJt1 = new_real(nbc);
    
    //------------compute the projection matrix
    Vector v, w, dv, dw;
    zero_real(nbv*nbc, J);
    
    //set up the Jacobian matrix J and the diagonals of J * Jt
    w  = posP(0);
    dw.set(0,0,0);
    for ( size_t jj = 0; jj < nbc ; ++jj )
    {
        //set J:
        v = w;
        w = posP(jj+1);
        dv = -dw;
        dw = w - v;
        for ( unsigned d = 0; d < DIM ; ++d )
        {
            J[jj+nbc*(DIM*jj+d)    ] = -dw[d];
            J[jj+nbc*(DIM*jj+DIM+d)] =  dw[d];
        }
        
        //set the diagonal and off-diagonal term of JJt:
        JJt0[jj] = 2 * dw.normSqr();  // diagonal
        JJt1[jj] = dot(dv, dw);       // off-diagonal (first term not used)
    }
    
    // JJtiJ <- J
    //blas::xcopy( nbc * nbv, J, 1, mtJJtiJ, 1 );
    copy_real(nbc*nbv, J, mtJJtiJ);
    
    // JJtiJ <- inv( JJt ) * J
    int info = 0;
    lapack::xptsv(nbc, nbv, JJt0, JJt1, mtJJtiJ, nbc, &info);
    if ( info ) ABORT_NOW("lapack::ptsv() failed");
    
    // mtProj <-  -Jt * JJtiJ
    blas::xgemm('T', 'N', nbv, nbv, nbc, -1.0, J, nbc, mtJJtiJ, nbc, 0., mtProj, nbv );
    
    // mtProj <- mtProj + I
    for ( unsigned j = 0; j < nbv*nbv; j += nbv+1 )
        mtProj[j] += 1.0;
    
    free_real(J);
    free_real(JJt0);
    free_real(JJt1);
}


/**
 Attention, the vector 'X' and 'Y' may point to the same address!
 */
void Mecafil::projectForces(const real* X, real* Y) const
{
    const size_t nbv = DIM * nbPoints();
    copy_real(nbv, X, rfLLG);
    blas::xsymv('U', nbv, 1.0, mtProj, nbv, rfLLG, 1, 0.0, Y, 1);
    //blas::xgemv('N', nbv, nbv, 1.0, mtProj, nbv, rfLLG, 1, 0.0, Y, 1);
}


void Mecafil::printProjection(std::ostream& os) const
{
    const size_t nbv = DIM * nbPoints();
    os << reference() << '\n';
    VecPrint::print(os, nbv, nbv, mtProj, nbv);
}


void Mecafil::computeTensions(const real* force)
{
    const size_t nbs = nbSegments();
    const size_t nbv = DIM * nbPoints();
    
    // calculate the lagrangian multipliers:
    blas::xgemv('N', nbs, nbv, 1., mtJJtiJ, nbs, force, 1, 0., rfLag, 1);
}

//------------------------------------------------------------------------------
#pragma mark -

void Mecafil::makeProjectionDiff(const real* force)
{
    const size_t nbs = nbSegments();             //number of constraints
    const size_t nbv = DIM * nbPoints();         //number of variables
    
    // calculate the lagrangian coefficients:
    blas::xgemv('N', nbs, nbv, 1., mtJJtiJ, nbs, force, 1, 0., rfLag, 1);
    
    //printf("Lagrange: "); VecPrint::print(std::clog, nbc, rfLag);
    
    // select expensive forces ( lagrangian > 0 )
    useProjectionDiff = false;
    for ( size_t ii = 0; ii < nbs; ++ii )
    {
        if ( rfLag[ii] > 0 )
        {
            mtJJtiJforce[ii] = rfLag[ii];
            useProjectionDiff = true;
        }
        else
            mtJJtiJforce[ii] = 0.0;
    }
    
    //printf("diffP ");VecPrint::print(std::clog, nbs, mtJJtiJforce);
    
    //set up the first term in the derivative of J with respect to variable x[ii]
    //set up term  P * (DJ)t (JJti) J force:
    for ( size_t jj = 0; jj < nbv; ++jj )
    {
        real* coljj = mtDiffP + nbv * jj;
        zero_real(nbv, coljj);
        unsigned lin = jj / DIM;
        if ( lin > 0 ) {
            coljj[jj-DIM] = +mtJJtiJforce[lin-1];
            coljj[jj    ] = -mtJJtiJforce[lin-1];
        }
        if ( lin < nbs ) {
            coljj[jj    ] += -mtJJtiJforce[lin];
            coljj[jj+DIM]  = +mtJJtiJforce[lin];
        }
    }

    /*
     The final matrix is symmetric, for any force,
     as can be seen from the above relations to set its columns
     */
    //printf("projectionDiff\n");
    //VecPrint::print(std::clog, nbv, nbv, mtDiffP, nbv);
}


void Mecafil::addProjectionDiff( const real* X, real* Y ) const
{
    assert_true(useProjectionDiff);
    size_t nbv = DIM * nbPoints();
    blas::xsymv('U', nbv, 1.0, mtDiffP, nbv, X, 1, 1.0, Y, 1);
}

