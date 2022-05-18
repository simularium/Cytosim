// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "iowrapper.h"
#include "exceptions.h"


///check the size of the type, as we rely on them to write byte-by-byte
int nonStandardTypes()
{
    if ( 2 != sizeof(uint16_t) ) return 1;
    if ( 4 != sizeof(uint32_t) ) return 2;
    if ( 8 != sizeof(uint64_t) ) return 4;
    if ( 4 != sizeof(float) )    return 8;
    if ( 8 != sizeof(double) )   return 16;
    return 0;
}


//==============================================================================
#pragma mark - INPUT

void Inputter::reset()
{
    format_  = 0;
    vecsize_ = 3;
    binary_  = 0;
    
    if ( nonStandardTypes() )
    {
        fprintf(stderr, "Error: non-standard types in Inputter\n");
        exit(EXIT_FAILURE);
    }
}


/**
 Reads a short and compares with the native storage, to set
 binary_=1, for same-endian or binary_ = 2, for opposite endian
*/
void Inputter::setEndianess(const char data[2])
{
    char native[3] = { 0 };
    *((uint16_t*)native) = 12592U;
    //binary_ = 1 for same-endianess, 2 for opposite-endianess:
    binary_ = 1 + ( data[0] != native[0] );
}


int16_t Inputter::readInt16()
{
    int16_t v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 2, 1, mFile) )
            throw InvalidIO("readInt16 failed");
        if ( binary_ == 2 )
            swap2(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        int u;
        if ( 1 != fscanf(mFile, " %i", &u) )
            throw InvalidIO("readInt16() failed");
        v = (int16_t)u;
        if ( v != u )
            throw InvalidIO("invalid int16");
    }
    return v;
}


int32_t Inputter::readInt32()
{
    int32_t v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 4, 1, mFile) )
            throw InvalidIO("readInt32 failed");
        if ( binary_ == 2 )
            swap4(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        int u;
        if ( 1 != fscanf(mFile, " %i", &u) )
            throw InvalidIO("readInt32() failed");
        v = (int32_t)u;
        if ( v != u )
            throw InvalidIO("invalid int32");
    }
    return v;
}


uint8_t Inputter::readUInt8()
{
    uint8_t v;
    if ( binary_ )
    {
        v = get_byte();
    }
    else
    {
        unsigned u;
        if ( 1 != fscanf(mFile, " %u", &u) )
            throw InvalidIO("readUInt8() failed");
        v = (uint8_t)u;
        if ( v != u )
            throw InvalidIO("invalid uint8");
    }
    return v;
}


uint16_t Inputter::readUInt16()
{
    uint16_t v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 2, 1, mFile) )
            throw InvalidIO("readUInt16 failed");
        if ( binary_ == 2 )
            swap2(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        unsigned u;
        if ( 1 != fscanf(mFile, " %u", &u) )
            throw InvalidIO("readUInt16() failed");
        v = (uint16_t)u;
        if ( v != u )
            throw InvalidIO("invalid uint16");
    }
    return v;
}


uint32_t Inputter::readUInt32()
{
    uint32_t v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 4, 1, mFile) )
            throw InvalidIO("readUInt32 failed");
        if ( binary_ == 2 )
            swap4(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        unsigned u;
        if ( 1 != fscanf(mFile, " %u", &u) )
            throw InvalidIO("readUInt32() failed");
        v = (uint32_t)u;
        if ( v != u )
            throw InvalidIO("invalid uint32");
    }
    return v;
}


uint64_t Inputter::readUInt64()
{
    uint64_t v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 8, 1, mFile) )
            throw InvalidIO("readUInt64 failed");
        if ( binary_ == 2 )
            swap8(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        unsigned long u;
        if ( 1 != fscanf(mFile, " %lu", &u) )
            throw InvalidIO("readUInt64() failed");
        v = (uint64_t)u;
        if ( v != u )
            throw InvalidIO("invalid uint64");
    }
    return v;
}


float Inputter::readFloat()
{
    float v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 4, 1, mFile) )
            throw InvalidIO("readFloat failed");
        if ( binary_ == 2 )
            swap4(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        if ( 1 != fscanf(mFile, " %f", &v) )
            throw InvalidIO("readFloat() failed");
    }
    return v;
}


