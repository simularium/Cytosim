// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PLAYER_PROP_H
#define PLAYER_PROP_H

#include "property.h"


/// Parameters for the Player
class PlayerProp : public Property
{
    
public:
    
    /// number of programmable keys
    static constexpr int NB_MAGIC_KEYS = 4;

public:
    
    /**
     @defgroup PlayPar Parameters of Play
     @ingroup DisplayParameters
     @{
     */
    
    /// direction of replay: 1: forward and -1: reverse
    int            play;
    
    /// if true, jump to first frame after last frame
    unsigned int   loop;
    
    /// number of simulation steps done between two drawings
    /**
     if period==2, only half of the frames will be displayed
     */
    unsigned int   period;
    
    /// number of milli-seconds between refresh
    unsigned int   delay;
    
    /// specifies information displayed near the bottom left corner of window
    std::string    report;

    /// associate a piece of custom code to a key
    /**
     Example:
     
         % define a magic key to delete fibers:
         set system display
         {
            magic_key1 = m, ( delete 10 microtubule )
            magic_key2 = C, ( cut microtubule { plane = 1 0 0, 0 } )
            label = (Press 'm' to delete fibers!)
         }
     
     up to 4 keys (magic_key, magic_key1 ... 3) can be defined.
     */
    char           magic_key[NB_MAGIC_KEYS];
    
    /// flag to export image files
    bool           save_images;
    
    /// format of exported images [png, ppm]
    std::string    image_format;
    
    /// directory where images are exported
    std::string    image_dir;
    
    /// if > 1, downsample images before writing them out
    /**
     This can be used to reduce pixelation artifacts:
     Specify a larger image size than desired, and the equivalent downsampling,
     For example, to produce a 512x256 image:
     
         window_size = 1024, 512
         downsample = 2
         
     */
    unsigned int   downsample;
    
    /** @} */

    /// if true, program will quit when end-of-file is reached
    bool           exit_at_eof;
    
    /// index of report which is displayed
    unsigned int   report_index;

    /// index used to build the name of the exported image
    unsigned int   image_index;
    
    /// index used to build the name of the exported poster
    unsigned int   poster_index;
   
    /// the piece of cytosim code executed when `magic_key` is pressed (set as `magic_key[1]`)
    std::string    magic_code[NB_MAGIC_KEYS];
    
public:

    /// constructor
    PlayerProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~PlayerProp() { }
    
    /// identifies the property
    std::string category() const { return "simul:display"; }

    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new PlayerProp(*this); }

    /// write all values
    void write_values(std::ostream&) const;
    
    /// change `report` to be one of `report?`
    void toggleReport(bool alt);
};


#endif


