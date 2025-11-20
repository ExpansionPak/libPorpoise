/*---------------------------------------------------------------------*

Project:  bitmap reader
File:     bmp.c

Copyright 1998, 1999 Nintendo.  All rights reserved.

These coded instructions, statements and computer programs contain
proprietary information of Nintendo of America Inc. and/or Nintendo
Company Ltd., and are protected by Federal copyright law.  They may
not be disclosed to third parties or copied or duplicated in any form,
in whole or in part, without the prior written consent of Nintendo.

Change History:

 $Log: /Dolphin/build/tests/vi/src/bmp.c $
    
    1     1/15/00 3:04a Hashida
    Initial revision
    
 $NoKeywords: $

-----------------------------------------------------------------------*/


#include <dolphin.h>
#include <assert.h>
#include "bmp.h"


#define CLAMP(x,l,h) ((x > h) ? h : ((x < l) ? l : x))
#define TENT(a, b, c) (0.25 * (a) + 0.5 * (b) + 0.25 * (c))

// for endianness
#define REV16(x)      ((u16)( ( (x) >> 8 ) + ( ((x) & 0xff) << 8 ) ))
#define REV32(x)        ((u32)( ( ((x) & 0xff000000) >> 24)         \
                              + ( ((x) & 0x00ff0000) >> 8 )         \
                              + ( ((x) & 0x0000ff00) << 8 )         \
                              + ( ((x) & 0x000000ff) << 24 ) ))

// decode a tga file into a 'DecodedFile' structure

BOOL openBmp(bmpInfo_s* bi, u8* header)
{
    u8*     bInfoHeader;

    assert( header != NULL );

    // fill bmpInfo
    assert(*header == 'B');
    assert(*(header + 1) == 'M');
    
    bi->bfOffBits   = REV32(*(u32*)(header + 0x0a));

    bInfoHeader = header + 0x0e;

    bi->width       = REV32(*(u32*)(bInfoHeader + 0x04));
    bi->height      = REV32(*(u32*)(bInfoHeader + 0x08));
    bi->biBitCount  = REV16(*(u16*)(bInfoHeader + 0x0e));
    bi->biCompression = REV32(*(u32*)(bInfoHeader + 0x10));

    bi->paletteOff  = 0x0e + REV32(*(u32*)(bInfoHeader));

    if (bi->biBitCount == 24)
    {
        // there should be no rgbquad
        assert(bi->bfOffBits == bi->paletteOff);
    }
    else
    {
        assert(bi->bfOffBits ==
               bi->paletteOff + 2^bi->biBitCount * sizeof(rgbQuad_s));
    }
    
    return TRUE;
}

