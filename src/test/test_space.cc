// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/*
 test_space provides a visual test of Cytosim's Space
*/

#include <ctime>
#include "dim.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"
#include "real.h"
#include "vector.h"
#include "random.h"

#include "space_prop.h"
#include "space.h"
#include "space_set.h"
#include "glapp.h"
#include "glut.h"
#include "gle.h"

using namespace gle;

// List of options
Glossary opt;

// property
SpaceProp prop("test_space");

// Space to be tested:
Space * spc = nullptr;

// number of points
const int maxpts = 1<<17;
int nbpts  = 1024;
int scan   = 100;

// INFLATION of the rectangle containing point to be projected
const real INFLATION = 1;

// regular or random distribution of the test-points
bool regular_distribution = false;


//coordinates of the points:
Vector point[maxpts];

//true if inside
int    inside[maxpts];

//coordinates of the projections
Vector project[maxpts];

//coordinates of the projections of the projections
Vector project2[maxpts];

//normals to the projections
Vector normal[maxpts];

//located on the edges
Vector edge[maxpts];

//max distance from projection to second projection
real  max_error_projection;

//slicing parameters
bool slicing = false;
const real sliceStep = 0.2;
real thickness = sliceStep;

Vector slicePos(0, 0, 0);
Vector sliceDir(1, 0, 0);

//show or hide points in or outside
int showInside    = true;
int showOutside   = true;
int showProject   = true;
int showReproject = true;
int showNormals   = false;
int showEdges     = false;

//use timer function on or off
int timerOn = false;
int timerDelay = 50;

//display parameter for OpenGL
GLfloat line_width = 0.5;

//amount of white added to colors
const GLfloat COL = 0.8;

//------------------------------------------------------------------------------

void generatePoints(real len)
{
    Vector inf, sup;
    spc->boundaries(inf, sup);
    inf -= Vector(len, len, len);
    Vector dif = sup - inf + Vector(len, len, len);
    
    if ( regular_distribution )
    {
        dif /= scan;
        int kk = 0;
        nbpts = 0;
        //follow a regular lattice:
        for ( int ii = 0; ii <= scan; ++ii )
            for ( int jj = 0; jj <= scan; ++jj )
#if ( DIM >= 3 )
                for ( kk = 0; kk <= scan; ++kk )
#endif
                {
                    point[nbpts++] = inf + dif.e_mul(Vector(ii, jj, kk));
                    if ( nbpts >= maxpts )
                        return;
                }
    }
    else
    {
        for ( int ii = 0; ii <= nbpts; ++ii )
            point[ii] = inf + dif.e_mul(Vector::randP());
        //point[ii] = Vector::randU();
        //point[ii] = spc->randomPlaceNearEdge(0.1);
    }
}


void distributePoints(real len = INFLATION)
{
    if ( !spc ) return;
    
    generatePoints(len);
    max_error_projection = 0;
    
    for ( int ii = 0; ii < nbpts; ++ii )
    {
        //see if space finds it inside:
        inside[ii] = spc->inside(point[ii]);
        //calculate the projection:
        project[ii] = spc->project(point[ii]);
        
        //calculate the projection of the projection:
        //project2[ii] = spc->project(project[ii]);
        project2[ii] = project[ii];
        
        if ( showNormals )
            normal[ii] = spc->normalToEdge(project[ii]);
        else
            normal[ii].reset();
        
        edge[ii] = spc->randomPlaceOnEdge(1);
        
        real d = (project[ii] - project2[ii]).normSqr();
        if ( d > max_error_projection ) max_error_projection = d;
    }
    max_error_projection = sqrt( max_error_projection );
    
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "error %.6f", max_error_projection);
    glApp::setMessage(tmp);
}

//------------------------------------------------------------------------------
void timerFunction(int)
{
    if ( timerOn )
    {
        distributePoints();
        glutPostRedisplay();
        glutTimerFunc(timerDelay, timerFunction, 0);
    }
}

//------------------------------------------------------------------------------
void setGeometry()
{
    prop.read(opt);
    
    try {
        delete(spc);
        spc = prop.newSpace(opt);
        if ( 1 )
        {
            fprintf(stdout, " >>> ");
            Outputter out(stdout, false);
            spc->write(out);
            fprintf(stdout, "\n");

        }
    }
    catch( Exception & e )
    {
        printf("Error: `%s'\n", e.msg());
    }
    
    try {
        if ( spc )
            distributePoints(INFLATION);
    }
    catch( Exception & e )
    {
        printf("Error: `%s'\n", e.msg());
    }

    glutPostRedisplay();
}


