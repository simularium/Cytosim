// Cytosim was created by Francois Nedelec. Copyright 2022 Cambridge University

/*
 Francois Nedelec, Nov. 2003,  nedelec@embl.de
 To compile on macOS:
 g++ test_opengl.cc -framework GLUT -framework openGL
 On Linux:
 g++ test_opengl.cc -L/usr/X11R6/lib -lglut -lGL -lGLU -lXt -lX11
 
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/glut.h>
#endif


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

#ifndef GL_DEPTH_CLAMP
#define GL_DEPTH_CLAMP 0x864F
#endif

unsigned delay = 13;       //delay 13 == 75 Hz display
float angle = 0;
float delta = 0.2;
float linewidth = 3.0;
float pointSize = 16;
float cubeSize = 0.25;

int transparency = 0;

// view range
float range = 1.0;

// window size:
int winW = 800;
int winH = 800;

//------------------------------------------------------------------------------

void printDepthRange(int X, int Y, int W, int H)
{
    size_t S = W * H;
    float * pixels = new float[S];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(X, Y, W, H, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
    
    if ( glGetError() == GL_NO_ERROR )
    {
        float i = 2, s = -1;
        for ( size_t u = 0; u < S; ++u )
        {
            i = std::min(i, pixels[u]);
            if ( pixels[u] < 1 ) s = std::max(s, pixels[u]);
        }
        printf("\rOpenGL depth buffer range [ %8.6f %8.6f ]", i, s);
    }
    delete[] pixels;
}

//------------------------------------------------------------------------------
void printCaps()
{
    GLint fog, depth, blend, clamp;
    glGetIntegerv(GL_BLEND, &blend);
    glGetIntegerv(GL_FOG, &fog);
    glGetIntegerv(GL_DEPTH_TEST, &depth);
    glGetIntegerv(GL_DEPTH_CLAMP, &clamp);
    
    printf("transparency %i - blend %i - fog %i - depth %i - clamp %i",
           transparency, int(blend), int(fog), int(depth), int(clamp));
    
    GLint point_smooth, line_smooth, multisample;
    glGetIntegerv(GL_POINT_SMOOTH, &point_smooth);
    glGetIntegerv(GL_LINE_SMOOTH, &line_smooth);
    glGetIntegerv(GL_MULTISAMPLE, &multisample);

    printf(" - point_smooth %i - line_smooth %i - multisample %i\n",
           int(point_smooth), int(line_smooth), int(multisample));
}


//------------------------------------------------------------------------------
void flip_cap( GLenum cap )
{
    GLint val;
    glGetIntegerv(cap, &val);
    //printf("OpenGL cap %i flipped : %i -> ", cap, val);
    if ( val )
        glDisable(cap);
    else
        glEnable(cap);
    glGetIntegerv(cap, &val);
    //printf("%i\n", val);
}


void processNormalKey(unsigned char c, int x, int y)
{
    switch (c)
    {
        case 'z':
            if ( delay > 1 ) delay /= 2;
            return;
        case 's':
            delay *= 2;
            return;
            
        case ']':
            linewidth += 0.5;
            return;
        case '[':
            if ( linewidth > 1 )
                linewidth -= 0.5;
            return;
            
        case 'c':
            flip_cap(GL_DEPTH_CLAMP);
            break;
        case 'd':
            flip_cap(GL_DEPTH_TEST);
            break;
        case 'f':
            flip_cap(GL_FOG);
            break;
        case 'b':
            flip_cap(GL_BLEND);
            break;
        case 'p':
            flip_cap(GL_POINT_SMOOTH);
            break;
        case 'l':
            flip_cap(GL_LINE_SMOOTH);
            break;
        case 'm':
            flip_cap(GL_MULTISAMPLE);
            break;
        case 't':
            transparency = !transparency;
            break;
            
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
    }
    
    printCaps();
}


//------------------------------------------------------------------------------
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    winW = w;
    winH = h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = w / float( h );
    
    if ( ratio > 1 )
        glOrtho(-range, range, -range/ratio, range/ratio, range, range+4);
    else
        glOrtho(-range*ratio, range*ratio, -range, range, range, range+4);
}


//------------------------------------------------------------------------------
void initGL()
{
    glClearDepth(1);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_POINT_SMOOTH);
    //glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 0);
    glFogf(GL_FOG_END,   4);
    GLfloat rgba[] = { 0.f, 0.f, 0.f, 1.f };
    glFogfv(GL_FOG_COLOR, rgba);
}

void setView(GLfloat angle)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.f, 0.f, -range*2);
    glRotatef(angle, 0.f, 0.f, 1.f);
    glRotatef(angle, 1.f, 0.f, 0.f);
}

//------------------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    setView(angle);
    glLineWidth(linewidth);
    glColor3f(1.f, 1.f, 1.f);
    glutWireCube(1.35);
    
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    glColor3f(1.f, 1.f, 1.f); glVertex3f(0.f, 0.f, 0.f);
    glColor3f(1.f, 0.f, 0.f); glVertex3f(1.f, 0.f, 0.f);
    glColor3f(0.f, 1.f, 0.f); glVertex3f(0.f, 1.f, 0.f);
    glColor3f(0.f, 0.f, 1.f); glVertex3f(0.f, 0.f, 1.f);
    glEnd();
    
    glColor3f(1.f, 0.4f, 0.4f); glTranslatef(-1,0,0);
    glutSolidCube(cubeSize); glTranslatef(1,0,0);
    
    glColor3f(0.4f, 1.f, 0.4f); glTranslatef(0,-1,0);
    glutSolidCube(cubeSize); glTranslatef(0,1,0);
    
    glColor3f(0.4f, 0.4f, 1.f); glTranslatef(0,0,-1);
    glutSolidCube(cubeSize); glTranslatef(0,0,1);

    if ( transparency )
    {
        glColor4f(0.5, 0.5, 0.5, 0.35);
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glutSolidSphere(1, 32, 32);
        glCullFace(GL_BACK);
        glutSolidSphere(1, 32, 32);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
    }
    else
    {
        glColor3f(.5f, .5f, .5f);
        glutSolidSphere(1, 32, 32);
    }
    printDepthRange(0, 0, winW, winH);
    glutSwapBuffers();
    glutReportErrors();
}

//------------------------------------------------------------------------------
void timerFunction(int win)
{
    angle += delta;
    glutPostWindowRedisplay(win);
    //register another timer call back in prop.delay milli-sec:
    glutTimerFunc(delay, timerFunction, win);
}


//------------------------------------------------------------------------------
void print_str(const char * name, GLenum cap)
{
    const GLubyte * str = glGetString(cap);
    printf("%s = %s\n", name, (unsigned char*)str);
}

void print_cap(const char * name, GLenum cap)
{
    GLint b[10];
    glGetIntegerv(cap, b);
    printf("%s = %i\n", name, (int)b[0]);
}

void printInfo()
{
    print_str("VENDOR  ", GL_VENDOR);
    print_str("RENDERER", GL_RENDERER);
    print_str("VERSION ", GL_VERSION);
    
#if 0
    
    GLint minor, major;
    aglGetVersion(&major, &minor);
    printf("AGL VERSION %i.%i\n", int(major), int(minor));
    
#endif

    print_cap("GL_MAX_CLIP_PLANES", GL_MAX_CLIP_PLANES);
    //print_cap("GL_STENCIL_BITS", GL_STENCIL_BITS);
    print_cap("GL_AUX_BUFFERS", GL_AUX_BUFFERS);
    print_cap("STEREO", GL_STEREO);
    //print_cap("GL_STENCIL_TEST", GL_STENCIL_TEST);
    //print_cap("GL_TEXTURE_2D", GL_TEXTURE_2D);
    //print_cap("GL_ALPHA_TEST", GL_ALPHA_TEST);
    //print_cap("GL_DITHER", GL_DITHER);
    print_cap("GL_MAX_SAMPLES", GL_MAX_SAMPLES);

#if ( 0 )
    printf("has keyboard %i\n",            glutDeviceGet(GLUT_HAS_KEYBOARD));
    printf("has mouse %i,  ",              glutDeviceGet(GLUT_HAS_MOUSE));
    printf("with %i buttons\n",            glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS));
    printf("color bit depth %i,  ",        glutGet(GLUT_WINDOW_BUFFER_SIZE));
    printf("alpha bit depth %i\n",         glutGet(GLUT_WINDOW_ALPHA_SIZE));
    printf("Current display is RGBA %i, ", glutGet(GLUT_WINDOW_RGBA));
    printf("Current mode possible %i\n",   glutGet(GLUT_DISPLAY_MODE_POSSIBLE));
    printf("Overlay possible %i\n",        glutLayerGet(GLUT_OVERLAY_POSSIBLE));
#endif
    
    //anti-aliasing of points and lines:
    //printf("GL_POINT_SMOOTH enabled: %i\n", glIsEnabled(GL_POINT_SMOOTH));
    GLfloat s[2];
    glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, s);
    printf("GL_SMOOTH_POINT_SIZE_RANGE: %.2f - %.2f\n", s[0],s[1]);
    
    //printf("GL_LINE_SMOOTH enabled: %i\n", glIsEnabled(GL_LINE_SMOOTH));
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, s);
    printf("GL_SMOOTH_LINE_WIDTH_RANGE: %.2f - %.2f\n", s[0], s[1]);
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, s);
    printf("GL_ALIASED_LINE_WIDTH_RANGE: %.2f - %.2f\n", s[0], s[1]);
}

void printExtensions()
{
    std::cout << "Extensions:\n";
    const GLubyte * str = glGetString(GL_EXTENSIONS);
    unsigned char const* c = str;
    while ( *c )
    {
        if ( isspace(*c) )
            putchar('\n');
        else
            putchar(*c);
        ++c;
    }
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow(argv[0]);
    
    //testglut -e reports some OpenGL info:
    if ( argc > 1 )
    {
        if ( isdigit(argv[1][0]) )
            sscanf(argv[1], "%f", &delta);
        else
        {
            if ( 0 == strncmp(argv[1], "cap", 3) )
                printExtensions();
            else
                printInfo();
            return EXIT_SUCCESS;
        }
    }
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(50, timerFunction, glutGetWindow());
    glutKeyboardFunc(processNormalKey);
    
    initGL();
    
    glutReportErrors();
    glutMainLoop();
}
