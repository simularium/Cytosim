// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef LINEAR_OPERATOR_H
#define LINEAR_OPERATOR_H

#include "real.h"
#include "cblas.h"

/// Iterative methods to solve a system of linear equations
namespace LinearSolvers
{
    /// interface for a linear system
    class LinearOperator
    {
    public:
        /// size of the matrix M
        virtual int dimension() const = 0;
        
        /// multiply a vector ( Y <- M * X )
        virtual void multiply(const real* X, real* Y) const = 0;
        
        /// transposed multiply a vector ( Y <- transpose(M) * X )
        virtual void trans_multiply(const real* X, real* Y) const
        {
            // not necessary for some solvers
        }
        
        /// apply preconditionning ( Y <- P * X )
        virtual void precondition(const real* X, real* Y) const
        {
            copy_real(dimension(), X, Y);
        }

    };
}

#endif

