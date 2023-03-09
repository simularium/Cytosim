// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University
#ifndef SAVE_IMAGE_H
#define SAVE_IMAGE_H

#include <cstdio>
#include <stdint.h>

/// Can save pixel array to files in PNG or PPM format
/**
 - PPM files do not require any library, and thus writing is supported on any platform.
   They can be read by various software, in particular the 'pbmplus' toolkit
   However, they are big and usually not supported by most viewers.
   Known viewers on Mac OS X: ImageJ, GraphicConverter, ToyViewer.
 - PNG is a modern all-purpose file format that supports RGBA format, and is thus
   very well suited to export OpenGL scenes.
 .
 
 PNG support requires 'libpng', which must be installed separately.
 */
namespace SaveImage
{
    /// error codes
    enum { NO_ERROR=0, FAILED_ALLOCATION=1, FAILED_READ=2, UNKNOWN_FORMAT=3, OPENGL_ERROR=4, FILE_ERROR=5, PNG_ERROR=10 };
    
    /// open a file for binary write
    FILE * openFile(const char * name);

    
    /// Netpbm pixel image format, 3 one-byte componentss per pixels (R, G, B)
    int saveColorPPM(FILE*, const uint8_t[], uint32_t width, uint32_t height);
    
    /// uncompressed RGB TGA format (https://en.wikipedia.org/wiki/Truevision_TGA)
    int saveTGA(FILE*, const uint8_t[], bool color, uint32_t width, uint32_t height);

    /// save RGB Truevision TGA format
    int saveColorTGA(FILE*, const uint8_t[], uint32_t width, uint32_t height);

    /// save Grayscale Truevision TGA format
    int saveGrayTGA(FILE*, const uint8_t[], uint32_t width, uint32_t height);

    
    /// write PNG image
    int savePNG(FILE*, const uint8_t[], uint8_t bit_depth, uint8_t num_colors, uint32_t width, uint32_t height);

    /// Portable Network Graphic format, 4 one-byte components per pixels (R, G, B, A)
    int saveAlphaPNG(FILE*, const uint8_t[], uint32_t width, uint32_t height);

    /// Portable Network Graphic format, 3 one-byte components per pixels (R, G, B)
    int saveColorPNG(FILE*, const uint8_t[], uint32_t width, uint32_t height);

    /// save 16-bit grayscale PNG file
    int saveGrayPNG(FILE*, const uint16_t[], uint32_t width, uint32_t height);
    
    
    /// save pixels[] and return error-code
    int savePixels(FILE*, const char format[], const uint8_t[], uint32_t width, uint32_t height);
    
    /// save pixels[] and return error-code
    int savePixels(FILE*, const char format[], const uint8_t[], uint32_t width, uint32_t height, int downsample);

    /// save pixels[] and return error-code
    int savePixels(const char* name, const char format[], const uint8_t[], uint32_t width, uint32_t height, int downsample);
    
    /// downsample `src` to set `dst`
    void downsampleRGB(uint8_t* dst, uint32_t W, uint32_t H, uint8_t const* src, unsigned bin);

    //-------------------- use the functions below: ---------------------
    
    /// true if 'format' is the 3-letter file-entension of a supported image format
    /**
     'ppm' is always supported, and 'png' may be supported depending on compilation
     The 3-letter format string can be lowercase or uppercase.
     */
    bool supported(const char format[]);

    /// save entire viewport in a new file called 'name'. Returns error-code
    int readPixels(int32_t x, int32_t y, uint32_t width, uint32_t height, void * pixels);
    
    /// save entire viewport in a new file called 'name'. Returns error-code
    int readDepthPixels(int32_t x, int32_t y, uint32_t width, uint32_t height, void * pixels);

    /// save entire viewport in a new file called 'name'. Returns error-code
    int saveEntireImage(const char* name, const char format[], int downsample=1);

    /// save a region of the current buffer in a new file called 'name'. Returns error-code
    int saveImage(const char* name, const char format[], const int viewport[], int downsample=1);
    
    /// save a region of the current depth buffer in a new PNG file called 'name'. Returns error-code
    int saveDepthBuffer(const char* name, const int viewport[]);

     /// save an image with higher resolution (this is better than saveCompositeImage)
    int saveMagnifiedImage(int mag, const char* name, const char format[], uint32_t width, uint32_t height, void (*display)(int, void *), void* arg, int downsample);

    /// save an image with higher resolution
    int saveCompositeImage(int mag, const char* name, const char format[], uint32_t width, uint32_t height, double pixel_size, void (*display)(int, void *), void* arg, int downsample);
}


#endif
