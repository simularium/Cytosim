// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "gle_color.h"
#include "gle_color_list.h"
#include "stream_func.h"
#include "exceptions.h"
#include <iomanip>
#include <cctype>
#include <cmath>


#pragma mark Color Input/Output

const char hex_rep[] = "0123456789ABCDEF";

/**
 Write '0xRRGGBBAA' if size > 10 and '0xRRGGBB' if size == 9 or 10 and nothing otherwise
 Write at most size-1 characters (the N'th character then gets the terminating `\0');
 return the number of characters printed (not including the trailing `\0' used to end output to strings)
 */
int gle_color::print(char * str, size_t size) const
{
    if ( size < 9 )
        return 0;
    
    uint32_t n = rgba_;

    str[0] = '0';
    str[1] = 'x';
    int c = 2;
    while ( c < 8 )
    {
        uint32_t d = ( n >> 28 );
        n <<= 4;
        str[c] = hex_rep[d];
        ++c;
    }
    // write alpha component is space permits:
    if ( size > 10 )
    {
        while ( c < 10 )
        {
            uint32_t d = ( n >> 28 );
            n <<= 4;
            str[c] = hex_rep[d];
            ++c;
        }
    }
    str[c] = '\0';
    return c-1;
}


std::string gle_color::to_string() const
{
    char str[12] = { 0 };
    print(str, 12);
    return std::string(str);
}


GLubyte hex2byte(int c)
{
    if ( '0' <= c && c <= '9' )
        return (GLubyte)(c - '0');
    else if ( 'A' <= c && c <= 'F' )
        return (GLubyte)(c - ( 'A' - 10 ));
    else if ( 'a' <= c && c <= 'f' )
        return (GLubyte)(c - ( 'a' - 10 ));
    throw InvalidSyntax("invalid hexadecimal digit");
}

GLubyte hex2byte(int a, int b)
{
    return (GLubyte)( hex2byte(a) << 4 ) | hex2byte(b);
}

