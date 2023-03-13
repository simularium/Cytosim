// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "iowrapper.h"
#include <vector>

class Simul;


/// Helper class to access a particular frame in a trajectory file
/** 
 FrameReader is used to find a particular frame (eg. frame 10) in a trajectory file.
 FrameReader will handle basic IO failures and will remember the starting positions
 of all frames that were sucessfully identified, using this information to speed up
 future access to these and other frames in the same file. For example, if the 
 position of frame 50 is known and frame 60 is requested, it will start searching
 the file from the end of frame 50.

 FrameReader makes minimal assumptions on what constitutes a 'frame':
 - seekFrame() looks for a string that identifies the beggining of a frame (Cytosim).
 - loadFrame() calls Simul::reloadObjects() to read the content of the frame.
 .
 
 Frames are recorded starting at index 0.
*/
class FrameReader
{
private:
    
    /// accessory class to store a position in a file
    class file_pos 
    {
    public:
        int    status;   ///< indicates if `position` is valid
        fpos_t position; ///< starting position in the file
        file_pos() { status=0; }
    };
    
    /// type for list of positions
    typedef  std::vector<file_pos> PosList;
    
    /// the stream from which input is made
    Inputter inputter;
    
    /// starting position for each frame
    PosList  framePos;
    
    /// index of frame stored currently
    size_t   frameIndex;
    
    /// last frame loaded successfully
    size_t   lastLoaded;
    
    /// remember position `pos` as the place where frame `frm` should start
    void     savePos(size_t frm, const fpos_t& pos, int status);
   
    /// go to a position where a frame close to `frm` is known to start
    size_t   seekPos(size_t frm);
    
    /// check file validity
    void     checkFile();
    
    /// return 0 if file is good for input
    int      badFile();
    
public:
    
    /// constructor, after which openFile() should be called
    FrameReader();
    
    /// open file for input
    void     openFile(std::string const& file);
    
    /// clear the buffer
    void     clearPositions();
    
    /// last frame seen in the file
    size_t   lastKnownFrame() const;
    
    /// return state of file object
    bool     hasFile() { return inputter.file(); }
    
    /// true when end of file is reached
    bool     eof() const  { return inputter.eof();  }
    
    /// rewind file
    void     rewind() { inputter.rewind(); frameIndex=0; }

    /// true if everything looks correct for input
    bool     good() const { return inputter.good(); }
    
    /// dimensionality of vectors
    unsigned vectorSize() const { return inputter.vectorSize(); }

    /// rewind file and clear position buffer
    void     clear();
    
    /// return index of current frame 
    size_t   currentFrame() const { return frameIndex; }
    
    /// find the starting point of frame `frm` by brute force !
    int      seekFrame(size_t frm);
    
    /// load specified frame into given Simul (the index of first frame is 1)
    int      loadFrame(Simul&, size_t frm, bool reload = false);
    
    /// read the next frame in the file, return 0 for SUCCESS, 1 for EOF
    int      loadNextFrame(Simul&);
    
    /// read the last frame in the file, return 0 for SUCCESS, 1 if no frame was found
    int      loadLastFrame(Simul&, size_t cnt = 0);

};


