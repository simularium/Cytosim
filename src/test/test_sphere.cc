// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Test for PointsOnSphere

#include <pthread.h>

#include "pointsonsphere.h"
#include "glapp.h"
#include "glut.h"
#include "gle.h"

PointsOnSphere S, T;
PointsOnSphere * front = &S;

pthread_t       thread;
pthread_mutex_t lock;

int n_points = 12;

//------------------------------------------------------------------------------

void batch(unsigned long nbp, int repeat)
{
    size_t iterations = 0;
    real energy = INFINITY;
    real distance = 0;
    
    printf("%4lu pts :", nbp);
    printf(" %6.4f :", S.expectedDistance(nbp));
    
    for ( int m=0; m < repeat; ++m )
    {
        iterations += S.distributePoints(nbp, 1e-4, 1<<14);
        
        if ( S.finalEnergy() < energy )
            energy = S.finalEnergy();
        
        if ( S.minimumDistance() > distance )
            distance = S.minimumDistance();
    }
    
    printf("   distance %9.6f",    distance);
    printf("   energy %14.5f",     energy);
    printf("   iterations %12lu\n", iterations);
}

//------------------------------------------------------------------------------

void* calculateSphere(void * arg)
{
    glApp::setMessage("Calculating...");
    glApp::postRedisplay();

    if ( front == &S )
    {
        T.distributePoints(n_points, 1e-4, 1<<14);
        front = &T;
    }
    else
    {
        S.distributePoints(n_points, 1e-4, 1<<14);
        front = &S;
    }
    
    glApp::setMessage("");
    glApp::postRedisplay();
        
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

//------------------------------------------------------------------------------
void processNormalKey(unsigned char c, int x, int y)
{
    switch (c)
    {
        case 'r': n_points-=256;   break;
        case 't': n_points-=32;    break;
        case 'y': n_points+=1;     break;
        case 'u': n_points+=16;    break;
        case 'i': n_points+=128;   break;
        case 'o': n_points+=1024;  break;
        case 'p': n_points+=8192;  break;
        case 'q' : exit(1);
            
        default:
            glApp::processNormalKey(c,x,y);
            return;
    }
    if ( n_points < 1 )
        n_points = 1;
    
    if ( 0 == pthread_mutex_trylock(&lock) )
    {
        pthread_create(&thread, 0, &calculateSphere, (void *)1);
    }
    else
    {
        glApp::flashText("already calculating...");
    }
}

//------------------------------------------------------------------------------
void display(View&)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glPointSize(7);
    
    if ( front == &T )
        glColor3f(0.0, 0.0, 0.7);
    else
        glColor3f(0.0, 0.7, 0.0);

    glBegin(GL_POINTS);
    for ( unsigned ii=0; ii < front->nbPoints(); ++ii )
    {
#if REAL_IS_DOUBLE
        glVertex3dv( front->addr(ii) );
#else
        glVertex3fv( front->addr(ii) );
#endif
    }
    glEnd();
    
#if ( 0 )
    glLineWidth(5);
    glBegin(GL_LINES);
    for ( unsigned ii=0; ii < front->nbPoints(); ++ii )
    {
        Vector3 p(front->addr(ii));
        Vector3 n = p.orthogonal();
        glColor3f(1.0, 1.0, 1.0);
        gle::gleVertex(p);
        glColor3f(0.0, 0.0, 0.0);
        gle::gleVertex(p+0.1*n);
    }
    glEnd();
#else
    const real e = 0.05;
    glLineWidth(4);
    glBegin(GL_LINES);
    for ( unsigned ii=0; ii < front->nbPoints(); ++ii )
    {
        Vector3 a(front->addr(ii));
        Vector3 b, c;
        a.orthonormal(b,c);
        
        glColor3f(0.0, 1.0, 0.0);
        gle::gleVertex(a);
        glColor3f(0.0, 0.0, 0.0);
        gle::gleVertex(a+e*b);
        glColor3f(0.0, 0.0, 1.0);
        gle::gleVertex(a);
        glColor3f(0.0, 0.0, 0.0);
        gle::gleVertex(a+e*c);
    }
    glEnd();
#endif
    
    if ( 0 )
    {
        glColor4f(0.3, 0.3, 0.3, 0.5);
        glDepthMask(GL_FALSE);
        glutSolidSphere(0.98,30,30);
        glDepthMask(GL_TRUE);
    }
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    RNG.seed();
    
    if ( argc == 3 ) 
    {
        unsigned min = (unsigned)strtoul(argv[1], 0, 10);
        unsigned max = (unsigned)strtoul(argv[2], 0, 10);
        
        for (unsigned nbp = min; nbp < max; nbp += 7)
            batch(nbp, 16);
        return EXIT_SUCCESS;
    }
    
    if ( argc == 2 ) 
        n_points = strtoul(argv[1], 0, 10);
    
    pthread_mutex_init(&lock, 0);
    front->distributePoints(n_points, 1e-4, 1<<14);
    
    glutInit(&argc, argv);
    glApp::setDimensionality(3);
    glApp::attachMenu(GLUT_RIGHT_BUTTON);
    glApp::normalKeyFunc(processNormalKey);
    glApp::createWindow(display);
    glApp::setScale(3);
    gle::initialize();

    glutMainLoop();
    return EXIT_SUCCESS;
}
