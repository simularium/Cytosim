// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cmath>
#include <cctype>
#include <cstdlib>
#include "assert_macro.h"
#include "gle.h"
#include "glut.h"
#include "platonic.h"
#include "smath.h"
#include "simd.h"


namespace gle
{    
    /// sinus
    GLfloat si_[ncircle+1] = { 0 };

    /// cosinus
    GLfloat co_[ncircle+1] = { 0 };
    
    /// vertex buffer objects for tubes
    GLuint tub_buf[12] = { 0 };
    
    /// vertex buffer objects for hex tubes
    GLuint hex_buf[2] = { 0 };

    /// vertex buffer objects for icosahedrons
    GLuint ico_buf[8] = { 0 };
    
    /// number of faces in icosahedrons
    GLuint ico_nfaces[4] = { 0 };
    
    // Fast method to calculate cosine and sinus over the entire circle
    /**
     co[] and si[] should be allocated to hold 'cnt+1' values.
     Fill in with a counter-clockwise circle starting at angle `start`
    */
    void circle(size_t cnt, GLfloat co[], GLfloat si[], GLfloat rad, double start)
    {
        const double theta = 2.0 * M_PI / (double)cnt;
        const double c = cos(theta);
        const double s = sin(theta);
    
        double t;
        double x = rad * cos(start);
        double y = rad * sin(start);
        
        for( size_t n = 0; n < cnt; ++n )
        {
            co[n] = GLfloat(x);
            si[n] = GLfloat(y);
            //apply the rotation matrix
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
            //std::clog << n << " " << x << " " << y << "\n";
        }
        co[cnt] = co[0];
        si[cnt] = si[0];
    }
    
    void arc(size_t cnt, GLfloat co[], GLfloat si[], GLfloat rad,
             double start, double end, GLfloat cx, GLfloat cy )
    {
        const double theta = ( end - start ) / (double)cnt;
        const double c = cos(theta);
        const double s = sin(theta);
    
        double t;
        double x = rad * cos(start);
        double y = rad * sin(start);
        
        for( size_t n = 0; n < cnt; ++n )
        {
            co[n] = GLfloat(x) + cx;
            si[n] = GLfloat(y) + cy;
            //apply the rotation matrix
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
            //std::clog << n << " " << x << " " << y << "\n";
        }
        co[cnt] = co[0];
        si[cnt] = si[0];
    }

    /** Calculates a full circle with only one call to trigonometric functions */
    void circle(size_t cnt, GLfloat cosi[], GLfloat rad)
    {
        const double theta = 2.0 * M_PI / (double)cnt;
        const double c = cos(theta);
        const double s = sin(theta);
    
        double t;
        double x = rad;
        double y = 0.0;
        
        for( size_t n = 0; n < cnt; ++n )
        {
            cosi[2*n  ] = GLfloat(x);
            cosi[2*n+1] = GLfloat(y);
            //apply the rotation matrix
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
            //std::clog << n << " " << x << " " << y << "\n";
        }
        cosi[2*cnt  ] = rad;
        cosi[2*cnt+1] = 0.0;
    }
    
    void initialize()
    {
#ifndef __APPLE__
        //need to initialize GLEW on Linux
        const GLenum err = glewInit();
        if ( GLEW_OK != err )
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
            exit(1);
        }
#endif
        circle(ncircle, co_, si_, 1);
        initializeIcoBuffers();
        initializeTubeBuffers();
        std::atexit(release);
    }
    
    void release()
    {
        if ( ico_buf[0] && glIsBuffer(ico_buf[0]) )
            glDeleteBuffers(8, ico_buf);
        ico_buf[0] = 0;
        if ( tub_buf[0] && glIsBuffer(tub_buf[0]) )
            glDeleteBuffers(12, tub_buf);
        tub_buf[0] = 0;
        if ( hex_buf[0] && glIsBuffer(hex_buf[0]) )
            glDeleteBuffers(2, hex_buf);
        hex_buf[0] = 0;
    }
    
    //-----------------------------------------------------------------------
    void gleAlignX(const Vector2 & v)
    {
        const real n = v.norm();
        //warning! this matrix is displayed transposed
        real mat[16] = {
            v.XX, -v.YY,  0,  0,
            v.YY,  v.XX,  0,  0,
            0,        0,  n,  0,
            0,        0,  0,  1 };
        gleMultMatrix(mat);
    }
    
    
    /**
     Graphical elements are aligned in 3D along Z and this function is used
     to rotate them in the XY plane for the 2D display.
     The rotation is chosen such that the Y face of the rotated object points
     down the Z axis. In this way, the lower part of the object is drawn first,
     such that the upper half overwrites it and become the only visible part.
     The display is thus correct even if DEPTH_TEST is disabled.
     */
    void gleAlignZ(const Vector2 & A, const Vector2 & B)
    {
        Vector2 D = B - A;
        real n = D.inv_norm();
        //warning! this matrix is displayed transposed
        real mat[16] = {
            D.YY*n, -D.XX*n,   0,  0,
            0,            0,  -1,  0,
            D.XX,      D.YY,   0,  0,
            A.XX,      A.YY,   0,  1 };
        gleMultMatrix(mat);
    }
    
    
    /**
     `ts` is the transverse scaling done in the XY plane after rotation
     */
    void gleAlignZ(const Vector2 & A, const Vector2 & B, real ts)
    {
        Vector2 D = B - A;
        real S = ts;
        real n = ts / D.norm();
        //warning! this matrix appears here transposed
        real mat[16] = {
            D.YY*n, -D.XX*n,  0,  0,
            0,            0, -S,  0,
            D.XX,      D.YY,  0,  0,
            A.XX,      A.YY,  0,  1 };
        gleMultMatrix(mat);
    }
    
    
    void gleRotate(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, bool inverse)
    {
        real mat[16];
        for ( int ii = 0; ii < 3; ++ii )
        {
            if ( inverse )
            {
                mat[4*ii  ] = v1[ii];
                mat[4*ii+1] = v2[ii];
                mat[4*ii+2] = v3[ii];
            }
            else
            {
                mat[ii  ]   = v1[ii];
                mat[ii+4]   = v2[ii];
                mat[ii+8]   = v3[ii];
            }
            mat[ii+12]  = 0;
            mat[ii*4+3] = 0;
        }
        mat[15] = 1;
        gleMultMatrix(mat);
    }
    
    
    void gleTransRotate(const Vector3 & v1, const Vector3 & v2,
                        const Vector3 & v3, const Vector3 & vt)
    {
        //warning! this matrix is displayed here transposed
        real mat[16] = {
            v1.XX, v1.YY, v1.ZZ, 0,
            v2.XX, v2.YY, v2.ZZ, 0,
            v3.XX, v3.YY, v3.ZZ, 0,
            vt.XX, vt.YY, vt.ZZ, 1};
        gleMultMatrix(mat);
    }
    
    // set rotation to align Z with 'dir' and translate to 'pos'
    void gleTransAlignZ(const Vector3& A, Vector3 const& B, real R)
    {
        float x = float(B.XX-A.XX);
        float y = float(B.YY-A.YY);
        float z = float(B.ZZ-A.ZZ);
        float N = invsqrt(x*x+y*y+z*z);
        float vec[3] = { N*x, N*y, N*z };
        float mat[16] = {
            0, 0, 0, 0,
            0, 0, 0, 0,
            x, y, z, 0,
            float(A.XX), float(A.YY), float(A.ZZ), 1};
        sMath::orthonormal(vec, mat, mat+4, (GLfloat)R);
        glMultMatrixf(mat);
    }
    
    // set rotation to align Z with 'dir' and scale S along `dir` and R orthogonally
    void gleTransAlignZ(const Vector3& dir, Vector3 const& pos, real S, real R)
    {
        float x = float(dir.XX);
        float y = float(dir.YY);
        float z = float(dir.ZZ);
        float N = invsqrt(x*x+y*y+z*z);
        float X = (GLfloat)S;
        float vec[3] = { N*x, N*y, N*z };
        float mat[16] = {
            0, 0, 0, 0,
            0, 0, 0, 0,
            X*vec[0], X*vec[1], X*vec[2], 0,
            float(pos.XX), float(pos.YY), float(pos.ZZ), 1};
        sMath::orthonormal(vec, mat, mat+4, (GLfloat)R);
        glMultMatrixf(mat);
    }

    void setClipPlane(GLenum glp, Vector1 const& dir, Vector1 const& pos)
    {
        GLdouble eq[4] = { dir.XX, 0, 0, -dot(dir, pos) };
        glClipPlane(glp, eq);
    }
    
    void setClipPlane(GLenum glp, Vector2 const& dir, Vector2 const& pos)
    {
        GLdouble eq[4] = { dir.XX, dir.YY, 0, -dot(dir, pos) };
        glClipPlane(glp, eq);
    }
    
    void setClipPlane(GLenum glp, Vector3 const& dir, Vector3 const& pos)
    {
        GLdouble eq[4] = { dir.XX, dir.YY, dir.ZZ, -dot(dir, pos) };
        glClipPlane(glp, eq);
    }
    
    
    //-----------------------------------------------------------------------
