// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "vector2.h"

/**
 Read a scalar value equal to zero
 */
void eatOneZero(std::istream& is)
{
    if ( is.good() )
    {
        std::streampos isp = is.tellg();
        real Z = 0;
        is >> Z;
        if ( is.fail() || Z != 0 )
        {
            // restore initial state:
            is.seekg(isp);
            is.clear();
        }
    }
}


/**
 This accepts 'X Y 0' but also 'X' and 'X Y'.
 At least one scalar must be read to be valid
 */
std::istream& operator >> (std::istream& is, Vector2& v)
{
    if ( is >> v.XX )
    {
        if ( is >> v.YY )
            eatOneZero(is);
        else
        {
            v.YY = 0;
            is.clear();
        }
    }
    return is;
}


std::ostream& operator << (std::ostream& os, Vector2 const& v)
{
    int w = (int)os.width();
    os << v.XX << " " << std::setw(w) << v.YY;
    return os;
}

