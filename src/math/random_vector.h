// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef RANDOM_VECTOR_H
#define RANDOM_VECTOR_H

#include <vector>
#include "vector.h"

/**
 Functions to generate single random vectors are members of Vector1, Vector2, etc.
 and defined in random_vector.cc
 */


/// Distribute points on the unit disc, at distance never below `sep`.
size_t tossPointsDisc(std::vector<Vector2>& pts, real sep, size_t limit_trials);


/// Distribute points on the unit disc, over a cap of solid-angle `cap`.
size_t tossPointsCap(std::vector<Vector3>& pts, real cap, real sep, size_t limit_trials);


/// Distribute points on the unit sphere, at distance never below `sep`.
/**
 Generate a random distribution of points on the unit circle,
 with the distance between two points never below `sep`.
 @return number of points stored in 'pts[]'
 */
template <typename VECTOR>
size_t tossPointsSphere(std::vector<VECTOR>& pts, real sep, size_t limit_trials)
{
    const real ss = sep * sep;
    size_t ouf = 0;
    size_t n = 0;
    
    VECTOR pos;
    for ( VECTOR& vec : pts )
    {
    toss:
        if ( ++ouf > limit_trials )
            break;
        
        pos = VECTOR::randU();
        
        // check distance will all the other points:
        for ( size_t i = 0; i < n; ++i )
            if ( distanceSqr(pos, pts[i]) < ss )
                goto toss;
        
        vec = pos;
        ouf = 0;
        ++n;
    }
    return n;
}

#endif

