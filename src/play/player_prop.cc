// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "player_prop.h"
#include "glossary.h"
#include "save_image.h"


void PlayerProp::clear()
{
    play         = 0;
    loop         = 0;
    exit_at_eof  = false;
    period       = 1;
    delay        = 32;

    report_index = 0;

    for ( int k = 0; k < NB_MAGIC_KEYS; ++k )
    {
        magic_key[k]  = 0;
        magic_code[k] = "";
    }
    
    save_images = false;
    if ( SaveImage::supported("png") )
        image_format = "png";
    else
        image_format = "ppm";

    image_dir    = "";
    downsample   = 1;
    image_index  = 0;
    poster_index = 0;
}


void PlayerProp::read(Glossary& glos)
{
    glos.set(play,         "play");
    glos.set(loop,         "loop");
    if ( glos.set(period,  "period") )
        period = std::max(1u, period);
    if ( glos.set(delay,   "delay") )
        delay = std::max(2u, delay);
    glos.set(save_images,  "save_images");
    glos.set(image_format, "image_format");
    glos.set(image_dir,    "image_dir");
    glos.set(downsample,   "downsample");
    glos.set(downsample,   "downsampling");
    glos.set(report,       "report");

    if ( ! SaveImage::supported(image_format.c_str()) )
        throw InvalidParameter("unsupported image format");
    
    std::string var = "magic_key";
    for ( int k = 0; k < NB_MAGIC_KEYS; ++k )
    {
        glos.set(magic_key[k], var);
        glos.set(magic_code[k], var, 1);
        var = "magic_key" + std::to_string(k+1);
    }
}


void PlayerProp::write_values(std::ostream& os) const
{
    write_value(os, "play",   play);
    write_value(os, "loop",   loop);
    write_value(os, "period", period);
    write_value(os, "delay",  delay);
    write_value(os, "report", report);
    write_value(os, "save_images", save_images);
    write_value(os, "image_format", image_format);
    write_value(os, "image_dir", image_dir);
    write_value(os, "downsample", downsample);

    for ( int k = 0; k < NB_MAGIC_KEYS; ++k )
    {
        std::string var = "magic_key" + std::to_string(k);
        write_value(os, var, magic_key[k], "("+magic_code[k]+")");
    }
}

//------------------------------------------------------------------------------

void PlayerProp::toggleReport(bool alt)
{
    report_index = ( report_index + 1 ) % 7;
    
    if ( alt )
    {
        switch( report_index )
        {
            case 0: report = "";                   break;
            case 1: report = "inventory";          break;
            case 2: report = "system";             break;
            case 3: report = "fiber:lattice_density,field"; break;
            case 4: report = "fiber:segment";      break;
            case 5: report = "fiber:cluster";      break;
            case 6: report = "fiber:age";          break;
        }
    }
    else
    {
        switch( report_index )
        {
            case 0: report = "";                   break;
            case 1: report = "fiber:length";       break;
            case 2: report = "fiber:dynamic";      break;
            case 3: report = "single";             break;
            case 4: report = "couple";             break;
            case 5: report = "couple:configuration"; break;
            case 6: report = "fiber:distribution"; break;
        }
    }
}