#pragma mark - 2D Primitives
    
    void gleTriangleS()
    {
        constexpr GLfloat H = 0.8660254037844386f; //sqrt(3)/2;
        const GLfloat pts[] = {
             0,  1.0, 0,
            -H, -0.5, 0,
             H, -0.5, 0 };

        const GLfloat dir[] = {
            0, 0, 1.0,
            0, 0, 1.0,
            0, 0, 1.0 };
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, pts);
        glNormalPointer(GL_FLOAT, 0, dir);
        glDrawArrays(GL_TRIANGLES, 0, 1);
        glDisableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    
    void gleTriangleL()
    {
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        const GLfloat H = 0.8660254037844386f; //sqrt(3)/2;
        glVertex2f( 0,  1.0);
        glVertex2f(-H, -0.5);
        glVertex2f( H, -0.5);
       glEnd();
    }
    
    //-----------------------------------------------------------------------
    
    void gleNablaS()
    {
        glBegin(GL_TRIANGLES);
        glNormal3f(0, 0, 1);
        const GLfloat H = 0.8660254037844386f; //sqrt(3)/2;
        glVertex2f( 0, -1.0);
        glVertex2f( H,  0.5);
        glVertex2f(-H,  0.5);
        glEnd();
    }
    
    void gleNablaL()
    {
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        const GLfloat H = 0.8660254037844386f; //sqrt(3)/2;
        glVertex2f( 0, -1.0);
        glVertex2f( H,  0.5);
        glVertex2f(-H,  0.5);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    void gleSquareS()
    {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f( 1,  1);
        glVertex2f(-1,  1);
        glVertex2f(-1, -1);
        glVertex2f( 1, -1);
        glEnd();
    }
    
    void gleSquareL()
    {
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        glVertex2f( 1,  1);
        glVertex2f(-1,  1);
        glVertex2f(-1, -1);
        glVertex2f( 1, -1);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    void gleRectangleS()
    {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f( 1,  0.5);
        glVertex2f(-1,  0.5);
        glVertex2f(-1, -0.5);
        glVertex2f( 1, -0.5);
        glEnd();
    }
    
    void gleRectangleL()
    {
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        glVertex2f( 1,  0.5);
        glVertex2f(-1,  0.5);
        glVertex2f(-1, -0.5);
        glVertex2f( 1, -0.5);
        glEnd();
    }
    
    
    //-----------------------------------------------------------------------
    /// draw pentagon that has the same surface as a disc of radius 1.
    void glePentagonS()
    {
        const GLfloat A = (GLfloat)M_PI * 0.1;
        const GLfloat B = (GLfloat)M_PI * 0.3;
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f(0, 0);
        const GLfloat R  = 1.3512958724134987f; //sqrt( 4 * M_PI / sqrt( 25 + 10 * sqrt(5)) );
        const GLfloat C1 = R * cosf(A), S1 = R * sinf(A);
        const GLfloat C3 = R * cosf(B), S3 = R * sinf(B);
        
        glVertex2f(  0,  R);
        glVertex2f(-C1,  S1);
        glVertex2f(-C3, -S3);
        glVertex2f( C3, -S3);
        glVertex2f( C1,  S1);
        glVertex2f(  0,  R);
        glEnd();
    }
    
    void glePentagonL()
    {
        const GLfloat A = (GLfloat)M_PI * 0.1;
        const GLfloat B = (GLfloat)M_PI * 0.3;
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        const GLfloat R  = 1.3512958724134987f; //sqrt( 4 * M_PI / sqrt( 25 + 10 * sqrt(5)) );
        const GLfloat C1 = R * cosf(A), S1 = R * sinf(A);
        const GLfloat C3 = R * cosf(B), S3 = R * sinf(B);
        
        glVertex2f(  0,  R);
        glVertex2f(-C1,  S1);
        glVertex2f(-C3, -S3);
        glVertex2f( C3, -S3);
        glVertex2f( C1,  S1);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    /// draw hexagon that has the same surface as a disc of radius 1.
    void gleHexagonS()
    {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f(0, 0);
        const GLfloat R = 1.0996361107912678f; //sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
        const GLfloat H = R * 0.8660254037844386f; // sqrtf(3)/2;
        const GLfloat X = R * 0.5f;
        glVertex2f( R,  0);
        glVertex2f( X,  H);
        glVertex2f(-X,  H);
        glVertex2f(-R,  0);
        glVertex2f(-X, -H);
        glVertex2f( X, -H);
        glVertex2f( R,  0);
        glEnd();
    }
    
    void gleHexagonL()
    {
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        const GLfloat R = 1.0996361107912678f; //sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
        const GLfloat H = R * 0.8660254037844386f; // sqrtf(3)/2;
        const GLfloat X = R * 0.5f;
        glVertex2f( R,  0);
        glVertex2f( X,  H);
        glVertex2f(-X,  H);
        glVertex2f(-R,  0);
        glVertex2f(-X, -H);
        glVertex2f( X, -H);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    
    void gleCircle()
    {
        glNormal3f(0, 0, 1);
        glBegin(GL_LINE_LOOP);
        for( size_t n = 0; n <= ncircle; ++n )
            glVertex2f(co_[n], si_[n]);
        glEnd();
    }
    
    void gleDisc()
    {
        glNormal3f(0, 0, 1);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0);
        for( size_t n = 0; n <= ncircle; ++n )
            glVertex2f(co_[n], si_[n]);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    
    void gleStarS()
    {
        const GLfloat A = (GLfloat)M_PI * 0.1;
        const GLfloat B = (GLfloat)M_PI * 0.3;
        const GLfloat R  = 1.2f, H = -0.6f;
        const GLfloat C1 = R * cosf(A), S1 = R * sinf(A);
        const GLfloat C3 = R * cosf(B), S3 = R * sinf(B);

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f(0, 0);
        glVertex2f(    0,     R);
        glVertex2f( H*C3, -H*S3);
        glVertex2f(  -C1,    S1);
        glVertex2f( H*C1,  H*S1);
        glVertex2f(  -C3,   -S3);
        glVertex2f(    0,   H*R);
        glVertex2f(   C3,   -S3);
        glVertex2f(-H*C1,  H*S1);
        glVertex2f(   C1,    S1);
        glVertex2f(-H*C3, -H*S3);
        glVertex2f(    0,     R);
        glEnd();
    }
    
    void gleStarL()
    {
        const GLfloat A = (GLfloat)M_PI * 0.1;
        const GLfloat B = (GLfloat)M_PI * 0.3;
        const GLfloat R  = 1.2f, H = -0.6f;
        const GLfloat C1 = R * cosf(A), S1 = R * sinf(A);
        const GLfloat C3 = R * cosf(B), S3 = R * sinf(B);
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        glVertex2f(    0,     R);
        glVertex2f( H*C3, -H*S3);
        glVertex2f(  -C1,    S1);
        glVertex2f( H*C1,  H*S1);
        glVertex2f(  -C3,   -S3);
        glVertex2f(    0,   H*R);
        glVertex2f(   C3,   -S3);
        glVertex2f(-H*C1,  H*S1);
        glVertex2f(   C1,    S1);
        glVertex2f(-H*C3, -H*S3);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    
    void glePlusS()
    {
        const GLfloat R = 1.1f;
        const GLfloat C = 0.4f;
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f( R,  C);
        glVertex2f(-R,  C);
        glVertex2f(-R, -C);
        glVertex2f( R, -C);
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f( C,  R);
        glVertex2f(-C,  R);
        glVertex2f(-C,  C);
        glVertex2f( C,  C);
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex2f( C, -C);
        glVertex2f(-C, -C);
        glVertex2f(-C, -R);
        glVertex2f( C, -R);
        glEnd();
    }
    
    void glePlusL()
    {
        const GLfloat R = 1.2f;
        const GLfloat C = 0.6f;
        
        glBegin(GL_LINE_LOOP);
        glNormal3f(0, 0, 1);
        glVertex2f( C,  R);
        glVertex2f(-C,  R);
        glVertex2f(-C,  C);
        glVertex2f(-R,  C);
        glVertex2f(-R, -C);
        glVertex2f(-C, -C);
        glVertex2f(-C, -R);
        glVertex2f( C, -R);
        glVertex2f( C, -C);
        glVertex2f( R, -C);
        glVertex2f( R,  C);
        glVertex2f( C,  C);
        glEnd();
    }
    
    
    //-----------------------------------------------------------------------
#pragma mark - Tubes
    
    void gleTube0(GLfloat B, GLfloat T, int inc)
    {
        assert_true( B <= T );
        glBegin(GL_TRIANGLE_STRIP);
        for( size_t n = 0; n <= ncircle; n += inc )
        {
            glNormal3f(co_[n], si_[n], 0);
            glVertex3f(co_[n], si_[n], T);
            glVertex3f(co_[n], si_[n], B);
        }
        glEnd();
    }

    void gleTube0(GLfloat B, GLfloat rB, GLfloat T, GLfloat rT, int inc)
    {
        assert_true( B <= T );
        const GLfloat N = 1.f/sqrtf((T-B)*(T-B)+(rT-rB)*(rT-rB));
        const GLfloat C = N * (T-B);
        const GLfloat S = N * (rB-rT);
        glBegin(GL_TRIANGLE_STRIP);
        for( size_t n = 0; n <= ncircle; n += inc )
        {
            glNormal3f(C*co_[n], C*si_[n], S);
            glVertex3f(rT*co_[n], rT*si_[n], T);
            glVertex3f(rB*co_[n], rB*si_[n], B);
        }
        glEnd();
    }

    void initTubeBuffer(GLuint buf1, GLuint buf2, GLfloat A, GLfloat B, size_t inc)
    {
        size_t sec = ncircle / inc;
        size_t nbf = 6 * sec + 6;     // number of coordinates
        GLfloat pos[nbf], dir[nbf];
        assert_true( A <= B );

        for( size_t n = 0, p = 0; n < nbf; n += 6, p += inc )
        {
            GLfloat c = co_[p];
            GLfloat s = si_[p];
            
            dir[n  ] = c;
            dir[n+1] = s;
            dir[n+2] = 0;
            
            dir[n+3] = c;
            dir[n+4] = s;
            dir[n+5] = 0;

            pos[n  ] = c;
            pos[n+1] = s;
            pos[n+2] = B;

            pos[n+3] = c;
            pos[n+4] = s;
            pos[n+5] = A;
        }
        glBindBuffer(GL_ARRAY_BUFFER, buf1);
        glBufferData(GL_ARRAY_BUFFER, nbf*sizeof(GLfloat), pos, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, buf2);
        glBufferData(GL_ARRAY_BUFFER, nbf*sizeof(GLfloat), dir, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //gleReportErrors(stderr, "initTubeBuffer");
    }

    void drawTubeBuffer(GLint buf1, GLint buf2, GLint n_triangles)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, buf1);
        glVertexPointer(3, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, buf2);
        glNormalPointer(GL_FLOAT, 0, nullptr);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, n_triangles);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //gleReportErrors(stderr, "drawTubeBuffer");
    }
    
    
    /// draw hexagon that has the same surface as a disc of radius 1.
    void initHexTubeBuffer(GLint buf1, GLint buf2, GLfloat A, GLfloat B)
    {
        const GLfloat R = 1.0996361107912678f; //sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
        const GLfloat C = 0.8660254037844386f; //sqrt(3)/2;
        const GLfloat S = 0.5f;
        const GLfloat H = R * C, X = R * S;
        
        const GLfloat pos[] = {
             R,  0, B,  R,  0, A,
             X,  H, B,  X,  H, A,
            -X,  H, B, -X,  H, A,
            -R,  0, B, -R,  0, A,
            -X, -H, B, -X, -H, A,
             X, -H, B,  X, -H, A,
             R,  0, B,  R,  0, A };
        
        const GLfloat dir[] = {
             1,  0, 0,  1,  0, 0,
             S,  C, 0,  S,  C, 0,
            -S,  C, 0, -S,  C, 0,
            -1,  0, 0, -1,  0, 0,
            -S, -C, 0, -S, -C, 0,
             S, -C, 0,  S, -C, 0,
             1,  0, 0,  1,  0, 0 };

        glBindBuffer(GL_ARRAY_BUFFER, buf1);
        glBufferData(GL_ARRAY_BUFFER, 42*sizeof(GLfloat), pos, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, buf2);
        glBufferData(GL_ARRAY_BUFFER, 42*sizeof(GLfloat), dir, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    
    void initializeTubeBuffers()
    {
        if ( !glIsBuffer(tub_buf[0]) )
        {
            glGenBuffers(12, tub_buf);
            initTubeBuffer(tub_buf[ 0], tub_buf[ 1],  0.0, 1.0, 8);
            initTubeBuffer(tub_buf[ 2], tub_buf[ 3],  0.0, 1.0, 4);
            initTubeBuffer(tub_buf[ 4], tub_buf[ 5],  0.0, 1.0, 2);
            initTubeBuffer(tub_buf[ 6], tub_buf[ 7],  0.0, 1.0, 1);
            initTubeBuffer(tub_buf[ 8], tub_buf[ 9], -0.25, 1.25, 8);
            initTubeBuffer(tub_buf[10], tub_buf[11], -0.25, 1.25, 4);
        }
        if ( !glIsBuffer(hex_buf[0]) )
        {
            glGenBuffers(2, hex_buf);
            initHexTubeBuffer(hex_buf[0], hex_buf[1], 0.0, 1.0);
        }
    }

#if ( 1 )
    // using Vertex Buffer Objects
    void gleTube1B()     { drawTubeBuffer(tub_buf[ 0], tub_buf[ 1], 2+ncircle/4); }
    void gleTube2B()     { drawTubeBuffer(tub_buf[ 2], tub_buf[ 3], 2+ncircle/2); }
    void gleTube4B()     { drawTubeBuffer(tub_buf[ 4], tub_buf[ 5], 2+ncircle  ); }
    void gleTube8B()     { drawTubeBuffer(tub_buf[ 6], tub_buf[ 7], 2+ncircle*2); }
    void gleLongTube1B() { drawTubeBuffer(tub_buf[ 8], tub_buf[ 9], 2+ncircle/4); }
    void gleLongTube2B() { drawTubeBuffer(tub_buf[10], tub_buf[11], 2+ncircle/2); }
    void gleHexTube1B()  { drawTubeBuffer(hex_buf[0], hex_buf[1], 14); }
#else
    // unbuffered functions:
    void gleTube1B()     { gleTube1(); }
    void gleTube2B()     { gleTube2(); }
    void gleTube4B()     { gleTube4(); }
    void gleTube8B()     { gleTube8(); }
    void gleLongTube1B() { gleLongTube1(); }
    void gleLongTube2B() { gleLongTube2(); }
    void gleHexTube1B()  { gleHexTube1(0, 1); }
#endif
 
    void gleTubeZ(GLfloat za, GLfloat ra, gle_color ca, GLfloat zb, GLfloat rb, gle_color cb)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for( size_t n = 0; n <= ncircle; ++n )
        {
            cb.load_load();
            glNormal3f(   co_[n],    si_[n],  0);
            glVertex3f(rb*co_[n], rb*si_[n], zb);
            ca.load_load();
            glNormal3f(   co_[n],    si_[n],  0);
            glVertex3f(ra*co_[n], ra*si_[n], za);
        }
        glEnd();
    }
    

    void gleHexTube1(GLfloat A, GLfloat B)
    {
        /// draw hexagon that has the same surface as a disc of radius 1.
        const GLfloat R = 1.0996361107912678f; //sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
        const GLfloat C = 0.8660254037844386f; //sqrt(3)/2;
        const GLfloat S = 0.5f;
        const GLfloat H = R * C, X = R * S;
        
        const GLfloat pts[] = {
             R,  0, B,  R,  0, A,
             X,  H, B,  X,  H, A,
            -X,  H, B, -X,  H, A,
            -R,  0, B, -R,  0, A,
            -X, -H, B, -X, -H, A,
             X, -H, B,  X, -H, A,
             R,  0, B,  R,  0, A };
        
        const GLfloat dir[] = {
             1,  0, 0,  1,  0, 0,
             S,  C, 0,  S,  C, 0,
            -S,  C, 0, -S,  C, 0,
            -1,  0, 0, -1,  0, 0,
            -S, -C, 0, -S, -C, 0,
             S, -C, 0,  S, -C, 0,
             1,  0, 0,  1,  0, 0 };

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, pts);
        glNormalPointer(GL_FLOAT, 0, dir);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        glDisableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    
    
    void gleCylinder1()
    {
        gleTube0(0, 1, 1);
        glTranslatef(0,0,1);
        gleDisc();
        glTranslatef(0,0,-1);
        glRotated(180,0,1,0);
        gleDisc();
    }
    
    //-----------------------------------------------------------------------
#pragma mark - Spheres

#ifdef PLATONIC_H
    
    /// using icosahedrons to render the sphere:
    Platonic::Solid ico1(Platonic::Solid::ICOSAHEDRON, gle::finesse/4);
    Platonic::Solid ico2(Platonic::Solid::ICOSAHEDRON, gle::finesse/2);
    Platonic::Solid ico4(Platonic::Solid::ICOSAHEDRON, gle::finesse);
    Platonic::Solid ico8(Platonic::Solid::ICOSAHEDRON, gle::finesse*2);
    
    void gleSpherePlatonic(Platonic::Solid & ico)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, ico.vertex_data());
        glNormalPointer(GL_FLOAT, 0, ico.vertex_data());
        glDrawElements(GL_TRIANGLES, 3*ico.nb_faces(), GL_UNSIGNED_INT, ico.faces_data());
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    
    void gleSphere1() { gleSpherePlatonic(ico1); }
    void gleSphere2() { gleSpherePlatonic(ico2); }
    void gleSphere4() { gleSpherePlatonic(ico4); }
    void gleSphere8() { gleSpherePlatonic(ico8); }
    
    
    //-----------------------------------------------------------------------
    
    GLuint initializeIcoBuffers(GLuint buf1, GLuint buf2, Platonic::Solid & ico)
    {
        //std::clog << "initializeIco ico " << ico.nb_faces() << std::endl;
        
        // upload vertex data:
        glBindBuffer(GL_ARRAY_BUFFER, buf1);
        glBufferData(GL_ARRAY_BUFFER, 3*ico.nb_vertices()*sizeof(float), ico.vertex_data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // upload indices:
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf2);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*ico.nb_faces()*sizeof(unsigned), ico.faces_data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return ico.nb_faces();
    }
    
    void initializeIcoBuffers()
    {
        if ( !glIsBuffer(ico_buf[0]) )
        {
            glGenBuffers(8, ico_buf);
            ico_nfaces[0] = initializeIcoBuffers(ico_buf[0], ico_buf[1], ico1);
            ico_nfaces[1] = initializeIcoBuffers(ico_buf[2], ico_buf[3], ico2);
            ico_nfaces[2] = initializeIcoBuffers(ico_buf[4], ico_buf[5], ico4);
            ico_nfaces[3] = initializeIcoBuffers(ico_buf[6], ico_buf[7], ico8);
        }
    }
    
    void drawIcoBuffer(GLuint nfaces, GLuint buf1, GLuint buf2)
    {
        if ( glIsBuffer(buf1) )
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            
            glBindBuffer(GL_ARRAY_BUFFER, buf1);
            glVertexPointer(3, GL_FLOAT, 0, nullptr);
            glNormalPointer(GL_FLOAT, 0, nullptr);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf2);
            glDrawElements(GL_TRIANGLES, 3*nfaces, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }
    
    void gleSphere1B() { drawIcoBuffer(ico_nfaces[0], ico_buf[0], ico_buf[1]); }
    void gleSphere2B() { drawIcoBuffer(ico_nfaces[1], ico_buf[2], ico_buf[3]); }
    void gleSphere4B() { drawIcoBuffer(ico_nfaces[2], ico_buf[4], ico_buf[5]); }
    void gleSphere8B() { drawIcoBuffer(ico_nfaces[3], ico_buf[6], ico_buf[7]); }
    
#else
    
    // Without using PlatonicSolid
    void initializeIcoBuffers() { }

    /// using trigonometric functions to draw a ball of radius 1
    void gleSphereF(unsigned inc)
    {
        for ( size_t n = 0; n < ncircle/2; n += inc )
        {
            real U = co[n], R = si[n];
            real L = co[n+inc], S = si[n+inc];
            glBegin(GL_TRIANGLE_STRIP);
            for ( size_t p = 0; p <= ncircle; p += inc )
            {
                glNormal3f(R*co[p], R*si[p], U);
                glVertex3f(R*co[p], R*si[p], U);
                glNormal3f(S*co[p], S*si[p], L);
                glVertex3f(S*co[p], S*si[p], L);
            }
            glEnd();
        }
    }
    
    void gleSphere1() { gleSphereF(8); }
    void gleSphere2() { gleSphereF(4); }
    void gleSphere4() { gleSphereF(2); }
    void gleSphere8() { gleSphereF(1); }
    
    void gleSphere1B() { gleSphereF(8); }
    void gleSphere2B() { gleSphereF(4); }
    void gleSphere4B() { gleSphereF(2); }
    void gleSphere8B() { gleSphereF(1); }

#endif
    
    /// draw a Torus of radius R and a thickness 2*T
    void gleTorus(GLfloat R, GLfloat T, size_t S)
    {
        for ( size_t n = 0; n < ncircle; n += S )
        {
            GLfloat X0 = co_[n  ], Y0 = si_[n  ];
            GLfloat X1 = co_[n+S], Y1 = si_[n+S];
            glBegin(GL_TRIANGLE_STRIP);
            for ( size_t p = 0; p <= ncircle; p += 2*S )
            {
                glNormal3f(X0*co_[p], Y0*co_[p], si_[p]);
                glVertex3f(X0*(R+T*co_[p]), Y0*(R+T*co_[p]), T*si_[p]);
                glNormal3f(X1*co_[p], Y1*co_[p], si_[p]);
                glVertex3f(X1*(R+T*co_[p]), Y1*(R+T*co_[p]), T*si_[p]);
            }
            glEnd();
        }
    }

    
    //-----------------------------------------------------------------------
#pragma mark - 3D primitives
    
    /**
     Draw a cylindrical band on the equator of a sphere of radius 1.
     The band is in the XY plane. The axis of the cylinder is Z.
     The band is made of triangles indicating the clockwise direction.
     */
    void gleArrowedBand(const unsigned nb_triangles, real width)
    {
        GLfloat A = (GLfloat)(2 * M_PI / nb_triangles);
        GLfloat W = (GLfloat)width * A * invsqrt(3.0f);
        GLfloat R = 1.0f / cosf(A*0.5f);
        
        glBegin(GL_TRIANGLES);
        glNormal3f(1, 0, 0);
        glVertex3f(1, 0, W);
        glVertex3f(1, 0,-W);
        for ( unsigned ii = 1; ii < nb_triangles; ++ii )
        {
            GLfloat ang = ii * A;
            GLfloat c = R * cosf(ang);
            GLfloat s = R * sinf(ang);
            
            glNormal3f(c, s, 0);
            glVertex3f(c, s, 0);
            glVertex3f(c, s, W);
            glVertex3f(c, s,-W);
        }
        glNormal3f(1, 0, 0);
        glVertex3f(1, 0, 0);
        glEnd();
    }
    
    
    void gleThreeBands(const unsigned nb_triangles)
    {
        gleArrowedBand(nb_triangles, 0.25);
        glRotated(-90,1,0,0);
        gleArrowedBand(nb_triangles, 0.25);
        glRotated(90,0,1,0);
        gleArrowedBand(nb_triangles, 0.25);
    }
    
    //-----------------------------------------------------------------------
    inline void icoFace(GLfloat* a, GLfloat* b, GLfloat* c)
    {
        glNormal3f((a[0]+b[0]+c[0])/3.0f, (a[1]+b[1]+c[1])/3.0f, (a[2]+b[2]+c[2])/3.0f);
        glVertex3fv(a);
        glVertex3fv(b);
        glVertex3fv(c);
    }
    
    void gleIcosahedron1()
    {
        const GLfloat tau=0.8506508084f;      /* t=(1+sqrt(5))/2, tau=t/sqrt(1+t^2)  */
        const GLfloat one=0.5257311121f;      /* one=1/sqrt(1+t^2) , unit sphere     */
        
        /* Twelve vertices of icosahedron on unit sphere */
        GLfloat pts[] = {
            +tau,  one,    0 , // 0
            -tau, -one,    0 , // 1
            -tau,  one,    0 , // 2
            +tau, -one,    0 , // 3
            +one,   0 ,  tau , // 4
            -one,   0 , -tau , // 5
            +one,   0 , -tau , // 6
            -one,   0 ,  tau , // 7
            0   ,  tau,  one , // 8
            0   , -tau, -one , // 9
            0   , -tau,  one , // 10
            0   ,  tau, -one };// 11
        
        /* The faces are ordered with increasing Z */
        glBegin(GL_TRIANGLES);
        icoFace(pts+3*5, pts+3*6,  pts+3*9);
        icoFace(pts+3*5, pts+3*11, pts+3*6);
        
        icoFace(pts+3*6, pts+3*3,  pts+3*9);
        icoFace(pts+3*2, pts+3*11, pts+3*5);
        icoFace(pts+3*1, pts+3*5,  pts+3*9);
        icoFace(pts+3*0, pts+3*6,  pts+3*11);
        
        icoFace(pts+3*0, pts+3*3,  pts+3*6);
        icoFace(pts+3*1, pts+3*2,  pts+3*5);
        
        icoFace(pts+3*1, pts+3*9,  pts+3*10);
        icoFace(pts+3*0, pts+3*11, pts+3*8);
        icoFace(pts+3*8, pts+3*11, pts+3*2);
        icoFace(pts+3*9, pts+3*3,  pts+3*10);
        
        icoFace(pts+3*0, pts+3*4,  pts+3*3);
        icoFace(pts+3*1, pts+3*7,  pts+3*2);
        
        icoFace(pts+3*0, pts+3*8,  pts+3*4);
        icoFace(pts+3*1, pts+3*10, pts+3*7);
        icoFace(pts+3*3, pts+3*4,  pts+3*10);
        icoFace(pts+3*7, pts+3*8,  pts+3*2);
        
        icoFace(pts+3*4, pts+3*8,  pts+3*7);
        icoFace(pts+3*4, pts+3*7,  pts+3*10);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
    
    void gleCube1()
    {
        constexpr GLfloat R = 0.5;
        constexpr GLfloat pts[] = {
            -R,  R,  R,
             R,  R,  R,
            -R, -R,  R,
             R, -R,  R,
             R, -R, -R,
             R,  R,  R,
             R,  R, -R,
            -R,  R,  R,
            -R,  R, -R,
            -R, -R,  R,
            -R, -R, -R,
             R, -R, -R,
            -R,  R, -R,
             R,  R, -R };
        
        constexpr GLfloat N = 0.5773502692;
        constexpr GLfloat dir[] = {
            -N,  N,  N,
             N,  N,  N,
            -N, -N,  N,
             N, -N,  N,
             N, -N, -N,
             N,  N,  N,
             N,  N, -N,
            -N,  N,  N,
            -N,  N, -N,
            -N, -N,  N,
            -N, -N, -N,
             N, -N, -N,
            -N,  N, -N,
             N,  N, -N };

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, pts);
        glNormalPointer(GL_FLOAT, 0, dir);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        glDisableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    
    void gleCylinderZ()
    {
        const GLfloat T =  0.5;
        const GLfloat B = -0.5;
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f( 0, 0, -1 );
        glVertex3f( 0, 0,  B );
        for ( size_t n = 0; n <= ncircle; ++n )
            glVertex3f(co_[n], -si_[n], B);
        glEnd();
        
        glBegin(GL_TRIANGLE_STRIP);
        for ( size_t n = 0; n <= ncircle; ++n )
        {
            glNormal3f(co_[n], si_[n], 0);
            glVertex3f(co_[n], si_[n], T);
            glVertex3f(co_[n], si_[n], B);
        }
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f( 0, 0, 1 );
        glVertex3f( 0, 0, T );
        for ( size_t n = 0; n <= ncircle; ++n )
            glVertex3f(co_[n], si_[n], T);
        glEnd();
    }
    
    
    void gleCone0(GLfloat B, GLfloat T, bool closed)
    {
        if ( closed )
        {
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f( 0, 0, B );
            glVertex3f( 0, 0, B );
            for ( size_t n = 0; n <= ncircle; ++n )
                glVertex3f(co_[n], -si_[n], B);
            glEnd();
        }
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f( 0, 0, 1 );
        glVertex3f( 0, 0, T );
        const GLfloat S = -1.f/sqrtf((T-B)*(T-B)+1);
        const GLfloat C = (B-T)*S;
        for ( size_t n = 0; n <= ncircle; ++n )
        {
            glNormal3f(C*co_[n], C*si_[n], S);
            glVertex3f(co_[n], si_[n], B);
        }
        glEnd();
    }
    
    /**
     Draw three fins similar to the tail of a V2 rocket
     */
    void gleArrowTail1()
    {
        GLfloat r = 0.1f;  //bottom inner radius
        GLfloat c = 0.5f, d = -0.5f;
        GLfloat s = 0.8660254037844386f; // sqrtf(3)/2
        GLfloat t = -s;
        GLfloat rc = r * c;
        GLfloat rs = r * s;
        GLfloat rt = -rs;
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(  0, -1, 0 );
        glVertex3f( rc, rt, -0.5 );
        glVertex3f(  1,  0, -1.5 );
        glVertex3f(  1,  0,  0.5 );
        glVertex3f(  0,  0,  1.5 );
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(  0, +1, 0 );
        glVertex3f( rc, rs, -0.5 );
        glVertex3f(  0,  0,  1.5 );
        glVertex3f(  1,  0,  0.5 );
        glVertex3f(  1,  0, -1.5 );
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(  s,  d, 0 );
        glVertex3f( rc, rt, -0.5 );
        glVertex3f(  0,  0,  1.5 );
        glVertex3f(  d,  t,  0.5 );
        glVertex3f(  d,  t, -1.5 );
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(  t, c, 0 );
        glVertex3f( -r, 0, -0.5 );
        glVertex3f(  d, t, -1.5 );
        glVertex3f(  d, t,  0.5 );
        glVertex3f(  0, 0,  1.5 );
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(  s, c, 0 );
        glVertex3f( rc, rs, -0.5 );
        glVertex3f(  d,  s, -1.5 );
        glVertex3f(  d,  s,  0.5 );
        glVertex3f(  0,  0,  1.5 );
        glEnd();
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(  t, d, 0 );
        glVertex3f( -r, 0, -0.5 );
        glVertex3f(  0, 0,  1.5 );
        glVertex3f(  d, s,  0.5 );
        glVertex3f(  d, s, -1.5 );
        glEnd();
        
        // closing the bottom gaps
        glBegin(GL_TRIANGLES);
        glNormal3f(  c,  t, -1 );
        glVertex3f( rc, rs, -0.5 );
        glVertex3f( -r,  0, -0.5 );
        glVertex3f(  d,  s, -1.5 );
        
        glNormal3f(  c,  s, -1 );
        glVertex3f( -r,  0, -0.5 );
        glVertex3f( rc, rt, -0.5 );
        glVertex3f(  d,  t, -1.5 );
        
        glNormal3f( -1,  0, -1 );
        glVertex3f( rc, rt, -0.5 );
        glVertex3f( rc, rs, -0.5 );
        glVertex3f(  1,  0, -1.5 );
        glEnd();
    }
    
    
    GLfloat dumbbellRadius(GLfloat z)
    {
        const GLfloat PI = (GLfloat)M_PI;
        return sinf(PI*z) * ( 1.3f + cosf(2*PI*z) );
    }
    
    
    /**
     Draw a surface of revolution around the Z-axis.
     The surface goes from Z to Z_max, and its radius is
     given by the function `radius`(z) provided as argument.
     */
    void gleRevolution(GLfloat (*radius)(GLfloat), GLfloat Z, GLfloat Zmax, GLfloat dZ)
    {
        GLfloat R = radius(Z);
        GLfloat R0, Z0, dR, dN;
        
        while ( Z < Zmax )
        {
            Z0 = Z;
            R0 = R;
            Z += dZ;
            R = radius(Z);
            
            dR = ( R - R0 ) / dZ;
            dN = 1.0f / sqrtf( 1 + dR * dR );
            dR = dR*dN;
            
            glBegin(GL_TRIANGLE_STRIP);
            for ( size_t n = 0; n <= ncircle; ++n )
            {
                glNormal3f(dN*co_[n], dN*si_[n],-dR);
                glVertex3f(R *co_[n], R *si_[n], Z);
                glVertex3f(R0*co_[n], R0*si_[n], Z0);
            }
            glEnd();
        }
    }

    void gleDumbbell1()
    {
        gleRevolution(dumbbellRadius, 0, 1, 0.0625);
    }
    
    GLfloat barrelRadius(GLfloat z)
    {
        return sinf(3.14159265359f*z);
    }
    
    void gleBarrel1()
    {
        gleRevolution(barrelRadius, 0, 1, 0.0625);
    }
    
    //-----------------------------------------------------------------------
#pragma mark - Object Placement
    
    
    /**
     draw back first, and then front of object,
     GL_CULL_FACE should be enabled
     */
    void gleDualPass(void primitive())
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        primitive();
        glCullFace(GL_BACK);
        primitive();
    }
    
    
    void gleObject( const real radius, void (*obj)() )
    {
        glPushMatrix();
        gleScale(radius);
        obj();
        glPopMatrix();
    }
    
    void gleObject( const Vector1 & x, const real radius, void (*obj)() )
    {
        glPushMatrix();
        gleTranslate(x);
        gleScale(radius);
        obj();
        glPopMatrix();
    }
    
    void gleObject( const Vector2 & x, const real radius, void (*obj)() )
    {
        glPushMatrix();
        gleTranslate(x);
        gleScale(radius);
        obj();
        glPopMatrix();
    }
    
    void gleObject( const Vector3 & x, const real radius, void (*obj)() )
    {
        glPushMatrix();
        gleTranslate(x);
        gleScale(radius);
        obj();
        glPopMatrix();
    }
    
    
    //-----------------------------------------------------------------------
    void gleObject(const Vector1 & a, const Vector1 & b, void (*obj)())
    {
        glPushMatrix();
        if ( a.XX < b.XX )
            glRotated( 90, 0.0, 1.0, 0.0);
        else
            glRotated(-90, 0.0, 1.0, 0.0);
        obj();
        glPopMatrix();
    }
    
    void gleObject(const Vector2 & a, const Vector2 & b, void (*obj)())
    {
        glPushMatrix();
        gleAlignZ(a, b);
        obj();
        glPopMatrix();
    }
    
    void gleObject(const Vector3 & a, const Vector3 & b, void (*obj)())
    {
        glPushMatrix();
        gleTransAlignZ(a, b, 1);
        obj();
        glPopMatrix();
    }
    
    
    //-----------------------------------------------------------------------
    void gleObject(const Vector1 & x, const Vector1 & d, const real R, void (*obj)() )
    {
        glPushMatrix();
        gleTranslate(x);
        if ( d.XX < 0 )
            glRotated(90, 0, 0, 1);
        gleScale(R);
        obj();
        glPopMatrix();
    }
    
    void gleObject(const Vector2 & x, const Vector2 & d, const real R, void (*obj)() )
    {
        glPushMatrix();
        gleAlignZ(x, x+d.normalized(R), R);
        obj();
        glPopMatrix();
    }
    
    void gleObject(const Vector3 & x, const Vector3 & d, const real R, void (*obj)() )
    {
        glPushMatrix();
        gleTransAlignZ(d, x, R, R);
        obj();
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
    void gleObject(const Vector1 & x, const Vector1 & d, const real R,
                   const real l, void (*obj)() )
    {
        glPushMatrix();
        gleTranslate(x);
        if ( d.XX < 0 )
            glRotated(90, 0, 0, 1);
        gleScale(l,R,R);
        obj();
        glPopMatrix();
    }
    
    void gleObject(const Vector2 & x, const Vector2 & d, const real R,
                   const real L, void (*obj)() )
    {
        glPushMatrix();
        gleAlignZ(x, x+d.normalized(L), R);
        obj();
        glPopMatrix();
    }
    
    void gleObject(const Vector3 & x, const Vector3 & d, const real R,
                   const real L, void (*obj)() )
    {
        glPushMatrix();
        gleTransAlignZ(d, x, L, R);
        obj();
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
#pragma mark - Tubes
    
    
    void gleTube(const Vector1 & a, const Vector1 & b, real radius, void (*obj)())
    {
        glPushMatrix();
        if ( a.XX < b.XX )
            glRotated(  90, 0.0, 1.0, 0.0 );
        else
            glRotated( -90, 0.0, 1.0, 0.0 );
        gleScale(1,radius,1);
        obj();
        glPopMatrix();
    }
    
    void gleTube(const Vector2 & a, const Vector2 & b, real radius, void (*obj)())
    {
        glPushMatrix();
        gleAlignZ(a, b, radius);
        obj();
        glPopMatrix();
    }
    
    void gleTube(const Vector3 & a, const Vector3 & b, real radius, void (*obj)())
    {
        glPushMatrix();
        gleTransAlignZ(a, b, radius);
        obj();
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
    
    void gleBand(const Vector2 & a, const Vector2 & b, real rad)
    {
        Vector2 d = ( b - a ).orthogonal();
        real n = d.norm();
        if ( n > 0 )
        {
            rad /= n;
            glBegin(GL_TRIANGLE_STRIP);
            gleVertex(a+rad*d);
            gleVertex(a-rad*d);
            gleVertex(b+rad*d);
            gleVertex(b-rad*d);
            glEnd();
        }
    }
    
    
    void gleBand(const Vector1 & a, real ra,
                 const Vector1 & b, real rb)
    {
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(a.XX,+ra);
        gleVertex(a.XX,-ra);
        gleVertex(b.XX,+rb);
        gleVertex(b.XX,-rb);
        glEnd();
    }
    
    void gleBand(const Vector2 & a, real ra,
                 const Vector2 & b, real rb)
    {
        Vector2 d = ( b - a ).orthogonal();
        real n = d.norm();
        if ( n > 0 )
        {
            d /= n;
            glBegin(GL_TRIANGLE_STRIP);
            gleVertex(a-ra*d);
            gleVertex(a+ra*d);
            gleVertex(b-rb*d);
            gleVertex(b+rb*d);
            glEnd();
        }
    }
    
    void gleBand(const Vector1 & a, real ra, gle_color ca,
                 const Vector1 & b, real rb, gle_color cb)
    {
        glBegin(GL_TRIANGLE_STRIP);
        ca.load();
        gleVertex(a.XX,-ra);
        gleVertex(a.XX,+ra);
        cb.load();
        gleVertex(b.XX,-rb);
        gleVertex(b.XX,+rb);
        glEnd();
    }
    
    void gleBand(const Vector2 & a, real ra, gle_color ca,
                 const Vector2 & b, real rb, gle_color cb)
    {
        Vector2 d = ( b - a ).orthogonal();
        real n = d.norm();
        if ( n > 0 )
        {
            d /= n;
            glBegin(GL_TRIANGLE_STRIP);
            ca.load();
            gleVertex(a+ra*d);
            gleVertex(a-ra*d);
            cb.load();
            gleVertex(b+rb*d);
            gleVertex(b-rb*d);
            glEnd();
        }
    }
    
    
    /**
     This will displays a rectangle if the connection is parallel,
     and a hourglass if the connection is antiparallel
     */
    void gleMan(Vector2 const& a, Vector2 const& da,
                Vector2 const& b, Vector2 const& db)
    {
        Vector2 pts[6] = { b-db, b, a-da, a+da, b, b+db };
        assert_true(sizeof(pts)==12*sizeof(double));
#if REAL_IS_DOUBLE
        glVertexPointer(2, GL_DOUBLE, 0, pts);
#else
        glVertexPointer(2, GL_FLOAT, 0, pts);
#endif
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    
    /**
     This will displays a rectangle if the connection is parallel,
     and a hourglass if the connection is antiparallel
     */
    void gleMan(Vector2 const& a, Vector2 const& da, gle_color ca,
                Vector2 const& b, Vector2 const& db, gle_color cb)
    {
        Vector2 pts[6] = { b-db, b, a-da, a+da, b, b+db };
        assert_true(sizeof(pts)==12*sizeof(double));
        GLfloat col[24];
        cb.store(col);
        cb.store(col+4);
        ca.store(col+8);
        ca.store(col+12);
        cb.store(col+16);
        cb.store(col+20);
        glEnableClientState(GL_VERTEX_ARRAY);
#if REAL_IS_DOUBLE
        glVertexPointer(2, GL_DOUBLE, 0, pts);
#else
        glVertexPointer(2, GL_FLOAT, 0, pts);
#endif
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, col);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    
    
    /**
     This will displays a rectangle if the connection is antiparallel,
     and a hourglass if the connection is parallel
     */
    void gleCross(Vector2 const& a, Vector2 const& da,
                  Vector2 const& b, Vector2 const& db, real rad)
    {
        glLineWidth(0.5);
        glBegin(GL_TRIANGLE_FAN);
        gleVertex(a);
        gleVertex(a-rad*da);
        gleVertex(b);
        gleVertex(b-rad*db);
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        gleVertex(a);
        gleVertex(a+rad*da);
        gleVertex(b);
        gleVertex(b+rad*db);
        glEnd();
    }
    
    void gleBar(Vector3 const& a, Vector3 const& da,
                Vector3 const& b, Vector3 const& db, real rad)
    {
        Vector3 ab = normalize( a - b );
        Vector3 ea = cross(ab, da);
        Vector3 eb = cross(ab, db);
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(a-rad*(da-ea));
        gleVertex(a-rad*(da+ea));
        gleVertex(b-rad*(db-eb));
        gleVertex(b-rad*(db+eb));
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(a+rad*(da-ea));
        gleVertex(a+rad*(da+ea));
        gleVertex(b+rad*(db-eb));
        gleVertex(b+rad*(db+eb));
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(a-rad*da);
        gleVertex(a+rad*da);
        gleVertex(b-rad*db);
        gleVertex(b+rad*db);
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(a-rad*da);
        gleVertex(a+rad*da);
        gleVertex(b-rad*db);
        gleVertex(b+rad*db);
        glEnd();
    }
    
    
    /**
     Two hexagons linked by a rectangle
     hexagons have the same surface as a disc of radius 1.
     */
    void gleDumbbell(const Vector2 & a, const Vector2 & b, GLfloat diameter)
    {
        const GLfloat S = 1.0996361107912678f; //sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
        const GLfloat R = diameter * S;
        const GLfloat H = R * 0.8660254037844386f; //0.5f * sqrtf(3);
        const GLfloat X = R * 0.5f;
        
        Vector2 x = ( b - a ).normalized(H);
        Vector2 y = x.orthogonal(X);
        
        glPushMatrix();
        gleTranslate(a);
        
        // this is an hexagon centered around 'a':
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0,0);
        gleVertex(x+y);
        gleVertex(2*y);
        gleVertex(-x+y);
        gleVertex(-x-y);
        gleVertex(-2*y);
        gleVertex(x-y);
        gleVertex(x+y);
        glEnd();
        
        // a band from 'a' to 'b'
        glBegin(GL_TRIANGLE_FAN);
        gleVertex(+y+x);
        gleVertex(-y+x);
        gleVertex(b-a-y-x);
        gleVertex(b-a+y-x);
        glEnd();
        
        // an hexagon centered around 'b'
        gleTranslate(b-a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0,0);
        gleVertex(x+y);
        gleVertex(2*y);
        gleVertex(-x+y);
        gleVertex(-x-y);
        gleVertex(-2*y);
        gleVertex(x-y);
        gleVertex(x+y);
        glEnd();
        
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
#pragma mark - Arrows
    
    void gleCone(const Vector1& pos, const Vector1 & dir, const real scale)
    {
        real dx = scale*dir.XX, cx = pos.XX;
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(cx-dx   , dx);
        gleVertex(cx-dx/2 , 0 );
        gleVertex(cx+dx+dx, 0 );
        gleVertex(cx-dx   ,-dx);
        glEnd();
    }
    
    void gleCone(const Vector2& pos, const Vector2 & dir, const real scale)
    {
        real dx = scale*dir.XX,  cx = pos.XX;
        real dy = scale*dir.YY,  cy = pos.YY;
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex(cx-dx-dy, cy-dy+dx);
        gleVertex(cx-dx/2,  cy-dy/2 );
        gleVertex(cx+dx+dx, cy+dy+dy);
        gleVertex(cx-dx+dy, cy-dy-dx);
        glEnd();
    }
    
    void gleCone(const Vector3 & pos, const Vector3 & dir, const real scale)
    {
        glPushMatrix();
        gleTransAlignZ(dir, pos, scale, scale);
        gleLongConeB();
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
    
    void gleCylinder(const Vector1 & pos, const Vector1 & dir, const real scale)
    {
        real cx = pos.XX;
        glBegin(GL_TRIANGLE_STRIP);
        real dx = scale * dir.XX / 2;
        gleVertex( cx-dx, -scale );
        gleVertex( cx-dx,  scale );
        gleVertex( cx+dx, -scale );
        gleVertex( cx+dx,  scale );
        glEnd();
    }
    
    void gleCylinder(const Vector2 & pos, const Vector2 & dir, const real scale)
    {
        real dx = scale * dir.XX, cx = pos.XX - dx / 2;
        real dy = scale * dir.YY, cy = pos.YY - dy / 2;
        glBegin(GL_TRIANGLE_STRIP);
        gleVertex( cx+dy, cy-dx );
        gleVertex( cx-dy, cy+dx );
        gleVertex( cx+dx+dy, cy+dy-dx );
        gleVertex( cx+dx-dy, cy+dy+dx );
        glEnd();
    }
    
    void gleCylinder(const Vector3 & pos, const Vector3 & dir, const real scale)
    {
        glPushMatrix();
        //build the rotation matrix, assuming dir is normalized
        gleTransAlignZ(dir, pos, scale, scale);
        gleCylinderZ();
        glPopMatrix();
    }
    
    
    //-----------------------------------------------------------------------
    
    void gleArrowTail(const Vector1& pos, const Vector1 & dir, const real scale)
    {
        GLfloat dx = scale * dir.XX;
        GLfloat cx = pos.XX - dx / 2;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f( cx,       0  );
        glVertex2f( cx-dx,   -dx );
        glVertex2f( cx+dx,   -dx );
        glVertex2f( cx+dx+dx, 0  );
        glVertex2f( cx+dx,    dx );
        glVertex2f( cx-dx,    dx );
        glEnd();
    }
    
    void gleArrowTail(const Vector2& pos, const Vector2 & dir, const real scale)
    {
        GLfloat dx = scale * dir.XX;
        GLfloat dy = scale * dir.YY;
        GLfloat cx = pos.XX - 1.5f * dx;
        GLfloat cy = pos.YY - 1.5f * dy;
        GLfloat ex = cx + 2 * dx;
        GLfloat ey = cy + 2 * dy;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f( cx+dx, cy+dy );
        glVertex2f( cx+dy, cy-dx );
        glVertex2f( ex+dy, ey-dx );
        glVertex2f( ex+dx, ey+dy );
        glVertex2f( ex-dy, ey+dx );
        glVertex2f( cx-dy, cy+dx );
        glEnd();
    }
    
    void gleArrowTail(const Vector3& pos, const Vector3 & dir, const real scale)
    {
        glPushMatrix();
        //assuming dir is normalized
        gleTransAlignZ(dir, pos, scale, scale);
        gleArrowTail1();
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
    void gleArrow(const Vector1 & a, const Vector1 & b, real radius)
    {
        glPushMatrix();
        if ( a.XX < b.XX )
            glRotated(  90, 0.0, 1.0, 0.0 );
        else
            glRotated( -90, 0.0, 1.0, 0.0 );
        gleScale(1,radius,1);
        gleTube1B();
        glTranslatef(0, 0, 1);
        glScaled(3.0, 3.0, 3*radius);
        gleLongConeB();
        glPopMatrix();
    }
    
    void gleArrow(const Vector2 & a, const Vector2 & b, real radius)
    {
        glPushMatrix();
        gleAlignZ(a, b, radius);
        gleTube1B();
        glTranslatef(0, 0, 1);
        glScaled(3.0, 3.0, 3*radius);
        gleLongConeB();
        glPopMatrix();
    }
    
    void gleArrow(const Vector3 & a, const Vector3 & b, real radius)
    {
        glPushMatrix();
        gleTransAlignZ(a, b, radius);
        gleTube1B();
        glTranslatef(0, 0, 1);
        glScaled(3.0, 3.0, 3*radius);
        gleLongConeB();
        glPopMatrix();
    }
    
    
    //-----------------------------------------------------------------------
#pragma mark - Links
    
    /// Display link between 2 positions
    void drawLink(Vector const& a, Vector const& b)
    {
        glLineStipple(1, 0xFFFF);
        glBegin(GL_LINES);
        gleVertex(a);
        gleVertex(b);
        glEnd();
    }
    
    /// Display link between 2 positions, with resting length `len`
    void drawLink(Vector const& a, Vector const& ab, real len)
    {
        Vector b = a + ab;
        Vector dx = ab * (( 1 - len / ab.norm() ) / 2);
        glLineStipple(1, 0x3333);
        glBegin(GL_LINES);
        gleVertex(a+dx);
        gleVertex(b-dx);
        glEnd();
        glLineStipple(1, 0xFFFF);
        glBegin(GL_LINES);
        gleVertex(a);
        gleVertex(a+dx);
        gleVertex(b-dx);
        gleVertex(b);
        glEnd();
        glBegin(GL_POINTS);
        gleVertex(a);
        gleVertex(b);
        glEnd();
    }
    
    /// Display link between 3 positions
    void drawLink(Vector const& a, Vector const& ab, Vector const& c)
    {
        Vector b = a + ab;
        glLineStipple(1, 0x7310);
        glBegin(GL_LINES);
        gleVertex(a);
        gleVertex(b);
        glEnd();
        glLineStipple(1, 0xFFFF);
        glBegin(GL_LINES);
        gleVertex(b);
        gleVertex(c);
        glEnd();
        glBegin(GL_POINTS);
        gleVertex(b);
        glEnd();
    }
    
    /// Display link between 4 positions
    void drawLink(Vector const& a, Vector const& ab, Vector const& dc, Vector const& d)
    {
        Vector b = a + ab;
        Vector c = d + dc;
        glLineStipple(1, 0x7171);
        glBegin(GL_LINES);
        gleVertex(a);
        gleVertex(b);
        gleVertex(c);
        gleVertex(d);
        glEnd();
        glLineStipple(1, 0xFFFF);
        glBegin(GL_LINES);
        gleVertex(b);
        gleVertex(c);
        glEnd();
        glBegin(GL_POINTS);
        gleVertex(b);
        gleVertex(c);
        glEnd();
    }
    
    //-----------------------------------------------------------------------
#pragma mark - Text
    
    
    int gleLineHeight(void* font)
    {
        if ( font == GLUT_BITMAP_8_BY_13 )        return 13;
        if ( font == GLUT_BITMAP_9_BY_15 )        return 15;
        if ( font == GLUT_BITMAP_TIMES_ROMAN_10 ) return 11;
        if ( font == GLUT_BITMAP_TIMES_ROMAN_24 ) return 26;
        if ( font == GLUT_BITMAP_HELVETICA_10 )   return 11;
        if ( font == GLUT_BITMAP_HELVETICA_12 )   return 15;
        if ( font == GLUT_BITMAP_HELVETICA_18 )   return 22;
        return 13;
    }
    
    
    /**
     Compute the max width of all the lines in the given text
     This uses GLUT, which should be initialized.
     */
    int gleComputeTextSize(const char text[], void* font, int& lines)
    {
        int width = 0;
        lines = 0;
        int w = 0;
        for (const char* c = text; *c != '\0' ; ++c)
        {
            if ( *c == '\n' )
            {
                if ( w > width ) width = w;
                ++lines;
                w = 0;
            }
            else if ( isspace(*c))
            {
                w += glutBitmapWidth(font, ' ');
            }
            else if ( isprint(*c))
            {
                w += glutBitmapWidth(font, *c);
            }
        }
        if ( w > width )
            width = w;
        if ( width > 0 && lines == 0 )
            lines = 1;
        return width;
    }
    
    //-----------------------------------------------------------------------
    /**
     draw the string character per character using:
     glutBitmapCharacter()
     */
    void gleBitmapText(const char text[], void* font, GLfloat vshift)
    {
        if ( !font )
        {
            font = GLUT_BITMAP_HELVETICA_12;
            vshift = std::copysign(1, vshift) * gleLineHeight(font);
        }
        if ( vshift == 0 )
            vshift = -gleLineHeight(font);
        
        GLfloat ori[4], pos[4];
        glGetFloatv(GL_CURRENT_RASTER_POSITION, ori);
        
        for (const char* p = text; *p; ++p)
        {
            if ( *p == '\n' )
            {
                glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
                glBitmap(0, 0, 0, 0, ori[0]-pos[0], vshift, nullptr);
            }
            else if ( isspace(*p) )
            {
                glutBitmapCharacter(font, ' ');
            }
            else if ( isprint(*p) )
            {
                glutBitmapCharacter(font, *p);
            }
        }
    }
    
    
    /**
     set the current raster position to `w`
     */
    void gleDrawText(const Vector3& vec, const char text[], void* font)
    {
        glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_ALPHA_TEST);
        int lh = gleLineHeight(font);
        gleRasterPos(vec);
        //translate to center the bitmap:
        glBitmap(0,0,0,0,1,-lh/3,nullptr);
        gleBitmapText(text, font, -lh);
        glPopAttrib();
    }
    
    void gleDrawText(const Vector2& w, const char text[], void* font)
    {
        gleDrawText(Vector3(w.XX, w.YY, 0), text, font);
    }
    
    void gleDrawText(const Vector1& w, const char text[], void* font)
    {
        gleDrawText(Vector3(w.XX, 0, 0), text, font);
    }
    
    //-----------------------------------------------------------------------
    
    /**
     The text is displayed in the current color.
     A background rectangle is displayed only if `bcol` is visible.
     
         glColor3f(1,1,1);
         gleDrawText(fKeyString, GLUT_BITMAP_8_BY_13, 0x0, 1);
     
     Possible values for `position`:
     - 0: bottom-left, text going up
     - 1: bottom-right, text going up
     - 2: top-right, text going down
     - 3: top-left, text going down
     - 4: center, text going down
     .
     
     Note: width and height are the current size of the viewport (window)
     */
    void gleDrawText(const char text[], void* font, const gle_color bcol,
                     const int position, int width, int height)
    {
        assert_true( width > 0 );
        assert_true( height > 0 );
        
        if ( !font )
            font = GLUT_BITMAP_9_BY_15;
        
        int lineHeight = gleLineHeight(font);
        int textWidth = 0;
        int nblines = 1;
        
        GLint px, py;
        switch( position )
        {
            case 0: {
                //bottom-left, text going up
                px = lineHeight/2;
                py = lineHeight/2;
            } break;
            case 1: {
                //bottom-right, text going up
                textWidth = gleComputeTextSize(text, font, nblines);
                px = width - textWidth - lineHeight/2;
                if ( px < 0 ) px = 0;
                py = lineHeight/2;
            } break;
            case 2: {
                //top-right, text going down
                textWidth = gleComputeTextSize(text, font, nblines);
                px = width - textWidth - lineHeight/2;
                if ( px < 0 ) px = 0;
                py = height - lineHeight;
                lineHeight *= -1;
            } break;
            default:
            case 3: {
                //top-left, text going down
                px = lineHeight/2;
                py = height - lineHeight;
                lineHeight *= -1;
            } break;
            case 4: {
                //center, text going down
                textWidth = gleComputeTextSize(text, font, nblines);
                px = ( width - textWidth ) / 2;
                if ( px < 0 ) px = 0;
                py = ( height + nblines*lineHeight ) / 2;
                lineHeight *= -1;
            } break;
        }
        
        //set pixel coordinate system:
        glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_ALPHA_TEST);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width, 0, height, 0, 1);
        
        glRasterPos2i(0, 0);
        glBitmap(0, 0, 0, 0, px, py, nullptr);
        
        if ( bcol.visible() )
        {
            glPushAttrib(GL_LIGHTING_BIT|GL_CURRENT_BIT);
            glDisable(GL_LIGHTING);
            int rd = abs(lineHeight);
            int bt = py;
            int tp = py + nblines*lineHeight;
            if ( lineHeight < 0 )
            {
                int x = tp;
                tp = bt;
                bt = x;
            }
            
            int rec[4] = { px-rd, bt, px+textWidth+rd, tp+rd+rd/2+rd/4 };
            
            bcol.load();
            glBegin(GL_TRIANGLE_FAN);
            gleNiceRectangle(rec, 4);
            glEnd();
            
            glPopAttrib();
            
            glLineWidth(1.0);
            glBegin(GL_LINE_STRIP);
            gleNiceRectangle(rec, 4);
            glEnd();
        }
        
        gleBitmapText(text, font, lineHeight);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPopAttrib();
    }
    
    
    //-----------------------------------------------------------------------
#pragma mark - Misc
    
    /**
     Draw an array of pixels using GL_TRIANGLE_STRIP
     
     The array rgba[] should ( nbc * width * height ) bytes,
     containing nbc-components (eg. RGBA) per pixel and
     stored by columns:
     
         load(i,j) = rgba[ nbc*(i+height*j) ]
         0 <= i < height
         0 <= j < width
     
     `pos` is the position of the top-left corner
     `dx` is the direction of the width
     `dy` is the direction of the height
     The magnitudes of `dx` and `dy` indicates the dimensions of a pixel.
     They may be of different magnitudes, and not necessarily orthogonal.
     */
    void gleDrawPixels(int width, int height, int nbc, GLubyte rgba[], Vector2 pos, Vector2 dx, Vector2 dy)
    {
        glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        
        Vector2 left, right;
        GLubyte * col = rgba;
        
        for ( int jj = 0; jj < width; ++jj )
        {
            left  = pos + dx * jj;
            right = left + dx;
            for ( int ii = 0; ii < height; ++ii )
            {
                if ( nbc == 3 )
                    glColor3ubv(col);
                else
                    glColor4ubv(col);
                glBegin(GL_TRIANGLE_STRIP);
                col += nbc;
                gleVertex(left);
                gleVertex(right);
                left  += dy;
                right += dy;
                gleVertex(left);
                gleVertex(right);
                glEnd();
            }
        }
        
        glPopAttrib();
    }
    
    //-----------------------------------------------------------------------
    
    
    /**
     rectangle should be specified as [ left, bottom, right, top ]
     The rectangle will be drawn counter-clockwise
     */
    void gleRectangle(const int rec[4])
    {
        glVertex2i(rec[0], rec[1]);
        glVertex2i(rec[2], rec[1]);
        glVertex2i(rec[2], rec[3]);
        glVertex2i(rec[0], rec[3]);
        glVertex2i(rec[0], rec[1]);
    }
    
    
    void gleNiceRectangle(const int rec[4], const int rad)
    {
        glVertex2i(rec[0], rec[1]+rad);
        glVertex2i(rec[0]+rad, rec[1]);
        glVertex2i(rec[2]-rad, rec[1]);
        glVertex2i(rec[2], rec[1]+rad);
        glVertex2i(rec[2], rec[3]-rad);
        glVertex2i(rec[2]-rad, rec[3]);
        glVertex2i(rec[0]+rad, rec[3]);
        glVertex2i(rec[0], rec[3]-rad);
        glVertex2i(rec[0], rec[1]+rad);
    }
    
    
    void gleDrawRectangle(const int rec[4], int width, int height)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width, 0, height, 0, 1);
        
        //disable advanced features
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glBegin(GL_LINE_LOOP);
        gleRectangle(rec);
        glEnd();
        
        glPopAttrib();
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    
    void gleDrawResizeBox(int width, int height)
    {
        //set the matrices
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(width, 0, 0, height, 0, 1 );
        
        //draw lines at 45 degrees
        glBegin(GL_LINES);
        glVertex2i(16, 1);    glVertex2i(1, 16);
        glVertex2i(12, 1);    glVertex2i(1, 12);
        glVertex2i(8,  1);    glVertex2i(1,  8);
        glVertex2i(4,  1);    glVertex2i(1,  4);
        glEnd();
        
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    
    //-----------------------------------------------------------------------
    void gleDrawAxes(const real size, int dim)
    {
        const GLfloat S = (GLfloat) size;
        const GLfloat R = S * 0.1f;
        
        glMatrixMode(GL_MODELVIEW);
        
        for (int d = 0; d < dim; ++d)
        {
            glPushMatrix();
            switch(d)
            {
                case 0:
                    gle_color(1.0, 0.0, 0.0, 1.0).load_load();
                    glRotatef( 90, 0, 1, 0);
                    break;
                case 1:
                    gle_color(0.0, 1.0, 0.0, 1.0).load_load();
                    glRotatef(-90, 1, 0, 0);
                    glRotatef(180, 0, 0, 1);
                    break;
                case 2:
                    gle_color(0.0, 0.0, 1.0, 1.0).load_load();
                    glRotatef(-90, 0, 0, 1);
                    break;
            }
            glScalef(R/2, R/2, S-R);
            gleTube1B();
            glTranslatef(0, 0, 1);
            glScalef(3, 3, R/(S-R));
            gleLongConeB();
            glPopMatrix();
        }
        // display a white ball at the origin
        gle_color(1.0, 1.0, 1.0, 1.0).load_load();
        glPushMatrix();
        gleScale(R);
        gleSphere4();
        glPopMatrix();
    }
    
    //-----------------------------------------------------------------------
    char const* gleErrorString(GLenum code)
    {
        switch ( code )
        {
            case GL_NO_ERROR:          return "GL_NO_ERROR";
            case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
            case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
            case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
            case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
            case GL_TABLE_TOO_LARGE:   return "GL_TABLE_TOO_LARGE";
            default:                   return "GL_UNKNOWN_ERROR";
        }
    }
    
    /**
     This is similart to glutReportError,
     but the additional argument can provide useful feedback for debugging
     */
    void gleReportErrors(FILE * out, const char* msg)
    {
        GLenum glError = glGetError();
        while ( glError != GL_NO_ERROR )
        {
            fprintf(out, "OpenGL error `%s' %s\n", gleErrorString(glError), msg);
            glError = glGetError();
        }
    }
    
    void print_cap(GLenum cap, const char * str)
    {
        GLint i = glIsEnabled(cap);
        std::clog << str << " " << i << "   ";
    }
    
    void dump()
    {
        GLfloat c[4] = { 0 };
        glGetFloatv(GL_CURRENT_COLOR, c);
        std::clog << "color = " << c[0] << " " << c[1] << " " << c[2] << " " << c[3] << '\n';
        
        print_cap(GL_LIGHTING, "light");
        print_cap(GL_BLEND, "blend");
        print_cap(GL_FOG, "fog");
        print_cap(GL_DEPTH_TEST, "depth");
        print_cap(GL_ALPHA_TEST, "alpha");
        print_cap(GL_STENCIL_TEST, "stencil");
        print_cap(GL_CULL_FACE, "cull");
        print_cap(GL_COLOR_LOGIC_OP, "logic");
        print_cap(GL_COLOR_ARRAY, "array");
        print_cap(GL_COLOR_MATERIAL, "material");
        print_cap(GL_LINE_STIPPLE, "stipple");
        
        std::clog << '\n';
        
#if ( 0 )
        GLint vp[4] = { 0 };
        glGetIntegerv(GL_VIEWPORT, vp);
        std::clog << "viewport = " << vp[0] << " " << vp[1] << " " << vp[2] << " " << vp[3] << '\n';
#endif
    }

}

