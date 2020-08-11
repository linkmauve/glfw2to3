/*************************************************************************
 * GLFW 2to3 - www.glfw.org
 * A library easing porting from GLFW 2 to GLFW 3.x
 *------------------------------------------------------------------------
 * Copyright © 2002-2006 Marcus Geelnard
 * Copyright © 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
 * Copyright © 2020 Emmanuel Gil Peyrot <linkmauve@linkmauve.fr>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Image/texture I/O support */

//========================================================================
// Description:
//
// This module acts as an interface for different image file formats (the
// image file format is detected automatically).
//
// By default the loaded image is rescaled (using bilinear interpolation)
// to the next higher 2^N x 2^M resolution, unless it has a valid
// 2^N x 2^M resolution. The interpolation is quite slow, even if the
// routine has been optimized for speed (a 200x200 RGB image is scaled to
// 256x256 in ~30 ms on a P3-500).
//
// Paletted images are converted to RGB/RGBA images.
//
// A convenience function is also included (glfwLoadTexture2D), which
// loads a texture image from a file directly to OpenGL texture memory,
// with an option to generate all mipmap levels. GL_SGIS_generate_mipmap
// is used whenever available, which should give an optimal mipmap
// generation speed (possibly performed in hardware). A software fallback
// method is included when GL_SGIS_generate_mipmap is not supported (it
// generates all mipmaps of a 256x256 RGB texture in ~3 ms on a P3-500).
//
//========================================================================

//========================================================================
// Description:
//
// TGA format image file loader. This module supports version 1 Targa
// images, with these restrictions:
//  - Pixel format may only be 8, 24 or 32 bits
//  - Colormaps must be no longer than 256 entries
//
//========================================================================

//========================================================================
// Opens a GLFW stream with a file
//========================================================================

//------------------------------------------------------------------------
// Abstract data stream (for image I/O)
//------------------------------------------------------------------------
typedef struct {
    FILE*   file;
    void*   data;
    long    position;
    long    size;
} _GLFWstream;

static int _glfwOpenFileStream( _GLFWstream *stream, const char* name, const char* mode )
{
    memset( stream, 0, sizeof(_GLFWstream) );

    stream->file = fopen( name, mode );
    if( stream->file == NULL )
    {
        return GL_FALSE;
    }

    return GL_TRUE;
}


//========================================================================
// Opens a GLFW stream with a memory block
//========================================================================

static int _glfwOpenBufferStream( _GLFWstream *stream, void *data, long size )
{
    memset( stream, 0, sizeof(_GLFWstream) );

    stream->data = data;
    stream->size = size;
    return GL_TRUE;
}


//========================================================================
// Reads data from a GLFW stream
//========================================================================

static long _glfwReadStream( _GLFWstream *stream, void *data, long size )
{
    if( stream->file != NULL )
    {
        return (long) fread( data, 1, size, stream->file );
    }

    if( stream->data != NULL )
    {
        // Check for EOF
        if( stream->position == stream->size )
        {
            return 0;
        }

        // Clamp read size to available data
        if( stream->position + size > stream->size )
        {
            size = stream->size - stream->position;
        }

        // Perform data read
        memcpy( data, (unsigned char*) stream->data + stream->position, size );
        stream->position += size;
        return size;
    }

    return 0;
}


//========================================================================
// Returns the current position of a GLFW stream
//========================================================================

static long _glfwTellStream( _GLFWstream *stream )
{
    if( stream->file != NULL )
    {
        return ftell( stream->file );
    }

    if( stream->data != NULL )
    {
        return stream->position;
    }

    return 0;
}


//========================================================================
// Sets the current position of a GLFW stream
//========================================================================

static int _glfwSeekStream( _GLFWstream *stream, long offset, int whence )
{
    long position;

    if( stream->file != NULL )
    {
        if( fseek( stream->file, offset, whence ) != 0 )
        {
            return GL_FALSE;
        }

        return GL_TRUE;
    }

    if( stream->data != NULL )
    {
        position = offset;

        // Handle whence parameter
        if( whence == SEEK_CUR )
        {
            position += stream->position;
        }
        else if( whence == SEEK_END )
        {
            position += stream->size;
        }
        else if( whence != SEEK_SET )
        {
            return GL_FALSE;
        }

        // Clamp offset to buffer bounds and apply it
        if( position > stream->size )
        {
            stream->position = stream->size;
        }
        else if( position < 0 )
        {
            stream->position = 0;
        }
        else
        {
            stream->position = position;
        }

        return GL_TRUE;
    }

    return GL_FALSE;
}


