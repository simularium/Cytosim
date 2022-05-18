// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 
 Francois Nedelec, Nov. 2003,  nedelec@embl.de
 To compile on mac-osx:
 g++ test_glut3D.cc -framework GLUT -framework openGL -framework Foundation
 On Linux:
 g++ test_glut3D.cc -L/usr/X11R6/lib -lglut -lGL -lGLU -lXt -lX11 -lXi -lXmu

 */

#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
   #include <GLUT/glut.h>
#else
   #include <GL/glut.h>    //use this on Linux & PC
#endif

unsigned int delay     = 13;       //delay 13 == 75 Hz display
double       angle     = 0;
double       angle_inc = 0.1;
double       linewidth = 2.0;
char         displayed_text[512] = "\0";
int          transparency = 0;

//-----------------------------------------------------------------
void gleReportErrors(const char* nameOfCallingFunction)
{
  GLenum glError = glGetError();
  while ( glError != GL_NO_ERROR ) {
    fprintf(stderr, "OpenGL error `%s' in %s\n", gluErrorString(glError), nameOfCallingFunction);
    glError = glGetError();
  }
}


//-----------------------------------------------------------------
void setDisplayedText()
{
  GLint fog, depth, smooth, blend;
  glGetIntegerv( GL_BLEND, &blend );
  glGetIntegerv( GL_FOG,          &fog );
  glGetIntegerv( GL_DEPTH_TEST,   &depth );
  glGetIntegerv( GL_POINT_SMOOTH, &smooth );
  snprintf(displayed_text, sizeof(displayed_text), "(b)lend %i (f)og %i (d)depth %i (s)mooth %i (t)ransparency %i",
           int(blend), int(fog), int(depth), int(smooth), transparency);
}


//-----------------------------------------------------------------
void flipOpenGLCap( GLenum cap )
{
  GLint enabled;
  glGetIntegerv( cap, &enabled);
  if ( enabled )
    glDisable( cap );
  else
    glEnable( cap );
}


//-----------------------------------------------------------------
void processNormalKey(unsigned char c, int x, int y)
{
  switch (c) {

  case 'p':
    if ( delay > 1 ) delay /= 2;
    return;
  case 'o':
    delay *= 2;
    return;

  case 'l':
    linewidth += 0.5;
    return;
  case 'k':
    if ( linewidth > 1 ) linewidth -= 0.5;
    return;

  case 'd':
    flipOpenGLCap( GL_DEPTH_TEST );
    break;
  case 'f':
    flipOpenGLCap( GL_FOG );
    break;
  case 'b':
    flipOpenGLCap( GL_BLEND );
    break;
  case 's':
    flipOpenGLCap( GL_POINT_SMOOTH );
    flipOpenGLCap( GL_LINE_SMOOTH );
    break;
  case 't':
    transparency = 1-transparency;
    linewidth = transparency ? 8.0 : 2.0;
    break;
	
  case 27:
  case 'q':
    exit(EXIT_SUCCESS);
  }
    
  setDisplayedText();
}


//-----------------------------------------------------------------
void reshapeWindow(int ww, int wh)
{
  glViewport(0, 0, ww, wh);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double ratio = ww / double( wh );

  if ( ratio > 1 )
    glFrustum(-1, 1, -1/ratio, 1/ratio, 1, 5);
  else
    glFrustum(-ratio, ratio, -1, 1, 1, 5);
}


//-----------------------------------------------------------------
void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  float alpha = transparency ? 0.3 : 1.0;
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, 512, 0, 512, -1, 1 );

  glColor4f(1.0, 1.0, 1.0, alpha);
  glRasterPos3i(10, 5, 0);
  //draw the string character per character:
  for (char* p = displayed_text; *p; p++)
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);

  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glTranslated( 0.0, 0.0, -2.1 );
  glRotated( angle, 0.0, 0.0, 1.0);
  glRotated( angle, 1.0, 0.0, 0.0);

  glLineWidth(2*linewidth);
  glColor4f(1.0, 0.0, 0.0, alpha);
  glutWireCube(1.5);

  glLineWidth(linewidth);
  glColor4f(1.0, 0.5, 0.0, alpha);
  glutWireSphere(1, 20, 20);

  glPointSize(32.0);
  glBegin(GL_POINTS);
  glColor4f(1.0, 1.0, 1.0, alpha);   glVertex3f(0.0, 0.0, 0.0);
  glColor4f(1.0, 0.0, 0.0, alpha);   glVertex3f(1.0, 0.0, 0.0);
  glColor4f(0.0, 1.0, 0.0, alpha);   glVertex3f(0.0, 1.0, 0.0);
  glColor4f(0.0, 0.0, 1.0, alpha);   glVertex3f(0.0, 0.0, 1.0);
  glEnd();

  glFlush();
  glutSwapBuffers();
  gleReportErrors("display");
}