void checkVolume()
{
    size_t cnt = 1<<22;
    real e1 = spc->estimateVolume(cnt);
    real e2 = spc->estimateVolume(cnt);
    
    printf("Monte-Carlo estimated volume of `%s` is", spc->prop->shape.c_str());
    printf("  %.6f +/- %.6f\n", e1, fabs(e2-e1));
    
    real v = spc->volume();
    
    real err = fabs( e1 - v ) / v;
    
    if ( err > 1e-3 )
        printf("    but given volume is %f  (difference %.2f %%)\n", v, 100*err);
}

//------------------------------------------------------------------------------
enum MENUS_ID {
    MENU_QUIT = 102, MENU_RESETVIEW = 103,
    MENU_INSIDE = 104, MENU_OUTSIDE = 105, MENU_PROJECT = 106,
    MENU_XSLICING = 107, MENU_YSLICING = 108, MENU_ZSLICING = 109,
    MENU_EDGES = 111
};

void processMenu(int item)
{
    switch( item )
    {
        case MENU_QUIT:
            exit(EXIT_SUCCESS);
        case MENU_RESETVIEW:
            glApp::resetView();
            break;
        case MENU_INSIDE:
            showInside = ! showInside;
            break;
        case MENU_OUTSIDE:
            showOutside = ! showOutside;
            break;
        case MENU_EDGES:
            showEdges = ! showEdges;
            break;
        case MENU_PROJECT:
            showProject = ! showProject;
            break;
        case MENU_XSLICING:
            slicing = !slicing;
            sliceDir.set(1,0,0);
            break;
        case MENU_YSLICING:
            slicing = !slicing;
            sliceDir.set(0,1,0);
            break;
        case MENU_ZSLICING:
            slicing = !slicing;
            sliceDir.set(0,0,1);
            break;
    }
    glutPostRedisplay();
}


