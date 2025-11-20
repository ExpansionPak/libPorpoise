/*---------------------------------------------------------------------------*
  Project:  simple color test
  File:     color.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/videmo/src/color.c $
    
    9     3/01/00 5:41p Hashida
    Moved NTSC definition from makefile to .c's
    
    8     2/18/00 10:46a Hashida
    Changed the code suitable for demos.
    
    7     1/28/00 10:37p Hashida
    Added VIFlush.
    Changed to wait for two frames when switching mode.
    
    6     1/26/00 3:56p Hashida
    Changed to use VIConfigure.
    
    5     1/21/00 2:15a Hashida
    Make it work for non-interlace mode
    
    4     1/12/00 6:56p Hashida
    VI_DEBUG is not allowed as an argument for VIConfigureTVScreen
    (the API decides whether or not debug mode should be used).
    
    3     1/11/00 11:06a Hashida
    It doesn't work temporarily so fixed.
    
    2     12/24/99 11:12a Hashida
    Fixed initial values.
    
    1     12/02/99 4:05p Hashida
    Initial revision
    
    2     11/30/99 7:01p Hashida
    Added a new test for single field frame buffer.
    
    2     11/17/99 3:31p Hashida
    Removed DS mode test when Marlin since Marlin doesn't support DS mode
    for now.
    
    
    1     11/12/99 5:24p Hashida
    Initial revision.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define NTSC      // if you change here, you can see PAL or M/PAL
/* #define NON_INTERLACE */   // if you define this, you can see non-interlace

#if defined(NTSC)
#ifndef NON_INTERLACE
GXRenderModeObj* rmode = &GXNtsc480Int;
#else
GXRenderModeObj* rmode = &GXNtsc240Ds;
#endif

#elif defined(PAL)

#ifndef NON_INTERLACE
GXRenderModeObj* rmode = &GXPal528Int;
#else
GXRenderModeObj* rmode = &GXPal264Ds;
#endif

#else

#ifndef NON_INTERLACE
GXRenderModeObj* rmode = &GXMpal480Int;
#else
GXRenderModeObj* rmode = &GXMpal240Ds;
#endif

#endif

// for X frame buffer 
static u8*  xfb1;
static u8*  xfb2;

typedef struct
{
    u32         Y;
    u32         Cb;
    u32         Cr;
    
} Color_s;

Color_s color[] = {
    { 180, 128, 128 },
    { 162,  44, 142 },
    { 131, 156,  44 },
    { 112,  72,  58 },
    {  84, 184, 198 },
    {  65, 100, 212 },
    {  35, 212, 114 },
    {  16, 128, 128 },
};

void allocateFB(u32 fbSize);
void fillColor(u32 code, u32 fbSize, u8* xfb);

/*---------------------------------------------------------------------------*
    Name:               allocateFB
    
    Description:        Allocates memory for two frame buffers
                                
    Arguments:          fbSize      frame buffer size
    
    Returns:            none
 *---------------------------------------------------------------------------*/
void allocateFB(u32 fbSize)
{
    void*       arenaLo;

    arenaLo = OSGetArenaLo();

    // allocate memory for frame buffer here.
    xfb1 = (u8*)OSRoundUp32B(arenaLo);
    xfb2 = (u8*)OSRoundUp32B(xfb1 + fbSize);

    arenaLo = (void*)(xfb1 + 2 * fbSize);
    OSSetArenaLo(arenaLo);
}

/*---------------------------------------------------------------------------*
    Name:               fillColor
    
    Description:        fill frame buffer by a single color
                                
    Arguments:          code        color code
                        fbSize      size of the frame buffer
                        xfb         frame buffer address

    Returns:            none
 *---------------------------------------------------------------------------*/
void fillColor(u32 code, u32 fbSize, u8* xfb)
{
    u32         colorVal;
    u8*         ptr;
    
    colorVal = (color[code].Y << 24)
        + (color[code].Cb << 16)
        + (color[code].Y << 8)
        + color[code].Cr;

    for (ptr = xfb; ptr < xfb + fbSize; ptr += VI_DISPLAY_PIX_SZ * 2)
        *(u32*)ptr = colorVal;

    DCStoreRange((void*)xfb, fbSize);
}

void main(void)
{
    u32         frame;
    u32         code;
    u32         fbSize;
    u8*         xfb;
    u32         first;

    OSInit();
    VIInit();

    // Calculate frame buffer size.
    // Note that each line width should be a multiple of 16.
    fbSize = (u32)(VIPadFrameBufferWidth(rmode->fbWidth)
        * rmode->xfbHeight * VI_DISPLAY_PIX_SZ);

    allocateFB(fbSize);

    VIConfigure(rmode);

    // Need to "flush" so that the VI changes so far takes effect
    // from the following field
    VIFlush();
    VIWaitForRetrace();

    // Since the tv mode is interlace after VIInit,
    // we need to wait for one more frame to make sure
    // that the mode is switched from interlace to non-interlace
#ifdef NON_INTERLACE
    VIWaitForRetrace();
#endif

    first = 1;
    frame = 0;
    code = 0;

    while(1)
    {
        xfb = (frame & 0x1)? xfb2 : xfb1;
        
        if (frame % 60 == 0)
        {
            code = (code + 1) % 8;
        }

        fillColor(code, fbSize, xfb);

        VISetNextFrameBuffer((void*)xfb);

        if (first == 1)
        {
            VISetBlack(FALSE);
            first = 0;
        }
        
        VIFlush();
        VIWaitForRetrace();

        frame++;
    }
}
