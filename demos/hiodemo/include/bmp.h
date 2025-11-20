/*---------------------------------------------------------------------*

Project:  definitions for BMP format 
File:     bmp.h

Copyright 1998, 1999 Nintendo.  All rights reserved.

These coded instructions, statements and computer programs contain
proprietary information of Nintendo of America Inc. and/or Nintendo
Company Ltd., and are protected by Federal copyright law.  They may
not be disclosed to third parties or copied or duplicated in any form,
in whole or in part, without the prior written consent of Nintendo.

Change History:

  $Log: /Dolphin/build/demos/hiodemo/include/bmp.h $
    
    1     3/05/01 11:49a Hashida
    Initial revision.
    
    1     1/15/00 3:04a Hashida
    Initial revision
  
  $NoKeywords: $

-----------------------------------------------------------------------*/



#ifndef __BMP_H__
#define __BMP_H__


typedef struct
{   
    u32     bfOffBits;
    u32     width;
    u32     height;
    u16     biBitCount;
    u32     biCompression;
    u32     biSizeImage;

    u32     paletteOff;
} bmpInfo_s;

typedef struct
{
    u8      blue;
    u8      green;
    u8      red;
    u8      reserved;
} rgbQuad_s;


BOOL openBmp(bmpInfo_s* bi, u8* header);

#ifdef XFB_SF
BOOL bmpToYCbCr(bmpInfo_s* bi, u8* rawData, u8* dest1, u8* dest2);
#else
BOOL bmpToYCbCr(bmpInfo_s* bi, u8* rawData, u8* dest);
#endif

#endif  // __TGA_H__