//========================================================================
// Closes a GLFW stream
//========================================================================

static void _glfwCloseStream( _GLFWstream *stream )
{
    if( stream->file != NULL )
    {
        fclose( stream->file );
    }

    // Nothing to be done about (user allocated) memory blocks

    memset( stream, 0, sizeof(_GLFWstream) );
}


//************************************************************************
//****            GLFW internal functions & declarations              ****
//************************************************************************

//========================================================================
// TGA file header information
//========================================================================

typedef struct {
    int idlen;                 // 1 byte
    int cmaptype;              // 1 byte
    int imagetype;             // 1 byte
    int cmapfirstidx;          // 2 bytes
    int cmaplen;               // 2 bytes
    int cmapentrysize;         // 1 byte
    int xorigin;               // 2 bytes
    int yorigin;               // 2 bytes
    int width;                 // 2 bytes
    int height;                // 2 bytes
    int bitsperpixel;          // 1 byte
    int imageinfo;             // 1 byte
    int _alphabits;            // (derived from imageinfo)
    int _origin;               // (derived from imageinfo)
} _tga_header_t;

#define _TGA_CMAPTYPE_NONE      0
#define _TGA_CMAPTYPE_PRESENT   1

#define _TGA_IMAGETYPE_NONE     0
#define _TGA_IMAGETYPE_CMAP     1
#define _TGA_IMAGETYPE_TC       2
#define _TGA_IMAGETYPE_GRAY     3
#define _TGA_IMAGETYPE_CMAP_RLE 9
#define _TGA_IMAGETYPE_TC_RLE   10
#define _TGA_IMAGETYPE_GRAY_RLE 11

#define _TGA_IMAGEINFO_ALPHA_MASK   0x0f
#define _TGA_IMAGEINFO_ALPHA_SHIFT  0
#define _TGA_IMAGEINFO_ORIGIN_MASK  0x30
#define _TGA_IMAGEINFO_ORIGIN_SHIFT 4

#define _TGA_ORIGIN_BL 0
#define _TGA_ORIGIN_BR 1
#define _TGA_ORIGIN_UL 2
#define _TGA_ORIGIN_UR 3


//========================================================================
// Read TGA file header (and check that it is valid)
//========================================================================

static int ReadTGAHeader( _GLFWstream *s, _tga_header_t *h )
{
    unsigned char buf[ 18 ];
    int pos;

    // Read TGA file header from file
    pos = _glfwTellStream( s );
    _glfwReadStream( s, buf, 18 );

    // Interpret header (endian independent parsing)
    h->idlen         = (int) buf[0];
    h->cmaptype      = (int) buf[1];
    h->imagetype     = (int) buf[2];
    h->cmapfirstidx  = (int) buf[3] | (((int) buf[4]) << 8);
    h->cmaplen       = (int) buf[5] | (((int) buf[6]) << 8);
    h->cmapentrysize = (int) buf[7];
    h->xorigin       = (int) buf[8] | (((int) buf[9]) << 8);
    h->yorigin       = (int) buf[10] | (((int) buf[11]) << 8);
    h->width         = (int) buf[12] | (((int) buf[13]) << 8);
    h->height        = (int) buf[14] | (((int) buf[15]) << 8);
    h->bitsperpixel  = (int) buf[16];
    h->imageinfo     = (int) buf[17];

    // Extract alphabits and origin information
    h->_alphabits = (int) (h->imageinfo & _TGA_IMAGEINFO_ALPHA_MASK) >>
                     _TGA_IMAGEINFO_ALPHA_SHIFT;
    h->_origin    = (int) (h->imageinfo & _TGA_IMAGEINFO_ORIGIN_MASK) >>
                     _TGA_IMAGEINFO_ORIGIN_SHIFT;

    // Validate TGA header (is this a TGA file?)
    if( (h->cmaptype == 0 || h->cmaptype == 1) &&
        ((h->imagetype >= 1 && h->imagetype <= 3) ||
         (h->imagetype >= 9 && h->imagetype <= 11)) &&
         (h->bitsperpixel == 8 || h->bitsperpixel == 24 ||
          h->bitsperpixel == 32) )
    {
        // Skip the ID field
        _glfwSeekStream( s, h->idlen, SEEK_CUR );

        // Indicate that the TGA header was valid
        return GL_TRUE;
    }
    else
    {
        // Restore file position
        _glfwSeekStream( s, pos, SEEK_SET );

        // Indicate that the TGA header was invalid
        return GL_FALSE;
    }
}

