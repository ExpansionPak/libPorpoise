/*---------------------------------------------------------------------------*
  Project:  libPorpoise Demo Library
  File:     DEMOInit.c
  
  Demo initialization framework - stubs that forward to libPorpoise functions.
  
  Based on Nintendo's Revolution SDK demo library.
 *---------------------------------------------------------------------------*/

#include <dolphin/demo/demo.h>
#include <dolphin/os.h>
#include <dolphin/vi.h>
#include <dolphin/gx.h>
#include <dolphin/dvd.h>
#include <string.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*
 * Static variables
 *---------------------------------------------------------------------------*/
static GXBool  DemoFirstFrame = GX_TRUE;

#define DEFAULT_FIFO_SIZE (256 * 1024)

void*         DemoFifoBuffer;
GXFifoObj*    DemoFifoObj;

static GXRenderModeObj *Rmode;
static GXRenderModeObj Rmodeobj;

static u32 allocatedFrameBufferSize = 0;

/*---------------------------------------------------------------------------*
 * Global variables
 *---------------------------------------------------------------------------*/
void*   DemoFrameBuffer1;
void*   DemoFrameBuffer2;
void*   DemoCurrentBuffer;

// Set to 1 before DEMOInit() if MEMHeap is prefered rather than OSHeap.
u32  DemoUseMEMHeap = 0;

// Set to TRUE before DEMOInit() if XFBs should be in MEM2 rather than MEM1.
BOOL DemoUseMEM2XFB = FALSE;

// Allocators (stub - not used on PC)
void* DemoAllocator1 = NULL;
void* DemoAllocator2 = NULL;

/*---------------------------------------------------------------------------*
 * Functions
 *---------------------------------------------------------------------------*/

void DEMOInit_Real(GXRenderModeObj *mode)
{
    // Initializes OS.
    // OSInit();    // called inside the startup routine.

    // Initializes disc drive interface.
    DVDInit();      // Initializes disc.
    
    // Initializes video interface.
    VIInit();

    // Initializes game PADs (controllers)
    DEMOPadInit();  // PADInit() is called inside.

    // Set up rendering mode
    // (which reflects the GX/VI configurations and XFB size below)
    DEMOSetRenderMode(mode);

    // Memory configuration (framebuffers / GX Fifo / heap)
    DEMOConfigureMem();

    // Initializes graphics
    DemoFifoObj = GXInit(DemoFifoBuffer, DEFAULT_FIFO_SIZE);
    DEMOInitGX();

    // Starts VI
    DEMOStartVI();
}

void DEMOInit(GXRenderModeObj *mode)
{
    DEMOInit_Real(mode);
}

void DEMOSetRenderMode(GXRenderModeObj* mode)
{
    // If an application specific render mode is provided,
    // override the default render mode
    if (mode != NULL) 
    {
        Rmodeobj = *mode;
        Rmode    = &Rmodeobj;
    }
    else
    {
        // On PC, use a default render mode
        // TODO: Map TV format to render mode if needed
        // For now, use a simple default
        memset(&Rmodeobj, 0, sizeof(Rmodeobj));
        Rmodeobj.fbWidth = 640;
        Rmodeobj.efbHeight = 480;
        Rmodeobj.xfbHeight = 480;
        Rmode = &Rmodeobj;
    }
}

void DEMOConfigureMem(void)
{
    u32 fbSize;

    /*----------------------------------------------------------------*
     *  Allocate GX FIFO buffer                                      *
     *----------------------------------------------------------------*/
    // On PC, just allocate from heap
    DemoFifoBuffer = malloc(DEFAULT_FIFO_SIZE);
    if (!DemoFifoBuffer) {
        OSPanic("DEMOInit.c", __LINE__, "Failed to allocate FIFO buffer\n");
    }

    /*----------------------------------------------------------------*
     *  Allocate external framebuffers                                *
     *----------------------------------------------------------------*/
    // On PC, framebuffers are handled by OpenGL/SDL
    // Allocate dummy buffers for compatibility
    fbSize = 640 * 480 * 2; // RGB565 = 2 bytes per pixel
    allocatedFrameBufferSize = fbSize;
    DemoFrameBuffer1 = malloc(fbSize);
    DemoFrameBuffer2 = malloc(fbSize);
    
    if (!DemoFrameBuffer1 || !DemoFrameBuffer2) {
        OSPanic("DEMOInit.c", __LINE__, "Failed to allocate frame buffers\n");
    }
    
    memset(DemoFrameBuffer1, 0, fbSize);
    memset(DemoFrameBuffer2, 0, fbSize);
}

