// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef FIELD_VALUES_H
#define FIELD_VALUES_H

#include "iowrapper.h"

#ifdef DISPLAY
   #include "opengl.h"
   #include "gle_color.h"
#endif


/// Scalar type (real) for a Field
/**
 Example of type that can be used in Field
 */
class FieldScalar
{
public:
    
    /// single scalar value
    real  val;
    
public:
    
    /// constructor
    FieldScalar()                     { val = 0; }
    /// implicit conversion from real
    FieldScalar(real a)        { val = a; }
    /// implicit conversion to real
    operator real&()                  { return val; }

    /// set to zero
    void clear()                      { val = 0; }
    /// write
    void write(Outputter& out) const  { out.writeFloat(val); }
    /// read
    void read(Inputter& in)           { val = in.readFloat(); }
    
#ifdef DISPLAY
    /// set OpenGL color associated with value
    void setColor(const real scale) const
    {
        GLfloat x = GLfloat(scale * val);
        if ( x > 0 )
            gle_color::jet_color_dark(x, 1.0).load();
        else
            glColor3f(-x, 0, -x);
    }
#endif
};


/// Vector of N scalar, suitable for a Field
/**
 Example of type that can be used in Field
 */
template < int N >
class FieldVector
{
    /// vector of values
    real  val[N];
    
public:
    
    /// the dimensionality (given as template argument)
    static const int nFields = N;
    
    /// access to the vector components
    real& operator[](size_t n)        { assert_true(n<N); return val[n]; }

    /// set to zero
    void clear()                      { for (int n=0; n<N; ++n) val[n] = 0; }
    /// write
    void write(Outputter& out) const  { for (int n=0; n<N; ++n) out.writeFloat(val[n]); }
    /// read
    void read(Inputter& in)           { for (int n=0; n<N; ++n) val[n] = in.readFloat(); }

#ifdef DISPLAY
    /// set OpenGL color associated with value
    void setColor(const real scale) const
    {
        //this maps val[0] to the red channel, val[1] to green and val[2] to blue
        GLfloat rgb[3] = { 0, 0, 0 };
        const int cmax = std::min(3, N);
        for ( int c = 0; c < cmax; ++c )
           rgb[c] = scale * val[c];
        glColor3fv(rgb);
    }
#endif
};


#endif