//-----------------------------------------------------------------
void initOpenGL()
{
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 0 );
  glFogf(GL_FOG_END,   4 );
  GLfloat fogColor[] = { 0.0, 0.0, 0.0, 1.0 };
  glFogfv(GL_FOG_COLOR, fogColor);
  
  setDisplayedText();
}

//-----------------------------------------------------------------
void timerFunction(int value)
{
  angle += angle_inc;
  glutPostRedisplay();
  //register another timer call back in PP.delay milli-sec:
  glutTimerFunc(delay, timerFunction, 1);
}


//----------------------------------------------------------------------------
void glStat()
{
    printf("VENDOR   = %s\n", (char*)glGetString(GL_VENDOR));
    printf("RENDERER = %s\n", (char*)glGetString(GL_RENDERER));
    printf("VERSION  = %s\n", (char*)glGetString(GL_VERSION));
    
    printf("has keyboard %i\n", glutDeviceGet(GLUT_HAS_KEYBOARD));
    printf("has mouse %i,  ", glutDeviceGet(GLUT_HAS_MOUSE));
    printf("with %i buttons\n", glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS));
    printf("color bit depth %i,  ", glutGet(GLUT_WINDOW_BUFFER_SIZE));
    printf("alpha bit depth %i\n", glutGet(GLUT_WINDOW_ALPHA_SIZE));
    printf("Current display is RGBA %i, ", glutGet(GLUT_WINDOW_RGBA));
    printf("Current mode possible %i\n", glutGet(GLUT_DISPLAY_MODE_POSSIBLE));
    
    //anti-aliasing of points and lines:
    printf("GL_POINT_SMOOTH enabled %i\n", glIsEnabled(GL_POINT_SMOOTH));
    GLfloat s[2];
    glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, s);
    printf("GL_SMOOTH_POINT_SIZE_RANGE %.2f - %.2f\n", s[0],s[1]);
    
    printf("GL_LINE_SMOOTH enabled %i\n", glIsEnabled(GL_LINE_SMOOTH));
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, s);
    printf("GL_SMOOTH_LINE_WIDTH_RANGE %.2f - %.2f\n", s[0], s[1]);
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, s);
    printf("GL_ALIASED_LINE_WIDTH_RANGE %.2f - %.2f\n", s[0], s[1]);
    
    GLint numBuffers;
    glGetIntegerv(GL_AUX_BUFFERS, &numBuffers);
    printf("GL_AUX_BUFFERS %i\n", int(numBuffers));
    
    char* ext = (char*)glGetString(GL_EXTENSIONS);
    printf("Extensions: %s \n", ext);
    
    printf("Overlay possible = %i\n", glutLayerGet(GLUT_OVERLAY_POSSIBLE));
}

//-----------------------------------------------------------------
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  
  glutInitWindowSize(512, 512);
  glutCreateWindow(argv[0]);

  //testglut2 -e reports some OpenGL info:
  if ( argc > 1 ) {
    if (argv[1][0]=='-') {
        glStat();
        return EXIT_SUCCESS;
    } else sscanf(argv[1], "%lf", &angle_inc);
  }

  initOpenGL();
  gleReportErrors("main");
  
  glutDisplayFunc(display);
  glutReshapeFunc(reshapeWindow);
  glutKeyboardFunc(processNormalKey);
  glutTimerFunc(50, timerFunction, 0);

  glutMainLoop();

  return EXIT_SUCCESS;
}