/**
 A color is composed of 4 components (Red, Green, Blue, Alpha),
 and can be specified in different ways:
 -# with a hexadecimal integer: 0xFF0000FF  or  0xff0000ff
 -# with 3 or 4 floats: (1 0 0) or (1 0 0 1)
 -# with a name:  red
 -# with a number: #1
 .
*/
std::istream& operator >> (std::istream& is, gle_color& col)
{
    col.set_white();

    std::istream::sentry s(is);
    if ( s )
    {
        int c = is.get();
        int d = is.peek();
        
        if ( isalpha(c) )
        {
            is.unget();
            std::string name;
            is >> name;
            try {
                col = gle::std_color(name);
            }
            catch ( InvalidSyntax & e )
            {
                is.setstate(std::istream::failbit);
                std::cerr << e.what() << " : using white" << std::endl;
                gle::print_std_colors(std::cerr);
                col = 0xFFFFFFFF;
            }
        }
        else if ( '#'==c  &&  isdigit(d) )
        {
            unsigned int nb;
            is >> nb;
            col = gle::alt_color(nb);
        }
        else if ( '0'==c  &&  'x'==d )
        {
            is.get();
            GLubyte u[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
            int i = 0;
            while ( i < 4 )
            {
                c = is.get();
                d = is.get();
                if ( c == EOF || d == EOF )
                {
                    if ( i < 3 )
                        throw InvalidSyntax("incomplete hexadecimal color specification");
                    is.clear();
                    break;
                }
                u[i++] = hex2byte(c, d);
            }
            col.set_bytes(u[0], u[1], u[2], u[3]);
        }
        else if ( isdigit(c) )
        {
            is.unget();
            GLfloat r, g, b, a=1;
            is >> r >> g >> b;
            if ( ! is.fail() )
            {
                is >> a;
                is.clear();
                col.set_float(r,g,b,a);
            }
        }
    }
    return is;
}


std::ostream& operator << (std::ostream& os, gle_color const& x)
{
    os << x.to_string();
    return os;
}


//-----------------------------------------------------------------------
#pragma mark - Color making primitives


/**
 r,g,b values are from 0 to 1
 h = [0, 360], s = [0,1], v = [0,1]
 if s == 0, then h = -1 (undefined)
*/

void gle_color::RGB2HSV(const GLfloat r, const GLfloat g, const GLfloat b, GLfloat* h, GLfloat* s, GLfloat* v)
{
    GLfloat mn, mx, delta;
    mn = std::min(r, std::min(g, b));
    mx = std::max(r, std::max(g, b));
    *v = mx;
    delta = mx - mn;
    if ( mx != 0 )
        *s = delta / mx;
    else {
        *s = 0;
        *h = -1;
        return;
    }
    if ( r == mx )
        *h = ( g - b ) / delta;       // between yellow & magenta
    else if ( g == mx )
        *h = 2 + ( b - r ) / delta;   // between cyan & yellow
    else
        *h = 4 + ( r - g ) / delta;   // between magenta & cyan
    *h *= 60;                         // degrees
    if ( *h < 0 )
        *h += 360;
}


void gle_color::HSV2RGB(const GLfloat h, const GLfloat s, const GLfloat v, GLfloat* r, GLfloat* g, GLfloat* b)
{
    int i;
    GLfloat f, p, q, t;
    if ( s == 0 ) {
        // achromatic (gray)
        *r = *g = *b = v;
        return;
    }
    GLfloat hc = h/60;               // sector 0 to 5
    i = (int)floor(hc);
    f = hc - (GLfloat)i;             // fractional part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );
    switch( i )
    {
        case 0:  *r = v; *g = t; *b = p; break;
        case 1:  *r = q; *g = v; *b = p; break;
        case 2:  *r = p; *g = v; *b = t; break;
        case 3:  *r = p; *g = q; *b = v; break;
        case 4:  *r = t; *g = p; *b = v; break;
        case 5:  *r = v; *g = p; *b = q; break;
        default: *r = 1; *g = 1; *b = 1; break;
    }
}


/**
 set a RGB color as a function of a Hue value `a` in [-PI, PI].
 The colors follow in this order: red, green, blue, red ...
*/
void gle_color::set_hue_components(GLfloat& r, GLfloat& g, GLfloat& b, const GLfloat h)
{
    GLfloat x = 3 * GLfloat( h * M_1_PI + 1 );
    int i = (int)floor(x);
    GLfloat f = x-(GLfloat)i;
    switch( i % 6 )
    {
        case 0: r = 1;   g = f;   b = 0;   break;
        case 1: r = 1-f; g = 1;   b = 0;   break;
        case 2: r = 0;   g = 1;   b = f;   break;
        case 3: r = 0;   g = 1-f; b = 1;   break;
        case 4: r = f;   g = 0;   b = 1;   break;
        case 5: r = 1;   g = 0;   b = 1-f; break;
        default: r = 1;  g = 0;   b = 0;   break; // never executed;
    }
}


/**
 set a RGB color as a function of a value h in [0, 4].
 The result vary from dark-blue, blue, cyan, yellow, orange to red:
 - 0 : dark-blue
 - 1 : blue
 - 2 : green
 - 3 : red
 - 4 : full red
 */
void gle_color::set_jet_components(GLfloat& r, GLfloat& g, GLfloat& b, const GLfloat h)
{
    if ( h <= 0.4 )
    {
        r = 0;
        g = 0;
        b = 0.4f;
    }
    else
    {
        int i = (int)floor(h);
        GLfloat f = h-(GLfloat)i;
        switch( i )
        {
            case 0:  r = 0;   g = 0;   b = f;   break;
            case 1:  r = 0;   g = f;   b = 1;   break;
            case 2:  r = f;   g = 1;   b = 1-f; break;
            case 3:  r = 1;   g = 1-f; b = 0;   break;
            default: r = 1;   g = 0;   b = 0;   break;
        }
    }
}


/**
 set a RGB color as a function of a value h in [0, 4].
 The result vary from black, blue, cyan, yellow, orange to red:
 - 0 : black
 - 1 : blue
 - 2 : green
 - 3 : red
 - 4 : yellow
 - 5 : white
 */
void gle_color::set_jet_components_dark(GLfloat& r, GLfloat& g, GLfloat& b, const GLfloat h)
{
    if ( h <= 0.1 )
    {
        r = 0;
        g = 0;
        b = 0.1f;
    }
    else
    {
        int i = (int)floor(h);
        GLfloat f = h-(GLfloat)i;
        switch( i )
        {
            case 0:  r = 0;   g = 0;   b = f;   break;
            case 1:  r = 0;   g = f;   b = 1-f; break;
            case 2:  r = f;   g = 1-f; b = 0;   break;
            case 3:  r = 1;   g = f;   b = 0;   break;
            case 4:  r = 1;   g = 1;   b = f;   break;
            default: r = 1;   g = 1;   b = 1;   break;
        }
    }
}


/**
 set a RGB color as a function of a 3D vector with components in [-1, 1],
 with alpha-component equal to `a`.
 Two opposite vectors gives approximately complementary colors.
 */
gle_color gle_color::radial_color(const float x, const float y, const float z, const float a)
{
    GLfloat pX = std::max(0.0f, x), nX = -0.5f * std::min(0.0f, x);
    GLfloat pY = std::max(0.0f, y), nY = -0.5f * std::min(0.0f, y);
    GLfloat pZ = std::max(0.0f, z), nZ = -0.5f * std::min(0.0f, z);
    return gle_color(pX+nY+nZ, nX+pY+nZ, nX+nY+pZ, a);
}

/**
 set a RGB color as a function of a 2D vector, using the angle in the XY plane
*/
gle_color gle_color::radial_color(const float x, const float y, const float a)
{
    GLfloat r, g, b;
    set_hue_components(r, g, b, atan2f(y, x));
    return gle_color(r, g, b, a);
}

/**
 set a RGB color as a function of a Hue value `h` in [-PI, PI],
 with alpha-component equal to `a`.
 The colors follow in this order: red, green, blue, red ...
 */
gle_color gle_color::hue_color(const float h, const float a)
{
    GLfloat r, g, b;
    set_hue_components(r, g, b, h);
    return gle_color(r, g, b, a);
}

gle_color gle_color::jet_color(const float h, const float a)
{
    GLfloat r, g, b;
    set_jet_components(r, g, b, h);
    return gle_color(r, g, b, a);
}

gle_color gle_color::jet_color_dark(const float h, const float a)
{
    GLfloat r, g, b;
    set_jet_components_dark(r, g, b, h);
    return gle_color(r, g, b, a);
}

