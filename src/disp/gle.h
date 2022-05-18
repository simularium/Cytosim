// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLE_H
#define GLE_H

#include "real.h"
#include "opengl.h"
#include "gle_color.h"
#include "vector.h"


/// Simple geometrical objects drawn with OpenGL
/**
 @todo namespace gle should be a class -> we use GL.vertex(v)
 Problem: the gle prefix is already used by standard OpenGL elements
 */
namespace gle
{
    /// this defines the number of triangles used to draw shapes
    /** higher finesse improves the rendering: 8 is good, 16 is nice and 32 is very nice */
    constexpr size_t finesse = 8;
    
    /// number of circle points stored in buffer
    constexpr size_t ncircle = finesse * 8;

    /// initialize the arrays
    void initialize();
    
    /// release requested memory
    void release();
    
    /// initialize the Vertex Buffer Objects
    void initializeTubeBuffers();

    /// initialize the Vertex Buffer Objects
    void initializeIcoBuffers();
   
    /// calculate sinus and cosinus for a circle
    void circle(size_t cnt, GLfloat c[], GLfloat s[], GLfloat radius, double start = 0);
    
    /// calculate sinus and cosinus for a circular arc
    void arc(size_t cnt, GLfloat c[], GLfloat s[], GLfloat radius, double start, double end, GLfloat cenx, GLfloat ceny);

    /// calculate sinus and cosinus
    void circle(size_t cnt, GLfloat cs[], GLfloat radius);

#pragma mark -
    
    inline void gleScale(float x)                          { glScalef(x,x,x); }
    inline void gleScale(double x)                         { glScaled(x,x,x); }

    inline void gleScale(float x, float y, float z)        { glScalef(x,y,z); }
    inline void gleScale(double x, double y, double z)     { glScaled(x,y,z); }
   
    inline void gleTranslate(float x, float y, float z)    { glTranslatef(x, y, z); }
    inline void gleTranslate(double x, double y, double z) { glTranslated(x, y, z); }

    inline void gleRotate(float a, float x, float y, float z)     { glRotatef(a, x, y, z); }
    inline void gleRotate(double x, double y, double z, double t) { glRotated(x, y, z, t); }

#if REAL_IS_DOUBLE
    
    inline void gleVertex(Vector1 const& v)               { glVertex2d(v.XX, 0); }
    inline void gleVertex(Vector2 const& v)               { glVertex2d(v.XX, v.YY); }
    inline void gleVertex(Vector3 const& v)               { glVertex3d(v.XX, v.YY, v.ZZ); }
    inline void gleVertex(real x, real y)                 { glVertex2d(x, y); }
    inline void gleVertex(real x, real y, real z)         { glVertex3d(x, y, z); }
    
    inline void gleNormal(Vector1 const& v)               { glNormal3d(v.XX, 0, 0); }
    inline void gleNormal(Vector2 const& v)               { glNormal3d(v.XX, v.YY, 0); }
    inline void gleNormal(Vector3 const& v)               { glNormal3d(v.XX, v.YY, v.ZZ); }
    
    inline void gleTranslate(Vector1 const& v)            { glTranslated(v.XX, 0, 0); }
    inline void gleTranslate(Vector2 const& v)            { glTranslated(v.XX, v.YY, 0); }
    inline void gleTranslate(Vector3 const& v)            { glTranslated(v.XX, v.YY, v.ZZ); }
    
    inline void gleMultMatrix(real mat[])                 { glMultMatrixd(mat); }

    inline void gleRasterPos(Vector1 const& v)            { glRasterPos2d(v.XX, 0); }
    inline void gleRasterPos(Vector2 const& v)            { glRasterPos2d(v.XX, v.YY); }
    inline void gleRasterPos(Vector3 const& v)            { glRasterPos3d(v.XX, v.YY, v.ZZ); }

#else

    inline void gleVertex(Vector1 const& v)               { glVertex2f(v.XX, 0); }
    inline void gleVertex(Vector2 const& v)               { glVertex2f(v.XX, v.YY); }
    inline void gleVertex(Vector3 const& v)               { glVertex3f(v.XX, v.YY, v.ZZ); }
    inline void gleVertex(real x, real y)                 { glVertex2f(x, y); }
    inline void gleVertex(real x, real y, real z)         { glVertex3f(x, y, z); }

