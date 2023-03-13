// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 This is a test for glApp
 mouse driven zoom and rotation with Quaternions and GLU unproject
 Francois Nedelec nedelec@embl.de,  Oct. 2002, modified Jan. 2006
*/

#include "glossary.h"
#include "glapp.h"
#include "glut.h"
#include "gle.h"

Vector3 origin(0,0,0), position(0,0,0);


void display(View&)
{
    glEnable(GL_LIGHTING);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // rendering of icosahedron:
    gle_color(1.0, 0.0, 1.0, 1.0).load_front();
    gle_color(0.0, 0.0, 0.5, 1.0).load_back();
    gle::gleIcosahedron1();

    gle_color(1.0, 1.0, 1.0, 1.0).load_both();
    glLineWidth(3.0);
    glutWireCube(2);

    gle_color(1.0, 1.0, 1.0, 0.2).load_both();
    glDepthMask(GL_FALSE);
    glutSolidCube(2);
    glDepthMask(GL_TRUE);
    
    glDisable(GL_LIGHTING);
    glPointSize(16.0);
    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3d(origin.XX, origin.YY, origin.ZZ);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3d(position.XX, position.YY, position.ZZ);
    glEnd();
    
    glPointSize(7.0);
    glBegin(GL_POINTS);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0, 0, 0);
    glEnd();
}


///set callback for shift-click, with unprojected click position
void processMouseClick(int, int, const Vector3 & a, int)
{
    origin = a;
    glApp::postRedisplay();
}

///set callback for shift-drag, with unprojected mouse and click positions 
void processMouseDrag(int, int, Vector3 & a, const Vector3 & b, int)
{
    origin   = a;
    position = b;
    glApp::postRedisplay();
}


int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glApp::setDimensionality(3);
    glApp::actionFunc(processMouseClick);
    glApp::actionFunc(processMouseDrag);
    glApp::attachMenu(GLUT_RIGHT_BUTTON);
    glApp::createWindow(display);
    glApp::setScale(4);
    gle::initialize();

    glutMainLoop();
    return EXIT_SUCCESS;
}
