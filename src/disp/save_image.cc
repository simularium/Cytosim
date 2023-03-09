// Cytosim was created by Francois Nedelec. Copyright 2021 Cambridge University
#include "save_image.h"
#include <cstdlib>
#include <cstring>
#include <new>

#ifdef DISPLAY
#  include "opengl.h"
#endif


/// destination of error messages (set to zero to suppress output)
static FILE * ERF = stderr;


bool SaveImage::supported(const char format[])
{
    if ( 0 == strcasecmp(format, "png") )
        return true;
    if ( 0 == strcasecmp(format, "ppm") )
        return true;
    if ( 0 == strcasecmp(format, "tga") )
        return true;

    return false;
}


static uint8_t* new_pixels(size_t s, size_t col)
{
    void * ptr = nullptr;
    if ( posix_memalign(&ptr, 64, s*col) )
        throw std::bad_alloc();
    return (uint8_t*)ptr;
}

static void free_pixels(uint8_t *& ptr)
{
    free(ptr);
    ptr = nullptr;
}


//------------------------------------------------------------------------------
#pragma mark Getting Images from OpenGL

#ifdef DISPLAY

/**
 saveImage(...) will read pixels from the current OpenGL read buffer,
 and save them in a file with the requested format
 */
int SaveImage::saveEntireImage(const char * filename,
                               const char format[],
                               int downsample)
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    //printf("saveImage viewport %i %i %i %i\n", vp[0], vp[1], vp[2], vp[3]);
    
    return saveImage(filename, format, vp, downsample);
}

/**
 saveImage(...) will read pixels from the current OpenGL read buffer,
 and save them in a file with the requested format
 */
int SaveImage::saveImage(const char * filename,
                         const char format[],
                         const int vp[4],
                         int downsample)
{
    int res = FAILED_READ;

    //allocate memory to hold image:
    uint8_t* pixels = new_pixels(vp[2]*vp[3], 3);

    if ( 0 == readPixels(vp[0], vp[1], vp[2], vp[3], pixels) )
        res = savePixels(filename, format, pixels, vp[2], vp[3], downsample);
    
    free_pixels(pixels);
    return res;
}


/**
 After setting a higher resolution, this will translate the ModelView to produce several
 images that will be stiched together in memory, into an image with higher resolution.
 This works even if the image is larger than the maximum OpenGL viewPort,
 but there can be artifacts caused by objects in the stitched zones.
 */
