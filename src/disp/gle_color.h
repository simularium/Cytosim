// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLE_COLOR_H
#define GLE_COLOR_H

#include "opengl.h"
#include <iostream>

/**
 gle_color implements colors with 4-components:
 - Red
 - Green
 - Blue
 - Alpha = transparency
 .
 
 This class implements the `RGBA` format using an 'unsigned integer'
 and an array of 4 GLfloat.
 
 F. Nedelec -- Merged two older color classes on 23 August 2015
 */
/// Color with 4 components: red, green, blue, alpha (RGBA)
class gle_color
{
private:
    
    /// 32-bits integer containing 4 one-byte components: red, green, blue, alpha
    uint32_t rgba_;
    
    /// array of 4 float components, matching the `rgba_` integer
    GLfloat col_[4];
    
    /// access to float components
    GLfloat& operator [] (int i) { return col_[i]; }

    /// concatenate 4 bytes into an int
    static uint32_t combine(uint32_t R, uint32_t G, uint32_t B, uint32_t A)
    {
        const uint32_t K = 0xFF;
        return (R&K) << 24 | (G&K) << 16 | (B&K) << 8 | (A&K);
    }

    /// concatenate 4 bytes into an int
    static uint32_t combine(GLfloat R, GLfloat G, GLfloat B, GLfloat A)
    {
        return combine(uint32_t(255*R), uint32_t(255*G), uint32_t(255*B), uint32_t(255*A));
    }

    /// return value clamped to [0, 1]
    static GLfloat clamp(GLfloat s) { return std::max(0.0f, std::min(s, 1.0f)); }

    /// update 'rgba_' to match values in 'col_'
    void update_rgba()
    {
        rgba_ = combine(col_[0], col_[1], col_[2], col_[3]);
    }
    
    /// update 'col_' to match values in 'rgba_'
    void update_float()
    {
        col_[0] = (float)( 0xFF & ( rgba_ >> 24 ) ) / 255.f;
        col_[1] = (float)( 0xFF & ( rgba_ >> 16 ) ) / 255.f;
        col_[2] = (float)( 0xFF & ( rgba_ >>  8 ) ) / 255.f;
        col_[3] = (float)( 0xFF & rgba_ ) / 255.f;
    }
    
#pragma mark -

public:
    
    /// set to white
    void set_white()
    {
        rgba_ = 0xFFFFFFFF;
        col_[0] = 1;
        col_[1] = 1;
        col_[2] = 1;
        col_[3] = 1;
    }
    
    /// set to black
    void set_black()
    {
        rgba_ = 0x000000FF;
        col_[0] = 0;
        col_[1] = 0;
        col_[2] = 0;
        col_[3] = 1;
    }

    /// specify floating point components
    void set_float(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        col_[0] = clamp(r);
        col_[1] = clamp(g);
        col_[2] = clamp(b);
        col_[3] = clamp(a);
        update_rgba();
    }
    
    /// export floating point components
    void store(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a)
    {
        r = col_[0];
        g = col_[1];
        b = col_[2];
        a = col_[3];
    }
    
    /// export floating point components to array
    void store(GLfloat c[4])
    {
        c[0] = col_[0];
        c[1] = col_[1];
        c[2] = col_[2];
        c[3] = col_[3];
    }

