// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "display_prop.h"
#include "glossary.h"

//------------------------------------------------------------------------------
void DisplayProp::clear()
{
    style          = 2;
    tile           = 0;
    fold           = 1;
    draw_links     = false;

    couple_select  = 7;
    single_select  = 3;
    
    point_value    = 0;
    point_size     = 5;
    link_width     = 4;
    line_width     = 2;
}

//------------------------------------------------------------------------------
void DisplayProp::read(Glossary& glos)
{
    glos.set(style,         "style");
    glos.set(tile,          "tile");
    glos.set(fold,          "fold");
    glos.set(fold,          "tile", 1);

    glos.set(tile,          "periodic");
    glos.set(fold,          "periodic", 1);
//#ifdef BACKWARD_COMPATIBILITY
    glos.set(tile,          "tiled");
    glos.set(fold,          "tiled", 1);
//#endif
    glos.set(draw_links,    "draw_links");

    glos.set(couple_select, "couple_select");
    glos.set(single_select, "single_select");
    
    glos.set(point_value,   "point_value");
    glos.set(point_size,    "point_size");
    // unless specified, `link_width` will be equal to `line_width`:
    if ( glos.set(line_width, "line_width") )
        link_width = line_width;
    glos.set(link_width,    "link_width") || glos.set(link_width, "link_size");
}


//------------------------------------------------------------------------------

void DisplayProp::write_values(std::ostream& os) const
{
    write_value(os, "style",         style);
    write_value(os, "tile",          tile, fold);
    write_value(os, "draw_links",    draw_links);
    write_value(os, "couple_select", couple_select);
    write_value(os, "single_select", single_select);
    write_value(os, "point_value",   point_value);
    write_value(os, "point_size",    point_size);
    write_value(os, "link_width",    link_width);
    write_value(os, "line_width",    line_width);
}


