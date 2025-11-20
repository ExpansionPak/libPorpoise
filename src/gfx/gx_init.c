/*---------------------------------------------------------------------------*
  gx_init.c - GX Graphics System Initialization
  
  Similar to Aurora's gfx::initialize(), this sets up the OpenGL backend
 *---------------------------------------------------------------------------*/

#include "gx_state.h"
#include "gx_helpers.h"
#include <dolphin/os.h>
#include <SDL.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* OpenGL headers - platform specific */
#ifdef _WIN32
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         GXInitGraphics
  
  Description:  Initializes the OpenGL graphics backend
                Similar to Aurora's gfx::initialize()
  
  Arguments:    None
  
  Returns:      TRUE on success, FALSE on failure
 *---------------------------------------------------------------------------*/
BOOL GXInitGraphics(void) {
    /* Initialize GX state */
    GXStateInit();
    
    /* Initialize command queue (similar to Aurora's initialization) */
    gfx_init_command_queue();
    
    /* Get OpenGL context from VI (VI should have created it) */
    /* For now, we assume VI has already set up the OpenGL context */
    
    OSReport("GXInitGraphics: Initializing OpenGL backend\n");
    
    /* Check if we have a valid OpenGL context */
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    if (!window) {
        OSReport("GXInitGraphics: Warning - No OpenGL context found. VI should initialize first.\n");
        return FALSE;
    }
    
    /* Get OpenGL version info */
    const char* glVersion = (const char*)glGetString(GL_VERSION);
    const char* glRenderer = (const char*)glGetString(GL_RENDERER);
    const char* glVendor = (const char*)glGetString(GL_VENDOR);
    
    if (glVersion) {
        OSReport("GXInitGraphics: OpenGL Version: %s\n", glVersion);
    }
    if (glRenderer) {
        OSReport("GXInitGraphics: OpenGL Renderer: %s\n", glRenderer);
    }
    if (glVendor) {
        OSReport("GXInitGraphics: OpenGL Vendor: %s\n", glVendor);
    }
    
    /* Set default OpenGL state */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    /* Clear color (black) */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    /* TODO: Create default buffers, shaders, etc. similar to Aurora */
    /* For now, we just set up basic OpenGL state */
    
    OSReport("GXInitGraphics: OpenGL backend initialized\n");
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GXShutdownGraphics
  
  Description:  Shuts down the graphics backend
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXShutdownGraphics(void) {
    /* Shutdown command queue */
    gfx_shutdown_command_queue();
    
    /* TODO: Cleanup OpenGL resources */
    
    GXStateShutdown();
    
    OSReport("GXShutdownGraphics: Graphics backend shutdown\n");
}

