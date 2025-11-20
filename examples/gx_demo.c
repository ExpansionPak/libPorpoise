/**
 * @file gx_demo.c
 * @brief GX Graphics Demo - Based on Aurora's simple.c example
 * 
 * This demo demonstrates basic GX graphics functionality:
 * - Initialization (OS, VI, GX)
 * - Frame loop with rendering
 * - Clear color setup
 * - Viewport and scissor configuration
 * - Basic state management
 * 
 * Adapted from Aurora's examples/simple.c to use libPorpoise's
 * standard Dolphin SDK API (OSInit, VIInit, GXInit).
 */

#include <dolphin/os.h>
#include <dolphin/vi.h>
#include <dolphin/gx.h>
#include "../src/gfx/gx_helpers.h"  // For gfx_begin_frame, gfx_end_frame, gfx_render
#include "../src/gfx/gx_state.h"    // For GXGetState
#include "../src/gfx/gx_gl.h"       // For OpenGL
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Draw function - called each frame
 * 
 * Sets up GX state and renders a frame.
 * This is similar to Aurora's draw() function.
 */
static void draw(void) {
    // Set clear color (similar to Aurora's example)
    GXColor clearColor = {
        .r = 0,
        .g = 0,
        .b = 100,
        .a = 255,
    };
    GXSetCopyClear(clearColor, GX_MAX_Z24);
    
    // Apply GX state to OpenGL
    GXStateApply();
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // TODO: Add actual drawing commands here
    // (GXBegin, GXPosition, GXColor, GXEnd, etc.)
}

/**
 * @brief Main function
 * 
 * Initializes systems and runs the main loop.
 * Similar to Aurora's main() but using libPorpoise API.
 */
int main(void) {
    OSReport("==================================\n");
    OSReport("GX Graphics Demo\n");
    OSReport("Based on Aurora's simple.c example\n");
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
    OSReport("Starting main loop...\n");
    OSReport("Close window to exit\n");
    OSReport("==================================\n\n");
    
    // Main loop - similar to Aurora's event loop
    BOOL running = TRUE;
    BOOL paused = FALSE;
    u32 frameCount = 0;
    
    while (running) {
        // Handle SDL events (window close, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = FALSE;
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                        paused = TRUE;
                    } else if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
                        paused = FALSE;
                    }
                    break;
                default:
                    break;
            }
        }
        
        // Skip frame if paused or exiting
        if (!running || paused) {
            continue;
        }
        
        // Wait for retrace (VSync)
        VIWaitForRetrace();
        
        // Begin frame (similar to Aurora's aurora_begin_frame())
        gfx_begin_frame();
        
        // Draw the frame
        draw();
        
        // End frame and render (similar to Aurora's aurora_end_frame())
        gfx_end_frame();
        gfx_render();
        
        // Copy to display
        GXCopyDisp(NULL, GX_FALSE);
        
        // Swap buffers to show the frame
        SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
        
        frameCount++;
        
        // Log every 60 frames (once per second at 60fps)
        if (frameCount % 60 == 0) {
            OSReport("[Frame %u] Running... (close window to exit)\n", frameCount);
        }
    }
    
    OSReport("\n==================================\n");
    OSReport("Shutting down...\n");
    OSReport("Total frames rendered: %u\n", frameCount);
    OSReport("==================================\n");
    
    return 0;
}