//========================================================================
// Read Run-Length Encoded data
//========================================================================

static void ReadTGA_RLE( unsigned char *buf, int size, int bpp,
                         _GLFWstream *s )
{
    int repcount, bytes, k, n;
    unsigned char pixel[ 4 ];
    char c;

    // Dummy check
    if( bpp > 4 )
    {
        return;
    }

    while( size > 0 )
    {
        // Get repetition count
	_glfwReadStream( s, &c, 1 );
        repcount = (unsigned int) c;
        bytes = ((repcount & 127) + 1) * bpp;
        if( size < bytes )
        {
            bytes = size;
        }

        // Run-Length packet?
        if( repcount & 128 )
        {
            _glfwReadStream( s, pixel, bpp );
            for( n = 0; n < (repcount & 127) + 1; n ++ )
            {
                for( k = 0; k < bpp; k ++ )
                {
                    *buf ++ = pixel[ k ];
                }
            }
        }
        else
        {
            // It's a Raw packet
            _glfwReadStream( s, buf, bytes );
            buf += bytes;
        }

        size -= bytes;
    }
}


//========================================================================
// Read a TGA image from a file
//========================================================================

static int _glfwReadTGA( _GLFWstream *s, GLFWimage *img, int flags )
{
    _tga_header_t h;
    unsigned char *cmap, *pix, tmp, *src, *dst;
    int cmapsize, pixsize, pixsize2;
    int bpp, bpp2, k, m, n, swapx, swapy;

    // Read TGA header
    if( !ReadTGAHeader( s, &h ) )
    {
        return 0;
    }

    // Is there a colormap?
    cmapsize = (h.cmaptype == _TGA_CMAPTYPE_PRESENT ? 1 : 0) * h.cmaplen *
               ((h.cmapentrysize+7) / 8);
    if( cmapsize > 0 )
    {
        // Is it a colormap that we can handle?
        if( (h.cmapentrysize != 24 && h.cmapentrysize != 32) ||
            h.cmaplen == 0 || h.cmaplen > 256 )
        {
            return 0;
        }

        // Allocate memory for colormap
        cmap = (unsigned char *) malloc( cmapsize );
        if( cmap == NULL )
        {
            return 0;
        }

        // Read colormap from file
        _glfwReadStream( s, cmap, cmapsize );
    }
    else
    {
        cmap = NULL;
    }

    // Size of pixel data
    pixsize = h.width * h.height * ((h.bitsperpixel + 7) / 8);

    // Bytes per pixel (pixel data - unexpanded)
    bpp = (h.bitsperpixel + 7) / 8;

    // Bytes per pixel (expanded pixels - not colormap indeces)
    if( cmap )
    {
        bpp2 = (h.cmapentrysize + 7) / 8;
    }
    else
    {
        bpp2 = bpp;
    }

    // For colormaped images, the RGB/RGBA image data may use more memory
    // than the stored pixel data
    pixsize2 = h.width * h.height * bpp2;

    // Allocate memory for pixel data
    pix = (unsigned char *) malloc( pixsize2 );
    if( pix == NULL )
    {
        if( cmap )
        {
            free( cmap );
        }
        return 0;
    }

    // Read pixel data from file
    if( h.imagetype >= _TGA_IMAGETYPE_CMAP_RLE )
    {
        ReadTGA_RLE( pix, pixsize, bpp, s );
    }
    else
    {
        _glfwReadStream( s, pix, pixsize );
    }

    // If the image origin is not what we want, re-arrange the pixels
    switch( h._origin )
    {
    default:
    case _TGA_ORIGIN_UL:
        swapx = 0;
        swapy = 1;
        break;

    case _TGA_ORIGIN_BL:
        swapx = 0;
        swapy = 0;
        break;

    case _TGA_ORIGIN_UR:
        swapx = 1;
        swapy = 1;
        break;

    case _TGA_ORIGIN_BR:
        swapx = 1;
        swapy = 0;
        break;
    }
    if( (swapy && !(flags & GLFW_ORIGIN_UL_BIT)) ||
        (!swapy && (flags & GLFW_ORIGIN_UL_BIT)) )
    {
        src = pix;
        dst = &pix[ (h.height-1)*h.width*bpp ];
        for( n = 0; n < h.height/2; n ++ )
        {
            for( m = 0; m < h.width ; m ++ )
            {
                for( k = 0; k < bpp; k ++ )
                {
                    tmp     = *src;
                    *src ++ = *dst;
                    *dst ++ = tmp;
                }
            }
            dst -= 2*h.width*bpp;
        }
    }
    if( swapx )
    {
        src = pix;
        dst = &pix[ (h.width-1)*bpp ];
        for( n = 0; n < h.height; n ++ )
        {
            for( m = 0; m < h.width/2 ; m ++ )
            {
                for( k = 0; k < bpp; k ++ )
                {
                    tmp     = *src;
                    *src ++ = *dst;
                    *dst ++ = tmp;
                }
                dst -= 2*bpp;
            }
            src += ((h.width+1)/2)*bpp;
            dst += ((3*h.width+1)/2)*bpp;
        }
    }

    // Convert BGR/BGRA to RGB/RGBA, and optionally colormap indeces to
    // RGB/RGBA values
    if( cmap )
    {
        // Convert colormap pixel format (BGR -> RGB or BGRA -> RGBA)
        if( bpp2 == 3 || bpp2 == 4 )
        {
            for( n = 0; n < h.cmaplen; n ++ )
            {
                tmp                = cmap[ n*bpp2 ];
                cmap[ n*bpp2 ]     = cmap[ n*bpp2 + 2 ];
                cmap[ n*bpp2 + 2 ] = tmp;
            }
        }

        // Convert pixel data to RGB/RGBA data
        for( m = h.width * h.height - 1; m >= 0; m -- )
        {
            n = pix[ m ];
            for( k = 0; k < bpp2; k ++ )
            {
                pix[ m*bpp2 + k ] = cmap[ n*bpp2 + k ];
            }
        }

        // Free memory for colormap (it's not needed anymore)
        free( cmap );
    }
    else
    {
        // Convert image pixel format (BGR -> RGB or BGRA -> RGBA)
        if( bpp2 == 3 || bpp2 == 4 )
        {
            src = pix;
            dst = &pix[ 2 ];
            for( n = 0; n < h.height * h.width; n ++ )
            {
                tmp  = *src;
                *src = *dst;
                *dst = tmp;
                src += bpp2;
                dst += bpp2;
            }
        }
    }

    // Fill out GLFWimage struct (the Format field will be set by
    // glfwReadImage)
    img->Width         = h.width;
    img->Height        = h.height;
    img->BytesPerPixel = bpp2;
    img->Data          = pix;

    return 1;
}



