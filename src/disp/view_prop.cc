// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#include "view_prop.h"
#include "glossary.h"
#include "dim.h"

//------------------------------------------------------------------------------
void ViewProp::clear()
{
    zoom         = 1;
    view_size    = 10;
    auto_scale   = 1;
    focus.reset();
    focus_shift.reset();
    rotation.set(1,0,0,0);
    perspective  = 0;
    slice        = 0;
    
    back_color   = (DIM==3)?0x161616FF:0x000000FF;
    front_color  = 0xFFFFFFFF;
    
    buffered     = 0;
    depth_test   = 1;
    depth_clamp  = 0;
    retina       = 0;
    stencil      = 0;
    multisample  = 0;
    
    label        = "Cytosim";
    memo         = "Please, visit www.cytosim.org";
    draw_memo    = 0;

    track_fibers = 0;
    
    window_size[0]     = 800;
    window_size[1]     = 800;
    window_position[0] = 8;
    window_position[1] = 32;
    
    scale_bar_size     = 10;
    scale_bar_color    = 0xFFFF88AA;
    scale_bar_mode     = 0;
    draw_axes          = 0;
    axes_size          = 1;
    
    for ( int k = 0; k < NB_CLIP_PLANES; ++k )
    {
        clip_plane_mode[k] = 0;
        clip_plane_vector[k].set(1,0,0);
        clip_plane_scalar[k] = 0;
    }
    
    fog_type           = 0;
    fog_param          = 1;
    fog_color          = 0x000000FF;
}


void ViewProp::read(Glossary& glos)
{
    glos.set(zoom,        "zoom");
    if ( glos.set(view_size, "view_size") )
        auto_scale = 0;
    glos.set(auto_scale,  "auto_scale");
    glos.set(auto_scale,  "autoscale");
    glos.set(focus,       "focus");
    glos.set(rotation,    "rotation");
    
    // normalize quaternion:
    if ( rotation.normSqr() > 0.00001 )
        rotation.normalize();
    else
        rotation.set(1,0,0,0);

    glos.set(perspective, "perspective");
    glos.set(slice, "slice", {{"off", 0},{"front", 1},{"back", 2},{"slice", 3}});

    glos.set(back_color, "background");
    if ( glos.set(back_color, "back_color") )
    {
        fog_color = back_color;
        front_color = back_color.inverted();
    }
    glos.set(front_color, "front_color");
    glos.set(buffered,    "buffered") || glos.set(buffered, "buffer");
    glos.set(depth_test,  "depth_test");
    glos.set(depth_clamp, "depth_clamp");
    glos.set(retina,      "retina");
    if ( glos.use_key("+") )
        retina = 1;
    glos.set(stencil,     "stencil");
    
    glos.set(multisample, "multisample");
    glos.set(multisample, "samples");
#ifdef BACKWARD_COMPATIBILITY
    glos.set(multisample, "gl_samples");
#endif
    glos.set(label,       "label");
    glos.set(draw_memo,   "draw_memo");
    
    glos.set(track_fibers,        "track_fibers");
    glos.set(window_position, 2,  "window_position");
    
    // A square window is made if only one value is given.
    if ( glos.set(window_size, 2, "window_size") == 1 )
        window_size[1] = window_size[0];

    // A square window is made if only one value is given.
    if ( glos.set(window_size, 2, "image_size") == 1 )
        window_size[1] = window_size[0];

    // 'size' is an alias to set the size of the window.
    if ( glos.set(window_size, 2, "size") == 1 )
        window_size[1] = window_size[0];
    /*
     window_size can be changed here, but we cannot resize
     the window, since we do not have access to GLUT 
     */
    //std::clog << this << " window " << window_size[0] << "x" << window_size[1] << '\n';
    
    glos.set(scale_bar_size,  "scale_bar");
    glos.set(scale_bar_color, "scale_bar", 1);
    glos.set(scale_bar_mode,  "scale_bar", 2);
    glos.set(scale_bar_color, "scale_bar_color");

    glos.set(draw_axes,       "draw_axes");
    glos.set(axes_size,       "draw_axes", 1);
    glos.set(axes_size,       "axes_size");

    for ( int k = 0; k < NB_CLIP_PLANES; ++k )
    {
        std::string var = "clip_plane" + std::to_string(k);
        glos.set(clip_plane_mode[k],   var);
        glos.set(clip_plane_vector[k], var, 1);
        glos.set(clip_plane_scalar[k], var, 2);
    }
    
    Glossary::dict_type<GLint> keys{{"off",          0},
                                    {"linear",       1},
                                    {"exponential",  2},
                                    {"exponential2", 3}};

    glos.set(fog_type,     "fog_type", keys);
    glos.set(fog_param,    "fog_param");
    glos.set(fog_color,    "fog_color");
 
    glos.set(fog_type,     "fog", keys);
    glos.set(fog_param,    "fog", 1);
    glos.set(fog_color,    "fog", 2);
}

//------------------------------------------------------------------------------

void ViewProp::write_values(std::ostream& os) const
{
    write_value(os, "zoom",          zoom);
    write_value(os, "view_size",     view_size);
    write_value(os, "auto_scale",    auto_scale);
    write_value(os, "focus",         focus+focus_shift);
    write_value(os, "rotation",      rotation);
    write_value(os, "perspective",   perspective);
    write_value(os, "slice",         slice);
    write_value(os, "back_color",    back_color);
    write_value(os, "front_color",   front_color);
    write_value(os, "buffered",      buffered);
    write_value(os, "depth_test",    depth_test);
    write_value(os, "depth_clamp",   depth_clamp);
    write_value(os, "stencil",       stencil);
    write_value(os, "multisample",   multisample);
    write_value(os, "label",         label);
    write_value(os, "track_fibers",  track_fibers);
    //write_value(os, "window_position", window_position, 2);
    write_value(os, "window_size",   window_size, 2);
    write_value(os, "scale_bar",     scale_bar_size, scale_bar_color, scale_bar_mode);
    write_value(os, "draw_axes",     draw_axes, axes_size);
    for ( int k = 0; k < NB_CLIP_PLANES; ++k )
    {
        std::string var = "clip_plane" + std::to_string(k);
        write_value(os, var, clip_plane_mode[k], clip_plane_vector[k], clip_plane_scalar[k]);
    }
    write_value(os, "fog",           fog_type, fog_param, fog_color);
}


