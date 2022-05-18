// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "pointsonsphere.h"
#include "random.h"

//------------------------------------------------------------------------------
PointsOnSphere::PointsOnSphere()
: num_points_(0), coord_(nullptr)
{
}


PointsOnSphere::PointsOnSphere(unsigned nbp, real precision, unsigned mx_nb_iterations)
: num_points_(0), coord_(nullptr)
{
    distributePoints(nbp, precision, mx_nb_iterations);
}


PointsOnSphere::PointsOnSphere(unsigned nbp)
: num_points_(0), coord_(nullptr)
{
    distributePoints(nbp, 1e-4, 1<<14);
}


PointsOnSphere::~PointsOnSphere( )
{
}


void PointsOnSphere::copyPoint( real x[3], const unsigned ii )
{
    x[0] = coord_[3*ii+0];
    x[1] = coord_[3*ii+1];
    x[2] = coord_[3*ii+2];
}


void PointsOnSphere::copyPoint( real* x, real* y, real* z, const unsigned ii )
{
    *x = coord_[3*ii+0];
    *y = coord_[3*ii+1];
    *z = coord_[3*ii+2];
}


void PointsOnSphere::copyPoints( real x[], const unsigned x_size )
{
    for ( unsigned ii = 0; ii < 3*num_points_ && ii < x_size; ++ii )
        x[ii] = coord_[ii];
}


void PointsOnSphere::scale(const real factor)
{
    for ( unsigned ii = 0; ii < 3*num_points_; ++ii )
        coord_[ii] *= factor;
}


void PointsOnSphere::printAllPositions( FILE* file )
{
    for ( unsigned ii = 0; ii < num_points_; ++ii )
        fprintf( file, "%f %f %f\n", coord_[3*ii], coord_[3*ii+1], coord_[3*ii+2]);
}


real PointsOnSphere::distance3( const real P[], const real Q[] )
{
    return sqrt( (P[0]-Q[0])*(P[0]-Q[0]) + (P[1]-Q[1])*(P[1]-Q[1]) + (P[2]-Q[2])*(P[2]-Q[2]) );
}


real PointsOnSphere::distance3Sqr( const real P[], const real Q[] )
{
    return (P[0]-Q[0])*(P[0]-Q[0]) + (P[1]-Q[1])*(P[1]-Q[1]) + (P[2]-Q[2])*(P[2]-Q[2]);
}

//------------------------------------------------------------------------------


bool PointsOnSphere::project(const real S[3], real P[3])
{
    real n = S[0]*S[0] + S[1]*S[1] + S[2]*S[2];
    if ( n > 0 )
    {
        n = sqrt(n);
        P[0] = S[0] / n;
        P[1] = S[1] / n;
        P[2] = S[2] / n;
        return false;
    }
    return true;
}


/**
 hypercube rejection method
 */
void PointsOnSphere::randomize(real P[3])
{
    real n;
    do {
        P[0] = RNG.sreal();
        P[1] = RNG.sreal();
        P[2] = RNG.sreal();
        n = P[0]*P[0] + P[1]*P[1] + P[2]*P[2];
        if ( n == 0 )
        {
            fprintf(stderr, "The Random Number Generator may not be properly initialized");
        }
    } while ( n > 1.0 );
    
    n = sqrt(n);
    P[0] /= n;
    P[1] /= n;
    P[2] /= n;
}


//------------------------------------------------------------------------------
/**
 With N points on the sphere according to a triagular lattice, 
 each of ~2N triangles should occupy an area of S = 4*PI/2*N, 
 and the distance between points should be ~2 * sqrt(S/sqrt(3)).
 */
real PointsOnSphere::expectedDistance(unsigned n)
{
    real surface = 2 * M_PI / n;
    return 2 * sqrt( surface / sqrt(3) );
}


real PointsOnSphere::minimumDistance()
{
    real res = INFINITY;
    for ( unsigned ii = 1; ii < num_points_; ++ii )
    {
        for ( unsigned jj = 0; jj < ii; ++jj )
        {
            real dis = distance3Sqr(&coord_[3*ii], &coord_[3*jj]);
            if ( dis < res )
                res = dis;
        }
    }
    return sqrt(res);
}


real PointsOnSphere::coulombEnergy( const real P[] )
{
    real dist, result = 0;
    for ( unsigned ii = 1; ii < num_points_; ++ii )
    {
        for ( unsigned jj = 0; jj < ii; ++jj )
        {
            dist = distance3( P + 3 * ii, P + 3 * jj );
            if ( dist > 0 ) result += 1.0 / dist;
        }
    }
    return result;
}

//------------------------------------------------------------------------------

