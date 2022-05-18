// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "lattice.h"


/// write data within [inf, sup[ to file
template <>
void Lattice<uint8_t>::write_data(Outputter& out, lati_t inf, lati_t sup) const
{
    out.writeUInt16(0);
    out.writeUInt8(0);
    out.writeUInt8(1);

    for ( lati_t s = inf; s < sup; ++s )
        out.writeUInt8(laSite[s]);
}

/// write data within [inf, sup[ to file
template <>
void Lattice<uint16_t>::write_data(Outputter& out, lati_t inf, lati_t sup) const
{
    out.writeUInt16(0);
    out.writeUInt8(0);
    out.writeUInt8(2);

    for ( lati_t s = inf; s < sup; ++s )
        out.writeUInt16(laSite[s]);
}

/// write data within [inf, sup[ to file
template <>
void Lattice<uint32_t>::write_data(Outputter& out, lati_t inf, lati_t sup) const
{
    out.writeUInt16(0);
    out.writeUInt8(0);
    out.writeUInt8(4);

    for ( lati_t s = inf; s < sup; ++s )
        out.writeUInt32(laSite[s]);
}

/// write data within [inf, sup[ to file
template <>
void Lattice<uint64_t>::write_data(Outputter& out, lati_t inf, lati_t sup) const
{
    out.writeUInt16(0);
    out.writeUInt8(0);
    out.writeUInt8(8);
    
    for ( lati_t s = inf; s < sup; ++s )
        out.writeUInt64(laSite[s]);
}

/// write data within [inf, sup[ to file
template <>
void Lattice<real>::write_data(Outputter& out, lati_t inf, lati_t sup) const
{
    out.writeUInt16(0);
    out.writeUInt8(0);
    out.writeUInt8(12);
    
    for ( lati_t s = inf; s < sup; ++s )
        out.writeFloat(laSite[s]);
}
