// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//Vector is defined as one of Vector1, 2 or 3, according to DIM
//Vector1, 2 or 3 are still available

#ifndef VECTOR_H
#define VECTOR_H

#include "dim.h"
#include "vector1.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

#if ( DIM == 1 )

typedef Vector1 Vector;
typedef real    Torque;
const   Torque  nullTorque(0);

/// helper function to normalize a 'Torque'
inline Torque normalize(Torque x) { return std::copysign(1.0, x); }

#elif ( DIM == 2 )

typedef Vector2 Vector;
typedef real    Torque;
const   Torque  nullTorque(0);

/// helper function to normalize a 'Torque'
inline Torque normalize(Torque x) { return std::copysign(1.0, x); }

#elif ( DIM == 3 )

typedef Vector3 Vector;
typedef Vector3 Torque;
const   Torque  nullTorque(0,0,0);

#endif

#endif

