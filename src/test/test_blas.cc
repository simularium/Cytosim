// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cstdio>

#include "real.h"
#include "cblas.h"
#include "clapack.h"


void test_blas(const int size)
{
    real * x = new_real(size);
    real * y = new_real(size);
    real * z = new_real(size);
    
    for ( int i = 0; i < size; ++i )
        x[i] = i;
    
    zero_real(size, y);
    blas::xcopy(size, x, 1, y, 1);
    blas::xcopy(size, x, 1, z, 1);
    blas::xscal(size, +3.14, z, 1);
    blas::xaxpy(size, -3.14, x, 1, z, 1);
    
    real sum = blas::xasum(size, z, 1);
    printf("zero = %f\n", sum);
    
    real nrm = blas::dot(size, x, y);
    printf("nrm^2 = %f\n", nrm);
    
    nrm = blas::nrm2(size, x);
    printf("nrm^2 = %f\n", nrm*nrm);
    
    free_real(z);
    free_real(y);
    free_real(x);
}


void test_lapack(const int size)
{
    real* mat = new real[size*size];
    int* ipiv = new int[size];
    
    for ( int ii = 0; ii < size; ++ii )
    {
        for ( int jj = 0; jj < size; ++jj )
            mat[ii+size*jj] = (ii+1) * (ii==jj);
    }
    
    int info = 0;
    int work_size = 1024;

    if ( 1 )
    {
        real tmpA, tmpW;
        lapack::xgetri(size, &tmpA, size, ipiv, &tmpW, -1, &info);
        if ( info == 0 )
        {
            work_size = (int)tmpW;
            printf("Lapack::dgetri optimal size is %i\n", work_size);
        }
    }
    
    real* work  = new real[work_size];


    lapack::xgetf2(size, size, mat, size, ipiv, &info);
    printf("lapack::dgetff returned %i\n", info);
    
    lapack::xgetri(size, mat, size, ipiv, work, work_size, &info);
    printf("lapack::getri returned %i\n", info);
    
    for ( int ii = 0; ii < size; ++ii )
    {
        for ( int jj = 0; jj < size; ++jj )
            printf("%9.5f ", mat[ii+size*jj]);
        printf("\n");
    }
    
    delete[] work;
    delete[] ipiv;
    delete[] mat;
}


int main(int argc, char* argv[])
{
    printf("BLAS:\n");
    test_blas(10);
    
    printf("\nLAPACK:\n");
    test_lapack(10);

    printf("\ndone!\n");
    return 0;
}
