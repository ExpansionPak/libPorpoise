/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - TV module for core loop manager
File:     coreTV.c

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// ========================================
// TV display module for core loop manager
// ========================================
// Initializes.
//  Initializes VI and allocates frame buffer.
//  Registers VSync interruption.
// Task.
//
// VSync interruption.


// HEADER INCLUDE
// ====================
#include <dolphin.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "coreTV.h"

// INTERNAL DEFINES
// ====================
// TV MODE
#define NTSC      // if you change here, you can see PAL or M/PAL
/* #define NON_INTERLACE */   // if you define this, you can see non-interlace
#if defined(NTSC)
# ifndef NON_INTERLACE
GXRenderModeObj* rmode = &GXNtsc480Int;
# else
GXRenderModeObj* rmode = &GXNtsc240Ds;
# endif
#elif defined(PAL)
# ifndef NON_INTERLACE
GXRenderModeObj* rmode = &GXPal528Int;
# else
GXRenderModeObj* rmode = &GXPal264Ds;
# endif
#else
# ifndef NON_INTERLACE
GXRenderModeObj* rmode = &GXMpal480Int;
# else
GXRenderModeObj* rmode = &GXMpal240Ds;
# endif
#endif
// color info.
typedef struct
{
    u32 Y;
    u32 Cb;
    u32 Cr;
} Color_s;

Color_s color[] = {
    { 180, 128, 128 },//White
    { 162,  44, 142 },//Yellow
    { 131, 156,  44 },//Cyan
    { 112,  72,  58 },//Green
    {  84, 184, 198 },//Magenta
    {  65, 100, 212 },//Red
    {  35, 212, 114 },//Blue
    {  16, 128, 128 },//Black
};

// INTERNAL VARIABLES
// ====================
//static u32    gFB_HT=720,gFB_WD=482,gSCREEN_WD=640,gSCREEN_HT=200;
static u32  fbWidth=0;  // Width of frame buffer.
static u32  fbSize=0;   // Size of frame buffer.
//
static u32 first = 1;
static u32 frame = 0;
static u32 code = 0;
static u8* xfb;
// for X frame buffer 
static u8*  xfb1;
static u8*  xfb2;
//
static CORE_TV_CALLBACK gCallback=NULL;

// CALLBACK PROTOTYPES
// ====================
void viRetraceCallback(u32 retraceCount);

// FUNCTION PROTOTYPES
// ====================
static void allocateFB( u32 fbSize );


// ============================================================
//  CORE API
// ============================================================
void CoreTV_Begin()
{
    // Calculates frame buffer size.
    // Each line width should be a multiple of 16.
    fbWidth = (u32)VIPadFrameBufferWidth(rmode->fbWidth) * VI_DISPLAY_PIX_SZ;
    fbSize = (u32)( fbWidth * rmode->xfbHeight );
    // Allocates frame buffer.
    allocateFB(fbSize);
    // Configures VI.
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

    // set callback
    VISetPostRetraceCallback( viRetraceCallback );
//  VISetPreRetraceCallback( viRetraceCallback );
}

void CoreTV_Task( void* dummy)
{
#pragma unused(dummy)
}


// ============================================================
//  API
// ============================================================

void CoreTV_SetCallback( CORE_TV_CALLBACK callback )
{
    gCallback = callback;
}

//  Converts color code to color value.
u32 ColorValue(int colorCode)
{
    return  ((color[(colorCode)].Y << 24) + \
            (color[(colorCode)].Cb << 16) + \
            (color[(colorCode)].Y << 8) + \
            color[(colorCode)].Cr);
}

//  Returns the width of frame buffer.
u32 tvGetFrameWidth()
{
    return fbWidth;
}

//  Returns the height of frame buffer.
u32 tvGetFrameHeight()
{
    return fbSize/fbWidth;
}


// ============================================================
//  CALLBACKS
// ============================================================