    /// specify components with bytes
    void set_bytes(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
    {
        rgba_ = combine(uint32_t(r), uint32_t(g), uint32_t(b), uint32_t(a));
        update_float();
    }

    /// export components as bytes
    void put_bytes(GLubyte& r, GLubyte& g, GLubyte& b, GLubyte& a)
    {
        r = 0xFF & (GLubyte)( rgba_ >> 24 );
        g = 0xFF & (GLubyte)( rgba_ >> 16 );
        b = 0xFF & (GLubyte)( rgba_ >> 8 );
        a = 0xFF & (GLubyte)( rgba_ );
    }
    
    /// set from 4-bytes integer
    void set_rgba(uint32_t u)
    {
        rgba_ = u;
        update_float();
    }

    /// default constructor
    gle_color() : rgba_(0)
    {
        col_[0] = 0;
        col_[1] = 0;
        col_[2] = 0;
        col_[3] = 0;
    }
    
    /// constructor
    gle_color(const uint32_t& u)
    {
        set_rgba(u);
    }
    
    /// constructor from RGB values, with Alpha component = 1.0
    gle_color(const GLfloat& r, const GLfloat& g, const GLfloat& b)
    {
        set_float(r,g,b,1.0f);
    }

    /// constructor from RGBA components
    gle_color(const GLfloat& r, const GLfloat& g, const GLfloat& b, const GLfloat& a)
    {
        set_float(r,g,b,a);
    }

    void operator = (const uint32_t& col)
    {
        set_rgba(col);
    }
    
    bool operator ==(const gle_color col) const { return rgba_ == col.rgba_; }
    bool operator !=(const gle_color col) const { return rgba_ != col.rgba_; }
    
    GLfloat const* data() const { return col_; }
    
    GLfloat   r() const { return col_[0]; }
    GLfloat   g() const { return col_[1]; }
    GLfloat   b() const { return col_[2]; }
    GLfloat   a() const { return col_[3]; }
    
    GLfloat   red()   const { return col_[0]; }
    GLfloat   green() const { return col_[1]; }
    GLfloat   blue()  const { return col_[2]; }
    GLfloat   alpha() const { return col_[3]; }

    void      set_red  (GLfloat s) { col_[0] = s; update_rgba(); }
    void      set_green(GLfloat s) { col_[1] = s; update_rgba(); }
    void      set_blue (GLfloat s) { col_[2] = s; update_rgba(); }
    void      set_alpha(GLfloat s) { col_[3] = s; update_rgba(); }

    gle_color red  (GLfloat s)  const { return gle_color(clamp(s), col_[1], col_[2], col_[3]); }
    gle_color green(GLfloat s)  const { return gle_color(col_[0], clamp(s), col_[2], col_[3]); }
    gle_color blue (GLfloat s)  const { return gle_color(col_[0], col_[1], clamp(s), col_[3]); }
    gle_color alpha(GLfloat s)  const { return gle_color(col_[0], col_[1], col_[2], clamp(s)); }

    gle_color match_r(gle_color c) const { return gle_color(c.col_[0], col_[1], col_[2], col_[3]); }
    gle_color match_g(gle_color c) const { return gle_color(col_[0], c.col_[1], col_[2], col_[3]); }
    gle_color match_b(gle_color c) const { return gle_color(col_[0], col_[1], c.col_[2], col_[3]); }
    gle_color match_a(gle_color c) const { return gle_color(col_[0], col_[1], col_[2], c.col_[3]); }
    
#pragma mark -

    bool      visible()            const { return ( rgba_ & 0xFF ); }
    bool      invisible()          const { return ( rgba_ & 0xFF ) == 0; }
    GLfloat   brightness()         const { return ( col_[0] + col_[1] + col_[2] ) * col_[3]; }
    
    bool      opaque()             const { return ( (rgba_ & 0xFF) == 0xFF ); }
    bool      transparent()        const { return ( (rgba_ & 0xFF) != 0xFF ); }
    GLfloat   transparency()       const { return col_[3]; }
    
#pragma mark -

    gle_color darken(GLfloat s) const
    {
        GLfloat x = clamp(s);
        return gle_color(x*col_[0], x*col_[1], x*col_[2], col_[3]);
    }
    
    gle_color lighten(GLfloat s) const
    {
        GLfloat r = clamp( s * col_[0] );
        GLfloat g = clamp( s * col_[1] );
        GLfloat b = clamp( s * col_[2] );
        return gle_color(r, g, b, col_[3]);
    }
    
    gle_color alpha_scaled(GLfloat s) const
    {
        return gle_color(col_[0], col_[1], col_[2], clamp(s*col_[3]));
    }
    
    gle_color blend(gle_color c) const
    {
        GLfloat s = a() + c.a();
        GLfloat h = a()   / s;
        GLfloat g = c.a() / s;
        return gle_color(h*col_[0]+g*c.col_[0], h*col_[1]+g*c.col_[1], h*col_[2]+g*c.col_[2], h+g);
    }
    
    friend gle_color blend(GLfloat g, gle_color a, GLfloat h, gle_color b)
    {
        return gle_color(g*a[0]+h*b[0], g*a[1]+h*b[1], g*a[2]+h*b[2], g*a[3]+h*b[3]);
    }

    gle_color inverted() const
    {
        return gle_color(1.f-col_[0], 1.f-col_[1], 1.f-col_[2], col_[3]);
    }
    
#pragma mark -
    
    /// set current OpenGL color by calling glColor
    void load() const
    {
        //std::clog << "OpenGL load " << col_[0] << " " << col_[1] << " " << col_[2] << " " << col_[3] << "\n";
        glColor4fv(col_);
        //glColor4f(col_[0], col_[1], col_[2], col_[3]);
    }
    
    /// set current OpenGL color, but with `a` as alpha component
    void load(GLfloat a) const
    {
        glColor4f(col_[0], col_[1], col_[2], clamp(a));
    }
    
    void load_clear() const
    {
        glClearColor(col_[0], col_[1], col_[2], col_[3]);
    }
    
    static void no_emission(GLint face)
    {
        GLfloat blk[4] = { 0, 0, 0, 1 };
        glMaterialfv(face, GL_EMISSION, blk);
    }

    /// set FRONT material property for lighting
    void load_front() const
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col_);
        no_emission(GL_FRONT);
    }
    
    /// set FRONT material property for lighting, and current color
    void load_load() const
    {
        glColor4fv(col_);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col_);
        no_emission(GL_FRONT);
    }
    
    /// set FRONT material property for lighting, and current color with given alpha-component
    void load_load(GLfloat a) const
    {
        GLfloat mat[4] = { col_[0], col_[1], col_[2], clamp(a) };
        glColor4fv(mat);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat);
        no_emission(GL_FRONT);
    }

    /// set front OpenGL color, with `a` as alpha component
    void load_front(GLfloat a) const
    {
        GLfloat mat[4] = { col_[0], col_[1], col_[2], clamp(a) };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat);
        no_emission(GL_FRONT);
    }

    /// set BACK material property for lighting
    void load_back() const
    {
        //glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, col_);
        GLfloat blk[4] = { 0, 0, 0, 1 };
        glMaterialfv(GL_BACK, GL_AMBIENT, col_);
        glMaterialfv(GL_BACK, GL_DIFFUSE, blk);
        glMaterialfv(GL_BACK, GL_EMISSION, blk);
    }
    
    /// set BACK material property for lighting, but with `a` as alpha component
    void load_back(GLfloat a) const
    {
        GLfloat mat[4] = { col_[0], col_[1], col_[2], clamp(a) };
        glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, mat);
        no_emission(GL_BACK);
    }
    
    /// set FRONT and BACK material property for lighting
    void load_both() const
    {
#if 0
        GLfloat blk[4] = { 0, 0, 0, 0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col_);
        glMaterialfv(GL_BACK, GL_AMBIENT, col_);
        glMaterialfv(GL_BACK, GL_DIFFUSE, blk);
#else
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col_);
#endif
        no_emission(GL_FRONT_AND_BACK);
    }
    
    /// set FRONT and BACK material property for lighting
    void load_both(GLfloat a) const
    {
        GLfloat blk[4] = { 0, 0, 0, 1 };
        GLfloat mat[4] = { col_[0], col_[1], col_[2], clamp(a) };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat);
        glMaterialfv(GL_BACK, GL_AMBIENT, mat);
        glMaterialfv(GL_BACK, GL_DIFFUSE, blk);
        no_emission(GL_FRONT_AND_BACK);
    }
    
    /// set FRONT and BACK material property for lighting
    void load_emission() const
    {
        GLfloat blk[4] = { 0, 0, 0, 1 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blk);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, col_);
    }
    