void initMenus()
{
    int gm = glApp::buildMenu();
    glutCreateMenu(processMenu);
    glutAddSubMenu("Control", gm);
    
    glutAddMenuEntry("Reset",                MENU_RESETVIEW);
    glutAddMenuEntry("Quit",                 MENU_QUIT);
    glutAddMenuEntry("-", 0);
    glutAddMenuEntry("Toggle inside  (i)",   MENU_INSIDE);
    glutAddMenuEntry("Toggle outside (o)",   MENU_OUTSIDE);
    glutAddMenuEntry("Toggle edges   (e)",   MENU_EDGES);
    glutAddMenuEntry("Toggle project (p)",   MENU_PROJECT);
    
    glutAddMenuEntry("Toggle x-slicing (x)", MENU_XSLICING);
    glutAddMenuEntry("Toggle y-slicing (y)", MENU_YSLICING);
    glutAddMenuEntry("Toggle z-slicing (z)", MENU_ZSLICING);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//------------------------------------------------------------------------------
void processSpecialKey(int key, int x=0, int y=0)
{
    switch (key)
    {
        case GLUT_KEY_LEFT:
            slicePos.XX -= 0.2;
            break;
        case GLUT_KEY_RIGHT:
            slicePos.XX += 0.2;
            break;
        case GLUT_KEY_UP:
            thickness += sliceStep;
            break;
        case GLUT_KEY_DOWN:
            thickness -= sliceStep;
            if ( thickness < sliceStep ) thickness = sliceStep;
            break;
        default:
            break;
    }
    
    glutPostRedisplay();
}

void processNormalKey(unsigned char c, int x=0, int y=0)
{
    switch (c)
    {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
            
        case ' ':
            distributePoints();
            break;
            
        case '0':
            glApp::resetView();
            break;
            
        case ']':
            scan *= 2;
            nbpts *= 2;
            if ( nbpts > maxpts )
                nbpts = maxpts;
            distributePoints();
            break;
            
        case '[':
            if ( scan > 2 ) scan /= 2;
            if ( nbpts > 2 ) nbpts /= 2;
            distributePoints();
            break;
            
        case 'x':
            slicing = !slicing;
            sliceDir.set(1,0,0);
            break;
            
        case 'y':
            slicing = !slicing;
            sliceDir.set(0,1,0);
            break;
            
        case 'z':
            slicing = !slicing;
            sliceDir.set(0,0,1);
            break;
            
        case 'i':
            showInside = ! showInside;
            break;
            
        case 'o':
            showOutside = ! showOutside;
            break;
            
        case 'r':
            showReproject = ! showReproject;
            break;
            
        case 'p':
            showProject = ! showProject;
            break;
            
        case 'e':
            showEdges = ! showEdges;
            break;
            
        case 'n':
            showNormals = ! showNormals;
            if ( showNormals)
                distributePoints();
            break;
            
        case 'R':
            regular_distribution = !regular_distribution;
            distributePoints();
            break;
            
        case 'd':
        {
            real val[] = { -2, -1, 0, 1, 2, 5 };
            opt.define("inflate", 0, val[ RNG.pint32(6) ]);
            setGeometry();
        } break;
            
        case 't':
            timerOn = ! timerOn;
            if ( timerOn )
                glutTimerFunc(timerDelay, timerFunction, 0);
            break;
            
        default:
            glApp::processNormalKey(c,x,y);
    }
    
    glutPostRedisplay();
}

//------------------------------------------------------------------------------

bool showPoint(int i)
{
    if ( inside[i] )
    {
        if ( ! showInside )
            return false;
    }
    else
    {
        if ( ! showOutside )
            return false;
    }
    
    if ( ! slicing )
        return true;
    
    Vector & pos = project[i];
    
    return (( (slicing & 0x01) &&  fabs(pos.XX-slicePos.XX) < thickness )
#if ( DIM > 1 )
            || ( (slicing & 0x02) &&  fabs(pos.YY-slicePos.YY) < thickness )
#endif
#if ( DIM > 2 )
            || ( (slicing & 0x04) &&  fabs(pos.ZZ-slicePos.ZZ) < thickness )
#endif
            );
}


void display(View&)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    spc->draw();
    
    //plot a gren dot for points inside, a red dot for point outside:
    glPointSize(2.0);
    glBegin(GL_POINTS);
    for ( int ii = 0; ii < nbpts; ++ii )
    {
        if ( showPoint(ii) )
        {
            if ( inside[ii] )
                glColor3f(0.0, COL, 0.0);
            else
                glColor3f(0.0, 0.0, COL);
            gleVertex( point[ii] );
        }
    }
    glEnd();
    
    if ( showProject )
    {
        //plot a blue line from the point to its projection:
        glLineWidth(line_width);
        glBegin(GL_LINES);
        for ( int ii = 0; ii < nbpts; ++ii )
        {
            if ( showPoint(ii) )
            {
                if ( inside[ii] )
                    glColor3f(0.0, COL, 0.0);
                else
                    glColor3f(0.0, 0.0, COL);
                gleVertex( point[ii] );
                gleVertex( project[ii] );
            }
        }
        glEnd();
    }
    
    if ( showNormals )
    {
        glLineWidth(line_width);
        glBegin(GL_LINES);
        for ( int ii = 0; ii < nbpts; ++ii )
        {
            glColor4f(1.0, 1.0, 1.0, 1.0);
            gleVertex(project[ii]);
            glColor4f(1.0, 1.0, 1.0, 0.0);
            gleVertex(project[ii] + normal[ii]);
        }
        glEnd();
    }
    
    if ( showReproject )
    {
        glLineWidth(2*line_width);
        glBegin(GL_LINES);
        for ( int ii = 0; ii < nbpts; ++ii )
        {
            if ( showPoint(ii) )
            {
                glColor3f(COL, 0.0, 0.0);
                gleVertex(project[ii]);
                gleVertex(project2[ii]);
            }
        }
        glEnd();
    }
    
    if ( showEdges )
    {
        glPointSize(2.0);
        glBegin(GL_POINTS);
        glColor3f(1.0, COL, COL);
        for ( int ii = 0; ii < nbpts; ++ii )
            gleVertex(edge[ii]);
        glEnd();
        glBegin(GL_POINTS);
        glColor3f( 0.0, COL, 0.0);
        for ( int ii = 0; ii < nbpts; ++ii )
            gleVertex(project[ii]);
        glEnd();
    }
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glApp::setDimensionality(DIM);
    glApp::normalKeyFunc(processNormalKey);
    glApp::specialKeyFunc(processSpecialKey);
    glApp::createWindow(display);
    glApp::setScale(20);
    gle::initialize();

    initMenus();
    RNG.seed();

    if ( argc > 1 )
    {
        if ( opt.read_strings(argc-1, argv+1) )
            return EXIT_FAILURE;
        setGeometry();
    }
    
    if ( ! spc )
    {
        printf("A geometry should be given in the command line, for example:\n");
        printf("    test_space shape=ellipse length=2,3,4\n");
        exit(EXIT_SUCCESS);
    }

    checkVolume();
    
    glutMainLoop();
    
    return EXIT_SUCCESS;
}