    inline void gleNormal(Vector1 const& v)               { glNormal3f(v.XX, 0, 0); }
    inline void gleNormal(Vector2 const& v)               { glNormal3f(v.XX, v.YY, 0); }
    inline void gleNormal(Vector3 const& v)               { glNormal3f(v.XX, v.YY, v.ZZ); }
    
    inline void gleTranslate(Vector1 const& v)            { glTranslatef(v.XX, 0, 0); }
    inline void gleTranslate(Vector2 const& v)            { glTranslatef(v.XX, v.YY, 0); }
    inline void gleTranslate(Vector3 const& v)            { glTranslatef(v.XX, v.YY, v.ZZ); }

    inline void gleMultMatrix(real mat[])                 { glMultMatrixf(mat); }

    inline void gleRasterPos(Vector1 const& v)            { glRasterPos2f(v.XX, 0); }
    inline void gleRasterPos(Vector2 const& v)            { glRasterPos2f(v.XX, v.YY); }
    inline void gleRasterPos(Vector3 const& v)            { glRasterPos3f(v.XX, v.YY, v.ZZ); }
 
#endif
    
    inline void gleVertex3v(const float* v)               { glVertex3fv(v); }
    inline void gleVertex3v(const double* v)              { glVertex3dv(v); }
    inline void gleNormal3v(const float* v)               { glNormal3fv(v); }
    inline void gleNormal3v(const double* v)              { glNormal3dv(v); }

    // colors that vary with the direction of a vector:
    inline gle_color radial_color(const Vector3& d) { return gle_color::radial_color((GLfloat)d.XX, (GLfloat)d.YY, (GLfloat)d.ZZ, 1.0f); }
    inline gle_color radial_color(const Vector2& d) { return gle_color::radial_color((GLfloat)d.XX, (GLfloat)d.YY, 1.0f); }
    inline gle_color radial_color(const Vector1& d) { if ( d.XX > 0 ) return gle_color(1,1,1); else return gle_color(0,1,0); }

    //------------------------------------------------------------------------------
#pragma mark -
        
    /// align the X-axis to the given vector, by rotating around Z
    void gleAlignX(Vector2 const& v1);
    /// translate by A, then rotate to align Z with AB (which is in the XY-plane)
    void gleAlignZ(Vector2 const& A, Vector2 const& B);
    /// translate by A, then rotate to align Z with AB, Z replaces X. The X-Y plane is scaled by ts
    void gleAlignZ(Vector2 const& A, Vector2 const& B, real ts);
    ///  align the view to the three orthogonal vectors given
    void gleRotate(Vector3 const& v1, Vector3 const& v2, Vector3 const& v3, bool inverse=false);
    /// translate by T, then rotate to align X with v1, Y with v2 and Z with v3
    void gleTransRotate(Vector3 const& v1, Vector3 const& v2, Vector3 const& v3, Vector3 const& T);
    /// translate by T, then rotate to align Z with dir
    void gleTransAlignZ(Vector3 const& A, Vector3 const& B, real radius);
    /// translate by T, then rotate to align Z with dir, scaling X and Y by radis
    void gleTransAlignZ(Vector3 const& dir, Vector3 const& pos, real scale, real radius);

    void setClipPlane(GLenum, Vector1 const& dir, Vector1 const& pos);
    void setClipPlane(GLenum, Vector2 const& dir, Vector2 const& pos);
    void setClipPlane(GLenum, Vector3 const& dir, Vector3 const& pos);

    //------------------------------------------------------------------------------
#pragma mark -