// We want to support automatic mipmap generation
#ifndef GL_SGIS_generate_mipmap
 #define GL_GENERATE_MIPMAP_SGIS       0x8191
 #define GL_GENERATE_MIPMAP_HINT_SGIS  0x8192
 #define GL_SGIS_generate_mipmap    1
#endif // GL_SGIS_generate_mipmap


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Upsample image, from size w1 x h1 to w2 x h2
//========================================================================

static void UpsampleImage( unsigned char *src, unsigned char *dst,
    int w1, int h1, int w2, int h2, int bpp )
{
    int m, n, k, x, y, col8;
    float dx, dy, xstep, ystep, col, col1, col2;
    unsigned char *src1, *src2, *src3, *src4;

    // Calculate scaling factor
    xstep = (float)(w1-1) / (float)(w2-1);
    ystep = (float)(h1-1) / (float)(h2-1);

    // Copy source data to destination data with bilinear interpolation
    // Note: The rather strange look of this routine is a direct result of
    // my attempts at optimizing it. Improvements are welcome!
    dy = 0.0f;
    y = 0;
    for( n = 0; n < h2; n ++ )
    {
        dx = 0.0f;
        src1 = &src[ y*w1*bpp ];
        src3 = y < (h1-1) ? src1 + w1*bpp : src1;
        src2 = src1 + bpp;
        src4 = src3 + bpp;
        x = 0;
        for( m = 0; m < w2; m ++ )
        {
            for( k = 0; k < bpp; k ++ )
            {
                col1 = *src1 ++;
                col2 = *src2 ++;
                col = col1 + (col2 - col1) * dx;
                col1 = *src3 ++;
                col2 = *src4 ++;
                col2 = col1 + (col2 - col1) * dx;
                col += (col2 - col) * dy;
                col8 = (int) (col + 0.5);
                if( col8 >= 256 ) col8 = 255;
                *dst++ = (unsigned char) col8;
            }
            dx += xstep;
            if( dx >= 1.0f )
            {
                x ++;
                dx -= 1.0f;
                if( x >= (w1-1) )
                {
                    src2 = src1;
                    src4 = src3;
                }
            }
            else
            {
                src1 -= bpp;
                src2 -= bpp;
                src3 -= bpp;
                src4 -= bpp;
            }
        }
        dy += ystep;
        if( dy >= 1.0f )
        {
            y ++;
            dy -= 1.0f;
        }
    }
}