double Inputter::readDouble()
{
    double v;
    if ( binary_ )
    {
        if ( 1 != fread(&v, 8, 1, mFile) )
            throw InvalidIO("readDouble failed");
        if ( binary_ == 2 )
            swap8(reinterpret_cast<unsigned char*>(&v));
    }
    else
    {
        if ( 1 != fscanf(mFile, " %lf", &v) )
            throw InvalidIO("readDouble() failed");
    }
    return v;
}


/**
 This will read vecsize_ floats, and store the first D ones in a[].
 VECSIZE can be changed by calling vectorSize(INT)
 */
void Inputter::readFloats(float a[], const unsigned D)
{
    size_t d;
    if ( vecsize_ <= D )
    {
        for ( d = 0; d < vecsize_; ++d )
            a[d] = readFloat();
        for (; d < D; ++d )
            a[d] = 0;
    }
    else
    {
        for ( d = 0; d < D; ++d )
            a[d] = readFloat();
        for (; d < vecsize_; ++d )
            readFloat();
    }
}


/**
 This will read vecsize_ floats, and store the first D ones in a[].
 */
void Inputter::readFloats(double a[], const unsigned D)
{
    size_t d;
    if ( vecsize_ <= D )
    {
        for ( d = 0; d < vecsize_; ++d )
            a[d] = readFloat();
        for (; d < D; ++d )
            a[d] = 0.0;
    }
    else
    {
        for ( d = 0; d < D; ++d )
            a[d] = readFloat();
        for (; d < vecsize_; ++d )
            readFloat();
    }
}


/**
 This will read `n * vecsize_` floats, and store `n * D` values in a[].
 */
void Inputter::readFloats(double a[], const size_t n, const unsigned D)
{
    const size_t nd = n * vecsize_;
    float * v = new float[nd];
    
    if ( binary_ )
    {
        if ( nd != fread(v, 4, nd, mFile) )
        {
            delete[] v;
            throw InvalidIO("readFloatVector(double) failed");
        }
        if ( binary_ == 2 )
            for ( size_t u = 0; u < nd; ++u )
                swap4(reinterpret_cast<unsigned char*>(v+u));
    }
    else
    {
        for ( size_t u = 0; u < nd; ++u )
            if ( 1 != fscanf(mFile, " %f", v+u) )
            {
                delete[] v;
                throw InvalidIO("readFloatVector(double) failed");
            }
    }

    const size_t m = ( vecsize_ < D ? vecsize_ : D );
    
    for ( size_t u = 0; u < n; ++u )
    {
        size_t i = 0;
        for ( ; i < m; ++i )
            a[D*u+i] = v[vecsize_*u+i];
        for ( ; i < D; ++i )
            a[D*u+i] = 0;
    }
    delete[] v;
}


//==============================================================================
#pragma mark - OUTPUT

Outputter::Outputter()
: FileWrapper(stdout) 
{
    binary_ = false;
    
    if ( nonStandardTypes() )
    {
        fprintf(stderr, "Error: non-standard types in Inputter\n");
        exit(EXIT_FAILURE);
    }
}


Outputter::Outputter(const char* name, const bool a, const bool b)
{
    open(name, a, b);
    
    if ( nonStandardTypes() )
    {
        fprintf(stderr, "Error: unsupported types in Outputter\n");
        exit(EXIT_FAILURE);
    }
}


int Outputter::open(const char* name, const bool a, const bool b)
{
    binary_ = b;
    
    //create a 'mode' string appropriate for Windows OS
    char m[3] = { 0 };
    
    if ( a )
        m[0] = 'a';
    else
        m[0] = 'w';
    
    if ( b )
        m[1] = 'b';
        
    return FileWrapper::open(name, m);
}


void Outputter::writeEndianess()
{
    //the value corresponds to the ASCII code of "01"
    uint16_t x = 12592U;
    if ( 2 != fwrite(&x, 1, 2, mFile) )
        throw InvalidIO("writeEndianess() failed");
}


