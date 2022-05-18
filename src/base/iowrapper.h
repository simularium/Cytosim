// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef  IOWRAPPER_H
#define  IOWRAPPER_H

#include <cstdio>
#include <stdint.h>
#include "filewrapper.h"

/// Input with automatic binary/text mode and byte-swapping for cross-platform compatibility
class Inputter : public FileWrapper
{
private:
        
    /// The format ID of the input: this allow backward compatibility with older formats
    unsigned  format_;
    
    /// The dimensionality of vectors stored in the file
    unsigned  vecsize_;
    
    /** if the state is stored in a binary format, binary_
        is set to 1 or 2. with 2, byte order is swapped automatically
        this occurs for example when reading a simulation calculated 
        on PC from mac, or vice et versa.
        */
    int       binary_;
    
    /// reverse order of bytes in c[2]
    /**
     Can use the Intel SIMD function _bswap() and _bswap64()
     */
    inline void swap2(unsigned char* c)
    {
        unsigned char s(c[0]);
        c[0] = c[1];
        c[1] = s;
    }
 
    /// reverse order of bytes in c[4]
    inline void swap4(unsigned char* c)
    {
        unsigned char s(c[0]);
        unsigned char t(c[1]);
        c[0] = c[3];
        c[1] = c[2];
        c[3] = s;
        c[2] = t;
    }

    /// reverse order of bytes in c[8]
    inline void swap8(unsigned char* c)
    {
        unsigned char s(c[0]);
        unsigned char t(c[1]);
        unsigned char u(c[2]);
        unsigned char v(c[3]);
        c[0] = c[7];
        c[1] = c[6];
        c[2] = c[5];
        c[3] = c[4];
        c[7] = s;
        c[6] = t;
        c[5] = u;
        c[4] = v;
    }

public:
    
    /// set defaults (not-binary)
    void      reset();
    
    /// Constructor
    Inputter(unsigned d) : FileWrapper(nullptr), vecsize_(d) { reset(); }
    
    /// Constructor
    Inputter(unsigned d, FILE * f, const char * path = nullptr) : FileWrapper(f, path), vecsize_(d) { reset(); }
    
    /// constructor which opens a file
    Inputter(unsigned d, const char* name, bool bin) : FileWrapper(name, bin?"rb":"r"), vecsize_(d) { reset(); }

    /// return dimensionnally of vectors
    unsigned  vectorSize()      const { return vecsize_; }
    
    /// Set dimentionnality of vectors
    void      vectorSize(unsigned d)  { vecsize_ = d; }
    
    /// returns the type of input
    unsigned  formatID()        const { return format_; }

    /// returns the type of input
    void      formatID(unsigned f)    { format_ = f; }

    /// Returns 1 for native binary format, 2 for non-native binary format, and 0 if not binary
    int       binary()          const { return binary_; }
    
    /// initialize the automatic swapping of bytes in the binary format
    void      setEndianess(const char[2]);
    
    /// Read integer on 2 bytes
    int16_t   readInt16();
    /// Read integer on 4 bytes
    int32_t   readInt32();

    /// Read unsigned integer on 1 byte
    uint8_t   readUInt8();
    /// Read unsigned integer on 2 bytes
    uint16_t  readUInt16();
    /// Read unsigned integer on 4 bytes
    uint32_t  readUInt32();
    /// Read unsigned integer on 8 bytes
    uint64_t  readUInt64();
    
    /// Reads one float on 4 bytes
    float     readFloat();
    /// Reads one double on 8 bytes
    double    readDouble();
    
    /// Reads one vector, returning D coordinates in the array of size D
    void      readFloats(float[], unsigned D);
    /// Reads one vector, returning D coordinates in the array of size D
    void      readFloats(double[], unsigned D);
    
    /// Reads `n` vector, returning D coordinates for each, in the array of size n*D
    void      readFloats(float[], size_t n, unsigned D);
    /// Reads `n` vector, returning D coordinates for each, in the array of size n*D
    void      readFloats(double[], size_t n, unsigned D);

};


#pragma mark -


///Output with automatic binary/text mode and byte-swapping for cross-platform compatibility
class Outputter : public FileWrapper
{
    
private:
        
    /// Flag for binary output
    bool    binary_;

public:

    /// constructor
    Outputter();
    
    /// constructor which opens a file
    Outputter(FILE* f, bool b) : FileWrapper(f, nullptr), binary_(b) {};

    /// constructor which opens a file where `a` specifies append and `b` binary mode.
    Outputter(const char* name, bool a, bool b=false);
    
    /// Open a file where `a` specifies append and `b` binary mode.
    int     open(const char* name, bool a, bool b=false);
    
    /// Sets to write in binary format
    void    binary(bool b) { binary_ = b; }
    
    /// Return the current binary format
    bool    binary() const { return binary_; }

    /// Puts given string, and '01' or '10', to specify the byte order 
    void    writeEndianess();
        
    /// Inserts a new line symbol, but only in text output mode
    void    writeSoftNewline();
    
    /// Inserts `N` space(s), but only in text output mode
    void    writeSoftSpace(size_t N = 1);
    
    /// Write integer on 1 byte 
    void    writeInt8(int, char before=' ');
    /// Write integer on 2 bytes
    void    writeInt16(int, char before=' ');
    /// Write integer on 4 bytes
    void    writeInt32(int, char before=' ');
    
    /// Write unsigned integer on 1 byte  
    void    writeUInt8(unsigned, char before=' ');
    /// Write unsigned integer on 2 bytes
    void    writeUInt16(unsigned, char before=' ');
    /// Write unsigned integer on 4 bytes
    void    writeUInt32(unsigned, char before=' ');
    /// Write unsigned integer on 4 bytes
    void    writeUInt64(unsigned long, char before=' ');

    /// Write value on 4 bytes
    void    writeFloat(float);
    /// Write value on 4 bytes
    void    writeFloat(double x) { writeFloat((float)x); }

    /// Write `n` values using 4 bytes each
    void    writeFloats(const float*, size_t n, char before=0);
    /// Write `n` values using 4 bytes each (converted to float)
    void    writeFloats(const double*, size_t n, char before=0);

    /// Write value on 8 bytes
    void    writeDouble(double);
    /// Write `n` values using 8 bytes each
    void    writeDoubles(const double*, size_t n, char before=0);

    int     writeChar(int c, int b)
    {
        if ( binary_ )
            return putc_unlocked(c|b, mFile);
        else
            return putc_unlocked(c, mFile);
    }

};

#endif
