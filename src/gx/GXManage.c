/*---------------------------------------------------------------------------*
  Name:         GXManage.c
  
  Description:  GX initialization and management functions
  
  Notes:        Starting with minimal implementation - GXInit
 *---------------------------------------------------------------------------*/

#include <dolphin/gx/GXManage.h>
#include <dolphin/gx/GXFifo.h>
#include <dolphin/os.h>
#include <string.h>
#include "../gfx/gx_init.h"

/*---------------------------------------------------------------------------*
  Name:         GXInit
  
  Description:  Initializes the GX graphics system. This is the first GX
                function that must be called before using any other GX
                functions.
                
                Similar to Aurora's approach:
                - GXInit() maintains API compatibility (returns GXFifoObj*)
                - Actual initialization happens in GXInitGraphics()
                - Sets up OpenGL backend and GX state
  
  Arguments:    base - Pointer to FIFO buffer (can be NULL on PC)
                size - Size of FIFO buffer (ignored on PC for now)
  
  Returns:      Pointer to GXFifoObj (or NULL on PC)
 *---------------------------------------------------------------------------*/

static BOOL s_gxInitialized = FALSE;
static GXDrawDoneCallback s_drawDoneCallback = NULL;

GXFifoObj* GXInit(void* base, u32 size) {
    (void)base;
    (void)size;
    /* Copy from Aurora exactly: GXInit is just a stub that returns NULL */
    /* Aurora's code: GXFifoObj* GXInit(void* base, u32 size) { return NULL; } */
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         GXFlush
  
  Description:  Flushes the GX command FIFO, ensuring all pending commands
                are processed.
                
                On original hardware: Waits for FIFO to empty
                On PC: No-op (similar to Aurora)
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXFlush(void) {
    // No-op on PC (Aurora's approach)
}

/*---------------------------------------------------------------------------*
  Name:         GXSetDrawDoneCallback
  
  Description:  Sets the draw done callback function. This callback is called
                when GXDrawDone() or GXSetDrawDone() is invoked.
                
                Similar to Aurora's implementation.
  
  Arguments:    cb - Callback function to set (NULL to clear)
  
  Returns:      Previous callback function (or NULL if none)
 *---------------------------------------------------------------------------*/
GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback cb) {
    GXDrawDoneCallback old = s_drawDoneCallback;
    s_drawDoneCallback = cb;
    return old;
}

/*---------------------------------------------------------------------------*
  Name:         GXDrawDone
  
  Description:  Signals that drawing is complete. Calls the draw done callback
                if one is registered.
                
                On original hardware: Waits for drawing to complete
                On PC: Just calls callback (similar to Aurora)
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXDrawDone(void) {
    if (s_drawDoneCallback != NULL) {
        s_drawDoneCallback();
    }
}

/*---------------------------------------------------------------------------*
  Name:         GXSetDrawDone
  
  Description:  Same as GXDrawDone() - signals that drawing is complete.
                Calls the draw done callback if one is registered.
                
                Similar to Aurora's implementation.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXSetDrawDone(void) {
    if (s_drawDoneCallback != NULL) {
        s_drawDoneCallback();
    }
}

/*---------------------------------------------------------------------------*
  Name:         GXPixModeSync
  
  Description:  Synchronizes pixel mode settings.
                
                On original hardware: Waits for pixel engine to be ready
                On PC: No-op (similar to Aurora)
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXPixModeSync(void) {
    // No-op on PC (Aurora's approach)
}

/*---------------------------------------------------------------------------*
  Name:         GXTexModeSync
  
  Description:  Synchronizes texture mode settings.
                
                On original hardware: Waits for texture unit to be ready
                On PC: No-op (similar to Aurora)
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXTexModeSync(void) {
    // No-op on PC (Aurora's approach)
}


