// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cstdio>
#include "real.h"
#include "random.h"
#include "quaternion.h"
#include "matrix33.h"
#include "vecprint.h"


/// calculate a distance to the subspace of rotations = maxNorm( M'*M - Id )
real maxDeviationFromRotation(Matrix33 const& mat)
{
    Matrix33 mm = mat.transposed() * mat - Matrix33::identity();
    return mm.norm_inf();
}


void testRotation(Vector3 vec, real angle)
{
    Matrix33 mat;
    Quaternion<real> q, p;
    Vector3 V, W, T;

    q.setFromAxis(vec, angle);
    real a = q.getAngle(T);
    
    printf("Quat ");
    q.print();
    printf("    norm %.2f ", q.norm() );
    
    printf(" |  %+6.2f %+6.2f %+6.2f  =", vec[0],vec[1],vec[2]);
    printf("%+6.2f %+6.2f %+6.2f  |  ", T[0],T[1],T[2]);
    
    q.setMatrix3(mat);
    //mat.print(stdout);
    printf("  deviation = %e", maxDeviationFromRotation(mat));
    
    Matrix33 rot = Matrix33::rotationAroundAxis(vec, cos(angle), sin(angle));
    
    printf("  error = %e  ", ( rot - mat ).norm_inf());
    printf("  angles  %+6.2f %+6.2f %+6.2f\n", angle, a, mat.rotationAngle());
    
    if ( 0 )
    {
        real vec[3] = { 0 };
        real m16[16];
        q.setOpenGLMatrix(m16, vec);
        VecPrint::print(std::cout, 4, 4, m16, 4);
    }
    if ( 0 )
    {
        V = Vector3::randS();
        W = mat*V;
        
        printf("   MATRIX*V            : ");
        W.println();
        
        p = q * Quaternion<real>(0, V.XX, V.YY, V.ZZ) * q.conjugated();
        printf("   q * (0, V) * inv(q) : ");
        p.println();
        
        q.rotateVector(W,V);
        printf("   Q.rotateVector(V)   : ");
        W.println();
    }
}


void test1()
{
    Matrix33 mat;
    Vector3 V, W;
    
    Quaternion<real> q, p;
    
    const real angle = M_PI/6.0;
    Vector3 vec(0,0,0);
    
    printf("------------------- rotations of PI/6 -----------------\n");
    
    for ( int ii = 0; ii < 16; ++ii )
    {
        vec = Vector3::randU();
        testRotation(vec, angle);
    }
    
    printf("------------------- identity ---------------------------\n");
    
    mat = Matrix33::identity();
    mat.print(stdout);
    q.setFromMatrix3(mat.data());
    q.println(stdout);
    
    printf("-------------- quat-quat multiplication ----------------\n");
    
    for ( int ii = 0; ii < 4; ++ii )
    {
        for ( int jj = 0; jj < 4; ++jj )
        {
            p = Quaternion<real>(0,0,0,0);
            p[ii] = 1;
            q = Quaternion<real>(0,0,0,0);
            q[jj] = 1;
            
            p.print();
            printf("  * ");
            q.print();
            printf("  = ");
            (p*q).println();
        }
        printf("\n");
    }
    
    printf("----------------- conversion quat-mat-quat --------------\n");
    
    real error = 0, e;
    for ( int ii = 0; ii < 1000; ++ii )
    {
        vec = Vector3::randU();
        real a = RNG.sreal() * M_PI;
        p.setFromAxis(vec, a);
        p.setMatrix3(mat);
        q.setFromMatrix3(mat.data());
        //printf("%f  :", a); p.print(); q.println();
        if ( q[0] * p[0] < 0 ) q = -q;
        e = (q-p).norm();
        if ( e > error ) error = e;
    }
    printf("  max error = %e\n", error);
    
    printf("------------ rotation mult. is not commutative -----------\n");
    
    for ( int ii = 0; ii<3; ++ii )
    {
        for ( int jj = 0; jj<3; ++jj )
        {
            q.setFromPrincipalAxis( ii, angle );
            p.setFromPrincipalAxis( jj, angle );
            
            (q*p).print(stdout);
            (p*q).println(stdout);
        }
    }
            
    printf("------------ rotation around principal axes -------------\n");

    for ( int ii = 0; ii<3; ++ii )
    {
        q.setFromPrincipalAxis(ii, angle);
        q.setMatrix3(mat);
        mat.print(stdout);
        printf("\n");
    }
}

void test2(const int max)
{
    //this test a way to generate a random matrix:
    Quaternion<real> Q;
    Quaternion<real> pos;
    Matrix33 rot;
    Vector3 vec;
    
    for(int i = 0; i < max; ++i)
    {
        Q = Quaternion<real>::randomRotation();
        pos = Q * Quaternion<real>(0,1,0,0) * Q.conjugated();
        rot = Matrix33::randomRotation();
        vec = rot * Vector3(0,0,1);
        vec.println();
    }
}


int main(int argc, char* argv[])
{
    RNG.seed();
    test1();
    //test2(10000);
    return 0;
}
