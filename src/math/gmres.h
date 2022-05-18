// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GMRES_H
#define GMRES_H

#include "real.h"
#include "cblas.h"
#include "allocator.h"
#include "monitor.h"
//#include "vecprint.h"

/**
 With this option set to 1, GMRES iterates with M*P instead of P*M,
 where M is the matrix and P the preconditionner.
 In both cases, the solution to M*sol = rhs is returned.
*/
#define RIGHTSIDED_PRECONDITIONNER 0

/// GMRES method to solve a system of linear equations
/**
 F. Nedelec, started 13.03.2018 and last updated 24.04.2018
*/
namespace LinearSolvers
{

    inline void gmres_rotate(real& dx, real& dy, real C, real S)
    {
        real tmp = C * dx + S * dy;
        dy = -S * dx + C * dy;
        dx = tmp;
    }
    
    inline void gmres_make_rotation(real dx, real dy, real& C, real& S)
    {
        if ( dy == 0.0 )
        {
            C = 1.0;
            S = 0.0;
        }
        else if ( fabs(dy) > fabs(dx) )
        {
            real t = dx / dy;
            S = 1.0 / sqrt(1.0 + t*t);
            C = t * S;
        } else
        {
            real t = dy / dx;
            C = 1.0 / sqrt(1.0 + t*t);
            S = t * C;
        }
    }
    
