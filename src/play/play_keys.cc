// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 It is possible to save images or many images by pressing a key..
 disabling this capacity in public distribution might be safer:
 It can be done by disabling ENABLE_WRITE below:
 */
#define ENABLE_WRITE 1


/// defines the amount added/subtracted to size/width when a key is pressed
template< typename FLOAT >
FLOAT grained(FLOAT x, int inc)
{
    const FLOAT grain = (FLOAT)0.25;
    FLOAT dx = inc * ( 1 + ( x >= 4 ) + 2 * ( x >= 16 ) + 4 * ( x >= 32 ) );
    FLOAT nx = grain * std::round( x / grain + dx );
    return std::max(abs(inc)*grain, nx);
}

//------------------------------------------------------------------------------

template< typename T >
void setVisible(T* p, int val)
{
    p->visible = val;
    flashText("%s:visible = %i", p->name_str(), val);
}

template< typename T >
void flipVisible(T* p)
{
    p->visible = !p->visible;
    flashText("%s:visible = %i", p->name_str(), p->visible);
}

void changeStyle(PointDisp * p, int)
{
    p->style = ( p->style + 1 ) % 8;
    flashText("%s:style = %i", p->name_str(), p->style);
}

void setSize(PointDisp * p, GLfloat s)
{
    if ( s >= 0.5 )
    {
        p->size = s;
        flashText("%s:size = %.2f", p->name_str(), s);
    }
}

void setWidth(PointDisp * p, GLfloat s)
{
    if ( s > 0.5 )
    {
        p->width = s;
        flashText("%s:width = %.2f", p->name_str(), s);
    }
}

void scaleSize(PointDisp * p, int s)
{
    if ( s > 0 )
    {
        p->size *= s;
        if ( p->size > 16 )
            p->size = 0.5;
        p->width = p->size / 2;
        flashText("%s:size = %.2f", p->name_str(), p->size);
    }
}

//------------------------------------------------------------------------------
#pragma mark - PointDisp lists

inline PointDisp* toPointDisp(Property * ptr)
{
    return static_cast<PointDisp*>(ptr);
}


/// apply function to all PointDisp is plist
void setPointDisp(PropertyList const& plist, void(*func)(PointDisp*, int), int val)
{
    if ( plist.empty() )
        flashText("no relevant object");

    for ( Property * i : plist )
        if ( toPointDisp(i)->visible )
            func(toPointDisp(i), val);
}


PointDisp * nextPointDisp(PropertyList const& plist, int& cnt)
{
    PointDisp* one = nullptr;
    cnt = 0;
    // find first one which is visible:
    for ( PropertyList::const_iterator i = plist.begin(); i < plist.end(); ++i )
    {
        PointDisp * dsp = toPointDisp(*i);
        if ( dsp->visible )
        {
            ++cnt;
            // choose follower:
            if ( i+1 < plist.end() )
                one = toPointDisp(*(i+1));
            else
                one = nullptr;
        }
    }
    return one;
}


void changePointDispSize(PropertyList const& plist, int inc,
                         bool dos, bool dow)
{
    for ( Property * i : plist )
    {
        PointDisp * p = toPointDisp(i);
        if ( dos ) p->size = grained(p->size, inc*2);
        if ( dow ) p->width = grained(p->width, inc);
    }
    
    if ( disp.style == 2 || plist.size() > 1 )
    {
        if ( dow ) {
            disp.link_width = grained(disp.link_width, inc);
            flashText("simul:link_width %.2f", disp.link_width);
        }
        if ( dos ) {
            disp.point_size = grained(disp.point_size, inc*2);
            flashText("simul:point_size %.2f", disp.point_size);
        }
    }
    else if ( plist.size() == 1 )
    {
        PointDisp * p = toPointDisp(plist.front());
        if ( dow ) flashText((p->name()+":width %.2f").c_str(), p->width);
        if ( dos ) flashText((p->name()+":size %.2f").c_str(), p->size);
    }
}


void setPointDispVisible(PropertyList const& plist, int val)
{
    if ( plist.empty() )
        flashText("no relevant object");
    
    for ( Property * i : plist )
        toPointDisp(i)->visible = val;
}


