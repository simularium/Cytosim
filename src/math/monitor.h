// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MONITOR_H
#define MONITOR_H

#include "real.h"
#include "cblas.h"

/// Iterative methods to solve a system of linear equations
namespace LinearSolvers
{
    /// records the number of iterations, and the convergence
    class Monitor
    {
    private:
        
        /// exit flag
        int      flag_;
        
        /// maximum allowed number of iterations
        size_t   limit_;

        /// counter for iterations or number of matrix-vector operations
        size_t   cnt_, cntOld_;
        
        /// desired residual
        real     target_;

        /// residual achieved in last step
        real     res_;
        
        /// residual from the past
        real     resOld_;
        
    public:
        
        /// set the maximum number of iterations, and the residual threshold
        Monitor(size_t i, real r) { reset(); limit_=i; target_=r; }
        
        /// reset state variables (counters, flags and residual)
        void reset() { flag_=0; cnt_=0; res_=INFINITY; cntOld_=0; resOld_=INFINITY; }
        
        /// increment counter
        void operator ++() { ++cnt_; }
        
        /// increment counter by `i`
        void operator +=(size_t i) { cnt_ += i; }
       
        /// value of return flag
        int flag()       const { return flag_; }
        
        /// set flag to `f`
        void flag(const int f) { flag_ = f; }

        /// iteration count
        size_t count() const { return cnt_; }
        
        /// last achieved residual
        real residual()  const { return res_; }
        
        /// true if achieve residual < residual threshold
        bool converged() const { return res_ < target_; }
        
        /// check given residual and return true if threshold is achieved
        bool finished(real res)
        {
            res_ = res;
            
            if ( cnt_ > limit_ )
                return true;
            
            return ( res < target_ );
        }

        /// calculate residual from `x` and return true if threshold is achieved
        bool finished(unsigned size, const real* x)
        {
            //fprintf(stderr, "Solver %4lu residual %9.6f %9.6f\n", cnt_, blas::nrm2(size, x), blas::nrm8(size, x));
#if ( 1 )
            // use the 'infinite' norm (i.e. the largest element)
            real res = blas::nrm8(size, x);
#else
            // use the standard Euclidian norm:
            real res = blas::nrm2(size, x);
#endif
#if ( 1 )
            // monitor convergence: the residual from a linear solver may occasionally 
            if ( 1 == (16&cnt_) )
            {
                if ( res > 2*resOld_ )
                {
                    printf("Warning: slow convergence (reduce time_step?)");
                    printf(" residual %.3e at iteration %lu, %.3e at %lu\n", resOld_, cntOld_, res, cnt_);
                }
                resOld_ = res;
                cntOld_ = cnt_;
            }
#endif
            
            if ( res != res )
            {
                fprintf(stderr, "Solver diverged at step %3lu (residual is not a number)\n", cnt_);
                return true;
            }
            
            return finished(res);
        }

        /// calculate residual from `x` and set flag to `f`
        void finish(int f, unsigned size, const real* x) { flag_ = f; finished(size, x); }
    };
}

#endif