#pragma mark -

    /// print color in human-friendly format
    int print(char* str, size_t size) const;

    /// conversion of color to human-friendly format
    std::string to_string() const;
    
    
    /// conversion function from RGB to HSV color space
    static void RGB2HSV(GLfloat r, GLfloat g, GLfloat b, GLfloat* h, GLfloat* s, GLfloat* v);
    
    /// conversion functions from HSV to RGB color space
    static void HSV2RGB(GLfloat h, GLfloat s, GLfloat v, GLfloat* r, GLfloat* g, GLfloat* b);
    
    
    /// set a RGB color from a factor in [-PI, PI], continuously varying through all colors
    static void set_hue_components(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat h);
    
    /// set a RGB values from a factor in [0, 1], continuously varying between blue, green, red
    static void set_jet_components(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat h);
    
    /// set a RGB values from a factor in [0, 1], continuously varying through blue, green, red, yellow, white
    static void set_jet_components_dark(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat h);

    /// return new saturated color with given Hue value `h` in [-PI, PI]
    static gle_color hue_color(float h, float alpha = 1.0f);
    
    /// return new saturated color with Hue value `atan2(y, x)`
    static gle_color radial_color(float x, float y, float alpha);

    /// return color build from a normalized 3D vector {x, y, z}
    static gle_color radial_color(float x, float y, float z, float alpha);

    /// return new jet color for h in [0, 5] with specified alpha component
    static gle_color jet_color(float h, float alpha = 1.0f);
    
    /// return new jet color extended
    static gle_color jet_color_dark(float h, float alpha = 1.0f);
    
    
    static void print_components(std::ostream& os, std::string const& str, GLfloat mat[4])
    {
        char tmp[128];
        snprintf(tmp, sizeof(tmp), " %4.2f %4.2f %4.2f %4.2f", mat[0], mat[1], mat[2], mat[3]);
        os << str << tmp;
    }

    /// print current color properties of OpenGL context
    static void print_materials(std::ostream& os)
    {
        GLfloat mat[4] = { 0 };
        glGetMaterialfv(GL_FRONT, GL_AMBIENT, mat);
        print_components(os, "front  amb", mat);
        glGetMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
        print_components(os, "  dif", mat);
        glGetMaterialfv(GL_FRONT, GL_EMISSION, mat);
        print_components(os, "  emi", mat);
        os << '\n';

        glGetMaterialfv(GL_BACK, GL_AMBIENT, mat);
        print_components(os, " back  amb", mat);
        glGetMaterialfv(GL_BACK, GL_DIFFUSE, mat);
        print_components(os, "  dif", mat);
        glGetMaterialfv(GL_BACK, GL_EMISSION, mat);
        print_components(os, "  emi", mat);
        os << '\n';
    }
};


/// input operator:
std::istream& operator >> (std::istream&, gle_color&);

/// output operator:
std::ostream& operator << (std::ostream&, const gle_color&);


#endif
