// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 08/08/2010.


#include "gle_color.h"


namespace gle
{
    /// Holds the name and the RGBA components of a color
    class named_color
    {
    public:
        /// name of color
        char const * name;
        /// components of color (RGBA)
        uint32_t hex;
    };
    
    /// a small set of contrasted colors (indx is wrapped to the number of colors)
    gle_color nice_color(unsigned indx);
  
    /// a set of standard colors (indx is wrapped to the number of colors)
    gle_color std_color(unsigned indx);

    /// a set of standard named html colors
    gle_color std_color(const std::string& name);

    /// a large set of colors from Crayola crayons
    gle_color alt_color(unsigned indx);
    
    /// extract colors having a brightness between `minb` and `maxb`
    int       select_colors(gle_color * array, unsigned asize, GLfloat minb, GLfloat maxb);
    
    /// one of the crayola color, with a brightness() > threshold
    gle_color bright_color(unsigned indx, GLfloat minb = 0.6f, GLfloat maxb = 3.0f);

    /// print a list of colors with names and values
    void      print_colors(std::ostream&, named_color* list, unsigned list_size);
    
    /// print the list of standard colors
    void      print_std_colors(std::ostream&);

}