void viRetraceCallback(u32 retraceCount)
{
    retraceCount=retraceCount;

    //  Flips frame buffer.
    xfb = (frame & 0x1)? xfb2 : xfb1;
    //  Callback processing specified by user.
    if(gCallback)
        gCallback( xfb );
    //  Mandatory processing.
    DCStoreRange((void*)xfb, fbSize);
    VISetNextFrameBuffer((void*)xfb);
    //  First-time limited process.
    if (first == 1)
    {
        VISetBlack(FALSE);
        first = 0;
    }
    
    VIFlush();
    frame++;
}


// ============================================================
//  INTERNAL FUNCTIONS
// ============================================================

//  Allocates frame buffer with the specified size.
static void allocateFB(u32 fbSize)
{
    void*   arenaLo;
    void*   arenaHi;

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    // Allocates memory for frame buffer here.
    xfb1 = (u8*)OSRoundUp32B(arenaLo);
    xfb2 = (u8*)OSRoundUp32B(xfb1 + fbSize);

    arenaLo = (void*)(xfb1 + 2 * fbSize);
    OSSetArenaLo(arenaLo);

    // OSInitAlloc should only ever be invoked once.
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1); // 1 heap
    OSSetArenaLo(arenaLo);
    // The boundaries given to OSCreateHeap should be 32B aligned
    OSSetCurrentHeap(OSCreateHeap((void*)OSRoundUp32B(arenaLo),
                                  (void*)OSRoundDown32B(arenaHi)));
    // From here on out, OSAlloc and OSFree behave like malloc and free
    // respectively
    OSSetArenaLo(arenaLo = arenaHi);
}


// ============================================================
//  GRAPHIC PRIMITIVE
// ============================================================

//  Fills whole screen.
void primFill( u8* xfb, u32 colorVal )
{
    u8* ptr;
    u32 pixSize = (VI_DISPLAY_PIX_SZ << 1);
    for (ptr = xfb; ptr < xfb + fbSize; ptr += pixSize)
        *(u32*)ptr = colorVal;
}

//  Dots.
void primPoint(
    u8* xfb, u32 colorVal, 
    u32 x, u32 y )
{
    u8  *ptr = xfb + ((VI_DISPLAY_PIX_SZ<<1) * x) + (fbWidth * y);
    *(u32*)ptr = colorVal;
}

//  Colors the width of the specified scan line.
void primLine(
    u8* xfb, u32 colorVal, 
    u32 x, u32 y, u32 w )
{
    u8  *ptr,*sPtr;
    u32 pixSize = (VI_DISPLAY_PIX_SZ << 1);
    u32 lineSize = pixSize * w;

    sPtr = xfb + (pixSize * x) + (fbWidth * y);
    for (ptr = sPtr; ptr < sPtr + lineSize; ptr += pixSize)
        *(u32*)ptr = colorVal;
}

//  Draws a box.
void primBox(
    u8* xfb, u32 colorVal, 
    u32 x, u32 y, u32 w, u32 h )
{
    u8  *ptr,*sPtr;
    u32 pixSize = (VI_DISPLAY_PIX_SZ << 1);
    u32 lineSize = pixSize * w;
    u32 i;

    for(i=0;i<h;i++) {
        sPtr = xfb + (pixSize * x) + (fbWidth * (y+i));
        for (ptr = sPtr; ptr < sPtr + lineSize; ptr += pixSize)
            *(u32*)ptr = colorVal;
    }
}

//  Copies image.
void primPutAt(
    u8* xfb, u8* image,
    u32 x, u32 y, u32 w, u32 h )
{
    u8  *ptr,*sPtr;
    u32 pixSize = (VI_DISPLAY_PIX_SZ << 1);
    u32 lineSize = pixSize * w;
    u32 i;

    for(i=0;i<h;i++) {
        sPtr = xfb + (pixSize * x) + (fbWidth * (y+i));
        for (ptr = sPtr; ptr < sPtr + lineSize; ptr += pixSize)
            *(u32*)ptr = *(u32*)image,
            image += pixSize;
    }
}