int SaveImage::saveCompositeImage(const int mag,
                                  const char * filename,
                                  const char format[],
                                  const uint32_t width, const uint32_t height,
                                  const double pixel_size,
                                  void (*drawFunc)(int, void *), void * arg,
                                  int downsample)
{
    if ( ! supported(format) )
        return UNKNOWN_FORMAT;
    int res = OPENGL_ERROR;
#ifdef GL_VERSION_2_1
    int mW = mag * width;
    int mH = mag * height;
    
    const int PIX = 3;  //number of bytes for each pixel
    uint8_t* pixels = new_pixels(mW*mH, PIX);
    uint8_t* sub    = new_pixels(width*height, PIX);
    
    const double cc = ( mag - 1 ) * 0.5;
    const double dx = width * pixel_size / mag;
    const double dy = height * pixel_size / mag;
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(mag, mag, mag);
    
    for ( int iy = 0; iy < mag; ++iy )
    for ( int ix = 0; ix < mag; ++ix )
    {
        glPushMatrix();
        glTranslated((cc-ix)*dx, (cc-iy)*dy, 0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        drawFunc(mag, arg);
        glPopMatrix();
        if ( 0 == readPixels(0, 0, width, height, sub) )
        {
            uint8_t * dst = &pixels[width*PIX*(ix+mH*iy)];
            for ( uint32_t u = 0; u < height; ++u )
                memcpy(&dst[u*mW*PIX], &sub[u*width*PIX], width*PIX);
        }
    }
    glPopMatrix();
    
    res = savePixels(filename, format, pixels, mW, mH, downsample);
    free_pixels(pixels);
    free_pixels(sub);
#endif
    return res;
}


/**
 This adjusts the Viewport to produce an image with higher resolution.
 The result should be better than saveCompositeImage, but uses more
 memory on the graphic card.
 */
int SaveImage::saveMagnifiedImage(const int mag,
                                  const char * filename,
                                  const char format[],
                                  const uint32_t width, const uint32_t height,
                                  void (*drawFunc)(int, void *), void * arg,
                                  int downsample)
{
    if ( ! supported(format) )
        return UNKNOWN_FORMAT;
    
    int mW = mag * width;
    int mH = mag * height;
    
    GLint dim[2] = { 0 };
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dim);
    if ( mW > dim[0] || mH > dim[1] )
    {
        fprintf(ERF, "SaveImage:: exceeding maximum supported size (%ix%i)\n", (int)dim[0], (int)dim[1]);
        return FAILED_ALLOCATION;
    }
    
    const int PIX = 3;  //number of bytes for each pixel
    //allocate memory to hold the full image:
    uint8_t* pixels = new_pixels(mW*mH, PIX);
    uint8_t* sub = new_pixels(width*height, PIX);

    GLint svp[4];
    glGetIntegerv(GL_VIEWPORT, svp);
    for ( int iy = 0; iy < mag; ++iy )
    for ( int ix = 0; ix < mag; ++ix )
    {
        glViewport(-ix*width, -iy*height, mW, mH);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        drawFunc(mag, arg);
        if ( 0 == readPixels(0, 0, width, height, sub) )
        {
            uint8_t * dst = &pixels[width*PIX*(ix+mH*iy)];
            for ( uint32_t h = 0; h < height; ++h )
                memcpy(&dst[h*mW*PIX], &sub[h*width*PIX], width*PIX);
        }
    }
    int res = savePixels(filename, format, pixels, mW, mH, downsample);
    free_pixels(pixels);
    //restore original viewport:
    glViewport(svp[0], svp[1], svp[2], svp[3]);
    free_pixels(sub);
    return res;
}

//------------------------------------------------------------------------------
#pragma mark - Pixels

