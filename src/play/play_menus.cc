// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

void processMenuFiber(int item)
{
    FiberDisp * FD = player.firstFiberDisp();
    
    if ( FD )
    {
        switch (item)
        {
            case 0:   break;
            case 1:   FD->line_style   = FD->line_style?0:1;      break;
            case 2:   FD->line_style   = FD->line_style==2?0:2;   break;
                
            case 3:   FD->point_style  = !FD->point_style;        break;
            case 5:   FD->point_style  = FD->point_style==2?0:2;  break;
                
            case 7:   FD->end_style[1] = 3*!FD->end_style[1];     break;
            case 8:   FD->end_style[0] = 2*!FD->end_style[0];     break;
                
            case 9:   FD->force_scale  = FD->force_scale>0?0:0.1; break;
            case 10:  FD->visible      = !FD->visible;            break;
                
            case 20:  FD->coloring = FiberDisp::COLORING_OFF;       break;
            case 21:  FD->coloring = FiberDisp::COLORING_RANDOM;    break;
            case 22:  FD->coloring = FiberDisp::COLORING_MARK;      break;
            case 23:  FD->coloring = FiberDisp::COLORING_CLUSTER;   break;
            case 24:  FD->coloring = FiberDisp::COLORING_DIRECTION; break;
            case 25:  FD->coloring = FiberDisp::COLORING_AGE;       break;
                
            case 30:  FD->draw_average = 0;  break;
            case 31:  FD->draw_average = 1;  break;
            case 32:  FD->draw_average = 2;  break;
                
            default:
                std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
                return;
        }
        glApp::postRedisplay();
    }
}


int buildMenuFiber()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuFiber);
    else
        glApp::clearMenu(menuID);
    
    FiberDisp * FD = player.firstFiberDisp();
    if ( FD )
    {
        glutAddMenuEntry(FD->visible        ? "Hide"             :"Show",             10);
        glutAddMenuEntry(FD->line_style     ? "Hide Lines"       :"Show Lines",        1);
        glutAddMenuEntry(FD->line_style==2  ? "Hide Tensions"    :"Show Tensions",     2);
        glutAddMenuEntry(FD->point_style    ? "Hide Points"      :"Show Points",       3);
        glutAddMenuEntry(FD->point_style==2 ? "Hide Arrows"      :"Show Arrows",       5);
        glutAddMenuEntry(FD->end_style[1]   ? "Hide Minus-ends"  :"Show Minus-end",    7);
        glutAddMenuEntry(FD->end_style[0]   ? "Hide Plus-ends"   :"Show Plus-end",     8);
        glutAddMenuEntry(FD->force_scale>0  ? "Hide Point-forces":"Show Point-Forces", 9);
        glutAddMenuEntry("No coloring",           20);
        glutAddMenuEntry("Coloring by number",    21);
        glutAddMenuEntry("Coloring by mark",      22);
        glutAddMenuEntry("Coloring by cluster",   23);
        glutAddMenuEntry("Coloring by direction", 24);
        glutAddMenuEntry("Coloring by age",       25);
        glutAddMenuEntry("draw_average=0", 30);
        glutAddMenuEntry("draw_average=1", 31);
        glutAddMenuEntry("draw_average=2", 32);
    }
    else
        glutAddMenuEntry("no fiber?", 0);

    return menuID;
}

//------------------------------------------------------------------------------
void processMenuCouple(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  disp.couple_select = 0;  break;
        case 2:  disp.couple_select = 1;  break;
        case 3:  disp.couple_select = 2;  break;
        case 4:  disp.couple_select = 4;  break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}

int buildMenuCouple()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuCouple);
    else
        glApp::clearMenu(menuID);
    
    glutAddMenuEntry("Hide all",    1);
    glutAddMenuEntry("Show free",   2);
    glutAddMenuEntry("Show bound",  3);
    glutAddMenuEntry("Show links",  4);
    return menuID;
}

