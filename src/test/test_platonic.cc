// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Test class Platonic::Solid

#include "gle.h"
#include "glut.h"
#include "glapp.h"
#include "platonic.h"
#include <cstdio>

using namespace gle;

int style = 2;
int kind  = 2;
int order = 1;

bool showVertices = false;
bool showEdges = true;

Platonic::Solid * ico = 0;


GLuint glbuffers[2] = { 0, 0 };

//------------------------------------------------------------------------------
void initVBO();

void setPlatonic()
{
    if ( ico )
        delete ico;
    
    ico = new Platonic::Solid((Platonic::Solid::Polyhedra)kind, order);

    char tmp[128];
    snprintf(tmp, sizeof(tmp), "%i points", ico->nb_vertices());
    glApp::setMessage(tmp);
    
    initVBO();
}

void processNormalKey(unsigned char c, int x, int y)
{
    switch (c) 
    {
        case 'p': kind = ( kind + 1 ) % 3;      break;
        case '=': order += 1;                   break;
        case '-': if ( order > 1 ) order -= 1;  break;
        case 's': style = ( style+1 ) % 4;      break;
        case 'e': showEdges = !showEdges;       break;
        case 'v': showVertices = !showVertices; break;
        case ' ': break; // update the Platonic
        default:
            glApp::processNormalKey(c,x,y);
            return;
    }
    
    setPlatonic();
    
    glApp::flashText("style = %i", style);
    glutPostRedisplay();
}

//------------------------------------------------------------------------------

void displayFaces1()
{
    glBegin(GL_TRIANGLES);
    for ( unsigned ii = 0; ii < ico->nb_faces(); ++ii )
    {
        glNormal3fv(ico->face_data0(ii));
        glVertex3fv(ico->face_data0(ii));
        glNormal3fv(ico->face_data1(ii));
        glVertex3fv(ico->face_data1(ii));
        glNormal3fv(ico->face_data2(ii));
        glVertex3fv(ico->face_data2(ii));
    }
    glEnd();
}


void displayFaces2()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, ico->vertex_data());
    glNormalPointer(GL_FLOAT, 0, ico->vertex_data());
    glDrawElements(GL_TRIANGLES, 3*ico->nb_faces(), GL_UNSIGNED_INT, ico->faces_data());
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}


void initVBO()
{
    //Create a new VBO for the vertex information
    glGenBuffers(2, glbuffers);
    glBindBuffer(GL_ARRAY_BUFFER, glbuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, 3*ico->nb_vertices()*sizeof(float), ico->vertex_data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Create a new VBO for the indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*ico->nb_faces()*sizeof(unsigned), ico->faces_data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void displayFacesVBO()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, glbuffers[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glNormalPointer(GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuffers[1]);
    glDrawElements(GL_TRIANGLES, 3*ico->nb_faces(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}


void displayFaces()
{
    if ( style == 1 )
        displayFaces1();
    else if ( style == 2 )
        displayFaces2();
    else if ( style == 3 )
        displayFacesVBO();
}


void display(View&)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glColor3f(0, 0, 0.75);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    displayFaces();
    glDisable(GL_CULL_FACE);
    
    if ( showEdges )
    {
        glDisable(GL_LIGHTING);
        glColor3f(1,1,1);
        glLineWidth(0.5);
        glBegin(GL_LINES);
        for ( unsigned int ii=0; ii < ico->nb_edges(); ++ii )
        {
            glVertex3fv(ico->edge_data0(ii));
            glNormal3fv(ico->edge_data0(ii));
            glVertex3fv(ico->edge_data1(ii));
            glNormal3fv(ico->edge_data1(ii));
        }
        glEnd();
    }
    
    glDisable(GL_LIGHTING);

    if ( showVertices )
    {
        char tmp[128];
        for ( unsigned ii=0; ii < ico->nb_vertices(); ++ii )
        {
            Platonic::Vertex & dv = ico->vertex(ii);
            if ( dv.weight(2) == 0  &&  dv.weight(1) == 0 )
                glColor3f(1.0, 1.0, 1.0);
            else if ( dv.weight(2) == 0 )
                glColor3f(0.0, 1.0, 0.0);
            else
                glColor3f(0.5, 0.5, 0.5);
            
            const float* v = ico->vertex_data(ii);
            Vector3 pos(v[0], v[1], v[2]);
            snprintf(tmp, sizeof(tmp), "%u", ii);
            gleDrawText(pos, tmp, GLUT_BITMAP_8_BY_13);
        }        
    }
    
    glutReportErrors();
}


//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glApp::setDimensionality(3);
    glApp::attachMenu(GLUT_RIGHT_BUTTON);
    glApp::normalKeyFunc(processNormalKey);
    glApp::createWindow(display);
    glApp::setScale(3);
    gle::initialize();

    setPlatonic();
    glutMainLoop();
    return EXIT_SUCCESS;
}

/*
int main(int argc, char* argv[])
 {
     Platonic::Solid::Polyhedra kind = Platonic::Solid::ICOSAHEDRON;
     printf("-------------- order 2:\n");
     Platonic::Solid T0(kind, 2);
     printf("-------------- order 3:\n");
     Platonic::Solid T1(kind, 2);
     printf("-------------- order 4:\n");
     Platonic::Solid T2(kind, 4);
     printf("-------------- order 8:\n");
     Platonic::Solid T3(kind, 8);
     printf("-------------- order 16:\n");
     Platonic::Solid T4(kind, 16);
     printf("-------------- order 32:\n");
     Platonic::Solid T5(kind, 32);
     printf("-------------- order 64:\n");
     Platonic::Solid T6(kind, 64);
    return 0;
}
*/