int SaveImage::readPixels(int32_t X, int32_t Y, uint32_t W, uint32_t H, GLvoid *pixels)
{
    // set the alignment to double-words
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
#if ( 0 )
    GLint readbuf = 0, drawbuf = 0;
    glGetIntegerv(GL_READ_BUFFER, &readbuf);
    glGetIntegerv(GL_DRAW_BUFFER, &drawbuf);
    printf("framebuffers: read %i draw %i\n", readbuf, drawbuf);
#endif
    
    //read the pixel values, from top-left corner:
    glReadPixels(X, Y, W, H, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    //printf(" read OpenGL pixels %ux%u\n", W, H);

    GLenum err = glGetError();
    
    if ( err != GL_NO_ERROR )
    {
        fprintf(ERF, "OpenGL error: could not read pixels (error %u)\n", err);
        return OPENGL_ERROR;
    }
    return NO_ERROR;
}

/**
 This will downsample pixelmap `src` and set destination `dst`. The pixel
 array `src` should be of size `3*W*H` with 3 bytes per pixels: R, G, B,
 while `src` will be `bin*bin` times smaller. Pixels are stored in row order
 from the lowest to the highest row, left to right in each row (as in OpenGL).
 The pixels components of `src` are averaged to produce `dst`.
 */
void SaveImage::downsampleRGB(uint8_t dst[], unsigned W, unsigned H,
                              const uint8_t src[], unsigned bin)
{
    const size_t s = bin * bin;
    const size_t sx = W / bin;
    const size_t sy = H / bin;
    
#if ( 0 )
    //reset destination:
    for ( size_t u = 0; u < sx*sy; ++u )
    {
        dst[3*u  ] = 0xFF;
        dst[3*u+1] = 0xFF;
        dst[3*u+2] = 0xFF;
    }
#endif
    
    for ( size_t x = 0; x < sx; ++x )
    for ( size_t y = 0; y < sy; ++y )
    {
        size_t r = 0, g = 0, b = 0;
        for ( size_t dx = 0; dx < bin; ++dx )
        for ( size_t dy = 0; dy < bin; ++dy )
        {
            uint8_t const* p = src + 3 * ( dx + bin*x + W*(dy+bin*y) );
            r += p[0];
            g += p[1];
            b += p[2];
        }
        
        dst[3*(x+sx*y)  ] = (uint8_t)( r / s );
        dst[3*(x+sx*y)+1] = (uint8_t)( g / s );
        dst[3*(x+sx*y)+2] = (uint8_t)( b / s );
    }
}


/// used to convert depth buffer image from float to uint16
static void convertPixels(uint32_t S, void * buf)
{
    float * src = reinterpret_cast<float*>(buf);
    uint16_t * dst = reinterpret_cast<uint16_t*>(buf);
    for ( size_t i = 0; i < S; ++i )
        dst[i] = uint16_t( src[i] * 65535 );
}


int SaveImage::readDepthPixels(int32_t X, int32_t Y, uint32_t W, uint32_t H, GLvoid *pixels)
{
    // set the alignment to double-words
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    //read the pixel values, from top-left corner:
    glReadPixels(X, Y, W, H, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

    GLenum err = glGetError();
    
    if ( err != GL_NO_ERROR )
    {
        fprintf(ERF, "OpenGL error: could not read pixels (error %u)\n", err);
        return OPENGL_ERROR;
    }
    convertPixels(W*H, pixels);
    return NO_ERROR;
}

#endif

int SaveImage::savePixels(FILE * file,
                          const char format[],
                          const uint8_t pixels[],
                          uint32_t width, uint32_t height)
{
    if ( 0 == strcasecmp(format, "ppm") )
        return saveColorPPM(file, pixels, width, height);
    
    if ( 0 == strcasecmp(format, "tga") )
        return saveColorTGA(file, pixels, width, height);

    if ( 0 == strcasecmp(format, "png") )
        return saveColorPNG(file, pixels, width, height);
    
    return UNKNOWN_FORMAT;
}


int SaveImage::savePixels(FILE * file,
                          const char format[],
                          const uint8_t pixels[],
                          uint32_t width, uint32_t height,
                          int downsample)
{
    if ( downsample > 1 )
    {
        //printf("downsampling %i to : %i %i\n", downsample, mw, mh);
        int W = width / downsample;
        int H = height / downsample;
        uint8_t* img = new_pixels(W*H, 3);
        downsampleRGB(img, W, H, pixels, downsample);
        int res = savePixels(file, format, img, W, H);
        free_pixels(img);
        return res;
    }
    
    return savePixels(file, format, pixels, width, height);
}


FILE * SaveImage::openFile(const char * filename)
{
    if ( filename[0] == '\0' )
        return nullptr;
    FILE * f = fopen(filename, "wb");
    if ( f )
    {
        if ( ferror(f) )
            fclose(f);
        else
            return f;
    }
    return nullptr;
}


int SaveImage::savePixels(const char * filename,
                          const char format[],
                          const uint8_t pixels[],
                          uint32_t width, uint32_t height,
                          int downsample)
{
    FILE * file = openFile(filename);
    
    if ( file )
    {
        int res = savePixels(file, format, pixels, width, height, downsample);
        
        fclose(file);
        
        if ( res )
        {
            fprintf(ERF, " error %i while saving %s\n", res, filename);
            remove(filename);
        }
        
        return res;
    }
    
    return FILE_ERROR;
}

//------------------------------------------------------------------------------
#pragma mark - Depth buffer export in PNG format


int SaveImage::saveDepthBuffer(const char * filename, const int vp[4])
{
    int res = FAILED_READ;
    uint8_t* pixels = new_pixels(vp[2]*vp[3], 4);

    if ( 0 == readDepthPixels(vp[0], vp[1], vp[2], vp[3], pixels) )
    {
        FILE * file = openFile(filename);
        if ( file )
        {
            res = saveGrayPNG(file, (uint16_t*)pixels, vp[2], vp[3]);
            fclose(file);
            if ( res )
            {
                fprintf(ERF, " error %i while saving %s\n", res, filename);
                remove(filename);
            }
            return res;
        }
        return FILE_ERROR;
    }
    free_pixels(pixels);
    return res;
}

//------------------------------------------------------------------------------
#pragma mark - PPM format

/**
 Write the image in the Portable Pixmap format, also Netpbm format (man -k ppm).
 We use here the 'raw' binary format starting with P6 
 */
int SaveImage::saveColorPPM(FILE* file,
                            const uint8_t pixels[],
                            const uint32_t width, const uint32_t height)
{
    fprintf(file, "P6\n");
    fprintf(file, "%i %i\n", width, height);
    fprintf(file, "255\n");
    
    //write the pixels binary, line by line:
    for ( int i = height-1; i >= 0; --i )
        fwrite(&pixels[3*i*width], 1, 3*width, file);
    return NO_ERROR;
}


//------------------------------------------------------------------------------
#pragma mark - TGA format

/**
 save RGB Truevision TGA format
 https://en.wikipedia.org/wiki/Truevision_TGA
 */
int SaveImage::saveTGA(FILE* file,
                       const uint8_t pixels[], const bool color,
                       const uint32_t width, const uint32_t height)
{
    uint8_t header[18] = { 0 };
    // Data code type -- 2 - uncompressed RGB image.
    header[2] = (color?2:3);
    // Image width - low byte
    header[12] = width & 0xFF;
    // Image width - high byte
    header[13] = (width >> 8) & 0xFF;
    // Image height - low byte
    header[14] = height & 0xFF;
    // Image height - high byte
    header[15] = (height >> 8) & 0xFF;
    // Color bit depth
    header[16] = (color?24:8);

    fwrite(header, 1, 18, file);
    fwrite(pixels, 1, width * height * (color?3:1), file);
    return NO_ERROR;
}


/// save RGB Truevision TGA format
int SaveImage::saveColorTGA(FILE* file, const uint8_t pixels[], uint32_t width, uint32_t height)
{
    return saveTGA(file, pixels, 1, width, height);
}

/// save Grayscale Truevision TGA format
int SaveImage::saveGrayTGA(FILE* file, const uint8_t pixels[], uint32_t width, uint32_t height)
{
    return saveTGA(file, pixels, 0, width, height);
}

//------------------------------------------------------------------------------
#pragma mark - PNG format using libpng

#ifdef HAS_PNG

#include <png.h>

static int savePNG(FILE* file,
                   png_bytep row_pointers[],
                   const uint8_t bit_depth, const uint8_t num_colors,
                   const uint32_t width, const uint32_t height)
{
    if ( !file )
        return SaveImage::FILE_ERROR;
    
    if ( bit_depth != 8 && bit_depth != 16 )
        return 19;
    
    int color_type = -1;
    
    if ( num_colors == 1 )
        color_type = PNG_COLOR_TYPE_GRAY;
    
    if ( num_colors == 3 )
        color_type = PNG_COLOR_TYPE_RGB;
    
    if ( num_colors == 4 )
        color_type = PNG_COLOR_TYPE_RGBA;
    
    if ( color_type < 0 )
        return 18;
    
    /* initialize stuff */
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (!png_ptr)
        return 17;
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return 16;
    
    if (setjmp(png_jmpbuf(png_ptr)))
        return 15;
    
    png_init_io(png_ptr, file);
    
    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        return 14;
    
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    png_write_info(png_ptr, info_ptr);
    
    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        return 13;
    
    if ( num_colors == 1 && bit_depth == 16 )
        png_set_swap(png_ptr);
    
    png_write_image(png_ptr, row_pointers);
    
    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        return 12;
    
    png_write_end(png_ptr, NULL);
 
    return 0;
}


int SaveImage::savePNG(FILE* file, const uint8_t pixels[],
                       const uint8_t bit_depth, const uint8_t num_colors,
                       const uint32_t width, const uint32_t height)
{
    int res = FAILED_ALLOCATION;
    png_bytep * rows = (png_bytep*)malloc(height*sizeof(png_bytep));
    if ( rows )
    {
        int bytes_per_row = ( bit_depth / 8 ) * num_colors * width;
        
        png_byte * start = (png_byte*)pixels;
        
        for ( int y = 0; y < height; ++y )
            rows[y] = start + bytes_per_row * ( height-y-1 );
        
        res = savePNG(file, rows, bit_depth, num_colors, width, height);
        
        free(rows);
    }
    
    return res;
}

#elif 1

//------------------------------------------------------------------------------
#pragma mark - PNG export using libspng (https://libspng.org)

#include "spng.h"

int SaveImage::savePNG(FILE* file, const uint8_t pixels[],
                       const uint8_t bit_depth, const uint8_t num_colors,
                       const uint32_t width, const uint32_t height)
{
    int res = 0;
    const size_t length = num_colors * width * height * ( bit_depth / 8 );

    uint8_t fmt = SPNG_COLOR_TYPE_TRUECOLOR;
    if ( num_colors == 4 )
        fmt = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
    if ( num_colors == 1 )
        fmt = SPNG_COLOR_TYPE_GRAYSCALE;
    
    /* Specify image dimensions, PNG format */
    struct spng_ihdr ihdr =
    {
        .width = width,
        .height = height,
        .bit_depth = bit_depth,
        .color_type = fmt
    };

    /* Creating an encoder context requires a flag */
    spng_ctx *enc = spng_ctx_new(SPNG_CTX_ENCODER);
    spng_set_option(enc, SPNG_IMG_COMPRESSION_LEVEL, 9);

    /* Encode to internal buffer managed by the library */
    //spng_set_option(enc, SPNG_ENCODE_TO_BUFFER, 1);
    
    /* Encode to file directly */
    res = spng_set_png_file(enc, file);
    if (res)
    {
        fprintf(ERF, "spng_set_png_file() error: %s\n", spng_strerror(res));
        goto done;
    }

    /* Image will be encoded according to ihdr.color_type, .bit_depth */
    res = spng_set_ihdr(enc, &ihdr);
    if (res)
    {
        fprintf(ERF, "spng_set_ihdr() error: %s\n", spng_strerror(res));
        goto done;
    }

    /* SPNG_FMT_PNG is a special value that matches the format in ihdr,
       SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
    res = spng_encode_image(enc, pixels, length, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE|SPNG_ENCODE_FLIP_Y);
    if (res)
        fprintf(ERF, "spng_encode_image() error: %s\n", spng_strerror(res));

done:
    /* Free context memory */
    spng_ctx_free(enc);
    return res;
}

#else

int SaveImage::savePNG(FILE* file, const uint8_t pixels[],
                       const uint8_t bit_depth, const uint8_t num_colors,
                       const uint32_t width, const uint32_t height)
{
    fprintf(ERF, "png export not supported\n");
    return 1;
}

#endif

/**
 Save RGBA image, 4 x 8-bits per pixel
 */
int SaveImage::saveAlphaPNG(FILE* file, const uint8_t pixels[],
                            const uint32_t width, const uint32_t height)
{
    return savePNG(file, pixels, 8, 4, width, height);
}

/**
 Save RGB image, 3 x 8-bits per pixel
 */
int SaveImage::saveColorPNG(FILE* file, const uint8_t pixels[],
                            const uint32_t width, const uint32_t height)
{
    return savePNG(file, pixels, 8, 3, width, height);
}

/**
 Save 16-bits gray-level image
 */
int SaveImage::saveGrayPNG(FILE* file, const uint16_t pixels[],
                           const uint32_t width, const uint32_t height)
{    
    return savePNG(file, (uint8_t*)pixels, 16, 1, width, height);
}