void shufflePointDispVisible(const PropertyList& plist)
{
    if ( plist.empty() )
        flashText("no relevant object");

    if ( plist.size() == 1 )
    {
        flipVisible(toPointDisp(plist.front()));
    }
    else
    {
        int cnt = 0;
        PointDisp * p = nextPointDisp(plist, cnt);
        
        if ( cnt > 1 )
            p = toPointDisp(plist.front());
        
        if ( p )
        {
            setPointDispVisible(plist, 0);
            p->visible = 1;
            flashText("Only `%s' is visible", p->name_str());
        }
        else
        {
            setPointDispVisible(plist, 1);
            flashText("All are visible");
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark - Single Couple


void changeSingleSelect()
{
    unsigned int & select = disp.single_select;
    switch( select )
    {
        case 3:  select = 0; flashText("single:select=0: hidden");      break;
        case 0:  select = 2; flashText("single:select=2: bound only");  break;
        case 2:  select = 1; flashText("single:select=1: free only");   break;
        default: select = 3; flashText("single:select=3: all");         break;
    }
}


void changeCoupleSelect()
{
    unsigned int & select = disp.couple_select;
    switch( select )
    {
        case 7:  select = 0; flashText("couple:select=0: hidden");        break;
        case 0:  select = 2; flashText("couple:select=2: bound only");    break;
        case 2:  select = 4; flashText("couple:select=4: bridging only"); break;
        case 4:  select = 1; flashText("couple:select=1: free only");     break;
        default: select = 7; flashText("couple:select=7: all");           break;
    }
}

void changeCoupleSelect2()
{
    unsigned int & select = disp.couple_select;
    if ( select & 8 )
    {
        select = 16+4;
        flashText("couple: parallel bridging only");
    }
    else if ( select & 16 )
    {
        select = 7;
        flashText("couple: all");
    }
    else
    {
        select = 8+4;
        flashText("couple: antiparallel bridging only");
    }
}

//---------------------------------------------------------------------
#pragma mark - Fibers


void changeExclude(FiberDisp* p, int val)
{
    if ( val )
        p->exclude >>= 2;
    p->exclude = ( p->exclude + 1 ) % 4;
    if ( val )
        p->exclude <<= 2;
    
    switch ( p->exclude )
    {
        case 0:  flashText("All fibers");                break;
        case 1:  flashText("Right-pointing fibers");     break;
        case 2:  flashText("Left-pointing fibers");      break;
        case 3:  flashText("No fibers");                 break;
        case 4:  flashText("Counter-clockwise fibers");  break;
        case 8:  flashText("Clockwise fibers");          break;
        case 12: flashText("No fibers");                 break;
    }
}


void changeScale(real& scale, int d)
{
    int s = (int) round( log2(scale) + d );
    if ( s < -10 ) s =  10;
    if ( s >  10 ) s = -10;
    scale = exp2(s);
}


void flipExplode(FiberDisp* p)
{
    p->explode = ! p->explode;
    if ( p->explode && p->explode_range == 0 )
        p->explode_range = 1.0;
    flashText("fiber:explode = %i", p->explode);
}


void changeScale(FiberDisp* p, int d)
{
    if ( p->lattice_style )
    {
        changeScale(p->lattice_scale, d);
        flashText("fiber:lattice_scale = %.3f", p->lattice_scale);
    }
    else if ( p->line_style == 2 )
    {
        changeScale(p->tension_scale, d);
        flashText("fiber:tension_scale = %.3f", p->tension_scale);
    }
    else if ( p->force_scale > 0 )
    {
        changeScale(p->force_scale, d);
        flashText("fiber:force_scale = %.3f", p->force_scale);
    }
    else if ( disp.style == 2 )
        flipExplode(p);
}


void changeColoring(FiberDisp* p, int)
{
    p->coloring = ( p->coloring + 1 ) % 7;
    switch( p->coloring )
    {
        case FiberDisp::COLORING_OFF:       flashText("Fibers: no coloring");           break;
        case FiberDisp::COLORING_RANDOM:    flashText("Fibers: coloring");              break;
        case FiberDisp::COLORING_DIRECTION: flashText("Fibers: coloring by direction"); break;
        case FiberDisp::COLORING_MARK:      flashText("Fibers: coloring by mark");      break;
        case FiberDisp::COLORING_FLAG:      flashText("Fibers: coloring by flag");      break;
        case FiberDisp::COLORING_CLUSTER:   flashText("Fibers: coloring by cluster");   break;
        case FiberDisp::COLORING_AGE:       flashText("Fibers: coloring by age");       break;
    }
    // the coloring will only apply if 'line_style==1', so make it visible now:
    if ( p->coloring )
        p->line_style = 1;
}


void setMask(FiberDisp* p, int val)
{
    p->mask = val;
    p->mask_bitfield = RNG.distributed_bits(p->mask);
    flashText("fiber:mask_bitfield=0x%X (%i bits)", p->mask_bitfield, p->mask);
}

void changeMask(FiberDisp* p, int val)
{
    p->mask = ( p->mask + val ) % 11;
    p->mask_bitfield = RNG.distributed_bits(p->mask);
    flashText("fiber:mask_bitfield=0x%X (%i bits)", p->mask_bitfield, p->mask);
}

void changePointStyle(FiberDisp* p, int)
{
    p->point_style = ( p->point_style + 1 ) % 3;
    switch ( p->point_style )
    {
        case 0:  flashText("Fibers: no points");     break;
        case 1:  flashText("Fibers: vertices");      break;
        case 2:  flashText("Fibers: arrowheads");    break;
        case 3:  flashText("Fibers: center point");  break;
    }
}


void changeLineStyle(FiberDisp* p, int)
{
    p->line_style = ( p->line_style + 1 ) % 5;
    switch ( p->line_style )
    {
        case 0:  flashText("Fibers: no lines");       break;
        case 1:  flashText("Fibers: lines");          break;
        case 2:  flashText("Fibers: axial tensions"); break;
        case 3:  flashText("Fibers: curvature");      break;
        case 4:  flashText("Fibers: orientation");    break;
    }
}


void changeSpeckleStyle(FiberDisp* p, int)
{
    p->speckle_style = ( p->speckle_style + 1 ) % 3;
    switch ( p->speckle_style )
    {
        case 0:  flashText("Fibers: no speckles");       break;
        case 1:  flashText("Fibers: random speckles");   break;
        case 2:  flashText("Fibers: regular speckles");  break;
    }
}


void changeLatticeStyle(FiberDisp* p, int)
{
    p->lattice_style = ( 1 + p->lattice_style ) % 4;
    flashText("Fibers: lattice_style=%i", p->lattice_style);
}


void changePointSize(FiberDisp* p, int inc)
{
    GLfloat s = grained(p->point_size, inc);
    
    if ( s > 0 )
    {
        p->point_size = s;
        flashText("%s:point_size=%0.2f", p->name_str(), s);
    }
}

void changeSize(FiberDisp* p, int inc)
{
    GLfloat s = grained(p->line_width, inc);
    
    if ( s > 0 )
    {
        real w = p->line_width;
        p->line_width   = s;
        p->point_size  *= s / w;
        p->end_size[0] *= s / w;
        p->end_size[1] *= s / w;
        flashText("Fibers: line_width %0.2f", s);
    }
}


void changeEndStyle(FiberDisp* p, int)
{
    int * style = p->end_style;
    // showing the plus ends -> the minus ends -> both -> none
    switch( bool(style[1]) + 2*bool(style[0]) )
    {
        case 0:
            style[0] = 2;
            style[1] = 0;
            break;
        case 1:
            style[0] = 0;
            style[1] = 0;
            break;
        case 2:
            style[0] = 2;
            style[1] = 1;
            break;
        case 3:
        default:
            style[0] = 0;
            style[1] = 1;
            break;
    }
    
    switch( (style[0]?1:0) + (style[1]?2:0) )
    {
        case 0: flashText("Fibers: no ends");    break;
        case 1: flashText("Fibers: plus-ends");  break;
        case 2: flashText("Fibers: minus-ends"); break;
        case 3: flashText("Fibers: both ends");  break;
    }
}


void changeEndSize(FiberDisp* p, int inc)
{
    const real MIN = 1.0;
    real* size = p->end_size;
    real s0 = std::max(MIN, size[0] + inc);
    real s1 = std::max(MIN, size[1] + inc);
    if ( p->end_style[0] )
    {
        size[0] = s0;
        if ( p->end_style[1] )
        {
            size[1] = s1;
            flashText((p->name()+":end_size %.2f %.2f").c_str(), s0, s1);
        }
        else
            flashText((p->name()+":plus_end %.2f").c_str(), s0);
    } else if ( p->end_style[1] )
    {
        size[1] = s1;
        flashText((p->name()+":minus_end %.2f").c_str(), s1);
    }
}

//---------------------------------------------------------------------
#pragma mark - FiberDisp lists

inline FiberDisp* toFiberDisp(Property * ptr)
{
    return static_cast<FiberDisp*>(ptr);
}

void setFiberDisp(PropertyList const& plist, void(*func)(FiberDisp*, int), int val)
{
    for ( Property * i : plist )
        if ( toFiberDisp(i)->visible )
            func(toFiberDisp(i), val);
}

PointDisp * findVisibleFiberDisp(PropertyList const& plist, int& cnt)
{
    PointDisp * one = nullptr;
    cnt = 0;
    // find first one which is visible:
    for ( Property * i : plist )
    {
        if ( toFiberDisp(i)->visible )
        {
            ++cnt;
            if ( !one )
                one = toPointDisp(i);
        }
    }
    return one;
}


FiberDisp * nextVisibleFiberDisp(PropertyList const& plist, int& cnt)
{
    FiberDisp* one = nullptr;
    cnt = 0;
    // find first one which is visible:
    for ( PropertyList::const_iterator i = plist.begin(); i < plist.end(); ++i )
    {
        FiberDisp * dsp = toFiberDisp(*i);
        if ( dsp->visible )
        {
            ++cnt;
            // choose follower:
            if ( i+1 < plist.end() )
                one = toFiberDisp(*(i+1));
            else
                one = nullptr;
        }
    }
    return one;
}


void setFiberDispVisible(PropertyList const& plist, int val)
{
    for ( Property * i : plist )
        toFiberDisp(i)->visible = val;
}


void shuffleFiberDispVisible(const PropertyList& plist)
{
    if ( plist.size() == 1 )
    {
        flipVisible(toFiberDisp(plist.front()));
    }
    else
    {
        int cnt = 0;
        FiberDisp * p = nextVisibleFiberDisp(plist, cnt);
        
        if ( cnt > 1 )
            p = toFiberDisp(plist.front());
        
        if ( p )
        {
            setFiberDispVisible(plist, 0);
            p->visible = 1;
            flashText("Only `%s' is visible", p->name_str());
        }
        else
        {
            setFiberDispVisible(plist, 1);
            flashText("All fibers are visible");
        }
    }
}

//------------------------------------------------------------------------------
//---------------------------- keyboard commands -------------------------------
//------------------------------------------------------------------------------
#pragma mark - Keyboard Commands

/// provide minimal on-screen summary of the most important key combinations
void helpKeys(std::ostream& os)
{
    os << "                          Keyboard Commands\n";
    os << "\n";
    os << "   SPACE       Start-stop animation or replay\n";
    os << "   < >         Show previous; show next frame ( , . also works)\n";
    os << "   O s o p     Play reverse; stop; play slower; play faster\n";
    os << "   z           Rewind to first frame / Restart live simulation\n";
    os << "   ALT-SPACE   Reset view (i.e. zoom, translation, rotation)\n";
    os << "   f F         Toggle full-screen mode; maximize window size\n";
    os << "   i v b       Invert colors; toggle slice view; toggle scale bar\n";
    os << "   l L         Read parameter file; Print display parameters\n";
    os << "   r R         Report various informations on display window\n";
#if ENABLE_WRITE
    os << "   y Y         Save current image; Play and save all images\n";
#endif
    os << "\nSimulation\n";
    os << "   a s         Start live mode; Perform one simulation step and stop\n";
    os << "   A a         Double period (num. steps per display); reset period\n";
    os << "   g G         Delete all mouse-controlled handles; release handle\n";
    os << "\nFibers\n";
    os << "   `           Address another type of fibers for modifications\n";
    os << "   1           Change display: line / color-coded tension / hide\n";
    os << "   2 3         Decrease; increase line width (ALT: point size)\n";
    os << "   !           Change display of tips: off / plus / both / minus\n";
    os << "   @ #         Decrease; increase fiber_end display size\n";
    os << "   4 $         Change speckle display; change lattice display\n";
    os << "   c d         Toggle fiber coloring; hide Right/left-pointing\n";
    os << "   m M         Mask a fraction of the fibers; change mask value\n";
    os << "   w e         decrease/increase tension/lattice/explode scale\n";
    os << "   t T         Toggle auto-tracking: 't':nematic; 'T':polar mode\n";
    os << "\nBeads - Solids - Spheres\n";
    os << "   5           Toggle between different bead/sphere display style\n";
    os << "   %           Change point size\n";
    os << "\nSingles - Couples\n";
    os << "   6           Change the Single selection mode\n";
    os << "   7 ALT-7     Change the Couple selection mode;\n";
    os << "   0           Toggle the visibility flags of Hands\n";
    os << "   8 9         Decrease; Increase point size of visible Hands\n";
    os << "   ALT-8 ALT-9 Decrease; Increase line width of visible Hands\n";
}


void processKey(unsigned char key, int modifiers = 0)
{
    //std::cerr << "processing key `" << key << "'\n";
    // the view associated with the current window
    View & view = glApp::currentView();
    
    const bool altKeyDown = modifiers & GLUT_ACTIVE_ALT;
    const bool shiftKeyDown = modifiers & GLUT_ACTIVE_SHIFT;
    /*
     In the switch below:
     - use break if the display need to be refreshed,
     - otherwise, use return.
    */
    switch (key)
    {
        case 'h':
        {
            view.draw_memo = ( view.draw_memo + 1 ) % 6;
            view.memo = player.buildMemo(view.draw_memo);
        } break;
        
#if ENABLE_WRITE
            
        case 'y':
            // save current image, without decorations
            player.displayCytosim();
            glFinish();
            player.saveView("image", prop.image_index++, 1);
            // with over sampling and downsampling to get super-resolution:
            //player.saveViewMagnified(3, "image", prop.image_index++, 3);
            return;
            
        case 'Y':
            // start player to save all images in file
            if ( prop.save_images == 0 )
            {
                if ( player.startPlayback() )
                    prop.save_images = 1;
            }
            else
            {
                prop.save_images = 0;
            }
            break;

#endif

        //------------------------- Global controls ----------------------------
        
        case 'l': {
            try {
                std::string file = simul.prop->config_file;
                thread.reloadParameters(file);
                flashText("Reloaded %s", file.c_str());
            }
            catch( Exception & e ) {
                flashText("Error in config: %s", e.msg());
            }
        } break;

        case 'L':
        {
            if ( altKeyDown )
                thread.writeProperties(std::cout, true);
            else
            {
                player.writePlayParameters(std::cout, true);
                player.writeDisplayParameters(std::cout, true);
            }
        } break;
            
        case 'z':
            if ( thread.goodFile() )
                player.rewind();
            else
                player.restart();
            break;
            
        case 'Z':
            thread.cancel();
            player.stop();
            break;
            
        case 'a':
            if ( altKeyDown )
            {
                player.setStyle(1);
                flashText("Style 1");
            }
            else
            {
                player.extendLive();
                prop.period = 1;
                thread.period(prop.period);
                flashText("period = 1");
            }
            break;
            
        case 'A':
            prop.period = 2 * prop.period;
            if ( prop.period > 1024 ) prop.period = 1;
            thread.period(prop.period);
            flashText("period = %i", prop.period);
            break;
            
        case 's':
            if ( altKeyDown )
            {
                player.setStyle(2);
                flashText("Style 2");
            }
            else
            {
                thread.step();
                player.stop();
            }
            break;
            
        case 'S':
            prop.period = 1;
            thread.period(prop.period);
            flashText("period = 1");
            break;
            
        case 'g':
            thread.deleteHandles();
            flashText("Deleted mouse-controled handles");
            break;
            
        case 'G':
            thread.releaseHandle();
            break;
            
        case 'i': {
            ViewProp& vp = glApp::currentView();
            vp.back_color = vp.back_color.inverted();
            vp.front_color = vp.front_color.inverted();
        } break;

        case 'r':
            prop.toggleReport(0);
            break;
            
        case 'R':
            prop.toggleReport(1);
            break;

        //------------------------- play / stop / reverse ----------------------
            
        case '<':
        case ',':
            if ( prop.play == 1 )
                player.stop();
            else
                player.previousFrame();
            break;
            
        case '>':
        case '.':
            if ( prop.play == -1 )
                player.stop();
            else
                player.nextFrame();
            break;
            
        case 'o':
            if ( prop.delay < 1 << 13 )
                prop.delay *= 2;
            flashText("Delay %i ms", prop.delay);
            return;
            
        case 'O':
            if ( !player.startBackward() )
                player.accelerate();
            return;
            
        case 'p':
            if ( !player.startPlayback() )
                player.accelerate();
            return;
        
        case ' ':
            if ( altKeyDown )
            {
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                view.reset();
                flashText("");
            }
            else
                player.startstop();
            return;
            
        //------------------------------ Fibers --------------------------------
           
        case '`':
            shuffleFiberDispVisible(player.allFiberDisp());
            break;
            
        case 't':
            view.track_fibers ^= 1;
            flashText("view.track_fibers = %i (translation)", view.track_fibers);
            break;
            
        case 'T':
            view.track_fibers ^= 3;
            flashText("view.track_fibers = %i (rotation)", view.track_fibers);
            break;
            
        case 'd':
            if ( altKeyDown )
            {
                player.setStyle(3);
                flashText("Style 3");
            }
            else
            {
                setFiberDisp(player.allVisibleFiberDisp(), changeExclude, 0);
            }
            break;
            
        case 'D':
            setFiberDisp(player.allVisibleFiberDisp(), changeExclude, 1);
            break;
            
        case 'w':
            setFiberDisp(player.allVisibleFiberDisp(), changeScale, -1);
            break;

        case 'e':
            setFiberDisp(player.allVisibleFiberDisp(), changeScale, 1);
            break;
            
        case 'm':
            if ( altKeyDown )
                setFiberDisp(player.allVisibleFiberDisp(), setMask, 0);
            else
                setFiberDisp(player.allVisibleFiberDisp(), changeMask, 1);
            break;

        case 'M':
            setFiberDisp(player.allVisibleFiberDisp(), changeMask, 0);
            break;
            
        case 'c':
            setFiberDisp(player.allVisibleFiberDisp(), changeColoring, 0);
            break;
            
        case '1':
            if ( altKeyDown )
                setFiberDisp(player.allVisibleFiberDisp(), changePointStyle, 0);
            else
                setFiberDisp(player.allVisibleFiberDisp(), changeLineStyle, 0);
            break;
            
        case '!':
            setFiberDisp(player.allVisibleFiberDisp(), changeEndStyle, 0);
            break;
            
        case '2':
            if ( altKeyDown)
                setFiberDisp(player.allVisibleFiberDisp(), changePointSize, -2);
            else
                setFiberDisp(player.allVisibleFiberDisp(), changeSize, -2);
            break;

        case '@':
            setFiberDisp(player.allVisibleFiberDisp(), changeEndSize, -2);
            break;

        case '3':
            if ( altKeyDown)
                setFiberDisp(player.allVisibleFiberDisp(), changePointSize, 2);
            else
                setFiberDisp(player.allVisibleFiberDisp(), changeSize, 2);
            break;
            
        case '#':
            setFiberDisp(player.allVisibleFiberDisp(), changeEndSize, 2);
            break;
            
        case '4':
            if ( altKeyDown )
                setFiberDisp(player.allVisibleFiberDisp(), changePointStyle, 0);
            else
                setFiberDisp(player.allVisibleFiberDisp(), changeSpeckleStyle, 0);
            break;
            
        case '$':
            setFiberDisp(player.allVisibleFiberDisp(), changeLatticeStyle, 0);
            break;

        //------------------------ Solid / Sphere ------------------------------
  
        case '5':
            if ( altKeyDown )
                shufflePointDispVisible(player.allSphereDisp());
            else
                setPointDisp(player.allSphereDisp(), changeStyle, 0);
            break;
        
        case '%':
            setPointDisp(player.allSphereDisp(), scaleSize, 2);
            break;
            
        //------------------------ Single/Couple + Hands -----------------------
           
        case '6':
            if ( altKeyDown )
            {
                disp.draw_links = !disp.draw_links;
                flashText("draw_links = %i", disp.draw_links);
            }
            else
                changeSingleSelect();
            break;
            
        case '&': case '^':
            shufflePointDispVisible(player.allSpaceDisp());
            break;

        case '7':
            if ( altKeyDown )
                changeCoupleSelect2();
            else
                changeCoupleSelect();
            break;
            
        case '8':
            changePointDispSize(player.allVisibleHandDisp(), -1, !altKeyDown, !shiftKeyDown);
            break;
            
        case '9':
            changePointDispSize(player.allVisibleHandDisp(), +1, !altKeyDown, !shiftKeyDown);
            break;

        case '0':
            if ( altKeyDown )
                setPointDispVisible(player.allHandDisp(), 1);
            else
                shufflePointDispVisible(player.allHandDisp());
            break;
        
#if 0
        case 185: //that is the key left of '=' on the numpad
        case '=':
            break;
        case '-':
            break;
        case '+':
            break;
#endif

        default:
            // other keys are handles by glApp
            glApp::processNormalKey(key, 0, 0);
            return;
    }
    
    // if break was called, redraw the scene:
    glApp::postRedisplay();
}


void processNormalKey(const unsigned char key, const int x, const int y)
{
    // check for user-defined `magic_key`
    for ( int k = 0; k < PlayerProp::NB_MAGIC_KEYS; ++k )
    {
        if ( key == prop.magic_key[k] )
        {
            thread.execute(prop.magic_code[k]);
            glApp::postRedisplay();
            return;
        }
    }
    
    processKey(key, glutGetModifiers());
}