void Outputter::writeInt8(const int n, char before)
{
    int8_t v = (int8_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeInt8()");

    if ( binary_ )
    {
        if ( 1 != fwrite(&v, 1, 1, mFile) )
            throw InvalidIO("writeInt8()-binary failed");
    }
    else
    {
        if ( 2 > fprintf(mFile, "%c%i", before, n) )
            throw InvalidIO("writeInt8() failed");
    }
}


void Outputter::writeInt16(const int n, char before)
{
    int16_t v = (int16_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeInt16()");

    if ( binary_ )
    {
        if ( 2 != fwrite(&v, 1, 2, mFile) )
            throw InvalidIO("writeInt16()-binary failed");
    }
    else
    {
        if (2 > fprintf(mFile, "%c%i", before, n))
            throw InvalidIO("writeInt16() failed");
    }
}


void Outputter::writeInt32(const int n, char before)
{
    int32_t v = (int32_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeInt32()");
    
    if ( binary_ )
    {
        if ( 4 != fwrite(&v, 1, 4, mFile) )
            throw InvalidIO("writeInt32()-binary failed");
    }
    else
    {
        if ( 2 > fprintf(mFile, "%c%d", before, n) )
            throw InvalidIO("writeInt32() failed");
    }
}


void Outputter::writeUInt8(const unsigned n, char before)
{
    uint8_t v = (uint8_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeUInt8()");
    
    if ( binary_ )
    {
        if ( 1 != fwrite(&v, 1, 1, mFile) )
            throw InvalidIO("writeUInt8()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%u", before, n) )
                throw InvalidIO("writeUInt8() failed");
        }
        else {
            if ( 1 > fprintf(mFile, "%u", n) )
                throw InvalidIO("writeUInt8() failed");
        }
    }
}


void Outputter::writeUInt16(const unsigned n, char before)
{
    uint16_t v = (uint16_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeUInt16()");

    if ( binary_ )
    {
        if ( 2 != fwrite(&v, 1, 2, mFile) )
            throw InvalidIO("writeUInt16()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%u", before, n) )
                throw InvalidIO("writeUInt16() failed");
        }
        else {
            if ( 1 > fprintf(mFile, "%u", n) )
                throw InvalidIO("writeUInt16() failed");
        }
    }
}


void Outputter::writeUInt32(const unsigned n, char before)
{
    uint32_t v = (uint32_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeUInt32()");
    
    if ( binary_ )
    {
        if ( 4 != fwrite(&v, 1, 4, mFile) )
            throw InvalidIO("writeUInt32()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%u", before, n) )
                throw InvalidIO("writeUInt32() failed");
        }
        else
        {
            if ( 1 > fprintf(mFile, "%u", n) )
                throw InvalidIO("writeUInt32() failed");
        }
    }
}


void Outputter::writeUInt64(const unsigned long n, char before)
{
    uint64_t v = (uint64_t)n;
    
    if ( n != v )
        throw InvalidIO("value out of range for writeUInt64()");
    
    if ( binary_ )
    {
        if ( 8 != fwrite(&v, 1, 8, mFile) )
            throw InvalidIO("writeUInt64()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%lu", before, n) )
                throw InvalidIO("writeUInt64() failed");
        }
        else
        {
            if ( 1 > fprintf(mFile, "%lu", n) )
                throw InvalidIO("writeUInt64() failed");
        }
    }
}


void Outputter::writeFloat(const float x)
{
    if ( binary_ )
    {
        if ( 4 != fwrite(&x, 1, 4, mFile) )
            throw InvalidIO("writeFloat()-binary failed");
    }
    else
    {
        if ( 6 > fprintf(mFile, " %.6f", x) )
            throw InvalidIO("writeFloat() failed");
    }
}


void Outputter::writeFloats(const float* a, const size_t n, char before)
{
    if ( before && !binary_ )
        putc(before, mFile);
    
    for ( size_t d = 0; d < n; ++d )
        writeFloat(a[d]);
}


void Outputter::writeFloats(const double* a, const size_t n, char before)
{
    if ( before && !binary_ )
        putc(before, mFile);
    
    for ( size_t d = 0; d < n; ++d )
        writeFloat(a[d]);
}


void Outputter::writeDouble(const double x)
{
    if ( binary_ )
    {
        if ( 8 != fwrite(&x, 1, 8, mFile) )
            throw InvalidIO("writeDouble()-binary failed");
    }
    else
    {
        if ( 10 > fprintf(mFile, " %.8lf", x) )
            throw InvalidIO("writeDouble() failed");
    }
}


void Outputter::writeDoubles(const double* a, const size_t n, char before)
{
    if ( before && !binary_ )
        putc(before, mFile);
    
    for ( size_t d = 0; d < n; ++d )
        writeDouble(a[d]);
}


void Outputter::writeSoftNewline()
{
    if ( !binary_ )
        putc('\n', mFile);
    fflush(mFile);
}


void Outputter::writeSoftSpace(size_t N)
{
    if ( !binary_ )
    {
        while ( N > 0 ) {
            fprintf(mFile, " ");
            N--;
        }
    }
}