    inline void gmres_make_rotation(Matrix& H, real C[], real S[], real ss[], int i)
    {
        for ( int k = 0; k < i; ++k )
            gmres_rotate(H(k,i), H(k+1,i), C[k], S[k]);
        gmres_make_rotation(H(i,i), H(i+1,i), C[i], S[i]);
        gmres_rotate(H(i,i), H(i+1,i), C[i], S[i]);
        gmres_rotate(ss[i], ss[i+1], C[i], S[i]);
    }


    
    /// GMRES with Preconditionning
    /*
     This solves `mat * x = rhs` with a tolerance specified in 'monitor'
     The matrix and its preconditionner are specified by functions of LinearOperator
     */
    template < typename LinearOperator, typename Monitor, typename Allocator >
    void GMRES(const LinearOperator& mat, const real* rhs, real* sol, int restart,
               Monitor& monitor, Allocator& allocator, Matrix& H, Matrix& V,
               Allocator& temporary)
    {
        const int dim = mat.dimension();
        real beta, resid, ratio = 1.0;
        
        //allocate workspace
        allocator.allocate(dim, 2);
        real * ww = allocator.bind(0);
        real * tt = allocator.bind(1);

        // Arnoldi matrix
        V.resize(dim, restart);
        
        temporary.allocate(restart+1, 3);
        real * sn = temporary.bind(0);
        real * cs = temporary.bind(1);
        real * ss = temporary.bind(2);  // vector of dim 'restart+1'
        
        // Hessenberg matrix
        H.resize(restart+1, restart);
        
        V.reset();
        //std::clog << "norm_rhs = " << blas::nrm2(dim, rhs) << '\n';

        while ( 1 )
        {
            ++monitor;
            // compute initial residual and its norm
#if RIGHTSIDED_PRECONDITIONNER
            mat.precondition(sol, tt);              // tt = P*x
            mat.multiply(tt, ww);                   // ww = M*P*x
            blas::xaxpy(dim, -1.0, rhs, 1, ww, 1);  // ww = ww - rhs = M*P*x - rhs
            // we get here the true residual, since P*x will be the solution:
            resid = blas::nrm2(dim, ww);
            if ( monitor.finished(resid) )
                break;
            beta = resid;                           // beta = norm(ww)
#else
            mat.multiply(sol, tt);                  // tt = M*x
            blas::xaxpy(dim, -1.0, rhs, 1, tt, 1);  // tt = tt - rhs = M*x - rhs
            // we get here the true residual:
            resid = blas::nrm2(dim, tt);
            // check for convergence:
            if ( monitor.finished(resid) )
                break;
            mat.precondition(tt, ww);            // ww = P * tt = P * ( M*x - rhs )
            beta = blas::nrm2(dim, ww);          // beta = norm(ww)
            ratio = resid / beta;                // ratio ~ x / Px
#endif
            blas::xscal(dim, -1.0/beta, ww, 1);  // ww = -ww/beta
            blas::xcopy(dim, ww, 1, V.column(0), 1);
            zero_real(restart+1, ss);            // ss = 0

            ss[0] = beta;
            int it = -1;
            //fprintf(stderr, "GMRES   %4i residual %10.6f\n", monitor.count(), resid);

            do {
                ++it;
                ++monitor;
#if RIGHTSIDED_PRECONDITIONNER
                mat.precondition(ww, tt);      // tt = P*ww
                mat.multiply(tt, ww);          // ww = M*tt = M*P*ww
#else
                mat.multiply(ww, tt);          // tt = M*ww
                mat.precondition(tt, ww);      // ww = P*tt = P*M*ww
#endif
                /*
                 The next loop has a dependency for 'ww', but because the columns
                 of V are orthogonal to each other, the scalar products H(k,it)
                 could be calculated independently in parallel
                */
                for (int k = 0; k <= it; ++k)
                {
                    // H(k,i) = <V(i+1), V(k)>
                    H(k,it) = blas::dot(dim, ww, V.column(k));
                    // V(i+1) -= H(k, i) * V(k)
                    blas::xaxpy(dim, -H(k,it), V.column(k), 1, ww, 1);
                }
                
                real nn = blas::nrm2(dim, ww);
                H(it+1,it) = nn;
                
                // V(i+1) = V(i+1) / H(i+1, i)
                blas::xscal(dim, 1.0/nn, ww, 1);
                if ( it+1 < restart )
                    blas::xcopy(dim, ww, 1, V.column(it+1), 1);
                
                gmres_make_rotation(H, cs, sn, ss, it);
                
                /*
                 here fabs(ss[it+1]) = norm(P*residual)
                 and we use `ratio` to estimate the residual of the problem
                 */
                resid = fabs(ss[it+1]) * ratio;
                
                //fprintf(stderr, " |ss[it+1]| = %10.6f   est. %10.6f\n", fabs(ss[it+1]), resid);

                if ( monitor.finished(resid) )
                {
                    //fprintf(stderr, " %4i finished?  %10.6f   est. %10.6f\n", monitor.count(), fabs(ss[it+1]), resid);
                    break;
                }
                
            } while ( it+1 < restart );
            
            // solve upper triangular system in place
            for (int j = it; j >= 0; --j)
            {
                ss[j] /= H(j,j);
                // S(0:j) = S(0:j) - ss[j] * H(0:j,j)
                blas::xaxpy(j, -ss[j], H.column(j), 1, ss, 1);
                //for (int k = 0; k < j; ++k)
                //    ss[k] -= H(k,j) * ss[j];
            }

            // update the solution `sol`: can be parallelized
            for (int j = 0; j <= it; ++j)
            {
                // sol = sol + ss[j] * V(j)
                blas::xaxpy(dim, ss[j], V.column(j), 1, sol, 1);
                //for (int k = 0; k < dim; ++k)
                //    sol[k] += V(k,j) * ss[j];
            }
        }
#if RIGHTSIDED_PRECONDITIONNER
        // we have calculated the solution to M*P*sol = rhs, and we need P*sol:
        mat.precondition(sol, tt);
        blas::xcopy(dim, tt, 1, sol, 1);
#endif
#if ( 0 )
        // calculate true residual = rhs - M * x
        mat.multiply(sol, tt);
        blas::xaxpy(dim, -1.0, rhs, 1, tt, 1);
        real r = blas::nrm2(dim, tt);
        fprintf(stderr, "GMRES count %4u true residual %10.6f\n", monitor.count(), r);
#endif
#if ( 0 )
        fprintf(stderr, "GMRES count %4u residual %10.6f\n", monitor.count(), resid);
#endif
        //allocator.release();
    }
}

#endif

