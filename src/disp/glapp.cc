// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// F. Nedelec started glApp in Dec 2005

#include "glut.h"
#include "glapp.h"
#include <unistd.h>
#include <cstdarg>
#include "exceptions.h"
#include "save_image.h"
#include "glossary.h"
#include "tictoc.h"
#include "gle.h"


using namespace gle;


namespace glApp
{
    std::vector<View> views;
    
    int   mDIM        = 3;     ///< current dimensionality
    bool  mFullScreen = false; ///< flag indicating full-screen mode
    int   specialKeys = 0;     ///< state of special keys given by GLUT
    
    // --------------- MOUSE
    
    /// actions that can be performed with the mouse
    enum UserMode
    {
        MOUSE_ROTATE     = 0,
        MOUSE_TRANSLATE  = 1,
        MOUSE_ACTIVE     = 2,
        MOUSE_TRANSLATEZ = 3,
        MOUSE_SPIN       = 4,
        MOUSE_SET_ROI    = 5,
        MOUSE_MOVE_ROI   = 6,
        MOUSE_SELECT     = 7,
        MOUSE_PASSIVE    = 8
    };
    
    /// Specifies in which dimensionality each action is valid
    int actionDimensionality[] = { 3, 1, 1, 3, 2, 1, 1, 4, 4, 4, 0 };
    
    /// the action controlled with the mouse
    UserMode     userMode = MOUSE_ROTATE;

    /// change action
    void         switchUserMode(int dir);

    View         savedView("backup");
    UserMode     mouseAction = MOUSE_TRANSLATE;  ///< the action being performed by the mouse
    Vector3      mouseDown;                      ///< position where mouse button was pressed down
    GLint        mouseX, mouseY;                 ///< current position of mouse in pixels
    Vector3      depthAxis;                      ///< vector normal to desired rotation
    Vector3      mouseAxis;                      ///< axis of rotation for MOUSE_SPIN
    Vector3      viewFocus;                      ///< unprojected center of screen
    
    Vector3      ROI[2] = { Vector3(0,0,0) };    ///< Regions of interest selected with the mouse
    int          savedWindowPos[4] = { 24, 24, 800, 800 };
    
    real         nearZ  = 0;       ///< normalized device Z-coordinate of front-plane
    real         midZ   = 0.5;     ///< normalized device Z-coordinate of middle
    real         farZ   = 1.0;     ///< normalized device Z-coordinate of back-plane

    unsigned int imageIndex = 0;   ///< index for image name
    double       flashEndTime;
    std::string  flashString;

    Vector3      ROIdown[2];
    void         setROI(Vector3);
    void         setROI(Vector3, Vector3);
    bool         insideROI(Vector3);
    void         drawROI(Vector3[2]);

    /// function pointer for shift-click actions
    void (*mouseClickCallback)(int, int, const Vector3 &, int) = nullptr;
    
    /// function pointer for shift-motion actions
    void (*mouseDragCallback)(int, int, Vector3 &, const Vector3 &, int) = nullptr;
    
    /// function pointer for shift-click actions
    void (*normalKeyCallback)(unsigned char, int, int) = processNormalKey;
    
