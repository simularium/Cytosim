// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VIEW_H
#define VIEW_H

#include "opengl.h"
#include "view_prop.h"

/// Handles the viewing angle, projection and other aspects of an OpenGL display
/**
 ViewProp does not depend on the window-system (GLUT),
 but only on the rendering engine (OpenGL)
 */
class View : public ViewProp
{
private:
    
    /// viewport obtained by getMatrices()
    GLint      mViewport[4];
    
    /// modelview obtained by getMatrices()
    GLdouble   mModelview[16];
    
    /// projection obtained by getMatrices()
    GLdouble   mProjection[16];
    
    /// half-size of the OpenGL visible region in OpenGL units
    GLfloat    visRegion[4];
    
    /// translation between center of volume and camera
    GLfloat    eyePosition[4];
    
    /// window number in GLUT
    int        mWindowId;
    
    /// used to check that getMatrices() was called
    bool       hasMatrices;
    
    /// text displayed near top right corner of window
    std::string top_message;
    
    /// text displayed near bottom left corner of window
    std::string full_label;

    /// display callback
    void (*displayCallback)(View&);
    
    /// set OpenGL Fog, with mode (GL_EXP or GL_LINEAR), intensity and color
    void       setFog(GLint mode, GLfloat density, gle_color) const;

public:
    
    /// constructor
    explicit View(const std::string& n);
    
    /// destructor
    ~View();
    
    /// return window-id
    int        window() const { return mWindowId; }
    
    /// set window-id
    void       window(int w) { mWindowId = w; }
    
    /// handle window resize events
    void       reshape(int, int);
    
    /// adjust parameters of projections
    void       adjust();
    
    /// set OpenGL Projection matrix
    void       setProjection();
    
    /// set OpenGL Model-View matrix
    void       setModelView() const;
    
    /// set OpenGL Projection and ModelView matrices
    void       load();

    /// adjust view to only show a slice of the world
    void       sliceView(int) const;
    
    /// reset the view (no-rotation, zoom=1), and enable auto_scale
    void       reset();
    
    /// width of display area in pixels
    int        width()     const { return window_size[0]; }
    
    /// height of display area in pixels
    int        height()    const { return window_size[1]; }
    
    /// size of pixel in drawing units
    GLfloat    pixelSize() const { return view_size / ( zoom * std::max(width(), height()) ); }

    //---------------------------------------------------------------------------
    
    /// set display callback
    void       setDisplayFunc(void (*f)(View&)) { displayCallback = f; }

    /// set clipping planes and fog parameters
    void       openDisplay();
    
    /// set clipping planes and fog parameters
    void       closeDisplay() const;
    
    /// display scale bar, info text, etc.
    void       drawInteractiveFeatures() const;
    
    /// call displayCallback
    void       callDraw() { displayCallback(*this); }
    
    //---------------------------------------------------------------------------
    
    /// init OpenGL parameters
    void       initGL();

    /// set OpenGL Lights for lighting effects
    void       setLights(bool local = false) const;
    
    /// set text displayed in center of window
    void       setLabel(std::string const& msg) { full_label = label + " " + msg; }
    
    /// set text displayed near top of window
    void       setMessage(std::string const& msg) { top_message = msg; }

    /// set OpenGL Fog, with mode (GL_EXP or GL_LINEAR), intensity and color
    void       enableFog(GLint mode, GLfloat param, gle_color);
    void       enableFog(GLint mode, GLfloat param);
    
    /// enable cliping plane in OpenGL
    void       setClipPlane(GLenum glp, Vector3 dir, real sca) const;

    /// enable cliping plane in OpenGL
    void       setClipPlaneEye(GLenum glp, Vector3 dir, real sca) const;
    
    /// call setClipPlane(int) for all enabled clipping planes
    void       setClipping() const;
    
    /// disable cliping planes in OpenGL
    void       endClipping() const;
    
    /// set equations for a clipping plane, and enable it in View
    void       enableClipPlane(int, Vector3 dir, real scal, bool absolute=true);
    
    /// disable cliping plane in View
    void       disableClipPlane(int);
    
    /// return enable/disable state
    int        hasClipPlane(int) const;
    
    //---------------------------------------------------------------------------
    
    /// set local projection matrix as if it had been set by glOrtho()
    void       setOrthoMat(GLdouble * mat);

    /// store the matrices defining the current OpenGL Model-View and Projection
    void       getMatrices();
    
    /// transform window coordinates to 3D world-coordinates
    Vector3    unproject(GLdouble x, GLdouble y, GLdouble z, bool get_matrices = false);
    
    //---------------------------------------------------------------------------
    
    /// position 'pos' in the center of the display
    void       move_to(const Vector3 & pos);
    
    /// set additional translation of focal point
    void       move_shift(const Vector3 & pos);
    
    /// translate view
    void       move_by(const Vector3 & trans)       { move_to( focus - trans ); }

    //---------------------------------------------------------------------------
    
    /// set rotation to given Quaternion
    void       rotate_to(const Quaternion<real>&);
    
    /// rotate to have `dir` aligned with the X-axis
    void       align_with(const Vector3& dir);

    /// rotate view
    void       rotate_by(const Quaternion<real> &q) { rotate_to( rotation * q ); }
    
    //---------------------------------------------------------------------------

    /// set absolute zoom
    void       zoom_to(GLfloat z);
    
    /// increase zoom (multiplicative)
    void       zoom_in(GLfloat z) { zoom_to( zoom * z ); }
    
    /// decrease zoom (multiplicative)
    void       zoom_out(GLfloat z) { zoom_to( zoom / z ); }
    
    //---------------------------------------------------------------------------
     
    /// adjust zoom and focus to match the ROI specificed by two corner points
    void       matchROI(Vector3, Vector3);
    
    //---------------------------------------------------------------------------
    
    /// draw text using gleDrawText
    void       drawText(std::string const& str, void* font, gle_color, int pos) const;

    /// display tiny vertical lines as elements of a scale bar
    void       drawScaleTicksH(GLfloat, GLfloat, GLfloat) const;

    /// display tiny horizontal lines as elements of a scale bar
    void       drawScaleTicksV(GLfloat, GLfloat, GLfloat) const;

    /// display a scale bar vertical or horizontal
    void       drawScaleH(GLfloat, GLfloat, GLfloat) const;
    
    /// display a scale bar vertical or horizontal
    void       drawScaleV(GLfloat, GLfloat, GLfloat) const;
    
    /// display a scale bar vertical or horizontal
    void       drawScaleX(GLfloat) const;

    /// display a scale bar (mode is vertical, horizontal, centered)
    void       drawScaleBar(int mode, real) const;
    
};

#endif