#ifdef XFB_SF
BOOL bmpToYCbCr(bmpInfo_s* bi, u8* rawData, u8* dest1, u8* dest2)
#else
BOOL bmpToYCbCr(bmpInfo_s* bi, u8* rawData, u8* dest)
#endif
{
    u8          r, g, b;
    u32         row, col;
    double      Y;
    double      Cb_pp, Cb_p, Cb;
    double      Cr_pp, Cr_p, Cr;
#ifdef XFB_SF
    u8*         dest;
#endif
    rgbQuad_s*  palette;
    u32         paletteNum;
    u32         bytesPerLine;
    u8*         imageData;
    u8*         imageDataStart;
    u8*         destStart;

    palette = (rgbQuad_s*)(rawData + bi->paletteOff);
    imageData = rawData + bi->bfOffBits;
    
    bytesPerLine = (bi->width * bi->biBitCount + 7) / 8;
    bytesPerLine = ((bytesPerLine + 3) / 4) * 4;

    // we start from the last line because BMP stores its image upside down
    imageData += bytesPerLine * (bi->height - 1);

    for(row = 0; row < bi->height; row++)
    {
#ifdef XFB_SF
        dest = ((row & 0x1) == 0)? dest1:dest2;
#endif        
        destStart = dest;
        imageDataStart = imageData;
        
        for(col = 0; col < bi->width; col++)
        {
            switch(bi->biBitCount)
            {
              case 1:           // monochrome
                paletteNum = (u32)( ((*imageData) >> (7 - col % 8)) & 1 );
                r = palette[paletteNum].red;
                g = palette[paletteNum].green;
                b = palette[paletteNum].blue;
                break;
                
              case 4:           // 16 colors
                paletteNum = (u32)(((*imageData) >> (4 - (col % 2)*4)) & 0x0f);
                r = palette[paletteNum].red;
                g = palette[paletteNum].green;
                b = palette[paletteNum].blue;
                break;
                
              case 8:           // 256 colors                                    
                paletteNum = (u32)*imageData;
                r = palette[paletteNum].red;
                g = palette[paletteNum].green;
                b = palette[paletteNum].blue;
                break;
                
              case 24:          // true color
                // be careful about the order
                b = *imageData;
                g = *(imageData + 1);
                r = *(imageData + 2);
                break;
                
              default:
                OSReport("biBitCount %d is not supported\n", bi->biBitCount);
                OSHalt("");
                break;
            } // end switch(bi->biBitCount)

            Y  =  0.257 * r + 0.504 * g + 0.098 * b +  16.0 + 0.5;
            Cb = -0.148 * r - 0.291 * g + 0.439 * b + 128.0 + 0.5;
            Cr =  0.439 * r - 0.368 * g - 0.071 * b + 128.0 + 0.5;

            Y  = CLAMP(Y , 16, 235);
            Cb = CLAMP(Cb, 16, 240);
            Cr = CLAMP(Cr, 16, 240);

            *dest = (u8)Y;
            if (col & 1)
            {
                // col is odd
                *(dest - 1) = (u8)TENT(Cb_pp, Cb_p, Cb);
                *(dest + 1) = (u8)TENT(Cr_pp, Cr_p, Cr);
            }

            // in case of col == 0, Cb_p doesn't have a valid value
            // use Cb instead
            Cb_pp = (col == 0)? Cb : Cb_p;
            Cb_p  = Cb;
            Cr_pp = (col == 0)? Cr : Cr_p;
            Cr_p  = Cr;
            
            if (bi->biBitCount == 1)
            {
                if (col % 8 == 7)
                    imageData++;
            }
            else if (bi->biBitCount == 4)
            {
                if (col % 2 == 1)
                    imageData++;
            }
            else
                imageData += bi->biBitCount / 8;
            
            dest += 2;      // 2 bytes per pixel for 422YCbCr
                
        } // end 'col' for loop     
            
        // if width is odd, add one pixel because 422YCbCr doesn't
        // allow odd number for width
        if (col % 2 == 1)
        {
            *dest = (u8)Y;
            // col is odd (Cb = Cb_p, Cr = Cr_p)
            *(dest - 1) = (u8)TENT(Cb_pp, Cb_p, Cb);
            *(dest + 1) = (u8)TENT(Cr_pp, Cr_p, Cr);
            dest += 2;

            col++;
        }
        
        for( ; col < (bi->width + 15) / 16 * 16; col += 2)
        {
            // fill rest by black so that the next line is 
            // aligned by 32bytes
            *dest++ = 16;
            *dest++ = 128;
            *dest++ = 16;
            *dest++ = 128;
        }
            
        dest = destStart + (bi->width + 15) / 16 * 16 * 2;
        imageData = imageDataStart - bytesPerLine;

#ifdef XFB_SF
        if ( (row & 0x1) == 0 )
            dest1 = dest;
        else
            dest2 = dest;
#endif        
    } // end 'row' for loop

    // if height is odd, add a black line on the bottom
    if (row % 2 == 1)
    {
        for(col = 0; col < (bi->width + 15) / 16 * 16; col += 2)
        {
            *dest++ = 16;
            *dest++ = 128;
            *dest++ = 16;
            *dest++ = 128;
        }
    }

    return TRUE;
}