//========================================================================
// Build the next mip-map level
//========================================================================

static int HalveImage( GLubyte *src, int *width, int *height,
    int components )
{
    int     halfwidth, halfheight, m, n, k, idx1, idx2;
    GLubyte *dst;

    // Last level?
    if( *width <= 1 && *height <= 1 )
    {
        return GL_FALSE;
    }

    // Calculate new width and height (handle 1D case)
    halfwidth  = *width > 1 ? *width / 2 : 1;
    halfheight = *height > 1 ? *height / 2 : 1;

    // Downsample image with a simple box-filter
    dst = src;
    if( *width == 1 || *height == 1 )
    {
        // 1D case
        for( m = 0; m < halfwidth+halfheight-1; m ++ )
        {
            for( k = 0; k < components; k ++ )
            {
                *dst ++ = (GLubyte) (((int)*src +
                                      (int)src[components] + 1) >> 1);
                src ++;
            }
            src += components;
        }
    }
    else
    {
        // 2D case
        idx1 = *width*components;
        idx2 = (*width+1)*components;
        for( m = 0; m < halfheight; m ++ )
        {
            for( n = 0; n < halfwidth; n ++ )
            {
                for( k = 0; k < components; k ++ )
                {
                    *dst ++ = (GLubyte) (((int)*src +
                                          (int)src[components] +
                                          (int)src[idx1] +
                                          (int)src[idx2] + 2) >> 2);
                    src ++;
                }
                src += components;
            }
            src += components * (*width);
        }
    }

    // Return new width and height
    *width = halfwidth;
    *height = halfheight;

    return GL_TRUE;
}


//========================================================================
// Rescales an image into power-of-two dimensions
//========================================================================

static int RescaleImage( GLFWimage* image )
{
    int     width, height, log2, newsize;
    unsigned char *data;

    // Calculate next larger 2^N width
    for( log2 = 0, width = image->Width; width > 1; width >>= 1, log2 ++ )
      ;

    width  = (int) 1 << log2;
    if( width < image->Width )
    {
        width <<= 1;
    }

    // Calculate next larger 2^M height
    for( log2 = 0, height = image->Height; height > 1; height >>= 1, log2 ++ )
      ;

    height = (int) 1 << log2;
    if( height < image->Height )
    {
        height <<= 1;
    }

    // Do we really need to rescale?
    if( width != image->Width || height != image->Height )
    {
        // Allocate memory for new (upsampled) image data
        newsize = width * height * image->BytesPerPixel;
        data = (unsigned char *) malloc( newsize );
        if( data == NULL )
        {
            free( image->Data );
            return GL_FALSE;
        }

        // Copy old image data to new image data with interpolation
        UpsampleImage( image->Data, data, image->Width, image->Height,
                       width, height, image->BytesPerPixel );

        // Free memory for old image data (not needed anymore)
        free( image->Data );

        // Set pointer to new image data, and set new image dimensions
        image->Data   = data;
        image->Width  = width;
        image->Height = height;
    }

    return GL_TRUE;
}


//************************************************************************
//****                    GLFW user functions                         ****
//************************************************************************

//========================================================================
// Read an image from a named file
//========================================================================

