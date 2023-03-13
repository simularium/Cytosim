// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
Simple GLUT application
 
To compile on mac OSX:
g++ test_glut.cc -framework GLUT -framework OpenGL

To compile on Windows using Cygwin
g++ test_glut.cc -lopengl32 -lglut32

To compile on Linux
g++ test_glut.cc -lglut -lGL

*/

#include <cstdlib>
#include <cstdio>
#include <cmath>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/glut.h>    //use this on Linux & PC
#endif

///------------ state of the simulated point
//size of point drawn:
int pointSize = 1;

//------------- size of the display window:

int winW = 800;
int winH = 800;

GLfloat pixelSize = 1;

//------------- delay for the Timer function in milli-seconds:

int timerDelay = 50;

//------------- user adjustable zoom:

GLfloat zoom = 0.8;

//------------- point of focus:

GLfloat focus[] = { 0.f, 0.f };

//------------- variables for the mouse driven zoom:

int mouseAction;
GLfloat zoomSave, zoomFactor;
GLfloat mouseDown[] = { 0.f, 0.f, 0.f };
GLfloat pointer[] = { 0.f, 0.f, 0.f };

//------------- function to set the OpenGL transformation:

void setModelView()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(zoom, zoom, zoom);
    glTranslatef(-focus[0], -focus[1], 0);
}

void unproject(const GLfloat X, const GLfloat Y, GLfloat res[2])
{
    res[0] = ( X - 0.5f * winW ) * pixelSize + focus[0];
    res[1] = ( 0.5f * winH - Y ) * pixelSize + focus[1];
    //printf("unproject (%+9.3f %+9.3f) --> (%+9.3f %+9.3f)\n", X, Y, res[0], res[2]);
}

void windowReshaped(int w, int h)
{
    glViewport(0, 0, w, h);
    winW = w;
    winH = h;
        
    // --- set-up the projection matrix:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    if ( w > h )
    {
        pixelSize = 2.0f / ( zoom * winW );
        GLfloat ratio = h / GLfloat(w);
        glOrtho(-1.0, 1.0, -ratio, ratio, 0, 1 );
    }
    else
    {
        pixelSize = 2.0f / ( zoom * winH );
        GLfloat ratio = w / GLfloat(h);
        glOrtho(-ratio, ratio, -1.0, 1.0, 0, 1 );
    }
}

//----------------------------- KEYS --------------------------------

void processNormalKey(unsigned char c, int x, int y)
{
    printf("key %i mouse (%i %i)\n", c, x, y);
    switch (c)
    {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
            
        case ' ':
            zoom = 1;
            break;   //we use break to call the setModelView() below
            
        default:
            printf("key %i with modifier %i\n", c, glutGetModifiers());
    }
    setModelView();
    glutPostRedisplay();
}


// handle special keys: arrows, ctrl, etc.
void processInputKey(int c, int, int)
{
    printf("special key %c\n", c);
}


//----------------------------- MENUS --------------------------------

enum MENUS_ID { MENU_QUIT, MENU_RESETVIEW };

void processMenu(int item)
{
    switch ( item )
    {
        case MENU_QUIT:
            exit(EXIT_SUCCESS);
        case MENU_RESETVIEW:
            zoom = 1;
            focus[0] = 0;
            focus[1] = 1;
            setModelView();
            glutPostRedisplay();
            break;
    }
}

