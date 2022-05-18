// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "matrix11.h"
#include "random.h"


real Matrix11::rotationAngle() const
{
    if ( val_ > 0 )
        return 0;
    else
        return M_PI;
}


/// returns a rotation of angle PI around axis Z
Matrix11 Matrix11::rotation180()
{
    return Matrix11(-1);
}


Matrix11 Matrix11::randomRotation()
{
    return Matrix11(RNG.sflip());
}


Matrix11 Matrix11::randomRotation(real)
{
    return Matrix11(RNG.sflip());
}


Matrix11 Matrix11::rotationToVector(const Vector1& vec)
{
    return Matrix11(std::copysign(1, vec.XX));
}