GLFWAPI int GLFWAPIENTRY glfwReadImage( const char *name, GLFWimage *img,
    int flags )
{
    _GLFWstream stream;

    // Start with an empty image descriptor
    img->Width         = 0;
    img->Height        = 0;
    img->BytesPerPixel = 0;
    img->Data          = NULL;

    // Open file
    if( !_glfwOpenFileStream( &stream, name, "rb" ) )
    {
        return GL_FALSE;
    }

    // We only support TGA files at the moment
    if( !_glfwReadTGA( &stream, img, flags ) )
    {
        _glfwCloseStream( &stream );
        return GL_FALSE;
    }

    // Close stream
    _glfwCloseStream( &stream );

    // Should we rescale the image to closest 2^N x 2^M resolution?
    if( !(flags & GLFW_NO_RESCALE_BIT) )
    {
        if( !RescaleImage( img ) )
        {
            return GL_FALSE;
        }
    }

    // Interpret BytesPerPixel as an OpenGL format
    switch( img->BytesPerPixel )
    {
        default:
        case 1:
            if( flags & GLFW_ALPHA_MAP_BIT )
            {
                img->Format = GL_ALPHA;
            }
            else
            {
                img->Format = GL_LUMINANCE;
            }
            break;
        case 3:
            img->Format = GL_RGB;
            break;
        case 4:
            img->Format = GL_RGBA;
            break;
    }

    return GL_TRUE;
}


//========================================================================
// Read an image file from a memory buffer
//========================================================================

GLFWAPI int GLFWAPIENTRY glfwReadMemoryImage( const void *data, long size, GLFWimage *img, int flags )
{
    _GLFWstream stream;

    // Start with an empty image descriptor
    img->Width         = 0;
    img->Height        = 0;
    img->BytesPerPixel = 0;
    img->Data          = NULL;

    // Open buffer
    if( !_glfwOpenBufferStream( &stream, (void*) data, size ) )
    {
        return GL_FALSE;
    }

    // We only support TGA files at the moment
    if( !_glfwReadTGA( &stream, img, flags ) )
    {
        _glfwCloseStream( &stream );
        return GL_FALSE;
    }

    // Close stream
    _glfwCloseStream( &stream );

    // Should we rescale the image to closest 2^N x 2^M resolution?
    if( !(flags & GLFW_NO_RESCALE_BIT) )
    {
        if( !RescaleImage( img ) )
        {
            return GL_FALSE;
        }
    }

    // Interpret BytesPerPixel as an OpenGL format
    switch( img->BytesPerPixel )
    {
        default:
        case 1:
            if( flags & GLFW_ALPHA_MAP_BIT )
            {
                img->Format = GL_ALPHA;
            }
            else
            {
                img->Format = GL_LUMINANCE;
            }
            break;
        case 3:
            img->Format = GL_RGB;
            break;
        case 4:
            img->Format = GL_RGBA;
            break;
    }

    return GL_TRUE;
}


//========================================================================
// Free allocated memory for an image
//========================================================================

GLFWAPI void GLFWAPIENTRY glfwFreeImage( GLFWimage *img )
{
    // Free memory
    if( img->Data != NULL )
    {
        free( img->Data );
        img->Data = NULL;
    }

    // Clear all fields
    img->Width         = 0;
    img->Height        = 0;
    img->Format        = 0;
    img->BytesPerPixel = 0;
}


//========================================================================
// Read an image from a file, and upload it to texture memory
//========================================================================

GLFWAPI int GLFWAPIENTRY glfwLoadTexture2D( const char *name, int flags )
{
    GLFWimage img;

    // Is GLFW initialized?
    if( !_glfw.window )
    {
        return GL_FALSE;
    }

    // Force rescaling if necessary
    if( glfwExtensionSupported("GL_ARB_texture_non_power_of_two") )
    {
        flags &= (~GLFW_NO_RESCALE_BIT);
    }

    // Read image from file
    if( !glfwReadImage( name, &img, flags ) )
    {
        return GL_FALSE;
    }

    if( !glfwLoadTextureImage2D( &img, flags ) )
    {
        return GL_FALSE;
    }

    // Data buffer is not needed anymore
    glfwFreeImage( &img );

    return GL_TRUE;
}


