// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef OFFSCREEN_H
#define OFFSCREEN_H


#include "opengl.h"


/// functions to open/close an OpenGL off-screen display
namespace OffScreen
{
    
    /// create OpenGL context suitable for offscreen rendering
    int openContext();

    /// allocate display buffer of requested size (width,height)
    GLuint createBuffer(int width, int height, int multisample);

    /// allocate display buffer of requested size (width,height)
    int blitBuffer();

    /// release display buffer
    void releaseBuffer();
    
    /// close the OpenGL context created by openContext()
    void closeContext();
    
}


#endif

