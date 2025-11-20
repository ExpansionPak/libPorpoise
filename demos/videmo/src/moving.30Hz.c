/*---------------------------------------------------------------------------*
  Project:  single field frame buffer demo
  File:     moving.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/videmo/src/moving.30Hz.c $
    
    2     3/01/00 5:41p Hashida
    Moved NTSC definition from makefile to .c's
    
    1     2/18/00 10:39a Hashida
    initial checkin
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin.h>

#define NTSC      // if you change here, you can see PAL or M/PAL
/* #define NON_INTERLACE */   // if you define this, you can see non-interlace

#define FLOOR(x, a)         ( (x < a)? a : x )
#define CEIL(x, b)          ( (x > b)? b : x )
#define CLAMP(x, a, b)      (CEIL(FLOOR(x, a), b))

#if defined(NTSC)

GXRenderModeObj* rmode = &GXNtsc480Int;

#elif defined(PAL)

GXRenderModeObj* rmode = &GXPal528Int;

#else

GXRenderModeObj* rmode = &GXMpal480Int;

#endif

// for X frame buffer 
static u8*  xfbA;
static u8*  xfbB;

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
void draw(u32 code, u32 fbSize, u8* xfb);
void waitFrame(u32 count);

/*---------------------------------------------------------------------------*
    Name:               allocateFB
    
    Description:        Allocates memory for two frame buffers
                                
    Arguments:          fbSize      each frame buffer size
    
    Returns:            none
 *---------------------------------------------------------------------------*/
void allocateFB(u32 fbSize)
{
    void*       arenaLo;

    arenaLo = OSGetArenaLo();

    // allocate memory for frame buffer here.
    xfbA = (u8*)OSRoundUp32B(arenaLo);
    xfbB = (u8*)OSRoundUp32B(xfbA + fbSize);

    arenaLo = (void*)(xfbA + 2 * fbSize);
    OSSetArenaLo(arenaLo);
}

/*---------------------------------------------------------------------------*
    Name:               draw
    
    Description:        draw screen
                                
    Arguments:          count       frame count
                        fbSize      size of the frame buffer
                        xfb         frame buffer address

    Returns:            none
 *---------------------------------------------------------------------------*/
void draw(u32 count, u32 fbSize, u8* xfb)
{
    u32         i, j;
    u32         colorVal;
    u32         cb, cr;
    u32         y;
    u8*         fbStart;
    
    count *= 2;

    fbStart = xfb;
    // draw full image
    for (j = 0; j < rmode->viHeight; j++)
    {
        for (i = 0; i < rmode->fbWidth; i += 2)
        {
            // clamping is needed because Cb and Cr should be
            // in range [16, 240].
            y = 128;
            cb = CLAMP( (i + count*2) % 256, 16, 240);
            cr = CLAMP( (j + count) % 256, 16, 240);
            
            colorVal = (y << 24)
                + (cb << 16)
                + (y << 8)
                + cr;

            *(u32*)xfb = colorVal;

            // We handle two pixels at a time.
            xfb += VI_DISPLAY_PIX_SZ * 2;
        }
    }

    DCStoreRange((void*)fbStart, fbSize);
}

/*---------------------------------------------------------------------------*
    Name:               waitFrame
    
    Description:        wait until the specified retrace count comes.
                        if it's already in or after the frame, just pass.
                                
    Arguments:          count       frame count

    Returns:            none
 *---------------------------------------------------------------------------*/
void waitFrame(u32 count)
{
    BOOL enabled;

    enabled = OSDisableInterrupts();
    while (VIGetRetraceCount() < count)
        VIWaitForRetrace();
    OSRestoreInterrupts(enabled);
}

void main(void)
{
    u32         frame;
    u32         fbSize;
    u8*         xfb;
    u32         first;
    u32         count;

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

    count = VIGetRetraceCount();
    first = 1;
    frame = 0;

    while(1)
    {
        xfb = (frame & 0x1)? xfbB : xfbA;

        draw(frame, fbSize, xfb);
        VISetNextFrameBuffer((void*)xfb);

        if (first == 1)
        {
            VISetBlack(FALSE);
            first = 0;
        }
        
        // wait for some frames in case the rendering finishes too early
        waitFrame(count + 1);

        VIFlush();
        VIWaitForRetrace();

        count = VIGetRetraceCount();
        frame++;
    }
}
