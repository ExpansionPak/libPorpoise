/*---------------------------------------------------------------------------*
  Project: viewer for BMP file
  File:    viewer.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/videmo/src/viewer.c $
    
    15    01/11/08 17:02 Hashida
    Added PAL support.
    
    14    5/10/01 3:18p Carl
    Added button control feature, XFB load feature.
    
    13    6/07/00 5:39p Tian
    Updated idle function API
    
    12    5/15/00 1:37p Hashida
    Changed maximum vi height from 482 to 480 for NTSC and MPAL.
    
    11    3/01/00 5:29p Hashida
    Modified so that it doesn't have to include vicommon.h.
    
    10    3/01/00 5:17p Hashida
    Removed tga file view.
    
    9     3/01/00 1:14p Hashida
    Renamed dirent->filename to name.
    
    8     2/28/00 7:26p Hashida
    Added to set viHeight for assertion.
    
    7     2/04/00 11:43a Hashida
    Changed to do the read and conversion as background.
    Support for single field interlaced images.
    
    6     1/28/00 9:02p Hashida
    Added VIFlush.
    Added code to safely change modes.
    Changed because VI APIs doesn't set registers immediately anymore.
    
    5     1/26/00 3:56p Hashida
    Changed to use VIConfigure.
    
    4     1/21/00 2:15a Hashida
    Make it work for non-interlace mode
    
    3     1/20/00 2:57p Hashida
    Make it work for NTSC_NONINTERLACE
    
    2     1/17/00 10:18a Hashida
    Removed a warning.
    
    1     1/15/00 3:04a Hashida
    Initial revision
    
    1     1/13/00 12:17p Hashida
    Initial revision
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*
 * Open files under /pictures and show it on screen
 */
#include <dolphin.h>
#include <string.h>
#include <ctype.h>
#include "bmp.h"

// for X frame buffer 
static u8*  xfb1;
static u8*  xfb2;

u32         first=1; // used to bring up first image immediately
u32         mode=0;  // flip mode; 0 = timer, 1 = button control

#if defined(NTSC) || defined(MPAL)

#define XFB_WD          720
#define XFB_HT          480

#define SCREEN_WD       720
#ifndef NON_INTERLACE
#define SCREEN_HT       480
#else   // #ifndef NON_INTERLACE
#define SCREEN_HT       240
#endif  // #ifndef NON_INTERLACE

#else   // #if defined(NTSC) || defined(MPAL)

#define XFB_WD          720
#define XFB_HT          574

#define SCREEN_WD       720
#ifndef NON_INTERLACE
#define SCREEN_HT       574
#else   // #ifndef NON_INTERLACE
#define SCREEN_HT       287
#endif  // #ifndef NON_INTERLACE

#endif  // #if defined(NTSC) || defined(MPAL)


#define IS_BMP(p)   (! mystrcmp(p, "bmp"))
#define IS_BMA(p)   (! mystrcmp(p, "bma"))
#define IS_XFB(p)   (! mystrcmp(p, "xfb"))

static void MyOSInit( void );
static void printOneLevel(char* pathName);

#define TYPE_INIT       0
#define TYPE_BMP        1
#define TYPE_BMA        2
#define TYPE_XFB        3

u8   Stack[4096];

typedef struct
{
    // input
    u8*         image;      // processed image
    DVDDir*     dir;

    // output
    u32         height;     // image height
    u32         width;      // image width
    u8*         image2;     // processed image2 (for INT-SF mode)
    u32         imageType;
    BOOL        end;
} idleParam_s;

struct dirs_t
{
    struct dirs_t*  next;       // must be first
    DVDDirEntry     dirEntry;
};

typedef struct dirs_t       dirs;


// case insensitive strcmp
static int mystrcmp(char* str1, char* str2)
{
    while(*str1)
    {
        if (tolower(*str1) != tolower(*str2))
            return (tolower(*str1) - tolower(*str2));
        str1++;
        str2++;
    }
    
    return 0;
}