//========================================================================
// Read an image from a buffer, and upload it to texture memory
//========================================================================

GLFWAPI int  GLFWAPIENTRY glfwLoadMemoryTexture2D( const void *data, long size, int flags )
{
    GLFWimage img;

    // Is GLFW initialized?
    if( !_glfw.window )
    {
        return GL_FALSE;
    }

    // Force rescaling if necessary
    if( glfwExtensionSupported("GL_ARB_texture_non_power_of_two") )
    {
        flags &= (~GLFW_NO_RESCALE_BIT);
    }

    // Read image from file
    if( !glfwReadMemoryImage( data, size, &img, flags ) )
    {
        return GL_FALSE;
    }

    if( !glfwLoadTextureImage2D( &img, flags ) )
    {
        return GL_FALSE;
    }

    // Data buffer is not needed anymore
    glfwFreeImage( &img );

    return GL_TRUE;
}


//========================================================================
// Upload an image object to texture memory
//========================================================================

GLFWAPI int  GLFWAPIENTRY glfwLoadTextureImage2D( GLFWimage *img, int flags )
{
    GLint   UnpackAlignment, GenMipMap;
    int     level, format, AutoGen, newsize, n;
    unsigned char *data, *dataptr;

    // Is GLFW initialized?
    if( !_glfw.window )
    {
        return GL_FALSE;
    }

    // TODO: Use GL_MAX_TEXTURE_SIZE or GL_PROXY_TEXTURE_2D to determine
    //       whether the image size is valid.
    // NOTE: May require box filter downsampling routine.

    // Do we need to convert the alpha map to RGBA format (OpenGL 1.0)?
    int glMajor, glMinor;
    glfwGetGLVersion(&glMajor, &glMinor, NULL);
    if( (glMajor == 1) && (glMinor == 0) &&
        (img->Format == GL_ALPHA) )
    {
        // We go to RGBA representation instead
        img->BytesPerPixel = 4;

        // Allocate memory for new RGBA image data
        newsize = img->Width * img->Height * img->BytesPerPixel;
        data = (unsigned char *) malloc( newsize );
        if( data == NULL )
        {
            free( img->Data );
            return GL_FALSE;
        }

        // Convert Alpha map to RGBA
        dataptr = data;
        for( n = 0; n < (img->Width*img->Height); ++ n )
        {
            *dataptr ++ = 255;
            *dataptr ++ = 255;
            *dataptr ++ = 255;
            *dataptr ++ = img->Data[n];
        }

        // Free memory for old image data (not needed anymore)
        free( img->Data );

        // Set pointer to new image data
        img->Data = data;
    }

    // Set unpack alignment to one byte
    _glfw.glGetIntegerv( GL_UNPACK_ALIGNMENT, &UnpackAlignment );
    _glfw.glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // Should we use automatic mipmap generation?
    AutoGen = ( flags & GLFW_BUILD_MIPMAPS_BIT ) &&
              glfwExtensionSupported("GL_SGIS_generate_mipmap");

    // Enable automatic mipmap generation
    if( AutoGen )
    {
        _glfw.glGetTexParameteriv( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,
            &GenMipMap );
        _glfw.glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,
            GL_TRUE );
    }

    // Format specification is different for OpenGL 1.0
    if( glMajor == 1 && glMinor == 0 )
    {
        format = img->BytesPerPixel;
    }
    else
    {
        format = img->Format;
    }

    // Upload to texture memeory
    level = 0;
    do
    {
        // Upload this mipmap level
        _glfw.glTexImage2D( GL_TEXTURE_2D, level, format,
            img->Width, img->Height, 0, format,
            GL_UNSIGNED_BYTE, (void*) img->Data );

        // Build next mipmap level manually, if required
        if( ( flags & GLFW_BUILD_MIPMAPS_BIT ) && !AutoGen )
        {
            level = HalveImage( img->Data, &img->Width,
                        &img->Height, img->BytesPerPixel ) ?
                    level + 1 : 0;
        }
    }
    while( level != 0 );

    // Restore old automatic mipmap generation state
    if( AutoGen )
    {
        _glfw.glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,
            GenMipMap );
    }

    // Restore old unpack alignment
    _glfw.glPixelStorei( GL_UNPACK_ALIGNMENT, UnpackAlignment );

    return GL_TRUE;
}