    /// call glVertex() along a centered 2D circle of radius 1 in plane XY
    void gleCircle();
    void gleDisc();
    /// draw a triangle of radius 1 in plane XY, normals pointing in +Z
    void gleTriangleS();
    void gleTriangleL();
    /// draw a triangle of radius 1 in plane XY, normals pointing in +Z
    void gleNablaS();
    void gleNablaL();
    /// draw a square of radius 1 in plane XY, normals pointing in +Z
    void gleSquareL();
    void gleSquareS();
    /// draw a rectangle of radius 1 in plane XY, normals pointing in +Z
    void gleRectangleL();
    void gleRectangleS();
    /// draw a PLUS of radius 1 in plane XY, normals pointing in +Z
    void glePlusS();
    void glePlusL();
    /// draw a pentagon of radius 1 in plane XY, normals pointing in +Z
    void glePentagonS();
    void glePentagonL();
    /// draw an hexagon of surface M_PI in plane XY, normals pointing in +Z
    void gleHexagonS();
    void gleHexagonL();
    /// draw a star of radius 1 in plane XY, normals pointing in +Z
    void gleStarS();
    void gleStarL();
    
    /// draw a sphere of radius 1 at origin, using a refined icosahedron
    void gleSphereN(int);
    /// draw a sphere of radius 1 at origin
    void gleSphere1();
    /// draw a nice sphere of radius 1 at origin
    void gleSphere2();
    /// draw a very nice sphere of radius 1 at origin
    void gleSphere4();
    /// draw a very nice sphere of radius 1 at origin
    void gleSphere8();
    /// draw a icosahedron of radius 1
    void gleIcosahedron1();
    /// display a Cube in 3D and a Square in 2D
    void gleCube1();
    /// draw Torus of radius `rad` and thickness `thick`
    void gleTorus(GLfloat rad, GLfloat thick, size_t inc = 1);
    
    /// draw an open tube from B to T along Z, of diameter 1
    void gleTube0(GLfloat B, GLfloat T, int inc);
    /// draw an open tube from B to T along Z, of diameter 1
    void gleTube0(GLfloat B, GLfloat RB, GLfloat T, GLfloat RT, int inc);
    /// draw a rough open tube along Z, of diameter 1 and length 1
    inline void gleTube1()     { gleTube0(0, 1, 8); }
    /// draw an open tube along Z, of diameter 1 and length 1
    inline void gleTube2()     { gleTube0(0, 1, 4); }
    /// draw a nice open tube along Z, of diameter 1 and length 1
    inline void gleTube4()     { gleTube0(0, 1, 2); }
    /// draw a nicer open tube along Z, of diameter 1 and length 1
    inline void gleTube8()     { gleTube0(0, 1, 1); }
    /// draw a tube along Z, of diameter 1 and length 1.5, Z=[-0.25, 1.25]
    inline void gleLongTube1() { gleTube0(-0.25f, 1.25f, 4); }
    /// draw a nicer tube along Z, of diameter 1 and length 1.5, Z=[-0.25, 1.25]
    inline void gleLongTube2() { gleTube0(-0.25f, 1.25f, 2); }
    /// draw an open tube along Z, of diameter 1 and length 1, Z=[0, 1]
    void gleHexTube1(GLfloat Zmin, GLfloat Zmax);
    /// draw a closed tube along Z, or diameter 1 and length 1
    void gleCylinder1();

    /// draw a 3-portion cylinder with a larger central section
    void gleBarrel1();
    /// display a cone directed along Z, of radius 1 in Z=[B, T], possibly closed by a disc
    void gleCone0(GLfloat B, GLfloat T, bool closed);
    /// display an open cone directed along Z, of radius 1 in Z=[0, 1]
    inline void gleCone1() { gleTube0(0, 1, 1, 0.25, 4); }
    /// display a closed cone directed along Z, of radius 1 in Z=[-1, +2]
    inline void gleLongCone1() { gleCone0(-1, 2, true); }
    /// display a cylindrical box, directed along Z, of length 1, radius 1 in Z=[-0.5, +0.5]
    void gleCylinderZ();
    /// display a dumbbell aligned with the Z axis, or radius 1/3, lenth 1
    void gleDumbbell1();
    /// display 3 arrow fins aligned with the Z axis, or radius 1, lenth 2, Z=[-0.5, 1.5]
    void gleArrowTail1();

    /// draw a circular band composed of little triangles
    void gleArrowedBand(unsigned nb_triangles, real width);
    /// draw 3 Arrowed Bands defining 8 quadrants on the sphere of radius 1
    void gleThreeBands(unsigned nb_triangles);
    