static void readConvertBmp(char* fileName, u8* dest, u32* width, u32* height)
{
    DVDFileInfo     finfo;
    u32             length;
    u8*             buf;
    bmpInfo_s       bInfo;

    if (FALSE == DVDOpen(fileName, &finfo))
    {
        OSReport("Can't open file %s\n", fileName);
        return;
    }
            
    length = DVDGetLength(&finfo);
            
    OSReport("F %9d %s\n", length, fileName);
            
    if( NULL == (buf = OSAlloc(OSRoundUp32B(length))) )
    {
        OSReport("Alloc failed. Exit\n");
        return;
    }

    if (OSRoundUp32B(length) !=
        DVDRead(&finfo, buf, (s32)OSRoundUp32B(length), 0))
    {
        OSReport("Error occurred when reading %s\n", fileName);
        OSHalt("");
    }
            
    if (FALSE == openBmp(&bInfo, buf))
    {
        OSReport("Failed to analyze %s as a bmp file\n",
                 fileName);
        OSHalt("");
    }
            
    OSReport("  bfOffBits: %d\n", bInfo.bfOffBits);
    OSReport("  width: %d\n", bInfo.width);
    OSReport("  height: %d\n", bInfo.height);
    OSReport("  biBitCount: %d\n", bInfo.biBitCount);
    OSReport("  biCompression: %d\n", bInfo.biCompression);
    OSReport("  biSizeImage: %d\n", bInfo.biSizeImage);
    OSReport("  paletteOff: %d\n", bInfo.paletteOff);
            
    if (FALSE == bmpToYCbCr(&bInfo, buf, dest))
    {
        OSReport("Failed to convert bmp to YCbCr\n");
        OSHalt("");
    }

    *width = (bInfo.width + 15) / 16 * 16;
    *height = (bInfo.height + 1) / 2 * 2;

    DVDClose(&finfo);
    OSFree(buf);
}


static void getRenderMode(GXRenderModeObj* rm, u16 dispPosX, u16 dispPosY,
                          u16 dispSizeX, VITVMode tvMode,
                          u16 xfbSizeX, u16 xfbSizeY,
                          VIXFBMode xfbMode)
{
    rm->viTVmode = tvMode;
    rm->fbWidth = xfbSizeX;
    rm->xfbHeight = xfbSizeY;
    rm->viXOrigin = dispPosX;
    rm->viYOrigin = dispPosY;
    rm->viWidth = dispSizeX;
    rm->xFBmode = xfbMode;

    rm->viHeight = (xfbMode == VI_XFBMODE_DF)? xfbSizeY : (u16)(xfbSizeY*2);
}

static void idle(void* param)
{
    DVDDirEntry     dirent;
    idleParam_s*    ip;
    u32             width;
    u32             height;
    char*       p;
    
    ip = (idleParam_s*)param;
    ip->end = FALSE;
    ip->imageType = 0;

    do
    {
        if (FALSE == DVDReadDir(ip->dir, &dirent))
        {
            ip->end = TRUE;
            return;
        }

        p = dirent.name;
        while(*p++ != '.')
            ;

        if (IS_BMP(p))
            ip->imageType = TYPE_BMP;
#ifndef NON_INTERLACE
        else if (IS_BMA(p))
            ip->imageType = TYPE_BMA;
#endif
        else if (IS_XFB(p))
            ip->imageType = TYPE_XFB;

    } while ( (ip->imageType == 0) || (dirent.isDir) );
    
    if (ip->imageType == TYPE_BMP)
    {
        readConvertBmp(dirent.name, ip->image, &width, &height);
    }
    else if (ip->imageType == TYPE_BMA)
    {
        u32         firstLength;
        u32         fnlength;
        char*       fn;
                
        readConvertBmp(dirent.name, ip->image, &width, &height);

        firstLength = width * height * 2;

        // read "bmb"
        fnlength = strlen(dirent.name) + 1;
        if( NULL == (fn = OSAlloc(OSRoundUp32B(fnlength))) )
        {
            OSReport("Alloc failed. Exit\n");
            return;
        }

        strcpy(fn, dirent.name);
        fn[fnlength - 2] = 'b';

        OSReport("fn is %s\n", fn);

        readConvertBmp(fn, ip->image + firstLength, &width, &height);

        ip->image2 = ip->image + firstLength;
    }
    else if (ip->imageType == TYPE_XFB)
    {
        DVDFileInfo     finfo;
        u32 length;
        
        // read file into ip->image

        if (FALSE == DVDOpen(dirent.name, &finfo))
        {
            OSReport("Can't open file %s\n", dirent.name);
            return;
        }
            
        length = DVDGetLength(&finfo);
            
        OSReport("F %9d %s\n", length, dirent.name);

        width = 640;
        height = length / (640*2);

        if (OSRoundUp32B(length) !=
            DVDRead(&finfo, ip->image, (s32)OSRoundUp32B(length), 0))
        {
            OSReport("Error occurred when reading %s\n", dirent.name);
            OSHalt("");
        }
        DVDClose(&finfo);
    }

    if (height > SCREEN_HT)
        height = SCREEN_HT;

    if (ip->imageType != TYPE_BMA)
        DCStoreRange((void*)ip->image, width * height * 2);
    else    // store caches for two images
        DCStoreRange((void*)ip->image, width * height * 2 * 2);

    ip->height = height;
    ip->width = width;
}

