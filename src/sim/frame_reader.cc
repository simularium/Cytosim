// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "frame_reader.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


// Use the second definition to get some verbose reports:
#define VLOG(ARG) ((void) 0)
//#define VLOG(ARG) std::clog << ARG;

//------------------------------------------------------------------------------

FrameReader::FrameReader() : inputter(DIM)
{
    frameIndex = 0;
    lastLoaded = ~0;
}


void FrameReader::clear()
{
    inputter.rewind();
    clearPositions();
}


void FrameReader::openFile(std::string const& file)
{
    int error = inputter.open(file.c_str(), "rb");
    
    if ( error )
    {
        //file was not found, we try 'gunzip'
        std::string tmp = file + ".gz";
        FILE* fp = fopen(tmp.c_str(), "r");
        if ( fp )
        {
            fclose(fp);
            tmp = "gunzip " + tmp;
            std::clog << tmp << std::endl;
            
            if ( 0 == system(tmp.c_str()) )
                inputter.open(file.c_str(), "rb");
        }
    }
    
    if ( !inputter.file() )
        throw InvalidIO("file `"+file+"' not found");

    if ( inputter.error() )
        throw InvalidIO("file `"+file+"' is invalid");
 
    inputter.vectorSize(DIM);
    clearPositions();
    //std::clog << "FrameReader: has openned " << obj_file << std::endl;
}


int FrameReader::badFile()
{
    if ( !inputter.file() )
        return 8;
    
    if ( inputter.eof() )
        inputter.clear();
    
    if ( ! inputter.good() )
        return 7;
    
    return 0;
}


void FrameReader::checkFile()
{
    if ( !inputter.file() )
        throw InvalidIO("No open file");
    
    if ( inputter.eof() )
        inputter.clear();
    
    if ( ! inputter.good() )
        throw InvalidIO("File has errors");
}

//------------------------------------------------------------------------------
#pragma mark -

void FrameReader::clearPositions()
{
    VLOG("FrameReader: clear\n");
    
    frameIndex = 0;
    framePos.clear();
    framePos.reserve(1024);
    framePos.resize(1);
    // store info for frame 0:
    fpos_t pos;
    if ( 0 == inputter.get_pos(pos) )
    {
        framePos[0].status = 1;
        framePos[0].position = pos;
    }
}


void FrameReader::savePos(size_t frm, const fpos_t& pos, int confidence)
{
    if ( frm >= framePos.capacity() )
    {
        constexpr size_t chunk = 1024;
        size_t sz = ( frm + chunk - 1 ) & ~( chunk -1 );
        framePos.reserve(sz);
    }
    
    if ( frm >= framePos.size() )
    {
        size_t i = framePos.size();
        framePos.resize(frm+1);
        while ( i <= frm )
            framePos[i++].status = 0;
    }
    
    if ( framePos[frm].status < confidence )
    {
        framePos[frm].status = confidence;
        framePos[frm].position = pos;
    
        //VLOG("FrameReader: position of frame " << frm << " is " << pos << '\n');
        VLOG("FrameReader: found position of frame "<<frm<<" ("<<confidence<<")\n");
    }
}


/**
 This uses the info stored in `framePos[]` to move to a position
 in the file where frame `frm` should start.
*/
size_t FrameReader::seekPos(size_t frm)
{
    if ( inputter.eof() )
        inputter.clear();
    
    if ( frm < 1 || framePos.empty() )
    {
        VLOG("FrameReader: seekPos rewind\n");
        inputter.rewind();
        return 0;
    }
    
    size_t inx = std::min(frm, framePos.size()-1);

    while ( inx > 0  &&  framePos[inx].status == 0 )
        --inx;
    
    //check if we know already were the frame starts:
    if ( 0 < inx )
    {
        VLOG("FrameReader: using known position of frame " << inx << '\n');
        inputter.set_pos(framePos[inx].position);
        return inx;
    }
    else {
        VLOG("FrameReader: rewind\n");
        inputter.rewind();
        return 0;
    }
}


