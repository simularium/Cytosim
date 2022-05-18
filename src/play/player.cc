// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "player.h"
#include "opengl.h"
#include "gle.h"
#include "glapp.h"

using namespace gle;
using glApp::flashText;

#include "fiber_disp.h"
#include "line_disp.h"
#include "point_disp.h"
#include "player_disp.cc"


Player::Player()
: disp("*"), prop("*"), thread(simul, glApp::postRedisplay), mDisplay(nullptr)
{
}

Player::~Player()
{
    clear();
}

void Player::clear()
{
    thread.stop();
    thread.clear();
    dproperties.erase();
    if ( mDisplay )
        delete(mDisplay);
    mDisplay = nullptr;
}

//------------------------------------------------------------------------------
#pragma mark - I/O


void Player::previousFrame()
{
    if ( thread.currentFrame() > 0 )
        thread.loadFrame(thread.currentFrame()-1);
    else {
        if ( prop.loop )
            thread.loadLastFrame();
        else
            stop();
    }
}

/**
 Reads the next frame from the current file position.
 */
void Player::nextFrame()
{    
    try
    {
        if ( thread.loadNextFrame() )
        {
            if ( prop.exit_at_eof )
                exit(EXIT_SUCCESS);
            if ( prop.loop )
                thread.loadFrame(0);
            else
            {
                flashText("end-of-file\n");
                stop();
            }
        }
    }
    catch( Exception & e )
    {
        flashText("Error:\n %s", e.msg());
        if ( thread.eof() )
            stop();
    }
}

//------------------------------------------------------------------------------
#pragma mark - Commands


void Player::rewind()
{
    if ( thread.goodFile() )
    {
        stop();
        thread.rewind();
        thread.loadFrame(0);
        glApp::postRedisplay();
    }
}


bool Player::startPlayback()
{
    if ( thread.goodFile()  &&  prop.play != 1  && !goLive )
    {
        //rewind file if its end was reached:
        if ( thread.eof() )
            thread.rewind();
        prop.play = 1;
        return true;
    }
    return false;
}


bool Player::startBackward()
{
    if ( prop.play != -1 )
    {
        if ( thread.currentFrame() == 0 )
            thread.loadLastFrame();
        else
            flashText("Play reverse");
        prop.play = -1;
        return true;
    }
    return false;
}


void Player::accelerate()
{
    prop.delay /= 2;
    //the delay should be compatible with graphic refresh rates:
    const unsigned int min_delay = 1;
    if ( prop.delay < min_delay )
    {
        prop.delay = min_delay;
        if ( goLive )
            flashText("Delay is %i ms! use 'A' to jump frames", prop.delay);
        else
            flashText("Delay is %i ms!", prop.delay);
    }
    else {
        flashText("Delay %i ms", prop.delay);
    }
}


void Player::stop()
{
    goLive = 0;
    prop.play = 0;
    prop.save_images = 0;
}


void Player::startstop()
{
    if ( thread.alive() )
        goLive = !goLive;
    else if ( thread.goodFile() )
    {
        if ( !prop.play )
            startPlayback();
        else
            stop();
    }
}


void Player::extendLive()
{
    if ( 0 == thread.extend() )
        flashText("Extend simulation...");
    goLive = 1;
}


void Player::restart()
{
    try
    {
        thread.stop();
        thread.clear();
        dproperties.erase();
        thread.start();
    }
    catch( Exception & e ) {
        flashText("Error: %s", e.msg());
    }
}


//------------------------------------------------------------------------------
#pragma mark - Display selection routines

inline FiberDisp* toFiberDisp(Property * ptr)
{
    return static_cast<FiberDisp*>(ptr);
}

inline PointDisp* toPointDisp(Property * ptr)
{
    return static_cast<PointDisp*>(ptr);
}


PropertyList Player::allFiberDisp()
{
    return dproperties.find_all("fiber:display");
}

PropertyList Player::allVisibleFiberDisp()
{
    PropertyList res, plist = dproperties.find_all("fiber:display");
    
    for ( Property * i : plist )
    {
        if ( toFiberDisp(i)->visible )
            res.push_back(i);
    }
    return res;
}

PropertyList Player::allHandDisp()
{
    return dproperties.find_all("hand:display");
}

PropertyList Player::allVisibleHandDisp()
{
    PropertyList res, plist = dproperties.find_all("hand:display");
    
    for ( Property * i : plist )
    {
        if ( toPointDisp(i)->visible )
            res.push_back(i);
    }
    return res;
}

PropertyList Player::allSphereDisp()
{
    return dproperties.find_all("bead:display", "solid:display", "sphere:display");
}

PropertyList Player::allSpaceDisp()
{
    return dproperties.find_all("space:display");
}

FiberDisp * Player::firstFiberDisp()
{
    PropertyList plist = allVisibleFiberDisp();
    if ( plist.size() )
        return toFiberDisp(plist.front());
    return nullptr;
}

/**
 Write global parameters that control the display:
 - GlappProp
 - DisplayProp
 - PlayerProp
 .
 */
void Player::writePlayParameters(std::ostream& os, bool prune) const
{
    os << "set " << simul.prop->name() << " display\n{\n";
    if ( glApp::views.size() > 0 )
    {
        View& view = glApp::currentView();
        view.write_values_diff(os, prune);
    }
    disp.write_values_diff(os, prune);
    //output parameters for the main view:
    prop.write_values_diff(os, prune);
    os << "}\n";
}

/**
 Write all the parameters that control the display:
 - GlappProp
 - DisplayProp
 - PlayerProp
 - ObjectDisp
 .
 */
void Player::writeDisplayParameters(std::ostream& os, bool prune) const
{
    dproperties.write(os, prune);
}