    /// function pointer for shift-motion actions
    void (*specialKeyCallback)(int, int, int) = processSpecialKey;
    
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 initialize a single View: views[0]
 */
void glApp::initialize()
{
    views.clear();
    views.push_back(View("*"));
}


/**
 This will disable OpenGL depth-test for DIM<3
 */
void glApp::setDimensionality(const int d)
{
    if ( mDIM != d )
    {
        //flashText("dimensionality changed to %i", d);
        mDIM = d;
        userMode = ( d == 3 ) ? MOUSE_ROTATE : MOUSE_TRANSLATE;
    }
    
    if ( 0 == views.size() )
        initialize();
}

//------------------------------------------------------------------------------

bool glApp::isFullScreen()
{
    return mFullScreen;
}

void glApp::setFullScreen(bool s)
{
    mFullScreen = s;
}


void glApp::enterFullScreen(bool saveWindowPos)
{
    if ( ! mFullScreen )
    {
        mFullScreen = true;
        if ( saveWindowPos )
        {
            //save the current window size and position:
            savedWindowPos[0] = glutGet(GLUT_WINDOW_X);
            savedWindowPos[1] = glutGet(GLUT_WINDOW_Y);
            savedWindowPos[2] = glutGet(GLUT_WINDOW_WIDTH);
            savedWindowPos[3] = glutGet(GLUT_WINDOW_HEIGHT);
        }
        //invoke full screen from GLUT
        glutFullScreen();
        //std::clog << "Fullscreen window " << glutGetWindow() << std::endl;
    }
}


void glApp::exitFullScreen()
{
    mFullScreen = false;
    // restore window dimensions:
    if ( savedWindowPos[2] > 8 && savedWindowPos[3] > 8 )
    {
        //std::clog << "saveWindow " << savedWindowPos[2] << " " << savedWindowPos[3] << '\n';
        glutReshapeWindow(savedWindowPos[2], savedWindowPos[3]);
        glutPositionWindow(savedWindowPos[0], savedWindowPos[1]);
    }
    else
        glutReshapeWindow(800, 800);
}


void glApp::toggleFullScreen()
{
    if ( mFullScreen )
        exitFullScreen();
    else
        enterFullScreen(1);
}


#if !defined(GLUT_WINDOW_SCALE)
#    define GLUT_WINDOW_SCALE 199
#endif

/**
 Adjust the size of window to maximize the vertical or horizontal dimension,
 without changing the aspect ratio of the window.
 */
void glApp::maximizeDisplay()
{
    int W = glutGet(GLUT_WINDOW_WIDTH);
    int H = glutGet(GLUT_WINDOW_HEIGHT);

    /// using addition by Renaud Blanch to handle Retina display:
    int s = std::max(1, glutGet(GLUT_WINDOW_SCALE));
 
    const int maxW = s * glutGet(GLUT_SCREEN_WIDTH);
    const int maxH = s * ( glutGet(GLUT_SCREEN_HEIGHT) - 49 );
    
    const GLfloat zx = GLfloat(maxW) / W;
    const GLfloat zy = GLfloat(maxH) / H;
    
    glutPositionWindow(0, 45);
    if ( zx < zy )
        glutReshapeWindow(maxW, int(zx*H));
    else
        glutReshapeWindow(int(zy*W), maxH);
}

//------------------------------------------------------------------------------
#pragma mark -

int glApp::createWindow(void (*func)(View&))
{
    View & view = views[0];
    
    std::ostringstream oss;
    oss << "rgba";
    if ( view.buffered )    oss << " double";
    if ( view.depth_test )  oss << " depth";
    if ( view.stencil )     oss << " stencil~" << view.stencil;
    if ( view.multisample ) oss << " samples~" << view.multisample;
    if ( view.retina )      oss << " hidpi";
    std::string mode = oss.str();
    
    //std::clog << "GLUT string mode " << mode << std::endl;
    
    // set GLUT display mode:
    glutInitDisplayString(mode.c_str());

    // set window size:
    glutInitWindowSize(view.width(), view.height());
    glutInitWindowPosition(view.window_position[0], view.window_position[1]);
    
    // create window with title containing current working directory:
    int win = 0;
    char str[256] = { 0 };
    strncpy(str, "Cytosim    ", sizeof(str));
    if ( getcwd(str+10, 246) )
        win = glutCreateWindow(str);
    else
        win = glutCreateWindow("Cytosim");
    assert_true( win > 0 );
    //std::clog << "new window " << win << std::endl;

    // create new View for this window, duplicating the current View:
    if ( win >= (int)views.size() )
        views.resize(win+1, view);
    
    // switch the new view:
    views[win].window(win);
    views[win].initGL();
    
    // set window position:
    views[win].window_position[0] += 20;
    views[win].window_position[1] += 20;
    
    if ( func )
        views[win].setDisplayFunc(func);

    glutReshapeFunc(resizeWindow);
    glutKeyboardFunc(normalKeyCallback);
    glutSpecialFunc(specialKeyCallback);
    glutMouseFunc(processMouseClick);
    glutMotionFunc(processMouseDrag);
    glutPassiveMotionFunc(processPassiveMouseMotion);
    attachMenu(GLUT_RIGHT_BUTTON);

    if ( win <= 1 )
        glutDisplayFunc(displayMain);
    else
        glutDisplayFunc(displayPlain);
    
    return win;
}


/**
 This will not destroy the main window
 */
void glApp::destroyWindow(int win)
{
    if ( win == 0 )
        win = glutGetWindow();
    
    if ( 1 < win  &&  win < (int)views.size()  &&  views[win].window() > 0 )
    {
        //std::clog << "Destroy window " << win << std::endl;
        assert_true( views[win].window() == win );
        glutDestroyWindow(views[win].window());
        views[win].window(0);
    }
}


void glApp::resizeWindow(int w, int h)
{
    unsigned win = glutGetWindow();
    views[win].reshape(w, h);
    flashText("window size %i %i", w, h);
    glClear(GL_COLOR_BUFFER_BIT);
    glutPostRedisplay();
}


void glApp::setScale(GLfloat s)
{
    if ( 0 == views.size() )
        initialize();

    views[0].view_size = s;
    
    if ( views.size() > 1 )
    {
        int win = glutGetWindow();
        // update all window-associated views:
        for ( unsigned n = 1; n < views.size(); ++n )
        {
            View & view = views[n];
            if ( view.window() > 0 )
            {
                glutSetWindow(view.window());
                view.view_size = s;
            }
        }
        glutSetWindow(win);
    }
}

/*
int glApp::saveImage(char const* name, unsigned mag, unsigned downsample)
{
    if ( mag > 1 )
    {
        // copy current view:
        View view = currentView();
        int W = mag * view.window_size[0];
        int H = mag * view.window_size[1];
        view.reshape(W, H);
        if ( OffScreen::createBuffer(W, H, 0) )
        {
            view.initGL();
            displayPlain();
            GLint vp[] = { 0, 0, W, H };
            int res = SaveImage::saveImage(name, "png", vp, downsample);
            OffScreen::releaseBuffer();
            return res;
        }
    }
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return SaveImage::saveImage(name, "png", vp, downsample);
}
*/

//------------------------------------------------------------------------------
#pragma mark -


/// this works even if no window is open
View& glApp::currentView()
{
    assert_true( views.size() > 0 );
    
    if ( views.size() <= 1 )
        return views[0];
    else
        return views[glutGetWindow()];
}


void glApp::resetView()
{
    assert_true( glutGetWindow() < (int)views.size() );
    glApp::currentView().reset();
}

//------------------------------------------------------------------------------
#pragma mark -

/** Only 2D */
bool glApp::insideROI(Vector3 pos)
{
    bool inX = ( ROI[0].XX < pos.XX  &&  pos.XX < ROI[1].XX );
    bool inY = ( ROI[0].YY < pos.YY  &&  pos.YY < ROI[1].YY );
    return inX && inY;
}

/** Only 2D */
void glApp::setROI(Vector3 a)
{
    ROI[0] = a;
    ROI[1] = a;
}

/** Only 2D */
void glApp::setROI(Vector3 a, Vector3 b)
{
    ROI[0].XX = std::min(a.XX, b.XX);
    ROI[1].XX = std::max(a.XX, b.XX);
    ROI[0].YY = std::min(a.YY, b.YY);
    ROI[1].YY = std::max(a.YY, b.YY);
    ROI[0].ZZ = std::min(a.ZZ, b.ZZ);
    ROI[1].ZZ = std::max(a.ZZ, b.ZZ);
}


//------------------------------------------------------------------------------
//------------------------------ keyboard commands -----------------------------
//------------------------------------------------------------------------------
#pragma mark -

void glApp::help(std::ostream& os)
{
    os << "                       Mouse Controls\n";
    os << "\n";
    os << " The display can be manipulated with click-and-drag movements,\n";
    os << " depending on the current `mode' selected by pressing `TAB':\n";
    os << "      Rotate                                     (3D only)\n";
    os << "      Translate in XY     (plane of the camera front lens)\n";
    os << "      Active                        (click to grab fibers)\n";
    os << "      Translate in XZ          (away/closer to the camera)\n";
    os << "      Spin & Zoom                   (Rotation in XY plane)\n";
    os << "      Select/Move region-of-interest\n";
    os << "\n";
    os << "  In the default mode, a SHIFT-CLICK can grab the filaments\n";
    os << "  A Menu is accessed by a right click\n";
    os << "  You might perhaps be able to zoom in/out with the mouse wheel\n";
    os << "\n";
    os << "                       Keyboard Controls\n\n";
    os << " + -         Zoom in and out; hold SHIFT for finer motion\n";
    os << " arrow keys  Translate; hold SHIFT for finer motion; hold ALT for rotation\n";
    os << " z           Reset view and refresh display\n";
    os << " h           Hide/show help\n";
    os << " b x         Show/hide a 10 um scale bar; Show/hide axes\n";
    os << " f ESC       Toggle fullscreen mode; exit full screen mode\n";
    os << " y           Save PPM/PNG image\n";
}


//------------------------------------------------------------------------------
void glApp::switchUserMode(int dir)
{
    int u = userMode;
    do {
        u = ( u + dir + MOUSE_PASSIVE ) % MOUSE_PASSIVE;
    } while ( actionDimensionality[u] > mDIM );

    userMode = (UserMode)u;
    switch ( userMode )
    {
        case MOUSE_ROTATE:    flashText("Mouse: Rotate");       break;
        case MOUSE_TRANSLATE: flashText("Mouse: Translate");    break;
        case MOUSE_ACTIVE:    flashText("Mouse: Active");       break;
        case MOUSE_TRANSLATEZ:flashText("Mouse: Translate-Z");  break;
        case MOUSE_SPIN:      flashText("Mouse: Spin & Zoom");  break;
        case MOUSE_SET_ROI:   flashText("Mouse: Select ROI");   break;
        case MOUSE_MOVE_ROI:  flashText("Mouse: Move ROI");     break;
        case MOUSE_SELECT:    flashText("Mouse: Select");       break;
        case MOUSE_PASSIVE:   flashText("Mouse: Passive");      break;
    }
}


///\todo flexible key assignment map to accomodate different keyboard layouts
void glApp::processNormalKey(unsigned char c, int modifiers)
{
    View & view = glApp::currentView();
    
    /* In the switch below:
     - use 'break' if the display needs a refresh
     - use 'return' if redrawing is not necessary.
    */
    switch (c)
    {
        case 17:
            if ( modifiers & GLUT_ACTIVE_CTRL )
                exit(EXIT_SUCCESS);
            break;
        
            
        case 9:          // ascii 9 is the horizontal tab
        case 25:         // ascii 25 is shift-tab
            switchUserMode(c==9 ? 1 : -1);
            break;
        
        
        case 27:             // ascii 27 is the escape key
            if ( mFullScreen )
                exitFullScreen();
            else
                destroyWindow(glutGetWindow());
            break;
        
            
        case 'f':
            toggleFullScreen();
            break;
        
        case 'F':
            maximizeDisplay();
            break;

        case 'z':
            view.reset();
            postRedisplay();
            break;
        
        
        case 'v':
            if ( mDIM == 3 )
            {
                view.slice = ( view.slice + 1 ) % 4;
                flashText("view:slice = %i", view.slice);
            }
            break;
        
        case 'b':
            view.scale_bar_mode = ( view.scale_bar_mode + 1 ) % 4;
            break;
        
            
        case 'h':
            view.draw_memo = ( view.draw_memo + 1 ) % 3;
            if ( view.draw_memo == 2 )
            {
                std::ostringstream oss;
                help(oss);
                view.memo = oss.str();
            }
            else
                view.memo = "Please, visit www.cytosim.org";
            break;
        
        
        case 'x':
            view.draw_axes = ( view.draw_axes ? 0 : mDIM );
            break;

#if ( 0 )
        case 'y': {
            char name[1024] = { 0 };
            snprintf(name, sizeof(name), "image%04i.png", imageIndex++);
            saveImage(name, 1, 1);
        } break;
        
        case 'Y': {
            char name[1024] = { 0 };
            snprintf(name, sizeof(name), "image%04i.png", imageIndex++);
            saveImage(name, 4, 2);
        } break;
#endif
        //------------------------- Zoom in and out:
        
        case '/':
            if ( modifiers & GLUT_ACTIVE_SHIFT )
                view.zoom_out(1.071773463);
            else
                view.zoom_out(1.4142135623f);
            break;
        
        case '*':
            if ( modifiers & GLUT_ACTIVE_SHIFT )
                view.zoom_in(1.071773463f);
            else
                view.zoom_in(1.4142135623f);
            break;
     
        case '-':
            view.zoom_out(1.4142135623f);
            break;
        
        case '=':
            view.zoom_in(1.4142135623f);
            break;
            
        case '_':
            view.zoom_out(1.071773463f);
            break;
        
        case '+':
            view.zoom_in(1.071773463);
            break;

        default:
            flashText("ignored key %i [%c]", c, c);
            return;
    }
    
    //if break was used, redisplay is needed:
    postRedisplay();
    buildMenu();
}


///\todo flexible key assignment map to accomodate different keyboard layouts
void glApp::processNormalKey(unsigned char c, int, int)
{
    processNormalKey(c, glutGetModifiers());
}

void glApp::normalKeyFunc(void (*func)(unsigned char, int, int))
{
    normalKeyCallback = func;
}

//------------------------------------------------------------------------------

/**
 arrow-keys controls translation, and
 arrow-keys with 'ALT' pressed controls rotation.
 
 motion is reduced by holding down SHIFT.
 */
void glApp::processSpecialKey(int key, int modifiers)
{
    Vector3 vec(0,0,0), dxy(0, 0, 0);
    View & view = glApp::currentView();
    real F = ( modifiers & GLUT_ACTIVE_SHIFT ) ? 0.0625 : 1;

    //std::clog << "special key " << key << ": " << modifiers << "  ";
    switch ( key )
    {
        case GLUT_KEY_HOME:      view.reset();            glutPostRedisplay(); return;
        case GLUT_KEY_PAGE_UP:   view.zoom_in(1.4142f);   glutPostRedisplay(); return;
        case GLUT_KEY_PAGE_DOWN: view.zoom_out(1.4142f);  glutPostRedisplay(); return;
        case GLUT_KEY_LEFT:      dxy.set(-F,0,0);         break;
        case GLUT_KEY_RIGHT:     dxy.set(+F,0,0);         break;
        case GLUT_KEY_DOWN:      dxy.set(0,-F,0);         break;
        case GLUT_KEY_UP:        dxy.set(0,+F,0);         break;
    }

    // inverse the rotation of the current view:
    Quaternion<real> rot = view.rotation.conjugated();
    
    if ( modifiers & GLUT_ACTIVE_ALT )
    {
        // Rotate view
        rot.rotateVector(vec, cross(Vector3(0, 0, 1), dxy));
        rot.setFromAxis(vec, F * (M_PI/8));
        view.rotate_by(rot);
    }
    else
    {
        // Translate view
        if ( (modifiers & GLUT_ACTIVE_CTRL) ^ (userMode == MOUSE_TRANSLATEZ) )
            dxy.set(dxy.XX, 0, dxy.YY);
        rot.rotateVector(vec, dxy);
        //std::clog << "vec " << dxy << " >>> " << vec << "\n";
        view.move_by((128*view.pixelSize())*vec);
    }
    glutPostRedisplay();
}

void glApp::processSpecialKey(int key, int, int)
{
    processSpecialKey(key, glutGetModifiers());
}

void glApp::specialKeyFunc(void (*func)(int, int, int))
{
    specialKeyCallback = func;
}

//------------------------------------------------------------------------------
#pragma mark -

int buildFogMenu()
{
    static int menu = 0;
    if ( menu == 0 )
    {
        menu = glutCreateMenu(glApp::processMenuEvent);
        glutAddMenuEntry("Disable",          100);
        glutAddMenuEntry("Linear ",          101);
        glutAddMenuEntry("Exponential 1/16", 102);
        glutAddMenuEntry("Exponential 1/8",  103);
        glutAddMenuEntry("Exponential 1/4",  104);
        glutAddMenuEntry("Exponential 1/2",  105);
        glutAddMenuEntry("Exponential 1",    106);
        glutAddMenuEntry("Exponential 2",    107);
        glutAddMenuEntry("Exponential 4",    108);
        glutAddMenuEntry("Exponential 8",    109);
        glutAddMenuEntry("Exponential 16",   110);
    }
    return menu;
}

int buildWindowSizeMenu()
{
    static int menu = 0;
    if ( menu == 0 )
    {
        menu = glutCreateMenu(glApp::processMenuEvent);
        glutAddMenuEntry("256x256",   200);
        glutAddMenuEntry("384x384",   201);
        glutAddMenuEntry("512x256",   202);
        glutAddMenuEntry("512x384",   203);
        glutAddMenuEntry("512x512",   204);
        glutAddMenuEntry("768x768",   205);
        glutAddMenuEntry("1024x128",  206);
        glutAddMenuEntry("1024x256",  207);
        glutAddMenuEntry("1024x512",  208);
        glutAddMenuEntry("1024x768",  209);
        glutAddMenuEntry("1024x1024", 210);
        glutAddMenuEntry("1280x640",  211);
        glutAddMenuEntry("1280x1280", 212);
        glutAddMenuEntry("-", 0);
        glutAddMenuEntry("426x240 (240p)",    220);
        glutAddMenuEntry("640x360 (360p)",    221);
        glutAddMenuEntry("854x480 (480p)",    222);
        glutAddMenuEntry("1280x720 (720p)",   223);
        glutAddMenuEntry("1920x1080 (1080p)", 224);
        glutAddMenuEntry("2560x1440 (1440p)", 225);
    }
    return menu;
}


int buildClipMenu()
{
    static int menu = 0;
    if ( menu == 0 )
    {
        menu = glutCreateMenu(glApp::processMenuEvent);
        glutAddMenuEntry("Disable",    300);
        
        glutAddMenuEntry(" X > 0",     301);
        glutAddMenuEntry(" X < 0",     302);
        glutAddMenuEntry("-1 < X < 1", 303);
        
        glutAddMenuEntry(" Y > 0",     311);
        glutAddMenuEntry(" Y < 0",     312);
        glutAddMenuEntry("-1 < Y < 1", 313);
        
        glutAddMenuEntry(" 0 < Z",     321);
        glutAddMenuEntry(" Z < 0",     322);
        glutAddMenuEntry("-1 < Z < 1", 323);
        glutAddMenuEntry("-0.5 < Z < 0.5", 324);
    }
    return menu;
}


int glApp::buildMenu()
{
    static int menu = 0;
    static int menu1, menu2, menu3;
    
    //std::clog << "buildMenu" << std::endl;
    if ( menu )
        clearMenu(menu);
    else {
        menu1 = buildFogMenu();
        menu2 = buildWindowSizeMenu();
        menu3 = buildClipMenu();
        menu  = glutCreateMenu(processMenuEvent);
    }
    
    glutAddSubMenu("Fog",            menu1);
    glutAddSubMenu("Window Size",    menu2);
    glutAddSubMenu("Slice",          menu3);
    glutAddMenuEntry("Reset View",         1);
    glutAddMenuEntry("Show/hide Scalebar", 2);
    glutAddMenuEntry("Show/hide XYZ-axes", 3);
    glutAddMenuEntry("Toggle fullscreen mode", 4);
    glutAddMenuEntry(mDIM==2?"Use 3D Controls":"Use 2D Controls", 7);
    glutAddMenuEntry("Quit",         20);
    
    return menu;
}

//------------------------------------------------------------------------------

void glApp::clearMenu(int menu)
{
    glutSetMenu(menu);
    const int mx = glutGet(GLUT_MENU_NUM_ITEMS);
    for ( int m = mx; m > 0; --m )
        glutRemoveMenuItem(m);
    assert_true( glutGet(GLUT_MENU_NUM_ITEMS) == 0 );
}

void glApp::attachMenu(int b)
{
    buildMenu();
    assert_true( b==GLUT_LEFT_BUTTON || b==GLUT_MIDDLE_BUTTON || b==GLUT_RIGHT_BUTTON );
    glutAttachMenu(b);
}

void glApp::processMenuEvent(int item)
{
    View & view = glApp::currentView();
    switch( item )
    {
        case 0:   return;
        case 1:   view.reset();                      break;
        case 2:   view.scale_bar_mode = ! view.scale_bar_mode;    break;
        case 3:   view.draw_axes = ( view.draw_axes ? 0 : mDIM ); break;
        case 4:   toggleFullScreen();                break;
        case 7:   setDimensionality(mDIM==2?3:2);    break;
        
        case 20:  exit(EXIT_SUCCESS);                break;
            
        case 100: view.enableFog(0, 0);              break;
        case 101: view.enableFog(1, 0);              break;
        case 102: view.enableFog(2, 0.0625);         break;
        case 103: view.enableFog(2, 0.125);          break;
        case 104: view.enableFog(2, 0.25);           break;
        case 105: view.enableFog(2, 0.5);            break;
        case 106: view.enableFog(2, 1);              break;
        case 107: view.enableFog(2, 2);              break;
        case 108: view.enableFog(2, 4);              break;
        case 109: view.enableFog(2, 8);              break;
        case 110: view.enableFog(2, 16);             break;
            
        case 200: glutReshapeWindow(256, 256);       break;
        case 201: glutReshapeWindow(384, 384);       break;
        case 202: glutReshapeWindow(512, 256);       break;
        case 203: glutReshapeWindow(512, 384);       break;
        case 204: glutReshapeWindow(512, 512);       break;
        case 205: glutReshapeWindow(768, 768);       break;
        case 206: glutReshapeWindow(1024, 128);      break;
        case 207: glutReshapeWindow(1024, 256);      break;
        case 208: glutReshapeWindow(1024, 512);      break;
        case 209: glutReshapeWindow(1024, 768);      break;
        case 210: glutReshapeWindow(1024, 1024);     break;
        case 211: glutReshapeWindow(1280, 640);      break;
        case 212: glutReshapeWindow(1280, 1280);     break;
        case 220: glutReshapeWindow(426, 240);       break;
        case 221: glutReshapeWindow(640, 360);       break;
        case 222: glutReshapeWindow(854, 480);       break;
        case 223: glutReshapeWindow(1280, 720);      break;
        case 224: glutReshapeWindow(1920, 1080);     break;
        case 225: glutReshapeWindow(2560, 1440);     break;
        
        case 300:
            view.disableClipPlane(0);
            view.disableClipPlane(1);
            break;
            
        case 301:
            view.enableClipPlane(0, Vector3(+1,0,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 302:
            view.enableClipPlane(0, Vector3(-1,0,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 303:
            view.enableClipPlane(0, Vector3(+1,0,0), 1);
            view.enableClipPlane(1, Vector3(-1,0,0), 1);
            break;
 
        case 311:
            view.enableClipPlane(0, Vector3(0,+1,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 312:
            view.enableClipPlane(0, Vector3(0,-1,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 313:
            view.enableClipPlane(0, Vector3(0,+1,0), 1);
            view.enableClipPlane(1, Vector3(0,-1,0), 1);
            break;
 
        case 321:
            view.enableClipPlane(0, Vector3(0,0,+1), 0);
            view.disableClipPlane(1);
            break;
            
        case 322:
            view.enableClipPlane(0, Vector3(0,0,-1), 0);
            view.disableClipPlane(1);
            break;
            
        case 323:
            view.enableClipPlane(0, Vector3(0,0,+1), 1);
            view.enableClipPlane(1, Vector3(0,0,-1), 1);
            break;

        case 324:
            view.enableClipPlane(0, Vector3(0,0,+1), 0.5);
            view.enableClipPlane(1, Vector3(0,0,-1), 0.5);
            break;

        default: ABORT_NOW("unknown menu item");
    }
    glutPostRedisplay();
    buildMenu();
}

//------------------------------------------------------------------------------
//--------------------------------  MOUSE  -------------------------------------
//------------------------------------------------------------------------------
#pragma mark -

void glApp::actionFunc(void (*func)(int, int, const Vector3 &, int))
{
    mouseClickCallback = func;
}

void glApp::actionFunc(void (*func)(int, int, Vector3 &, const Vector3 &, int))
{
    mouseDragCallback = func;
}

#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP    3
#  define GLUT_WHEEL_DOWN  4
#endif

//------------------------------------------------------------------------------
void glApp::processMouseClick(int button, int state, int mX, int mY)
{
    View & view = glApp::currentView();
    int winX = view.width();
    int winY = view.height();

    //printf("mouse button %i (%4i %4i) state %i key %i\n", button, mx, my, state, specialKeys);

    mouseX = mX;
    mouseY = winY-mY;
    
    savedView = view;
    savedView.getMatrices();
    mouseDown = savedView.unproject(mouseX, mouseY, nearZ);
    viewFocus = savedView.unproject(winX/2.0, winY/2.0, nearZ);
    
    if ( state == GLUT_UP )
    {
         /*
         Zooming with the mouse-wheel requires an extended GLUT.
         http://iihm.imag.fr/blanch/howtos/MacOSXGLUTMouseWheel.html
         
         The zoom preserves the position pointed at by the mouse.
         */
        GLfloat wz = 1.0;

        if ( button == GLUT_WHEEL_UP )
            wz = 0.96969696f;
        if ( button == GLUT_WHEEL_DOWN )
            wz = 1.031250f;

        if ( wz != 1 )
        {
            /* 
             in 2D, we do not allow any shift in Z,
             and in 3d, we zoom in on the middle Z-plane
             */
            if ( mDIM == 3 )
                mouseDown = savedView.unproject(mouseX, mouseY, midZ);
            else
                mouseDown.ZZ = 0;
            view.zoom_out(wz);
            view.move_to((1.0-wz)*mouseDown+wz*view.focus);
        }

        glutSetCursor(GLUT_CURSOR_INHERIT);
        mouseAction = MOUSE_PASSIVE;
        glutPostRedisplay();
        return;
    }

    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
    
    // action is primarily decided by current mode
    mouseAction = userMode;
    specialKeys = glutGetModifiers();

    // change the mouse action if the CONTROL is pressed:
    if ( specialKeys & GLUT_ACTIVE_CTRL )
    {
        switch ( mouseAction )
        {
            case MOUSE_TRANSLATE: mouseAction = (mDIM==2)?MOUSE_SPIN:MOUSE_ROTATE; break;
            case MOUSE_SPIN:      mouseAction = (mDIM==2)?MOUSE_TRANSLATE:MOUSE_TRANSLATEZ; break;
            case MOUSE_SET_ROI:   mouseAction = MOUSE_TRANSLATE; break;
            case MOUSE_ROTATE:    mouseAction = MOUSE_TRANSLATE; break;
            case MOUSE_TRANSLATEZ:mouseAction = (mDIM==2)?MOUSE_TRANSLATE:MOUSE_ROTATE;  break;
            default: break;
        }
    }
    
    // change the mouse action because the shift key is down:
    if ( specialKeys & GLUT_ACTIVE_SHIFT )
    {
        mouseAction = MOUSE_ACTIVE;
        specialKeys -= GLUT_ACTIVE_SHIFT;
    }
    
    switch( mouseAction )
    {
        case MOUSE_TRANSLATE:
        {
        } return;
            
            
        case MOUSE_TRANSLATEZ:
        {
            depthAxis  = normalize( viewFocus - savedView.focus );
            Vector3 ud = savedView.unproject(winX/2.0, winY, nearZ);
            mouseAxis  = normalize( ud - viewFocus );
        } break;
            
            
        case MOUSE_ROTATE:
        {
            /* 
            Choose the amplification factor for mouse controlled rotation:
            for a value of one, the rotation exactly follows the mouse pointer 
            */
            const real amplification = 3.0;
            depthAxis  = mouseDown - savedView.focus;
            depthAxis *= amplification / depthAxis.normSqr();
        } break;
            
            
        case MOUSE_SPIN:
        {
            mouseAxis  = normalize( viewFocus - savedView.focus );
            depthAxis  = mouseDown - viewFocus;
        } break;
            
        case MOUSE_SET_ROI:
        case MOUSE_MOVE_ROI:
        {
            mouseDown = savedView.unproject(mouseX, mouseY, midZ);
            if ( insideROI(mouseDown) )
            {
                ROIdown[0] = ROI[0];
                ROIdown[1] = ROI[1];
                mouseAction = MOUSE_MOVE_ROI;
            }
            if ( mouseAction == MOUSE_SET_ROI )
            {
                setROI(mouseDown);
                flashText("click at %.4f %.4f %.4f", ROI[0].XX, ROI[0].YY, ROI[0].ZZ);
            }
        } break;
        
            
        case MOUSE_ACTIVE:
        {
            if ( mouseClickCallback )
            {
                mouseDown = savedView.unproject(mouseX, mouseY, midZ);
                //std::clog << "Action down at "<<mouseDown<<std::endl;
                mouseClickCallback(mouseX, mouseY, mouseDown, specialKeys);
            }
        }
        
        case MOUSE_SELECT:
        case MOUSE_PASSIVE:
            return;
    }
    glutPostRedisplay();
}


//------------------------------------------------------------------------------
void glApp::processMouseDrag(int mX, int mY)
{
    //printf("mouse motion (%i %i) %i\n", mx, my, mouseAction);
    View & view = glApp::currentView();
    int winY = view.height();

    mouseX = mX;
    mouseY = winY-mY;

    Vector3 mouse = savedView.unproject(mouseX, mouseY, nearZ);

    switch( mouseAction )
    {
        case MOUSE_ROTATE:
        {
            /* we should rotate after: Q <- dQ * sQ, however dQ is defined in the 
            reference frame rotated by sQ already, so dQ = sQ * dM * inv(sQ).
            This leads to the multiplication on the right: Q <- sQ * dM. */
            Quaternion<real> q;
            q.setFromAxis( cross(depthAxis, mouse-mouseDown) );
            view.rotate_to( savedView.rotation * q );
        } break;
        
        
        case MOUSE_SPIN:
        {
            real cos = dot(depthAxis, mouse - viewFocus);
            real sin = dot(mouseAxis, cross(depthAxis, mouse - viewFocus));
            Quaternion<real> q;
            q.setFromAxis(mouseAxis, atan2( sin, cos ));
            view.rotate_to(savedView.rotation * q);
            real Z = norm( mouse - viewFocus ) / norm( mouseDown - viewFocus );
            if ( Z > 0.001 ) view.zoom_to(savedView.zoom * (GLfloat)Z);
        } break;

        
        case MOUSE_TRANSLATE:
        {
            view.move_to( savedView.focus - ( mouse - mouseDown ) );
        } break;
        
        
        case MOUSE_TRANSLATEZ:
        {
            real S = dot(mouse - mouseDown, mouseAxis);
            Vector3 move = mouse - mouseDown - S * ( depthAxis + mouseAxis );
            view.move_to( savedView.focus - move );
        } break;
        

        case MOUSE_SET_ROI:
        {
            setROI(mouseDown, savedView.unproject(mouseX, mouseY, midZ));
            Vector3 d = ROI[1] - ROI[0];
            flashText("ROI %.3fx%.3f diag. %.3f", d.XX, d.YY, d.norm());
        } break;
        
        
        case MOUSE_MOVE_ROI:
        {
            Vector3 d = savedView.unproject(mouseX, mouseY, midZ) - mouseDown;
            ROI[0] = ROIdown[0] + d;
            ROI[1] = ROIdown[1] + d;
            flashText("ROI moved %.3f %.3f", d.XX, d.YY);
        } break;
            
        
        case MOUSE_ACTIVE:
        {
            if ( mouseDragCallback )
            {
                mouse = savedView.unproject(mouseX, mouseY, midZ);
                //std::clog << "Action move at " << mouse << std::endl;
                mouseDragCallback(mouseX, mouseY, mouseDown, mouse, specialKeys);
            }
        } break;
        
        
        case MOUSE_SELECT:
        case MOUSE_PASSIVE: 
            break;
    }
    glutPostRedisplay();
}

//------------------------------------------------------------------------------
void glApp::processPassiveMouseMotion(int mx, int my)
{
    //printf("passive mouse (%i %i)\n", mx, my);
    //int x = glutGet(GLUT_WINDOW_WIDTH)-8;
    //int y = glutGet(GLUT_WINDOW_HEIGHT)-8;
}

//------------------------------------------------------------------------------
#pragma mark -

void glApp::flashText0(const char* str)
{
    flashString = str;
    flashEndTime = TicToc::seconds_today() + 3.0;
    
    if ( views.size() > 1  &&  views[1].window()==1 )
        glutPostWindowRedisplay(1);
}

void glApp::flashText(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char tmp[1024];
    vsnprintf(tmp, 1024, fmt, args);
    va_end(args);
    flashText0(tmp);
}

//------------------------------------------------------------------------------

void glApp::drawROI(Vector3 roi[2])
{
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,0);
    glLineWidth(0.5);

    glBegin(GL_LINE_LOOP);
    glVertex3d(roi[0].XX, roi[0].YY, roi[0].ZZ);
    glVertex3d(roi[1].XX, roi[0].YY, roi[0].ZZ);
    glVertex3d(roi[1].XX, roi[1].YY, roi[0].ZZ);
    glVertex3d(roi[0].XX, roi[1].YY, roi[0].ZZ);
    glEnd();

    if ( mDIM == 3 )
    {
        glBegin(GL_LINE_LOOP);
        glVertex3d(roi[0].XX, roi[0].YY, roi[1].ZZ);
        glVertex3d(roi[1].XX, roi[0].YY, roi[1].ZZ);
        glVertex3d(roi[1].XX, roi[1].YY, roi[1].ZZ);
        glVertex3d(roi[0].XX, roi[1].YY, roi[1].ZZ);
        glEnd();
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 This is used for any secondary window.
 It does not show the interactive feedback to user.
 */
void glApp::displayPlain()
{
    gle::gleReportErrors(stderr, "in glApp::displayPlain()");
    View & view = glApp::currentView();

    view.openDisplay();
    view.callDraw();
    view.closeDisplay();

    glFinish();
    if ( view.buffered )
        glutSwapBuffers();
    else
        glFlush();
}


/**
 This is used for the main window
 */
void glApp::displayMain()
{
    gle::gleReportErrors(stderr, "in glApp::displayMain()");
    View & view = views[1];
    
    view.openDisplay();
    view.callDraw();
    view.closeDisplay();
    view.drawInteractiveFeatures();

    if ( flashString.size() )
    {
        /// check time
        if ( TicToc::seconds_today() > flashEndTime )
            flashString = "";
        else
        {
            glColor3f(0.6f,0.6f,1.0f);
            view.drawText(flashString, GLUT_BITMAP_9_BY_15, 0x0, 2);
        }
    }
    
    if ( userMode == MOUSE_SET_ROI )
        drawROI(ROI);

    glFinish();
    if ( view.buffered )
        glutSwapBuffers();
    else
        glFlush();
}

/**
 call glutPostRedisplay() if only the current window needs to be updated
 */
void glApp::postRedisplay()
{
    for ( size_t n = 1; n < views.size(); ++n )
        if ( views[n].window() > 0 )
            glutPostWindowRedisplay(n);
}


void glApp::setMessage(std::string const& msg)
{
    glApp::currentView().setMessage(msg);
}

