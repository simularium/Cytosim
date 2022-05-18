// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cctype>
#include "point_disp.h"
#include "offscreen.h"
#include "glossary.h"
#include "opengl.h"
#include "glut.h"
#include "gle.h"


/// if this is defined, the pixelmap are stored in graphical memory
#define POINTDISP_USES_PIXEL_BUFFERS 0


PointDisp::PointDisp(const std::string& k, const std::string& n)
: Property(n)
{
    mKind = k;
    clear();
}


PointDisp::PointDisp(PointDisp const& o) : Property(o)
{
    mKind        = o.mKind;
    visible      = o.visible;
    color        = o.color;
    color2       = o.color2;
    coloring     = o.coloring;
    size         = o.size;
    width        = o.width;
    shape        = o.shape;
    style        = o.style;
    symbol       = o.symbol;
    symbol_color = o.symbol_color;
}


PointDisp& PointDisp::operator = (PointDisp const& o)
{
    mKind        = o.mKind;
    visible      = o.visible;
    color        = o.color;
    color2       = o.color2;
    coloring     = o.coloring;
    size         = o.size;
    width        = o.width;
    shape        = o.shape;
    style        = o.style;
    symbol       = o.symbol;
    symbol_color = o.symbol_color;
    
    return *this;
}


PointDisp::~PointDisp()
{
}


void PointDisp::clear()
{
    visible      = 1;
    color        = 0x888888FF;
    color2       = 0x777777FF;
    coloring     = 0;
    size         = 5;
    width        = 2;
    shape        = 'o';
    style        = 7;
    symbol       = 0;
    symbol_color = 0xFFFFFFFF;
}


void PointDisp::strokeShape() const
{
    switch ( tolower(shape) )
    {
        case 'v': gle::gleNablaL();     break;
        case 't': gle::gleTriangleL();  break;
        case 'q': gle::gleSquareL();    break;
        case 'r': gle::gleRectangleL(); break;
        case 'p': gle::glePentagonL();  break;
        case 'h': gle::gleHexagonL();   break;
        case 's': gle::gleStarL();      break;
        case '+': gle::glePlusL();      break;
        case 'c': gle::gleCircle();    break;
        default: break;
    }
}


void PointDisp::paintShape() const
{
    switch ( tolower(shape) )
    {
        case 'v': gle::gleNablaS();     break;
        case 't': gle::gleTriangleS();  break;
        case 'q': gle::gleSquareS();    break;
        case 'r': gle::gleRectangleS(); break;
        case 'p': gle::glePentagonS();  break;
        case 'h': gle::gleHexagonS();   break;
        case 's': gle::gleStarS();      break;
        case '+': gle::glePlusS();      break;
        case 'c': break;
        default:  gle::gleDisc();    break;
    }
}

void PointDisp::strokeA() const
{
    paintShape();
    if ( width > 0.5 )
    {
        //draw a bright rim
        color.darken(2.0).load();
        glLineWidth(3.0);
        strokeShape();
    }
    
    if ( symbol )
    {
        glScalef(1.0f/80, 1.0f/80, 1);
        /*  glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, C)
         character C of width ~104.76 units, and ~150 unit high max
         The translation brings it near the center. */
        if ( islower(symbol) )
            glTranslatef(-52.35f, -35, 0);
        else
            glTranslatef(-52.35f, -50, 0);
        symbol_color.load();
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, symbol);
    }
}

void PointDisp::strokeI() const
{
    paintShape();
    
    // radius of the spot that indicate an inactive Hand
    const GLfloat DOT_SIZE = 0.55f;

    // draw a transparent hole in the center:
    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_ENABLE_BIT);
    glPushMatrix();
    glScalef(DOT_SIZE, DOT_SIZE, DOT_SIZE);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glColor4f(0, 0, 0, 0);
    gle::gleDiscB(); //strokeShape();
    glPopMatrix();
    glPopAttrib();
}


#pragma mark - Bitmaps



void PointDisp::prepare(GLfloat uf, GLfloat sf, bool make_maps)
{
    realSize    = size * sf;
    perceptible = visible && ( uf*(size+width) > 0.25 );
}


#pragma mark - I/O


void PointDisp::read(Glossary& glos)
{
    glos.set(visible,      "visible");
    
    // set 'color2' as a darker tone of 'color':
    if ( glos.set(color,   "color") )
        color2 = color.alpha(0.5);
    glos.set(color2,       "color", 1) || glos.set(color2, "back_color");
    glos.set(coloring,     "coloring");
    
    // if 'size' is specified, width is set accordingly:
    if ( glos.set(size,    "size") )
        width = 2 * size / 3;

    // alternative syntax:
    glos.set(size,         "point_size");
#ifdef BACKWARD_COMPATIBILITY
    glos.set(size,         "points");
    glos.set(shape,        "points", 1);
#endif

    glos.set(width,        "width");
    glos.set(style,        "style");
    glos.set(shape,        "shape");
    glos.set(symbol,       "symbol");
    glos.set(symbol_color, "symbol", 1);
    
    if ( ! isprint(symbol) )
        symbol = 0;
}


void PointDisp::write_values(std::ostream& os) const
{
    write_value(os, "visible",     visible);
    if ( color2 != color.alpha(0.5) )
        write_value(os, "color",   color, color2);
    else
        write_value(os, "color",   color);
    write_value(os, "coloring",    coloring);
    write_value(os, "size",        size);
    write_value(os, "width",       width);
    write_value(os, "shape",       shape);
    write_value(os, "style",       style);
    write_value(os, "symbol",      symbol, symbol_color);
}