    /// a rectangle ( rect = [ left, bottom, right, top ] )
    void gleRectangle(const int rect[4]);
    
    /// a rectangle with cut corners
    void gleNiceRectangle(const int rect[4], int);

    //------------------------------------------------------------------------------

    inline void gleDumbbellB()     { gleDumbbell1();    }
    inline void gleIcosahedronB()  { gleIcosahedron1(); }
    
    inline void gleArrowTailB()    { gleArrowTail1();   }
    inline void gleCircleB()       { gleCircle();       }
    inline void gleDiscB()         { gleDisc();         }
    inline void gleCylinderB()     { gleCylinderZ();    }
    inline void gleConeB()         { gleCone1();        }
    inline void gleLongConeB()     { gleLongCone1();    }

    void gleTube1B();
    void gleTube2B();
    void gleTube4B();
    void gleTube8B();
    void gleLongTube1B();
    void gleLongTube2B();
    void gleHexTube1B();
    
    void gleSphere1B();
    void gleSphere2B();
    void gleSphere4B();
    void gleSphere8B();

    //------------------------------------------------------------------------------
#pragma mark -
    
    /// display a surface of revolution around the Z axis
    void gleRevolution(GLfloat (*radius)(GLfloat), GLfloat, GLfloat, GLfloat);
    
    /// display back first, and then front
    void gleDualPass(void primitive());
    
    /// draw the object specified by obj, scaled by radius
    void gleObject(real radius, void (*obj)());
    
    /// draw 'obj' scaled by radius at position 'x'
    void gleObject(Vector1 const& x, real radius, void (*obj)());
    void gleObject(Vector2 const& x, real radius, void (*obj)());
    void gleObject(Vector3 const& x, real radius, void (*obj)());
 
    /// draw 'obj' scaled by radius at position 'x', oriented along 'd'
    void gleObject(Vector1 const& x, Vector1 const& d, real radius, void (*obj)());
    void gleObject(Vector2 const& x, Vector2 const& d, real radius, void (*obj)());
    void gleObject(Vector3 const& x, Vector3 const& d, real radius, void (*obj)());

    /// draw 'obj' scaled by radius at position 'x', oriented along 'd'
    void gleObject(Vector1 const& a, Vector1 const& b, void (*obj)());
    void gleObject(Vector2 const& a, Vector2 const& b, void (*obj)());
    void gleObject(Vector3 const& a, Vector3 const& b, void (*obj)());
    
    /// draw 'obj' scaled by radius at position 'x', oriented along 'd'
    void gleObject(Vector1 const& x, Vector1 const& d, real radius, real length, void (*obj)());
    void gleObject(Vector2 const& x, Vector2 const& d, real radius, real length, void (*obj)());
    void gleObject(Vector3 const& x, Vector3 const& d, real radius, real length, void (*obj)());

    //------------------------------------------------------------------------------

    /// draw 'obj' with its ends at [a,b], of specified radius
    void gleTube(Vector1 const& a, Vector1 const& b, real radius, void (*obj)()=gleTube1B);
    void gleTube(Vector2 const& a, Vector2 const& b, real radius, void (*obj)()=gleTube1B);
    void gleTube(Vector3 const& a, Vector3 const& b, real radius, void (*obj)()=gleTube1B);
    
    /// draw a band from A to B, with specified radius
    void gleBand(Vector2 const& a, Vector2 const& b, real);
    void gleBand(Vector3 const& a, Vector3 const& b, real);

    /// draw a band from A to B, with specified radius in A and B
    void gleBand(Vector1 const& a, real, Vector1 const& b, real);
    void gleBand(Vector2 const& a, real, Vector2 const& b, real);
    
    /// draw a band from A to B, with specified radius and colors in A and B
    void gleBand(Vector1 const& a, real, gle_color, Vector1 const& b, real, gle_color);
    void gleBand(Vector2 const& a, real, gle_color, Vector2 const& b, real, gle_color);

