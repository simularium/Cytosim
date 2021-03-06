// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef POINTSONSPHERE_H
#define POINTSONSPHERE_H

#include "real.h"
#include <cstdio>

///\todo we could replace here the Coulomb repulsive interaction by a linear force
/* 
A linear forces will allow us to solve an associated linear system on 
the coordinates of the point, using an interative solver.
that might be quite fast. The repulsive interaction only need to take
the neighbour into account, so we could have local force only, which
would scale better than having all points interact (Coulomb)
*/

/// Distribute points on the unit sphere, minimizing the 'electrostatic' energy
/**  The number of points is arbitrary, see
http://mathworld.wolfram.com/SphericalCode.html
\n

Algorithm:
 -# The points are distributed randomly on the sphere
 -# A 1/r^3 repulsive force is assumed for all points, 
 to which corresponds a certain potential energy in 1/r^2
 -# New positions are calculated form the current one, the forces
 and an adjustable scaling factor:  dx = scale * ( force at x )
 -# The potential energy of the new configuration is calculated, and:
     - If the total energy is lower, the move is accepted and the scaling factor is increased,
     - If the energy is higher, the move is rejected and the scaling factor is reduced.
     .
 .
 
The procedure (steps 2-4) is continues until convergence.\n

The main method is the class constructor, or equivalently distributePoints(),
which take the number of points as argument and makes the calculation.
Points coordinates can then be retrieved using either:
    - copyPositionsForAllPoints()
    - copyCoordinatesOfPoint()

\author
F. Nedelec, created August 2002, last modified May, 2014
*/
class PointsOnSphere
{
public:
    
    /// a-priori expected distance between neighboring points, as a function of number of points
    static real expectedDistance(unsigned);

    /// default constructor, does nothing
    PointsOnSphere();
    
    /// constructor that also calls distributePoints(),
    PointsOnSphere(unsigned nbp);
    
    /// constructor that also calls distributePoints(), 
    PointsOnSphere(unsigned nbp, real precision, unsigned mx_nb_iterations);
    
    /// distribute the nbp points on the sphere and store their coordinates
    unsigned distributePoints(unsigned nbp, real precision, unsigned mx_nb_iterations);

    /// default destructor
    virtual ~PointsOnSphere();
    
    /// number of points in the configuration
    unsigned nbPoints()  const { return num_points_;  }
    
    /// the 'virtual' total energy of the configuration
    real   finalEnergy() const { return energy_; }
    
    /// minimum distance in the actual configuration, in 3D space
    real   minimumDistance();
    
    /// multiply all coordinates by `factor`
    void   scale(real factor);
    
    /// address where the coordinates for point `inx` are
    const real* addr(const unsigned inx) const { return &coord_[3 * inx]; }
    
    /// copy the coordinates from point `inx` onto the given 3-dim array x
    void   copyPoint(real x[3], unsigned inx);
    
    /// copy the coordinates from point `inx` onto x,y,z
    void   copyPoint(real* x, real* y, real* z, unsigned inx);
    
    /// copy the points coordinates onto `x`
    void   copyPoints(real x[], const unsigned x_size);
    
    /// write points coordinates
    void   printAllPositions(FILE* file = stdout);
    
    
private:

    /// This number affects convergence speed but not the result
    static constexpr unsigned SEVEN = 7;
    
    /// number of point on the sphere
    unsigned num_points_;
    
    /// coordinates of the points in a array
    /** in the array all the coordinates are together (x,y,z) point 1, (x,y,z) point 2, etc.
     so the coordinates for the first point are:
     x = coord_[0], y = coord_[1], z = coord_[3]
     the coordinates of point `i` are:
     x = coord_[3*i+0], y = coord_[3*i+1], z = coord_[3*i+2]
     */
    real* coord_;
    
    /// Coulomb energy of current configuration
    real energy_;
    
    /// project point on the sphere
    bool project(const real W[3], real P[3]);
    
    /// set coordinates point P randomly on the sphere
    void randomize(real P[3]);
    
    /// Calculate distance between point given their coordinates P and Q (3-dim)
    real distance3(const real P[], const real Q[]);
    
    /// Calculate distance between point given their coordinates P and Q (3-dim)
    real distance3Sqr(const real P[], const real Q[]);
    
    /// calculate Coulomb energy
    real coulombEnergy(const real P[]);
    
    /// calculate Coulomb forces
    void setForces(real forces[], real threshold);
    
    /// move point from old to new coordinates
    void refinePoints(real Pnew[], const real Pold[], real forces[], real S);

};

#endif
