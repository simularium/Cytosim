// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//F. Nedelec, Dec 2005

#ifndef GLAPP_H
#define GLAPP_H

#include "real.h"
#include "gle_color.h"
#include "vector3.h"
#include "quaternion.h"
#include "view.h"
#include <vector>

///glApp extends GLUT to manipulate a 2D or 3D display window
namespace glApp
{
    /// different View associated with the display windows
    extern std::vector<View> views;

    /// initialize first view
    void       initialize();
    
    /// change dimensionnality (this affects mostly the mouse controls availability)
    void       setDimensionality(int d);

    /// set display function `func`
    int        createWindow(void (*func)(View&));
    
    /// destroy window
    void       destroyWindow(int win);
    
    /// enter full-screen mode
    void       enterFullScreen(bool saveWindowPos);

    /// exit full-screen mode
    void       exitFullScreen();
    
    /// enter or exit full-screen mode
    void       toggleFullScreen();
    
    /// return current mode
    bool       isFullScreen();
    
    /// return current mode
    void       setFullScreen(bool);

    /// maximize window size within the current screen
    void       maximizeDisplay();
    
    /// callback function for window resize event
    void       resizeWindow(int, int);
    
    /// set the range normally visible for zoom = 1
    void       setScale(GLfloat);

    /// return view associated with current window
    View&      currentView();
    
    /// reset current view
    void       resetView();
    
    /// save higher resolution image with magnification 'mag'
    //int        saveImage(const char* name, unsigned mag, unsigned downsample);
    
    //--------------------------------- MENUS -----------------------------------
    
    /// create a menu with cnt 'unset' entries, using func for callbacks
    void       clearMenu(int menuID);
    
    /// build menu, and attach it if argument is a valid button (-1: do not attach)
    int        buildMenu();
    
    /// callback function for the menu build by buildMenu()
    void       processMenuEvent(int item);
    
    /// attach default menu to button
    void       attachMenu(int button);

    //-----------------------------------KEYS------------------------------------

    /// returns a string describing mouse and keyboard commands
    void       help(std::ostream&);
    
    /// callback function for normal keys
    void       processNormalKey(unsigned char, int modifiers);
    
    /// callback function for normal keys
    void       processNormalKey(unsigned char, int mouse_x, int mouse_y);

    /// set callback for keyboard events
    void       normalKeyFunc(void (*func)(unsigned char, int, int));
    
    /// callback function for normal keys
    void       processSpecialKey(int key, int modifiers);

    /// callback function for arrow/function keys
    void       processSpecialKey(int key, int mouse_x, int mouse_y);
    
    /// set callback for special key pressed events
    void       specialKeyFunc(void (*func)(int, int, int));

    //-----------------------------------MOUSE-----------------------------------
        
    /// callback function for mouse button down/up
    void       processMouseClick(int button, int state, int x, int y);
    
    /// callback function for mouse motion, when a button is pressed
    void       processMouseDrag(int x, int y);
    
    /// callback function for mouse motion, when no button is pressed
    void       processPassiveMouseMotion(int x, int y);
    
    /// set callback for shift-click, with unprojected down-position
    //func(mouseX, mouseY, mouseDown, specialKeys);
    void       actionFunc(void (*func)(int, int, const Vector3&, int));
    
    /// set callback for shift-click, with unprojected down- and current- mouse positions
    //func(mouseX, mouseY, mouseDown, mousePosition, specialKeys);
    void       actionFunc(void (*func)(int, int, Vector3&, const Vector3&, int));
    
    //---------------------------------MESSAGES---------------------------------
    
    /// display given text on screen for 3 sec
    void       flashText0(const char* str);
    
    ///@todo: replace flashText(...) by a std::ostream&
    /// draw text for 3 sec (to report that something has been done)
    void       flashText(const char* fmt, ...);
    
    /// set message displayed on current window
    void       setMessage(std::string const&);

    //-------------------------------DISPLAY------------------------------------
    
    /// called after display of scene
    void       displayInfo(int W, int H);

    /// display function for main window
    void       displayMain();
    
    /// display function for secondary windows
    void       displayPlain();
    
    /// this will call refresh all windows
    void       postRedisplay();
}


#endif