static void printOneLevel(char* pathName)
{
    DVDDir          dir;
    dirs*           start = (dirs*)NULL;
    dirs*           curr;
    char            path[256];
    static u32      imageReady = 0;
    static u8*      xfb;
    static u32      frame = 0;
    static u32      prevCount;
    static u32      width;
    static u32      height;
    static u32      isBma;
    GXRenderModeObj rm;
    OSThread*       background;
    idleParam_s     ip;
    u32             startProcessNext;

    PADStatus       pstat[PAD_MAX_CONTROLLERS];
    u32             plast=0;

#pragma unused(pathName)

    curr = (dirs*)&start;
    
    if (FALSE == DVDOpenDir(".", &dir))
    {
        OSReport("Can't open dir %s\n", path);
        return;
    }
    
    xfb = (frame)? xfb2 : xfb1;

    startProcessNext = 1;
    while(1)
    {
        VIWaitForRetrace();

        if (startProcessNext)
        {
            ip.image = (xfb == xfb1)? xfb2 : xfb1;
            ip.dir = &dir;
            background = OSSetIdleFunction(idle, (void*)&ip,
                                           Stack + sizeof Stack,
                                           sizeof Stack);
            startProcessNext = 0;
        }
        
        PADRead(pstat);

        if ((pstat[0].button & PAD_BUTTON_B) &&
            !(plast & PAD_BUTTON_B)) 
        {
            mode = 1 - mode;
            if (mode) 
                OSReport("Button A control mode\n");
            else
                OSReport("Timer control mode\n");
        }

        if (OSGetIdleFunction() == 0)
        {
            if (ip.end)
                return;

            if ( (mode && (pstat[0].button & PAD_BUTTON_A) &&
                  !(plast & PAD_BUTTON_A)) || first ||
                 (!mode && (VIGetRetraceCount() - prevCount >= 300)) )
            {
                first = 0;

                prevCount = VIGetRetraceCount();
                
                frame ^= 1;
                xfb = (frame)? xfb2 : xfb1;

                width = ip.width;
                height = ip.height;
                isBma = (ip.imageType == TYPE_BMA);

#ifndef NON_INTERLACE
                if (isBma)
                    getRenderMode(&rm, (u16)((SCREEN_WD - width) / 2),
                                  (u16)((SCREEN_HT - height * 2) / 2),
                                  (u16)width,
#ifdef PAL
                                  VI_TVMODE_PAL_INT,
#else
                                  VI_TVMODE_NTSC_INT,
#endif
                                  (u16)width, (u16)height,
                                  VI_XFBMODE_SF);
                else
                    getRenderMode(&rm, (u16)((SCREEN_WD - width) / 2),
                                  (u16)((SCREEN_HT - height) / 2),
                                  (u16)width,
#ifdef PAL
                                  VI_TVMODE_PAL_INT,
#else
                                  VI_TVMODE_NTSC_INT,
#endif
                                  (u16)width, (u16)height,
                                  VI_XFBMODE_DF);
#else
                getRenderMode(&rm, (u16)((SCREEN_WD - width) / 2),
                              (u16)((SCREEN_HT - height) / 2),
                              (u16)width,
#ifdef PAL
                              VI_TVMODE_PAL_DS,
#else
                              VI_TVMODE_NTSC_DS,
#endif
                              (u16)width, (u16)height,
                              VI_XFBMODE_SF);
#endif

                VIConfigure(&rm);

                startProcessNext = 1;
                imageReady = 1;     // holds 1 after the first image gets ready
            }

        }
        
        plast = pstat[0].button;

        if (isBma)
        {
            u8*     xfbSF;
                
            xfbSF = xfb;

            if (VIGetNextField() == VI_FIELD_ABOVE)
                xfbSF = xfb;
            else
                xfbSF = xfb + width * height * 2;
                    
            VISetNextFrameBuffer(xfbSF);
        }
        else
            VISetNextFrameBuffer(xfb);

        if (imageReady)
        {
            VISetBlack(FALSE);
        }

        VIFlush();
        
    } // while(1)
    
} // void printOneLevel(char*)


void main(void)
{
    MyOSInit();
    
    PADInit();

    OSReport("Image File Viewer\n\n");
    OSReport("Press Button B to change flip mode\n\n");

    if (FALSE == DVDChangeDir("pictures"))
    {
        OSReport("Can't change dir to pictures\n");
        return;
    }

    while(1)
        printOneLevel(".");
    
    OSHalt("End of program");

    // NOT REACHED HERE
}


 /*---------------------------------------------------------------------------*
    Name:               MyOSInit
    
    Description:        Initialize the operating system.
                        Create a heap so we can use OSAlloc().
                                
    Arguments:          none
    
    Returns:            none
 *---------------------------------------------------------------------------*/
static void MyOSInit ( void )
{
    void*               arenaLo;
    void*               arenaHi;

    OSInit();
    DVDInit();
    VIInit();
    
    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    // allocate memory for frame buffer here.
    xfb1 = (u8*)OSRoundUp32B(arenaLo);
    xfb2 = (u8*)OSRoundUp32B(arenaLo)
                + XFB_HT * XFB_WD * VI_DISPLAY_PIX_SZ;
    arenaLo = (void*)(xfb1 + XFB_HT * XFB_WD * VI_DISPLAY_PIX_SZ * 2);
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

    return;
}
