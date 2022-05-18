// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 12/12/2011.

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cmath>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/glut.h>
#endif


unsigned int delay = 8;       //< delay 16 == 60 Hz display
GLfloat angle = 0;
GLfloat angle_inc = 0.2;


//------------------------------------------------------------------------------
void processNormalKey(unsigned char c, int x, int y)
{
    switch (c)
    {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
    }
}


//------------------------------------------------------------------------------
void reshape(int ww, int wh)
{
    glViewport(0, 0, ww, wh);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ratio = ww / double( wh );
    
    if ( ratio > 1 )
        glOrtho(-1, 1, -1/ratio, 1/ratio, 1, 5);
    else
        glOrtho(-ratio, ratio, -1, 1, 1, 5);
}


//------------------------------------------------------------------------------
void initGL()
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
    GLfloat rgba[] = { 0.0, 0.0, 0.0, 1.0 };
    glFogfv(GL_FOG_COLOR, rgba);
}

void setView(GLfloat angle)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -2.1);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glRotatef(angle, 1.0, 0.0, 0.0);
}

//------------------------------------------------------------------------------
GLuint buffer[2] = { 0 };


void initVBO()
{
    //Vertices of a triangle (counter-clockwise winding)
    GLfloat a = 1/3.0;
    GLfloat b = sqrt(2.0)/3.0;
    GLfloat c = sqrt(2.0/3.0);
    
    GLfloat vertices[] = { 0, 2*b, -a,
                        -c,  -b, -a,
                         c,  -b, -a,
                         0,   0,  1,
                        -c,  -b, -a };
    
    // indices of the vertices that make each face
    GLuint indices[] = { 0, 2, 1,  1, 3, 0,  0, 3, 2,  1, 2, 3 };

    
    //Create a new VBO and use the variable id to store the VBO id
    glGenBuffers(2, buffer);
    
    //Make the new VBO active
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    
    //Upload vertex data to the video device
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Make VBO inactive
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Make VBO active
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
    
    //Upload vertex data to the video device
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    //Make VBO inactive
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void releaseVBO()
{
    glDeleteBuffers(2, buffer);
}


void displayVBO()
{
    // Make the new VBO active. Repeat here incase changed since initialisation
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    // Establish its 3 coordinates per vertex with zero stride in this array; necessary here
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Draw the vertices
    glPointSize(10);
    glColor3f(0,0,1);
    glDrawArrays(GL_POINTS, 0, 4);

    /*
    /// Draw lines
    glLineWidth(3);
    glColor3f(1.0, 1.0, 1.0);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
     */
    
    glDepthMask(GL_FALSE);
    // Draw 4 triangles, giving the number of vertices provided
    glColor4f(1,1,1,0.3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDepthMask(GL_TRUE);
}

//------------------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setView(angle);
    displayVBO();
    glutSwapBuffers();
    glutReportErrors();
}

//------------------------------------------------------------------------------
void timerFunction(int win)
{
    angle += angle_inc;
    glutPostWindowRedisplay(win);
    //register another timer call back in prop.delay milli-sec:
    glutTimerFunc(delay, timerFunction, win);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
        
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processNormalKey);
    glutTimerFunc(50, timerFunction, glutGetWindow());

    initGL();
    initVBO();
    
    glutReportErrors();
    glutMainLoop();
    
    return EXIT_SUCCESS;
}