size_t FrameReader::lastKnownFrame() const
{
    if ( framePos.empty() )
        return 0;
    size_t res = framePos.size()-1;
    while ( 0 < res  &&  framePos[res].status < 2 )
        --res;
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -


/// return code
enum FrameReaderCode { SUCCESS = 0, END_OF_FILE = 1, NOT_FOUND = 2, BAD_FILE = 4 };

/**
 scan file forward from current position to find the next Cytosim frame
 @return 0 if no frame was found
*/
int FrameReader::seekFrame(size_t frm)
{
    VLOG("FrameReader: seekFrame("<< frm <<")\n");
    
    size_t inx = seekPos(frm);
    
    if ( inx == frm )
        return SUCCESS;
    
    while ( ! inputter.eof() )
    {
        fpos_t pos;
        bool has_pos = false;
        std::string line;

        do {
            has_pos = !inputter.get_pos(pos);
            line = inputter.get_line();
            
            if ( inputter.eof() )
                return END_OF_FILE;
            
#ifdef BACKWARD_COMPATIBILITY // 2012
            if ( 0 == line.compare(0, 7, "#frame ") )
                break;
#endif
            
        } while ( line.compare(0, 9, "#Cytosim ") );
        
        //std::clog << "******\n";
        VLOG("           : " << line << '\n');

        if ( ! inputter.eof() )
        {
            if ( has_pos ) savePos(inx, pos, 2);
            if ( inx == frm )
            {
                if ( has_pos ) inputter.set_pos(pos);
                return SUCCESS;
            }
            ++inx;
        }
    }
    
    VLOG("FrameReader: seekFrame("<< frm <<") reached EOF\n");
    return END_OF_FILE;
}

//------------------------------------------------------------------------------
/**
 returns 0 for success, an error code, or throws an exception
 */
int FrameReader::loadFrame(Simul& sim, size_t frm, const bool reload)
{
    if ( badFile() )
        return BAD_FILE;

    VLOG("FrameReader: loadFrame(frame="<<frm<<", reload="<<reload<<")\n");
    
    // what we are looking for might already be in the buffer:
    if ( frm == lastLoaded && ! reload )
        return SUCCESS;
    
    // it might be the next one in the buffer:
    if ( frm > 0  &&  frm-1 == lastLoaded )
        return loadNextFrame(sim);
    
    // otherwise, try to find the start tag from there:
    if ( SUCCESS != seekFrame(frm) )
        return NOT_FOUND;
    
    // store the position in the file:
    fpos_t pos;
    bool has_pos = !inputter.get_pos(pos);
    
    VLOG("FrameReader: reading frame " << frm << " from " << pos << '\n');
    //VLOG("FrameReader: reading frame " << frm << '\n');
    
    // ask cytosim to read the file:
    if ( !sim.reloadObjects(inputter) )
    {
        VLOG("FrameReader: loadFrame("<< frm <<") successful\n");
        frameIndex = frm;
        lastLoaded = frameIndex;
        if ( has_pos )
            savePos(frameIndex, pos, 4);
        // the next frame should start at the current position:
        if ( 0 == inputter.get_pos(pos) )
            savePos(frameIndex+1, pos, 1);
        return SUCCESS;
    }
    else
    {
        VLOG("FrameReader: loadFrame("<< frm <<") EOF at frame " << frm << '\n');
        return END_OF_FILE;
    }
}


/**
 returns 0 for success, an error code, or throws an exception
 */
int FrameReader::loadNextFrame(Simul& sim)
{
    if ( badFile() )
        return BAD_FILE;
    
    fpos_t pos;
    bool has_pos = !inputter.get_pos(pos);

    if ( !sim.reloadObjects(inputter) )
    {
        if ( lastLoaded == frameIndex )
            ++frameIndex;
        lastLoaded = frameIndex;

        VLOG("FrameReader: loadNextFrame() has read frame " << currentFrame() << '\n');
        
        // the position we used was good, to read this frame
        if ( has_pos )
            savePos(frameIndex, pos, 4);
       
        // the next frame should start from the current position:
        if ( !inputter.get_pos(pos) )
            savePos(frameIndex+1, pos, 1);
        return SUCCESS;
    }
    else
    {
        VLOG("FrameReader: loadNextFrame() EOF while seeking frame " << currentFrame() << '\n');
        return END_OF_FILE;
    }
}


/**
 returns 0 for success, an error code, or throws an exception
 */
int FrameReader::loadLastFrame(Simul& sim, size_t cnt)
{
    if ( badFile() )
        return BAD_FILE;
    
    /// seek last known position:
    size_t frm = lastKnownFrame();
    if ( frm > 1 )
        inputter.set_pos(framePos[frm].position);
    else
        inputter.rewind();
    
    /// go from here to last frame:
    int res = NOT_FOUND;
    while ( !sim.reloadObjects(inputter) )
    {
        frameIndex = frm++;
        lastLoaded = frameIndex;
        res = SUCCESS;
    }
    
    if ( res == SUCCESS && cnt > 0 )
    {
        frm = frm - 1 - cnt;
        // go back up by 'cnt' frames:
        if ( SUCCESS != seekFrame(frm) )
            return NOT_FOUND;
        
        if ( !sim.reloadObjects(inputter) )
            return NOT_FOUND;

        frameIndex = frm;
        lastLoaded = frameIndex;
        VLOG("FrameReader: loadFrame("<< frm <<") successful\n");
    }
    
    return res;
}