void DEMOInitGX(void)
{
    u16     fbWidth;
    u16     xfbHeight;

    /*----------------------------------------------------------------*
     *  GX configuration by a render mode obj                         *
     *----------------------------------------------------------------*/
    fbWidth = Rmode->fbWidth;
    if (fbWidth > 640) {
        fbWidth = 640;
    }
    xfbHeight = Rmode->xfbHeight;
    if (xfbHeight > 528) {
        xfbHeight = Rmode->efbHeight;
    }

    // Forward to our GX functions
    GXSetViewport(0.0F, 0.0F, (f32)fbWidth, (f32)Rmode->efbHeight, 
                  0.0F, 1.0F);
    GXSetScissor(0, 0, (u32)fbWidth, (u32)Rmode->efbHeight);

    // Clear embedded framebuffer for the first frame
    GXCopyDisp(DemoFrameBuffer2, GX_TRUE);
}

void DEMOStartVI(void)
{
    // Configure VI with given render mode
    VIConfigure(Rmode);

    // Double-buffering initialization
    VISetNextFrameBuffer(DemoFrameBuffer1);
    DemoCurrentBuffer = DemoFrameBuffer2;

    // Tell VI device driver to write the current VI settings so far
    VIFlush();
    
    // Wait for retrace to start first frame
    VIWaitForRetrace();
}

void DEMOBeforeRender(void)
{
    // Set up viewport
    if (Rmode->field_rendering)
    {
        GXSetViewportJitter(
          0.0F, 0.0F, (float)Rmode->fbWidth, (float)Rmode->efbHeight, 
          0.0F, 1.0F, VIGetNextField());
    }
    else
    {
        GXSetViewport(
          0.0F, 0.0F, (float)Rmode->fbWidth, (float)Rmode->efbHeight, 
          0.0F, 1.0F);
    }

    // Invalidate vertex cache in GP
    GXInvalidateVtxCache();
    // Invalidate texture cache in GP
    GXInvalidateTexAll();
}

void DEMODoneRender(void)
{
    // Set Z/Color update to make sure eFB will be cleared at GXCopyDisp.
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    
    // Issue display copy command
    GXCopyDisp(DemoCurrentBuffer, GX_TRUE);

    // Wait until everything is drawn and copied into XFB.
    GXDrawDone();

    // Set the next frame buffer
    DEMOSwapBuffers();
}

void DEMOSwapBuffers(void)
{
    // Display the buffer which was just filled by GXCopyDisplay
    VISetNextFrameBuffer(DemoCurrentBuffer);

    // If this is the first frame, turn off VIBlack
    if(DemoFirstFrame)
    {
        VISetBlack(FALSE);
        DemoFirstFrame = GX_FALSE;
    }
    
    // Tell VI device driver to write the current VI settings so far
    VIFlush();
    
    // Wait for vertical retrace.
    VIWaitForRetrace();
    
    // Swap buffers
    if (DemoCurrentBuffer == DemoFrameBuffer1)
    {
        DemoCurrentBuffer = DemoFrameBuffer2;
        VISetNextFrameBuffer(DemoFrameBuffer1);
    }
    else
    {
        DemoCurrentBuffer = DemoFrameBuffer1;
        VISetNextFrameBuffer(DemoFrameBuffer2);
    }
}

GXRenderModeObj* DEMOGetRenderModeObj(void)
{
    return Rmode;
}

void* DEMOGetCurrentBuffer(void)
{
    return DemoCurrentBuffer;
}

void DEMOReInit(GXRenderModeObj *mode)
{
    // Re-initialize with new render mode
    DEMOSetRenderMode(mode);
    DEMOConfigureMem();
    DEMOInitGX();
    DEMOStartVI();
}