//------------------------------------------------------------------------------
void processMenuDisplay(int item)
{
    View & view = glApp::currentView();
    switch (item)
    {
        case 0:   return;
        case 1:   view.reset();                            break;
        case 3:   disp.tile = !disp.tile;                      break;
        case 4:   glApp::toggleFullScreen();               break;
        case 6:   view.track_fibers = !view.track_fibers;  break;
        
        case 101: player.setStyle(1);  break;
        case 102: player.setStyle(2);  break;
        case 103: player.setStyle(3);  break;
            
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}


int buildMenuStyle()
{
    static int menuID = 0;
    if ( menuID == 0 )
    {
        menuID = glutCreateMenu(processMenuDisplay);
        glutAddMenuEntry("Detailled (style 1)", 101);
        glutAddMenuEntry("Fastest (style 2)", 102);
        glutAddMenuEntry("Best Looking (style 3)", 103);
    }
    return menuID;
}


int buildMenuDisplay()
{
    static int menuID = 0;
    int m0 = buildMenuStyle();
    int m1 = buildMenuFiber();
    int m2 = buildMenuCouple();
    
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuDisplay);
    else
        glApp::clearMenu(menuID);
    
    glutAddMenuEntry("Reset View",  1);
    glutAddSubMenu("Style",   m0);
    glutAddSubMenu("Fibers",  m1);
    glutAddSubMenu("Couple",  m2);
    
    View & view = glApp::currentView();
    glutAddMenuEntry("Toggle fullscreen mode (f)", 4);
    glutAddMenuEntry(disp.tile?"Non-tiled Display":"Tiled Display", 3);
    glutAddMenuEntry(view.track_fibers?"stop tracking":"Track Fibers", 6);
    
    return menuID;
}


//------------------------------------------------------------------------------
#pragma mark -

void processMenuFiberSelect(int item)
{
    FiberDisp * FD = player.firstFiberDisp();
    if ( FD )
    {
        switch (item)
        {
            case 0:  return;
            case 1:  FD->exclude  = 0;   break;
            case 2:  FD->exclude ^= 1;   break;
            case 3:  FD->exclude ^= 2;   break;
            default:
                std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
                return;
        }
        glApp::postRedisplay();
    }
}

int buildMenuFiberSelect()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuFiberSelect);
    else
        glApp::clearMenu(menuID);
    
    glutAddMenuEntry("Hide All", 1);
    FiberDisp * FD = player.firstFiberDisp();
    if ( FD )
    {
        glutAddMenuEntry(FD->exclude&1?"Show right pointing":"Hide right pointing", 2);
        glutAddMenuEntry(FD->exclude&2?"Show left pointing":"Hide left pointing", 3);
    }
    return menuID;
}


//------------------------------------------------------------------------------
void processMenuCoupleSelect(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  disp.couple_select  = 0;   break;
        case 2:  disp.couple_select ^= 1;   break;
        case 3:  disp.couple_select ^= 2;   break;
        case 4:  disp.couple_select ^= 4;   break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}

int buildMenuCoupleSelect()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuCoupleSelect);
    else
        glApp::clearMenu(menuID);
    
    glutAddMenuEntry("Hide All", 1);
    glutAddMenuEntry(disp.couple_select&1?"Hide Free":"Show Free",     2);
    glutAddMenuEntry(disp.couple_select&2?"Hide Bound":"Show Bound",   3);
    glutAddMenuEntry(disp.couple_select&4?"Hide Links":"Show Links", 4);
    return menuID;
}

//------------------------------------------------------------------------------
void processMenuSingleSelect(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  disp.single_select  = 0;   break;
        case 2:  disp.single_select ^= 1;   break;
        case 3:  disp.single_select ^= 2;   break;
        
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}

int buildMenuSingleSelect()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuSingleSelect);
    else
        glApp::clearMenu(menuID);
    
    glutAddMenuEntry("Hide All",     1);
    glutAddMenuEntry(disp.single_select&1?"Hide Free":"Show Free",     2);
    glutAddMenuEntry(disp.single_select&2?"Hide Bound":"Show Bounds", 3);
    return menuID;
}

int buildSubMenu8()
{
    static int menuID = 0;
    if ( menuID == 0 ) {
        menuID = glutCreateMenu(processMenuSingleSelect);
        glutAddMenuEntry("-", 0);
    }
    return menuID;
}


//------------------------------------------------------------------------------

int buildMenuSelect()
{
    static int menuID = 0;
    int m1 = buildMenuFiberSelect();
    int m2 = buildMenuCoupleSelect();
    int m3 = buildMenuSingleSelect();
    
    if ( menuID == 0 )
        menuID = glutCreateMenu(nullptr);
    else
        glApp::clearMenu(menuID);

    glutAddSubMenu("Fibers",  m1);
    glutAddSubMenu("Couple",  m2);
    glutAddSubMenu("Singles", m3);
    
    return menuID;
}


//------------------------------------------------------------------------------
#pragma mark -

void processMenuAnimation(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  processKey('z');   break;
        case 2:  processKey('a');   break;
        case 4:  processKey('s');   break;
        case 5:  processKey('r');   break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}

int buildMenuAnimation()
{
    static int menuID = 0;
    
    if ( menuID == 0 )
    {
        menuID = glutCreateMenu(processMenuAnimation);
        glutAddMenuEntry("(z) Reset State",      1);
        glutAddMenuEntry("(a) Start Live",       2);
        glutAddMenuEntry("(s) One Step & Stop",  4);
        glutAddMenuEntry("(r) Read Parameters",  5);
    }
    return menuID;
}