void initMenus()
{
    // --- one menu with two entries:
    glutCreateMenu(processMenu);
    glutAddMenuEntry("Reset", MENU_RESETVIEW);
    glutAddMenuEntry("Quit", MENU_QUIT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//----------------------------- MOUSE -------------------------------

//----------different actions controled by the mouse
enum { MOUSE_PASSIVE, MOUSE_ZOOM, MOUSE_MOVE, MOUSE_CLICK };

//this is called when the mouse button is pressed or released:
void processMouse(int button, int state, int x, int y)
{
#if 0
    static int mx, my;
    if ( state == GLUT_DOWN )
    {
        mx = x;
        my = y;
        printf("  mouse %i %i (%4i %4i) ", button, state, x, y);
    }
    else
    {
        if ( mx == x && my == y )
            printf("  ---- %i %i\n", button, state);
        else
            printf("  ---- %i %i (%4i %4i)\n", button, state, x, y);
    }
#endif
    // for a button release event, do nothing:
    if ( state != GLUT_DOWN ) return;
    
    mouseAction = MOUSE_PASSIVE;
    
    // action depends on mouse button:
    switch( button )
    {
        case GLUT_LEFT_BUTTON:
        {
            if ( glutGetModifiers() & GLUT_ACTIVE_SHIFT )
                mouseAction = MOUSE_CLICK;
            else
                mouseAction = MOUSE_MOVE;
        } break;
        case GLUT_MIDDLE_BUTTON:
            mouseAction = MOUSE_ZOOM;
            break;
    }
    //printf("button %i (%4i %4i) action %i\n", button, x, y, mouseAction);

    // perform action...
    switch( mouseAction )
    {
        case MOUSE_MOVE:
        {
            unproject(x, y, mouseDown);
        } break;
            
        case MOUSE_ZOOM:
        {
            GLfloat xx = x - 0.5*winW;
            GLfloat yy = y - 0.5*winH;
            zoomFactor = std::sqrt( xx*xx + yy*yy );
            if ( zoomFactor > 0 )
                zoomFactor = 1 / zoomFactor;
            zoomSave = zoom;
        } break;
            
        case MOUSE_CLICK:
        {
            unproject(x, y, pointer);
        } break;
            
        case MOUSE_PASSIVE:
            return;
    }
}


void processMotion(int x, int y)
{
    //printf("mouse (%4i %4i)\n", x, y);
    switch( mouseAction )
    {
        case MOUSE_MOVE:
        {
            GLfloat up[2];
            unproject(x, y, up);
            focus[0] += mouseDown[0] - up[0];
            focus[1] += mouseDown[1] - up[1];
            //printf("focus (%+9.4f %+9.4f) ", focus[0], focus[1]);
        } break;
            
        case MOUSE_ZOOM:
        {
            // --- we set the zoom from how far the mouse is from the window center
            GLfloat X = x - 0.5*winW;
            GLfloat Y = y - 0.5*winH;
            GLfloat Z = zoomFactor * std::sqrt(X*X+Y*Y);
            if ( Z <= 0 ) return;
            zoom = zoomSave * Z;
        } break;
        
        case MOUSE_CLICK:
        {
            unproject(x,y, pointer);
        } break;
                        
        case MOUSE_PASSIVE:
            return;
    }
    setModelView();
    glutPostRedisplay();
}

//----------------------------- INIT GL --------------------------------
void initGL()
{
    // --- choose the clearing color: black
    glClearColor(0.f, 0.f, 0.f, 0.f);
    
    //--- hints for OpenGL rendering:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    initMenus();
    setModelView();
}

//----------------------------- DISPLAY --------------------------------

void display()
{
    // --- clear window to the current clearing color:
    glClear(GL_COLOR_BUFFER_BIT);
    
    // --- set line width to 1 and color to white:
    glColor3f(1.f, 1.f, 1.f);
    glLineWidth(1);
    
    // Display of the simulation state:
    // --- draw a wireframe triangle:
    glBegin(GL_LINE_LOOP);
    glVertex2f( 1.f, -1.f);
    glVertex2f(-1.f, -1.f);
    glVertex2f( 0.f,  1.f);
    glEnd();
    
    // --- a point of variable size at the last mouse click:
    glColor3f(1.f, 1.f, 0.f);
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    glVertex2d(pointer[0], pointer[1]);
    glEnd();
    
    glFinish();
    // OpenGL cleanup:
    glutSwapBuffers();
    // --- check for OpenGL errors:
    glutReportErrors();
}

//------------------------- TIMER FUNCTION -----------------------------
void timerFunction(int value)
{
    //This is a very basic simulation!
    pointSize = 1 + ( pointSize + 1 ) % 16;
    
    glutPostRedisplay();
    //register another automatic timer call back (timerDelay is in milli-sec):
    glutTimerFunc(timerDelay, timerFunction, 1);
}


//-----------------------------   MAIN  --------------------------------
int main(int argc, char* argv[])
{
    // --- initialization of GLUT:
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(50, 50);
    glutCreateWindow(argv[0]);
    
    // --- further initialization of OpenGL:
    initGL();
    
    // --- register all the necessary functions:
    glutDisplayFunc(display);
    glutReshapeFunc(windowReshaped);
    glutMouseFunc(processMouse);
    glutMotionFunc(processMotion);
    glutSpecialFunc(processInputKey);
    glutKeyboardFunc(processNormalKey);
    glutTimerFunc(50, timerFunction, 0);
    
    // --- starts the event loop, which will never return:
    glutMainLoop();
}
