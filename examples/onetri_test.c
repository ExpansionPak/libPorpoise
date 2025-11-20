/**
 * @file onetri_test.c
 * @brief Simple test program for GX initialization and frame loop
 * 
 * Tests:
 * - OSInit()
 * - VIInit() (creates OpenGL context)
 * - GXInit() (initializes GX graphics system)
 * - Frame loop with GX function calls (to see stub messages)
 */

#include <dolphin/os.h>
#include <dolphin/vi.h>
#include <dolphin/gx.h>
#include "../src/gfx/gx_helpers.h"  // For gfx_begin_frame, gfx_end_frame, gfx_render
#include "../src/gfx/gx_state.h"  // For GXGetState
#include "../src/gfx/gx_gl.h"  // For OpenGL
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    OSReport("==================================\n");
    OSReport("onetri_test - GX Initialization Test\n");
    OSReport("==================================\n");
    
    // Step 1: Initialize OS
    OSReport("\n[1/3] Initializing OS...\n");
    OSInit();
    OSReport("  OS initialized successfully\n");
    
    // Step 2: Initialize VI (creates OpenGL context)
    OSReport("\n[2/3] Initializing VI (Video Interface)...\n");
    VIInit();
    OSReport("  VI initialized successfully\n");
    OSReport("  OpenGL context created\n");
    
    // Step 3: Initialize GX (graphics system)
    OSReport("\n[3/3] Initializing GX (Graphics)...\n");
    GXFifoObj* fifo = GXInit(NULL, 0);
    if (fifo == NULL) {
        OSReport("  GX initialized successfully (returned NULL as expected)\n");
    } else {
        OSReport("  GX initialized successfully (fifo=%p)\n", fifo);
    }
    
    OSReport("\n==================================\n");
    OSReport("All systems initialized!\n");
    OSReport("==================================\n");
    OSReport("\nStarting frame loop...\n");
    OSReport("Watch for [GX STUB] messages showing unimplemented functions\n");
    OSReport("Close window or press ESC to exit\n\n");
    
    // Frame loop - call GX functions to see stub messages
    BOOL running = TRUE;
    u32 frameCount = 0;
    
    while (running) {
        // Handle window close events (SDL_QUIT)
        // Note: Input should be handled via PAD API, not SDL events
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = FALSE;
            }
        }
        
        // Wait for retrace (VSync)
        VIWaitForRetrace();
        
        // Call GX functions (these will trigger stub messages)
        // This mimics what a real game would do each frame
        
        // Begin frame (Aurora's begin_frame)
        gfx_begin_frame();
        
        // Set clear color (like Aurora's example)
        GXColor clearColor = {.r = 0, .g = 0, .b = 100, .a = 255};
        GXSetCopyClear(clearColor, GX_MAX_Z24);
        
        // Apply GX state and clear screen (temporary - should be in render)
        GXStateApply();
        
        // Check for OpenGL errors before clearing
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            OSReport("[GX ERROR] OpenGL error before glClear: 0x%04X\n", err);
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Check for OpenGL errors after clearing
        err = glGetError();
        if (err != GL_NO_ERROR) {
            OSReport("[GX ERROR] OpenGL error after glClear: 0x%04X\n", err);
        }
        
        // Debug logging removed to reduce spam
        
        // Call a few more common setup functions
        GXFlush();
        GXDrawDone();
        
        // End frame and render (Aurora's end_frame and render)
        gfx_end_frame();
        gfx_render();
        
        // Copy to display (Aurora's GXCopyDisp equivalent)
        GXCopyDisp(NULL, GX_FALSE);
        
        // Swap buffers to show the frame
        SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
        
        // Add some more GX calls to see what else needs implementation
        // (These will show stub messages)
        if (frameCount == 1) {
            // Only call these once to avoid spam
            GXSetViewport(0, 0, 640, 480, 0.0f, 1.0f);
            GXSetScissor(0, 0, 640, 480);
            GXSetCullMode(GX_CULL_BACK);
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
            
            // Call some unimplemented functions to see stub alerts
            // GXSetProjection(NULL, GX_PERSPECTIVE);  // Commented out - NULL matrix causes error
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, clearColor);
            GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        }
        
        frameCount++;
        
        // Log every 60 frames (once per second at 60fps)
        if (frameCount % 60 == 0) {
            OSReport("[Frame %u] Still running... (close window to exit)\n", frameCount);
        }
        
        // Limit to ~10 seconds for testing (600 frames at 60fps)
        if (frameCount >= 600) {
            OSReport("\nReached 10 seconds, exiting...\n");
            running = FALSE;
        }
    }
    
    OSReport("\nShutting down...\n");
    OSReport("Total frames rendered: %u\n", frameCount);
    
    return 0;
}