//------------------------------------------------------------------------------

void processMenuReplay(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  processKey('p');  break;
        case 2:  processKey('o');  break;
        case 3:  processKey('s');  break;
        case 4:  processKey('z');  break;
        case 5:  player.previousFrame();  break;
        case 6:  player.nextFrame();      break;
        case 7:  prop.loop = 0;      break;
        case 8:  prop.loop = 1;      break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}

int buildMenuReplay()
{
    static int menuID = 0;
    
    if ( menuID == 0 )
    {
        menuID = glutCreateMenu(processMenuReplay);
        glutAddMenuEntry("(p) Play/Faster",       1);
        glutAddMenuEntry("(o) Slower",            2);
        glutAddMenuEntry("(s) Stop",              3);
        glutAddMenuEntry("-",                     0);
        glutAddMenuEntry("(z) First Frame",       4);
        glutAddMenuEntry("(<) Previous Frame",    5);
        glutAddMenuEntry("(>) Next Frame",        6);
        if ( prop.loop )
            glutAddMenuEntry("Do not loop", 7);
        else
            glutAddMenuEntry("Loop movie", 8);
    }
    return menuID;
}


//------------------------------------------------------------------------------
void processMenuExport(int item)
{
    switch (item)
    {
        case 0: return;
        case 1: player.saveView("image", prop.image_index++, 1); return;
        case 2: player.saveScene(2, "image", prop.image_index++, 2); return;
        case 3: player.saveScene(3, "image", prop.image_index++, 3); return;
        case 4: player.saveScene(6, "image", prop.image_index++, 3); return;
        case 5: player.saveScene(9, "image", prop.image_index++, 3); return;
        case 6: player.saveScene(4, "poster", prop.poster_index++); return;
        case 7: player.saveScene(8, "poster", prop.poster_index++); return;
        case 8: player.saveScene(16, "poster", prop.poster_index++); return;

        case 9:  prop.save_images = 1; player.startPlayback();     return;
        case 10: prop.image_index = 0;                             return;
        
        case 20: player.writePlayParameters(std::cout, true);    return;
        case 21: player.writeDisplayParameters(std::cout, true); return;
        case 22: thread.writeProperties(std::cout, true);        return;
        case 23: thread.exportObjects(false);                    return;
        case 24: thread.exportObjects(true);                     return;
            
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}


int buildMenuExport()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuExport);
    else
        glApp::clearMenu(menuID);
    
    glutAddMenuEntry("Save Image (y)",            1);
    glutAddMenuEntry("Save Fine Image",           2);
    glutAddMenuEntry("Save 2x Fine Image",        3);
    glutAddMenuEntry("Save 3x Fine Image",        4);
    glutAddMenuEntry("Save 4x Poster",            5);
    glutAddMenuEntry("Save 8x Poster",            6);
    glutAddMenuEntry("Play & Save Images (Y)",    9);
    glutAddMenuEntry("Reset Image-file Index",   10);
    glutAddMenuEntry("-",                         0);
    glutAddMenuEntry("Write Play Parameters",    20);
    glutAddMenuEntry("Write Display Parameters", 21);
    glutAddMenuEntry("Write Object Properties",  22);
    glutAddMenuEntry("Export Objects",           23);
    glutAddMenuEntry("Export Objects as Binary", 24);
    
    return menuID;
}

//------------------------------------------------------------------------------
//                    MAIN MENU
//------------------------------------------------------------------------------
#pragma mark -

void processTopMenu(int item)
{
    if ( item == 9 )
        exit(EXIT_SUCCESS);
}


void rebuildMenus()
{
    static int menuID = 0;
    int m1 = buildMenuDisplay();
    int m2 = buildMenuSelect();
    int m3 = buildMenuAnimation();
    int m4 = buildMenuReplay();
    int m5 = buildMenuExport();
    int m6 = glApp::buildMenu();

    if ( menuID == 0 )
        menuID = glutCreateMenu(processTopMenu);
    else
        glApp::clearMenu(menuID);
    
    glutAddSubMenu("Display",           m1);
    glutAddSubMenu("Object-Selection",  m2);
    glutAddSubMenu("Live-Simulation",   m3);
    glutAddSubMenu("File-Replay",       m4);
    glutAddSubMenu("Export",            m5);
    glutAddSubMenu("More",              m6);
    glutAddMenuEntry("Quit",             9);
}


void menuCallback(int status, int x, int y)
{
    //printf("menu status(%i, %i, %i)\n", status, x, y);
    
    if ( status == GLUT_MENU_NOT_IN_USE )
        rebuildMenus();
}