void PointsOnSphere::setForces( real forces[], real threshold )
{
    real dx[3];
    real dist;
    
    //--------- reset forces:
    for ( unsigned ii = 0; ii < 3 * num_points_; ++ii )
        forces[ii] = 0.0;
    
    //--------- calculate Coulomb pair interactions:
    // first particle is ii, second one is jj:
    for ( unsigned ii = 1; ii < num_points_; ++ii )
    {
        for ( unsigned jj = 0; jj < ii; ++jj )
        {
            //calculate vector and distance^2 between from jj to ii
            dist = 0;
            for ( int dd = 0; dd < 3 ; ++dd )
            {
                dx[dd] = coord_[3*ii+dd] - coord_[3*jj+dd];
                dist += dx[dd] * dx[dd];
            }
            
            if ( dist == 0 )
            {   //if ii and jj overlap, we use a random force
                for ( int dd = 0 ; dd < 3; ++dd )
                {
                    dx[dd] = 0.1 * RNG.sreal();
                    forces[3*ii+dd] += dx[dd];
                    forces[3*jj+dd] -= dx[dd];
                }
            }
            else if ( dist < threshold )
            {
                // points do not overlap:
                //force = vector / r^3, but here dist = r^2
                dist = 1.0 / ( dist * sqrt(dist) );
                //update forces for jj and ii:
                for ( unsigned dd = 0 ; dd < 3; ++dd )
                {
                    dx[dd] *= dist;
                    forces[3*ii+dd] += dx[dd];
                    forces[3*jj+dd] -= dx[dd];
                }
            }
        }
    }
    

#if ( 1 )
    /*
     Remove centripetal contribution of forces:
     assuming here that points are already on the sphere (norm=1)
     ( the algorithm converge even without this, but slower )
     */
    for ( unsigned ii = 0; ii < num_points_; ++ii )
    {
        dist = 0;
        for ( int dd = 0; dd < 3; ++dd )
            dist += coord_[3*ii+dd] * forces[3*ii+dd];
        
        for ( int dd = 0; dd < 3; ++dd )
            forces[3*ii+dd] -= dist * coord_[3*ii+dd];
    }
#endif
}


/**
 Move the points in the direction of the forces, with scaling factor S 
 */
void PointsOnSphere::refinePoints( real Pnew[], const real Pold[], real forces[], real S )
{
    for ( unsigned ii = 0; ii < num_points_; ++ii )
    {
        real W[3];
        
        W[0] = Pold[3*ii+0] + S * forces[3*ii+0];
        W[1] = Pold[3*ii+1] + S * forces[3*ii+1];
        W[2] = Pold[3*ii+2] + S * forces[3*ii+2];
        
        if ( project(W, Pnew+3*ii) )
            randomize(Pnew+3*ii);
    }
}


/**
 create a relatively even distribution of nbp points on the sphere
 the coordinates are stored in real array coord_[]
 */
unsigned PointsOnSphere::distributePoints(unsigned nbp, real precision, unsigned mx_nb_iterations)
{
    //reallocate the array if needed:
    if ( num_points_ != nbp || ! coord_ )
    {
        free_real(coord_);
        coord_ = new_real(3*nbp);
        num_points_ = nbp;
    }
    
    // the precision is rescaled with the expected distance:
    real distance = expectedDistance(nbp);
    
    /* 
     Threshold cut-off for repulsive force:
     The best results are obtained for threshold > 2
     */
    real threshold = 10 * distance;
    real mag = 0.1 * distance * distance * distance * distance / num_points_;
    precision *= mag;

    //------------ distribute the points randomly on the sphere:
    for ( unsigned ii = 0; ii < num_points_; ++ii )
        randomize(coord_+ii*3);
    
    //--------- for one point only, we return:
    if ( num_points_ < 2 )
    {
        energy_ = 0;
        return 0;
    }
    
    //------------ calculate the initial energy:
    energy_ = coulombEnergy(coord_);
    
    // allocate forces and new coordinates:
    real * coord = new_real(3*num_points_);
    real * force = new_real(3*num_points_);
    
    //make an initial guess for the step size:
    unsigned history = 0;
    
    unsigned step = 0;
    for ( step = 0; step < mx_nb_iterations; ++step )
    {
        setForces(force, threshold);
        
        while ( 1 ) 
        {
            refinePoints(coord, coord_, force, mag);
            
            // energy of new configuration:
            real energy = coulombEnergy(coord);
            
            //printf("%3i : step %5i : energy_ = %18.8f   mag = %8.5f %s\n",
            //     nbp, step, energy_, mag, (energy_new<energy_?"yes":"no"));

            if ( energy < energy_ )
            {
                // swap pointers to accept configuration:
                real* m = coord_;
                coord_  = coord;
                coord   = m;
                energy_ = energy;
                
                /*
                 After 'SEVEN' successful moves at a given step size, we increase
                 the step size. Values for 'magic_seven' were tested in term of
                 convergence, and 7 seems to work well.
                 */
                if ( ++history >= SEVEN )
                {
                    mag *= 1.4147;   //this value is somewhat arbitrary
                    history = 0;
                }
                break;
            }
            else
            {
                /*
                 If the new configuration has higher energy,
                 we try a smaller step size with the same forces:
                 */
                history = 0;
                mag /= 2;
                
                //exit when the desired precision is reached
                if ( mag < precision )
                {
                    free_real(coord);
                    free_real(force);
                    return step;
                }
            }
        }
    }
    free_real(coord);
    free_real(force);
    return step;
}