    /// draw symbol linking A to B
    void gleMan(Vector2 const& a, Vector2 const&, Vector2 const& b, Vector2 const&);
    void gleMan(Vector2 const&, Vector2 const&, gle_color,
                Vector2 const&, Vector2 const&, gle_color);
    /// draw symbol linking A to B
    void gleCross(Vector2 const& a, Vector2 const&, Vector2 const& b, Vector2 const&, real);
    void gleBar(Vector3 const& a, Vector3 const& da, Vector3 const& b, Vector3 const& db, real);
    
    /// draw two discs in A and B, connected with a line
    void gleDumbbell(Vector2 const& a, Vector2 const& b, GLfloat diameter);
    /// draw two spheres in A and B, connected with a cylinder
    void gleDumbbell(Vector3 const& a, Vector3 const& b, GLfloat diameter);

    /// display cone, dir should be normalized
    void gleCone(Vector1 const& center, Vector1 const& dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCone(Vector2 const& center, Vector2 const& dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCone(Vector3 const& center, Vector3 const& dir, real scale);
    
    /// display arrow-head, dir should be normalized
    void gleCylinder(Vector1 const& center, Vector1 const& dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCylinder(Vector2 const& center, Vector2 const& dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCylinder(Vector3 const& center, Vector3 const& dir, real scale);

    /// display arrow-head, dir should be normalized
    void gleArrowTail(Vector1 const& center, Vector1 const& dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleArrowTail(Vector2 const& center, Vector2 const& dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleArrowTail(Vector3 const& center, Vector3 const& dir, real scale);

    /// draw an arrow with ends [a,b], of specified radius
    void gleArrow(Vector1 const& a, Vector1 const& b, real radius);
    void gleArrow(Vector2 const& a, Vector2 const& b, real radius);
    void gleArrow(Vector3 const& a, Vector3 const& b, real radius);
    
    /// Display link between 2 positions
    void drawLink(Vector const& a, Vector const& b);
    /// Display link between 2 positions, with resting length
    void drawLink(Vector const& a, Vector const& ab, real len);
    /// Display link between 3 positions
    void drawLink(Vector const& a, Vector const& ab, Vector const& c);
    /// Display link between 4 positions
    void drawLink(Vector const& a, Vector const& ab, Vector const& dc, Vector const& d);

    //------------------------------------------------------------------------------
#pragma mark -

    /// return height in pixel of GLUT font
    int  gleLineHeight(void* font);

    /// compute size of text
    int gleComputeTextSize(const char text[], void* font, int& lines);
    
    /// display text on a rectangle of color `bcol`, in a corner of the center of the display window
    void gleDrawText(const char text[], void* font, gle_color bcol, int position, int width, int height);
    
    /// draw text at the current OpenGL raster position and raster color
    void gleBitmapText(const char text[], void* font = nullptr, GLfloat vshift = 0);
    
    /// draw `text` at position `pos`
    void gleDrawText(Vector1 const& pos, const char text[], void* font);
    /// draw `text` at position `pos`
    void gleDrawText(Vector2 const& pos, const char text[], void* font);
    /// draw `text` at position `pos`
    void gleDrawText(Vector3 const& pos, const char text[], void* font);
                        
    //------------------------------------------------------------------------------
#pragma mark -
    
    /// draw pixel array `rgba` containing 4 bytes per pixels
    void gleDrawPixels(int width, int height, int nbc, GLubyte rgba[], Vector2 pos, Vector2 dx, Vector2 dy);
    
    /// display rectangle specified in pixel-coordinates
    void gleDrawRectangle(const int rect[4], int window_width, int window_height);
    
    /// draw a rectangle to indicate the GLUT window-resize handle
    void gleDrawResizeBox(int window_width, int window_height);
    
    /// draw a set of 2 or 3 axes, depending on `dim`
    void gleDrawAxes(real size, int dim);
    
    /// convert OpenGL error code to string
    const char* gleErrorString(GLenum code);

    /// check and print OpenGL error(s)
    void gleReportErrors(FILE*, const char* msg);
 
    /// print some info for debugging purpose
    void dump();
}


#ifdef __SSE3__
inline float invsqrt(float x)
{
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
}
#else
inline float invsqrt(float x)
{
    return 1.0f/sqrtf(x);
}
#endif


#endif
