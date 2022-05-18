// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BICGSTAB_H
#define BICGSTAB_H

#include "real.h"
#include "cblas.h"
#include "allocator.h"
#include "monitor.h"

/// Bi-Conjugate Gradient Stabilized method to solve a system of linear equations
/**
 F. Nedelec, 27.03.2012 - 13.03.2017
*/
namespace LinearSolvers
{
    /// Bi-Conjugate Gradient Stabilized without Preconditionning
    /*
     This solves `mat * x = rhs` with a tolerance specified in 'monitor'
     */
    template < typename LinearOperator, typename Monitor, typename Allocator >
    void BCGS(const LinearOperator& mat, const real* rhs, real* sol,
              Monitor& monitor, Allocator& allocator)
    {
        double rho = 1.0, rho_old = 1.0, alpha = 0.0, beta = 0.0, omega = 1.0;
        
        const int dim = mat.dimension();
        allocator.allocate(dim, 5);
        real * r  = allocator.bind(0);
        real * r0 = allocator.bind(1);
        real * p  = allocator.bind(2);
        real * t  = allocator.bind(3);
        real * v  = allocator.bind(4);
        
        mat.multiply(sol, r0);
        blas::xcopy(dim, rhs, 1, r, 1);
        blas::xaxpy(dim, -1.0, r0, 1, r, 1);    // r = rhs - A * x
        blas::xcopy(dim, r, 1, r0, 1);          // r0 = r
        
        rho = blas::dot(dim, r, r);
        blas::xcopy(dim, r, 1, p, 1);

        if ( monitor.finished(dim, r) )
            return;

        goto start;
        
        while ( ! monitor.finished(dim, r) )
        {
            rho_old = rho;
            rho = blas::dot(dim, r0, r);
            
            if ( rho == 0.0 )
            {
#if ( 1 )
                /* The residual vector became nearly orthogonal to the
                 arbitrarily chosen direction r0, and we restart with a new r0 */
                blas::xcopy(dim, rhs, 1, r, 1);         // r = rhs
                mat.multiply(sol, r0);                  // r0 = A*x
                blas::xaxpy(dim, -1.0, r0, 1, r, 1);    // r = rhs - A * x
                blas::xcopy(dim, r, 1, r0, 1);          // r0 = r
                rho = blas::dot(dim, r0, r0);
#else
                monitor.finish(2, dim, r);
                break;
#endif
            }
            
            beta = ( rho / rho_old ) * ( alpha / omega );
            // p = r + beta * ( p - omega * v )
            blas::xaxpy(dim, -omega, v, 1, p, 1);
#ifdef __INTEL_MKL__
            blas::xaxpby(dim, 1.0, r, 1, beta, p, 1);
#else
            blas::xscal(dim, beta, p, 1);
            blas::xaxpy(dim, 1.0, r, 1, p, 1);
#endif
        start:
            
            mat.multiply(p, v);                     // v = A * p;
            alpha = rho / blas::dot(dim, r0, v);

            blas::xaxpy(dim, -alpha, v, 1, r, 1);   // r = r - alpha * v;
            blas::xaxpy(dim,  alpha, p, 1, sol, 1); // x = x + alpha * p;
            
            //if ( monitor.finished(dim, r) )
            //    break;
            
            mat.multiply(r, t);                     // t = A * r;
            monitor+=2;

            double tdt = blas::dot(dim, t, t);
            
            if ( tdt > 0.0 )
            {
                omega = blas::dot(dim, t, r) / tdt;
                
                if ( omega == 0.0 )
                {
                    monitor.finish(3, dim, r);
                    break;
                }
                
                blas::xaxpy(dim,  omega, r, 1, sol, 1);  // x = x + omega * r
                blas::xaxpy(dim, -omega, t, 1, r, 1);    // r = r - omega * t
            }
            else
                omega = 0.0;
        }
        
#if ( 0 )
        // calculate true residual = rhs - A * x
        mat.multiply(sol, r0);
        blas::xaxpy(dim, -1.0, rhs, 1, r0, 1);
        real resid = blas::nrm2(dim, r0);
        fprintf(stderr, "BCGS  %4i count %4u residual %10.6f\n", dim, monitor.count(), resid);
#endif
        allocator.release();
    }
    
    
    /// Bi-Conjugate Gradient Stabilized with Preconditionning
    /*
     This solves `mat * x = rhs` with a tolerance specified in 'monitor'
     */
    template < typename LinearOperator, typename Monitor, typename Allocator >
    void BCGSP(const LinearOperator& mat, const real* rhs, real* sol,
               Monitor& monitor, Allocator& allocator)
    {
        double rho = 1.0, rho_old = 1.0, alpha = 0.0, beta = 0.0, omega = 1.0, delta;
        
        const int dim = mat.dimension();
        allocator.allocate(dim, 7);
        real * r    = allocator.bind(0);
        real * r0   = allocator.bind(1);
        real * p    = allocator.bind(2);
        real * t    = allocator.bind(3);
        real * v    = allocator.bind(4);
        real * phat = allocator.bind(5);
        real * shat = allocator.bind(6);
        
        mat.multiply(sol, r0);
        blas::xcopy(dim, rhs, 1, r, 1);
        blas::xaxpy(dim, -1.0, r0, 1, r, 1);    // r = rhs - A * x
        blas::xcopy(dim, r, 1, r0, 1);          // r0 = r
        
        rho = blas::dot(dim, r, r);
        blas::xcopy(dim, r, 1, p, 1);

        if ( monitor.finished(dim, r) )
            return;

        goto start;

        while ( ! monitor.finished(dim, r) )
        {
            rho_old = rho;
            rho = blas::dot(dim, r0, r);
            
            if ( rho == 0.0 )
            {
#if ( 1 )
                /* The residual vector became nearly orthogonal to the
                 arbitrarily chosen direction r0, and we restart with a new r0 */
                blas::xcopy(dim, rhs, 1, r, 1);
                mat.multiply(sol, r0);
                blas::xaxpy(dim, -1.0, r0, 1, r, 1);  // r = rhs - A * x
                blas::xcopy(dim, r, 1, r0, 1);        // r0 = r
                rho = blas::dot(dim, r0, r0);
#else
                monitor.finish(2, dim, r);
                break;
#endif
            }
            
            beta = ( rho / rho_old ) * ( alpha / omega );
            // p = r + beta * ( p - omega * v )
            blas::xaxpy(dim, -omega, v, 1, p, 1);
#ifdef __INTEL_MKL__
            blas::xaxpby(dim, 1.0, r, 1, beta, p, 1);
#else
            blas::xscal(dim, beta, p, 1);
            blas::xaxpy(dim, 1.0, r, 1, p, 1);
#endif
        start:
            
            mat.precondition(p, phat);                // phat = PC * p;
            mat.multiply(phat, v);                    // v = M * PC * p;

            delta = blas::dot(dim, r0, v);
            if ( delta == 0.0 )
            {
                ++monitor;
                monitor.finish(4, dim, r);
                break;
            }
            
            alpha = rho / delta;
            blas::xaxpy(dim, -alpha,    v, 1,   r, 1);// r = r - alpha * v;
            blas::xaxpy(dim,  alpha, phat, 1, sol, 1);// x = x + alpha * phat;

            mat.precondition(r, shat);                // shat = PC * r
            mat.multiply(shat, t);                    // t = M * PC * r
            monitor+=2;

            double tdt = blas::dot(dim, t, t);
            
            if ( tdt > 0.0 )
            {
                omega = blas::dot(dim, t, r) / tdt;
            
                if ( omega == 0.0 )
                {
                    monitor.finish(3, dim, r);
                    break;
                }
                
                blas::xaxpy(dim,  omega, shat, 1, sol, 1); // x = x + omega * shat
                blas::xaxpy(dim, -omega,    t, 1,   r, 1); // r = r - omega * t
            }
            else
                omega = 0.0;
        }
#if ( 0 )
        // calculate true residual = rhs - A * x
        mat.multiply(sol, r);
        blas::xaxpy(dim, -1.0, rhs, 1, r, 1);
        real resid = blas::nrm2(dim, r);
        fprintf(stderr, "BCGSP count %4u residual %10.6f\n", monitor.count(), resid);
#endif
        
        allocator.release();
    }
    
    
    /**
     This is an alternative implementation adapted from the CUPS project
     https://cusplibrary.github.io/index.html
     */
    template < typename LinearOperator, typename Monitor, typename Allocator >
    void bicgstab(const LinearOperator& mat, const real* rhs, real* sol,
                  Monitor& monitor, Allocator& allocator)
    {
        const int dim = mat.dimension();
        
        allocator.allocate(dim, 8);
        real * p     = allocator.bind(0);
        real * r     = allocator.bind(1);
        real * rstar = allocator.bind(2);
        real * s     = allocator.bind(3);
        real * Mp    = allocator.bind(4);
        real * AMp   = allocator.bind(5);
        real * Ms    = allocator.bind(6);
        real * AMs   = allocator.bind(7);
        
        // r <- A*x
        mat.multiply(sol, p);
        
        // r <- b - A*x
        blas::xcopy(dim, rhs, 1, r, 1);
        blas::xaxpy(dim, -1.0, p, 1, r, 1);
        
        // p <- r
        blas::xcopy(dim, r, 1, p, 1);
        
        // r_star <- r
        blas::xcopy(dim, r, 1, rstar, 1);
        
        double r_rstar_old = blas::dot(dim, rstar, r);
        
        while ( !monitor.finished(dim, r) )
        {
            // Mp = M*p
            mat.precondition(p, Mp);
            
            // AMp = A*Mp
            mat.multiply(Mp, AMp);

            // alpha = (r_j, r_star) / (A*M*p, r_star)
            double alpha = r_rstar_old / blas::dot(dim, rstar, AMp);
            
            // s_j = r_j - alpha * AMp
            blas::xcopy(dim, r, 1, s, 1);
            blas::xaxpy(dim, -alpha, AMp, 1, s, 1);
            
            if (monitor.finished(dim, s))
            {
                // x += alpha*M*p_j
                blas::xaxpy(dim, alpha, Mp, 1, sol, 1);
                ++monitor;
                break;
            }
            
            // Ms = M*s_j
            mat.precondition(s, Ms);
            
            // AMs = A*Ms
            mat.multiply(Ms, AMs);
            monitor+=2;

            // omega = (AMs, s) / (AMs, AMs)
            double omega = blas::dot(dim, AMs, s) / blas::dot(dim, AMs, AMs);
            
            // x_{j+1} = x_j + alpha*M*p_j + omega*M*s_j
            blas::xaxpy(dim, alpha, Mp, 1, sol, 1);
            blas::xaxpy(dim, omega, Ms, 1, sol, 1);
            
            // r_{j+1} = s_j - omega*A*M*s
            blas::xcopy(dim, s, 1, r, 1);
            blas::xaxpy(dim, -omega, AMs, 1, r, 1);

            // beta_j = (r_{j+1}, r_star) / (r_j, r_star) * (alpha/omega)
            double r_rstar_new = blas::dot(dim, rstar, r);
            double beta = (r_rstar_new / r_rstar_old) * (alpha / omega);
            r_rstar_old = r_rstar_new;
            
            // p_{j+1} = r_{j+1} + beta*(p_j - omega*A*M*p)
            blas::xaxpy(dim, -omega, AMp, 1, p, 1);
            blas::xscal(dim, beta, p, 1);
            blas::xaxpy(dim, 1.0, r, 1, p, 1);
        }
#if ( 0 )
        // calculate true residual = rhs - A * x
        mat.multiply(sol, r);
        blas::xaxpy(dim, -1.0, rhs, 1, r, 1);
        real resid = blas::nrm2(dim, r);
        fprintf(stderr, "bicgs %4i count %4u residual %10.6f\n", dim, monitor.count(), resid);
#endif
        allocator.release();
    }

}

#endif

