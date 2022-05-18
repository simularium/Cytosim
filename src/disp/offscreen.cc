// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "offscreen.h"
#include "opengl.h"


#if defined(__APPLE__) && defined(GL_VERSION_2_1)

// OpenGL Frame Buffer Objects
#include "offscreen_fbo.cc"

#elif defined(__linux)

// X-windows offscreen rendering routines (Linux)
#include "offscreen_glx.cc"

#else

// dummy routines
#include <cstdio>

int OffScreen::openContext()
{
    fprintf(stderr,"This program cannot render off-screen\n");
    return 0;
}

GLuint OffScreen::createBuffer(int, int, int)
{
    fprintf(stderr,"This program cannot render off-screen\n");
    return 0;
}

void OffScreen::releaseBuffer()
{
}

void OffScreen::closeContext()
{
}

#endif
