/*---------------------------------------------------------------------------*
  gx_helpers.c - GX Helper Functions (Aurora-style)
  
  Implements helper functions that mirror Aurora's internal functions.
  Functions we don't have yet show alert stubs.
 *---------------------------------------------------------------------------*/

#include "gx_helpers.h"
#include "gx_state.h"  // For GXGetState
#include "gx_state_helpers.h"  // For GXColorF32
#include "gx_shader.h"  // For GXShaderGetProgram
#include "gx_gl.h"
#include <dolphin/os.h>
#include <dolphin/gx/GXEnum.h>
#include <SDL.h>  // For SDL_GL_SwapWindow
#include <stdlib.h>
#include <string.h>

/* Cached viewport and scissor (similar to Aurora's g_cachedViewport, g_cachedScissor) */
static SetViewportCommand g_cachedViewport = {0};
static SetScissorCommand g_cachedScissor = {0};

/* Command queue system (similar to Aurora's g_renderPasses, g_currentRenderPass) */
#define MAX_RENDER_PASSES 16
static GfxRenderPass g_renderPasses[MAX_RENDER_PASSES];
static u32 g_currentRenderPass = 0xFFFFFFFF; /* UINT32_MAX equivalent */
static BOOL g_commandQueueInitialized = FALSE;

/* Initial capacity for command lists */
#define INITIAL_COMMAND_CAPACITY 256

/* Helper to ensure we have an active render pass (create default if needed) */
/* Copy from Aurora: push_command checks if g_currentRenderPass is valid */
/* Aurora's code checks: if (g_currentRenderPass == UINT32_MAX) { ... } */
static void ensure_render_pass(void) {
    if (g_currentRenderPass == 0xFFFFFFFF) {
        /* No active render pass - create a default one (pass 0) */
        /* This should normally be created by gfx_begin_frame(), but handle edge case silently */
        g_currentRenderPass = 0;
        
        /* Initialize command list if needed */
        if (g_renderPasses[0].commands.commands == NULL) {
            g_renderPasses[0].commands.commands = NULL;
            g_renderPasses[0].commands.count = 0;
            g_renderPasses[0].commands.capacity = 0;
        }
        
        /* Set default clear color if not set */
        if (g_renderPasses[0].clearColor.r == 0.0f && 
            g_renderPasses[0].clearColor.g == 0.0f &&
            g_renderPasses[0].clearColor.b == 0.0f &&
            g_renderPasses[0].clearColor.a == 0.0f) {
            /* Get clear color from GX state */
            GXState* state = GXGetState();
            if (state) {
                g_renderPasses[0].clearColor = state->clearColor;
            } else {
                /* Default black */
                g_renderPasses[0].clearColor.r = 0.0f;
                g_renderPasses[0].clearColor.g = 0.0f;
                g_renderPasses[0].clearColor.b = 0.0f;
                g_renderPasses[0].clearColor.a = 1.0f;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         gfx_init_command_queue
  
  Description:  Initializes the command queue system.
                Similar to Aurora's initialization.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_init_command_queue(void) {
    if (g_commandQueueInitialized) {
        return;
    }
    
    /* Initialize render passes */
    memset(g_renderPasses, 0, sizeof(g_renderPasses));
    g_currentRenderPass = 0xFFFFFFFF; /* UINT32_MAX - no active render pass */
    g_commandQueueInitialized = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         gfx_shutdown_command_queue
  
  Description:  Shuts down the command queue system and frees memory.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_shutdown_command_queue(void) {
    if (!g_commandQueueInitialized) {
        return;
    }
    
    /* Free all command lists */
    for (u32 i = 0; i < MAX_RENDER_PASSES; i++) {
        if (g_renderPasses[i].commands.commands) {
            free(g_renderPasses[i].commands.commands);
            g_renderPasses[i].commands.commands = NULL;
            g_renderPasses[i].commands.count = 0;
            g_renderPasses[i].commands.capacity = 0;
        }
    }
    
    g_currentRenderPass = 0xFFFFFFFF;
    g_commandQueueInitialized = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         gfx_clear_commands
  
  Description:  Clears all commands from the current render pass.
                Similar to Aurora's command clearing.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_clear_commands(void) {
    if (g_currentRenderPass != 0xFFFFFFFF && g_currentRenderPass < MAX_RENDER_PASSES) {
        g_renderPasses[g_currentRenderPass].commands.count = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         gfx_push_command
  
  Description:  Pushes a command to the render command list.
                Matches Aurora's push_command() exactly.
  
  Arguments:    type - Command type
                data - Command data (must match the command type)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_push_command(CommandType type, const void* data) {
    /* Ensure we have an active render pass (create default if needed) */
    ensure_render_pass();
    
    /* Copy from Aurora: push_command checks if g_currentRenderPass is valid */
    if (g_currentRenderPass == 0xFFFFFFFF) {
        /* This shouldn't happen after ensure_render_pass, but check anyway */
        OSReport("[GX WARN] gfx_push_command: Dropping command %d (no active render pass)\n", type);
        return;
    }
    
    if (g_currentRenderPass >= MAX_RENDER_PASSES) {
        OSReport("[GX ERROR] gfx_push_command: Invalid render pass index %u\n", g_currentRenderPass);
        return;
    }
    
    GfxCommandList* cmdList = &g_renderPasses[g_currentRenderPass].commands;
    
    /* Grow command list if needed (similar to std::vector::push_back) */
    if (cmdList->count >= cmdList->capacity) {
        u32 newCapacity = cmdList->capacity == 0 ? INITIAL_COMMAND_CAPACITY : cmdList->capacity * 2;
        GfxCommand* newCommands = (GfxCommand*)realloc(cmdList->commands, newCapacity * sizeof(GfxCommand));
        if (!newCommands) {
            OSReport("[GX ERROR] gfx_push_command: Failed to allocate command list memory\n");
            return;
        }
        cmdList->commands = newCommands;
        cmdList->capacity = newCapacity;
    }
    
    /* Create command (similar to Aurora's Command structure) */
    GfxCommand* cmd = &cmdList->commands[cmdList->count];
    cmd->type = type;
    
    /* Copy command data based on type (similar to Aurora's union) */
    switch (type) {
        case COMMAND_TYPE_SET_VIEWPORT:
            if (data) {
                cmd->data.setViewport = *(const SetViewportCommand*)data;
            }
            break;
        case COMMAND_TYPE_SET_SCISSOR:
            if (data) {
                cmd->data.setScissor = *(const SetScissorCommand*)data;
            }
            break;
        case COMMAND_TYPE_DRAW:
            if (data) {
                cmd->data.draw = *(const DrawCommand*)data;
            }
            break;
        default:
            OSReport("[GX WARN] gfx_push_command: Unknown command type %d\n", type);
            return;
    }
    
    cmdList->count++;
}

/*---------------------------------------------------------------------------*
  Name:         gfx_set_viewport
  
  Description:  Sets the viewport. Matches Aurora's set_viewport() exactly.
                Calls push_command() like Aurora does.
  
  Arguments:    left, top, width, height, znear, zfar - Viewport parameters
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_set_viewport(f32 left, f32 top, f32 width, f32 height, f32 znear, f32 zfar) {
    /* Update GXState */
    GXState* state = GXGetState();
    if (state) {
        state->viewportX = (s32)left;
        state->viewportY = (s32)top;
        state->viewportWidth = (u32)width;
        state->viewportHeight = (u32)height;
    }
    /* Copy from Aurora: set_viewport implementation */
    SetViewportCommand cmd = {left, top, width, height, znear, zfar};
    
    /* Check if viewport changed (Aurora compares with g_cachedViewport) */
    if (cmd.left != g_cachedViewport.left || cmd.top != g_cachedViewport.top ||
        cmd.width != g_cachedViewport.width || cmd.height != g_cachedViewport.height ||
        cmd.znear != g_cachedViewport.znear || cmd.zfar != g_cachedViewport.zfar) {
        
        /* Aurora calls: push_command(CommandType::SetViewport, Command::Data{.setViewport = cmd}); */
        gfx_push_command(COMMAND_TYPE_SET_VIEWPORT, &cmd);
        g_cachedViewport = cmd;
        
        /* Also apply directly to OpenGL for immediate effect */
        glViewport((GLint)left, (GLint)top, (GLsizei)width, (GLsizei)height);
    }
}

/*---------------------------------------------------------------------------*
  Name:         gfx_set_scissor
  
  Description:  Sets the scissor rectangle. Matches Aurora's set_scissor() exactly.
                Calls push_command() like Aurora does.
  
  Arguments:    x, y, w, h - Scissor rectangle parameters
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_set_scissor(u32 x, u32 y, u32 w, u32 h) {
    /* Update GXState */
    GXState* state = GXGetState();
    if (state) {
        state->scissorX = (s32)x;
        state->scissorY = (s32)y;
        state->scissorWidth = w;
        state->scissorHeight = h;
    }
    /* Copy from Aurora: set_scissor implementation */
    SetScissorCommand cmd = {x, y, w, h};
    
    /* Check if scissor changed (Aurora compares with g_cachedScissor) */
    if (cmd.x != g_cachedScissor.x || cmd.y != g_cachedScissor.y ||
        cmd.w != g_cachedScissor.w || cmd.h != g_cachedScissor.h) {
        
        /* Aurora calls: push_command(CommandType::SetScissor, Command::Data{.setScissor = cmd}); */
        gfx_push_command(COMMAND_TYPE_SET_SCISSOR, &cmd);
        g_cachedScissor = cmd;
        
        /* Also apply directly to OpenGL for immediate effect */
        glScissor((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
    }
}

/*---------------------------------------------------------------------------*
  Name:         gfx_begin_frame
  
  Description:  Begins a new frame. Similar to Aurora's begin_frame().
                Sets up render passes and clears state.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_begin_frame(void) {
    /* Copy from Aurora exactly: begin_frame() implementation */
    /* Aurora's code:
     *   g_renderPasses.emplace_back();
     *   g_renderPasses[0].clearColor = gx::g_gxState.clearColor;
     *   g_currentRenderPass = 0;
     */
    
    /* Clear all render passes and start fresh (emplace_back equivalent) */
    for (u32 i = 0; i < MAX_RENDER_PASSES; i++) {
        if (g_renderPasses[i].commands.commands) {
            free(g_renderPasses[i].commands.commands);
            g_renderPasses[i].commands.commands = NULL;
        }
        g_renderPasses[i].commands.count = 0;
        g_renderPasses[i].commands.capacity = 0;
    }
    
    /* Create new render pass (pass 0) */
    g_currentRenderPass = 0;
    
    /* Set clear color from GX state (Aurora: g_renderPasses[0].clearColor = gx::g_gxState.clearColor) */
    GXState* state = GXGetState();
    if (state) {
        g_renderPasses[0].clearColor = state->clearColor;
    } else {
        /* Default clear color (black) */
        g_renderPasses[0].clearColor.r = 0.0f;
        g_renderPasses[0].clearColor.g = 0.0f;
        g_renderPasses[0].clearColor.b = 0.0f;
        g_renderPasses[0].clearColor.a = 1.0f;
    }
    
    /* Aurora also resets draw call counts - we don't have those yet, so stub with warning */
    /* TODO: Add draw call counting when draw commands are implemented */
    /* Aurora's code: g_drawCallCount = 0; g_mergedDrawCallCount = 0; */
    
    /* Aurora has buffer mapping (WebGPU specific) - not needed for OpenGL backend */
    /* Aurora's code:
     *   while (!bufferMapped) { g_instance.ProcessEvents(); }
     *   mapBuffer(g_verts, VertexBufferSize);
     *   mapBuffer(g_uniforms, UniformBufferSize);
     *   etc.
     */
    /* Note: OpenGL doesn't need buffer mapping like WebGPU, so this is intentionally a no-op */
}

/*---------------------------------------------------------------------------*
  Name:         gfx_end_frame
  
  Description:  Ends the current frame. Similar to Aurora's end_frame().
                Processes commands and prepares for rendering.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_end_frame(void) {
    /* Copy from Aurora exactly: end_frame() implementation */
    /* Aurora's code:
     *   g_stagingBuffers[currentStagingBuffer].Unmap();
     *   writeBuffer(...);
     *   g_currentRenderPass = UINT32_MAX;
     *   etc.
     */
    
    /* Mark frame as ended - set current render pass to invalid */
    /* Aurora's code: g_currentRenderPass = UINT32_MAX; */
    g_currentRenderPass = 0xFFFFFFFF;
    
    /* Aurora has buffer unmapping and writing (WebGPU specific) - not needed for OpenGL */
    /* Aurora's code:
     *   g_stagingBuffers[currentStagingBuffer].Unmap();
     *   writeBuffer(g_verts, g_vertexBuffer, ...);
     *   writeBuffer(g_uniforms, g_uniformBuffer, ...);
     *   writeBuffer(g_indices, g_indexBuffer, ...);
     *   writeBuffer(g_storage, g_storageBuffer, ...);
     */
    /* Note: OpenGL doesn't need buffer unmapping like WebGPU, so this is intentionally a no-op */
    
    /* Aurora has texture uploads (WebGPU specific) - not needed for OpenGL */
    /* Aurora's code:
     *   for (const auto& item : g_textureUploads) {
     *     cmd.CopyBufferToTexture(...);
     *   }
     *   g_textureUploads.clear();
     */
    /* Note: OpenGL handles texture uploads differently, so this is intentionally a no-op */
    
    /* Aurora clears cached array ranges - we don't have this yet */
    /* Aurora's code:
     *   for (auto& array : gx::g_gxState.arrays) {
     *     array.cachedRange = {};
     *   }
     */
    /* TODO: Clear cached array ranges when we implement vertex arrays */
}

/*---------------------------------------------------------------------------*
  Name:         gfx_render
  
  Description:  Renders all queued commands. Similar to Aurora's render().
                Processes render passes and executes draw commands.
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void gfx_render(void) {
    /* Copy from Aurora exactly: render() implementation */
    /* Aurora's code:
     *   for (u32 i = 0; i < g_renderPasses.size(); ++i) {
     *     const auto& passInfo = g_renderPasses[i];
     *     auto pass = cmd.BeginRenderPass(&renderPassDescriptor);
     *     render_pass(pass, i);
     *     pass.End();
     *   }
     */
    
    /* Ensure OpenGL state is set up (depth test, blend, cull face) */
    /* This should match what GXInitGraphics() sets up */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    /* Iterate through all render passes */
    for (u32 i = 0; i < MAX_RENDER_PASSES; i++) {
        if (g_renderPasses[i].commands.count == 0 && i > 0) {
            /* Skip empty passes after the first one */
            continue;
        }
        
        const GfxRenderPass* passInfo = &g_renderPasses[i];
        
        /* Apply clear color for this render pass */
        /* Aurora's code: .clearValue = { .r = passInfo.clearColor.x(), ... } */
        /* On GameCube, clear color alpha might be 0, but OpenGL needs alpha = 1.0 for opaque clear */
        /* If alpha is 0 or very small, assume it should be 1.0 (opaque) */
        f32 clearAlpha = passInfo->clearColor.a;
        if (clearAlpha < 0.5f) {
            clearAlpha = 1.0f;  /* Force opaque if alpha is 0 or very small */
        }
        glClearColor(passInfo->clearColor.r, passInfo->clearColor.g,
                     passInfo->clearColor.b, clearAlpha);
        
        /* Debug: Log the actual clear color used (with clamped alpha) - only once */
        static u32 clearLogFrame = 0;
        if (clearLogFrame == 0 && i == 0) {
            OSReport("[GX DEBUG] Clear color: original=(%.2f, %.2f, %.2f, %.2f), used=(%.2f, %.2f, %.2f, %.2f)\n",
                     passInfo->clearColor.r, passInfo->clearColor.g, 
                     passInfo->clearColor.b, passInfo->clearColor.a,
                     passInfo->clearColor.r, passInfo->clearColor.g, 
                     passInfo->clearColor.b, clearAlpha);
            clearLogFrame++;
        }
        
        /* Set default viewport if no viewport command exists (safety) */
        /* This ensures we always have a valid viewport before drawing */
        BOOL viewportSet = FALSE;
        for (u32 j = 0; j < passInfo->commands.count; j++) {
            if (passInfo->commands.commands[j].type == COMMAND_TYPE_SET_VIEWPORT) {
                viewportSet = TRUE;
                break;
            }
        }
        if (!viewportSet) {
            /* Use default viewport from GXState */
            GXState* state = GXGetState();
            if (state && state->viewportWidth > 0 && state->viewportHeight > 0) {
                glViewport(state->viewportX, state->viewportY, 
                          state->viewportWidth, state->viewportHeight);
            } else {
                /* Fallback: use window size */
                SDL_Window* window = SDL_GL_GetCurrentWindow();
                if (window) {
                    int w, h;
                    SDL_GL_GetDrawableSize(window, &w, &h);
                    glViewport(0, 0, w, h);
                }
            }
        }
        
        /* Clear the screen with the render pass clear color */
        /* Set depth clear value to 1.0 (far plane) - GameCube uses [0,1] depth range */
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        /* Execute commands for this render pass */
        /* Aurora's code: render_pass(pass, i); */
        for (u32 j = 0; j < passInfo->commands.count; j++) {
            const GfxCommand* cmd = &passInfo->commands.commands[j];
            
            switch (cmd->type) {
                case COMMAND_TYPE_SET_VIEWPORT:
                    glViewport((GLint)cmd->data.setViewport.left,
                               (GLint)cmd->data.setViewport.top,
                               (GLsizei)cmd->data.setViewport.width,
                               (GLsizei)cmd->data.setViewport.height);
                    break;
                    
                case COMMAND_TYPE_SET_SCISSOR:
                    glEnable(GL_SCISSOR_TEST);
                    glScissor((GLint)cmd->data.setScissor.x,
                              (GLint)cmd->data.setScissor.y,
                              (GLsizei)cmd->data.setScissor.w,
                              (GLsizei)cmd->data.setScissor.h);
                    break;
                    
                case COMMAND_TYPE_DRAW:
                    /* Render the draw command using OpenGL */
                    {
                        DrawCommand* draw = &cmd->data.draw;  /* Non-const so we can clear pointers after freeing */
                        GXState* state = GXGetState();
                        
                        if (!state || !draw->vertexData || draw->vertexDataSize == 0) {
                            /* Free and skip empty draw commands */
                            if (draw->vertexData) free((void*)draw->vertexData);
                            if (draw->indices) free((void*)draw->indices);
                            break;
                        }
                        
                        /* Process indexed attributes and build OpenGL-compatible vertex data */
                        /* For now, implement a basic version that handles indexed position/color */
                        
                        /* Count indexed attributes */
                        u32 numIndexedAttrs = 0;
                        GXAttr indexedAttrs[GX_VA_MAX_ATTR];
                        for (GXAttr attr = 0; attr < GX_VA_MAX_ATTR; attr++) {
                            if (state->vtxDesc[attr] == GX_INDEX8 || state->vtxDesc[attr] == GX_INDEX16) {
                                indexedAttrs[numIndexedAttrs++] = attr;
                            }
                        }
                        
                        if (numIndexedAttrs == 0) {
                            OSReport("[GX WARN] gfx_render: No indexed attributes found\n");
                            if (draw->vertexData) free((void*)draw->vertexData);
                            if (draw->indices) free((void*)draw->indices);
                            break;
                        }
                        
                        /* Check if arrays are set */
                        for (u32 i = 0; i < numIndexedAttrs; i++) {
                            GXAttr attr = indexedAttrs[i];
                            const AttrArray* array = &state->arrays[attr];
                            if (!array->data) {
                                OSReport("[GX ERROR] gfx_render: Array for attr %u is NULL\n", attr);
                            }
                        }
                        
                        /* Calculate number of vertices from index data */
                        /* Each indexed attribute contributes one index per vertex */
                        u32 numVertices = draw->vertexDataSize / (numIndexedAttrs * sizeof(u16));
                        if (numVertices == 0) {
                            OSReport("[GX WARN] gfx_render: No vertices in draw command\n");
                            if (draw->vertexData) free((void*)draw->vertexData);
                            if (draw->indices) free((void*)draw->indices);
                            break;
                        }
                        
                        /* Build interleaved vertex data for OpenGL */
                        /* Format: position (3 floats) + color (4 floats) */
                        u32 vertexStride = 7 * sizeof(f32);  /* 3 pos + 4 color */
                        f32* glVertices = (f32*)malloc(numVertices * vertexStride);
                        if (!glVertices) {
                            OSReport("[GX ERROR] gfx_render: Failed to allocate vertex buffer\n");
                            if (draw->vertexData) free((void*)draw->vertexData);
                            if (draw->indices) free((void*)draw->indices);
                            break;
                        }
                        
                        /* Resolve indices to actual vertex data */
                        u16* indexPtr = (u16*)draw->vertexData;
                        u32 maxIndices = draw->vertexDataSize / sizeof(u16);
                        
                        /* First pass: Find maximum index used per attribute to calculate effective size */
                        u16 maxIndexPerAttr[GX_VA_MAX_ATTR];
                        memset(maxIndexPerAttr, 0, sizeof(maxIndexPerAttr));
                        
                        for (u32 v = 0; v < numVertices; v++) {
                            for (u32 i = 0; i < numIndexedAttrs; i++) {
                                u32 indexOffset = v * numIndexedAttrs + i;
                                if (indexOffset >= maxIndices) {
                                    break;
                                }
                                GXAttr attr = indexedAttrs[i];
                                u16 idx = indexPtr[indexOffset];
                                if (idx > maxIndexPerAttr[attr]) {
                                    maxIndexPerAttr[attr] = idx;
                                }
                            }
                        }
                        
                        /* Calculate effective size for each array (if size was 0, infer from max index) */
                        u32 effectiveSizePerAttr[GX_VA_MAX_ATTR];
                        for (u32 i = 0; i < numIndexedAttrs; i++) {
                            GXAttr attr = indexedAttrs[i];
                            const AttrArray* array = &state->arrays[attr];
                            if (array->size > 0) {
                                effectiveSizePerAttr[attr] = array->size;
                            } else if (array->stride > 0) {
                                /* Infer size from max index: (maxIndex + 1) * stride */
                                effectiveSizePerAttr[attr] = ((u32)maxIndexPerAttr[attr] + 1) * array->stride;
                            } else {
                                effectiveSizePerAttr[attr] = 0;  /* Unknown - no bounds checking */
                            }
                        }
                        
                        /* Second pass: Resolve indices to actual vertex data with bounds checking */
                        BOOL drawFailed = FALSE;
                        for (u32 v = 0; v < numVertices && !drawFailed; v++) {
                            f32* vtx = &glVertices[v * 7];
                            
                            /* Process each indexed attribute */
                            for (u32 i = 0; i < numIndexedAttrs; i++) {
                                u32 indexOffset = v * numIndexedAttrs + i;
                                if (indexOffset >= maxIndices) {
                                    OSReport("[GX ERROR] gfx_render: Index out of bounds: %u >= %u\n", indexOffset, maxIndices);
                                    drawFailed = TRUE;
                                    break;
                                }
                                GXAttr attr = indexedAttrs[i];
                                u16 idx = indexPtr[indexOffset];
                                const AttrArray* array = &state->arrays[attr];
                                
                                if (!array->data) continue;
                                
                                /* Bounds check using effective size */
                                u32 effectiveSize = effectiveSizePerAttr[attr];
                                if (effectiveSize > 0) {
                                    u32 offset = (u32)idx * array->stride;
                                    if (offset >= effectiveSize) {
                                        OSReport("[GX ERROR] gfx_render: Array access out of bounds: attr=%u, idx=%u, offset=%u, size=%u\n",
                                                 attr, idx, offset, effectiveSize);
                                        drawFailed = TRUE;
                                        break;
                                    }
                                }
                                
                                if (attr == GX_VA_POS) {
                                    /* Position: s16 x, y, z */
                                    const s16* pos = (const s16*)((const u8*)array->data + idx * array->stride);
                                    vtx[0] = (f32)pos[0];
                                    vtx[1] = (f32)pos[1];
                                    vtx[2] = (f32)pos[2];
                                } else if (attr == GX_VA_CLR0) {
                                    /* Color: u8 r, g, b, a */
                                    const u8* clr = (const u8*)((const u8*)array->data + idx * array->stride);
                                    vtx[3] = (f32)clr[0] / 255.0f;
                                    vtx[4] = (f32)clr[1] / 255.0f;
                                    vtx[5] = (f32)clr[2] / 255.0f;
                                    vtx[6] = (f32)clr[3] / 255.0f;
                                }
                            }
                        }
                        
                        /* Check if draw failed during index resolution */
                        if (drawFailed) {
                            free(glVertices);
                            if (draw->vertexData) free((void*)draw->vertexData);
                            if (draw->indices) free((void*)draw->indices);
                            break;
                        }
                        
                        /* Use shader program */
                        GLuint shaderProgram = GXShaderGetProgram();
                        if (shaderProgram == 0) {
                            OSReport("[GX ERROR] gfx_render: No shader program available\n");
                            free(glVertices);
                            if (draw->vertexData) free((void*)draw->vertexData);
                            if (draw->indices) free((void*)draw->indices);
                            break;
                        }
                        glUseProgram(shaderProgram);
                        
                        /* Set up projection and modelview matrices */
                        GLint projLoc = glGetUniformLocation(shaderProgram, "uProjection");
                        GLint mvLoc = glGetUniformLocation(shaderProgram, "uModelView");
                        
                        /* Always set projection matrix (initialized to identity if not set) */
                        if (projLoc >= 0) {
                            /* GameCube SDK projection matrices are already in the correct format for OpenGL */
                            /* They use the same row-major layout, so we can pass them directly */
                            glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)state->proj);
                            
                            /* Debug: Print projection matrix once per frame */
                            static u32 projLogFrame = 0;
                            if (projLogFrame == 0) {
                                OSReport("[GX DEBUG] Projection Matrix (type=%u, first frame):\n", state->projType);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", 
                                         state->proj[0][0], state->proj[0][1], state->proj[0][2], state->proj[0][3]);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", 
                                         state->proj[1][0], state->proj[1][1], state->proj[1][2], state->proj[1][3]);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", 
                                         state->proj[2][0], state->proj[2][1], state->proj[2][2], state->proj[2][3]);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", 
                                         state->proj[3][0], state->proj[3][1], state->proj[3][2], state->proj[3][3]);
                                projLogFrame++;
                            }
                            
                            /* Debug: Check if projection matrix is identity (bad) */
                            if (state->proj[0][0] == 1.0f && state->proj[1][1] == 1.0f && 
                                state->proj[2][2] == 1.0f && state->proj[3][3] == 1.0f &&
                                state->proj[0][1] == 0.0f && state->proj[0][2] == 0.0f) {
                                OSReport("[GX WARN] gfx_render: Projection matrix appears to be identity - may not be set!\n");
                            }
                        } else {
                            OSReport("[GX WARN] gfx_render: uProjection uniform location not found in shader\n");
                        }
                        
                        
                        /* Always set modelview matrix (position matrix) */
                        if (mvLoc >= 0 && state->currentPnMtx < GX_PNMTX_COUNT) {
                            /* Convert 3x4 position matrix to 4x4 for OpenGL */
                            f32 mvp[16];
                            const f32 (*posMtx)[4] = (const f32 (*)[4])state->pnMtx[state->currentPnMtx].pos;
                            
                            /* Copy 3x4 matrix and add bottom row [0,0,0,1] */
                            /* GX matrices are stored as Mtx[3][4] (row-major: 3 rows, 4 columns) */
                            /* OpenGL expects column-major, but glUniformMatrix4fv with GL_FALSE means we pass row-major */
                            /* So we store it row-major, and OpenGL will transpose it internally */
                            /* Layout: mvp[0-3] = row 0, mvp[4-7] = row 1, mvp[8-11] = row 2, mvp[12-15] = row 3 */
                            mvp[0] = posMtx[0][0]; mvp[1] = posMtx[0][1]; mvp[2] = posMtx[0][2]; mvp[3] = posMtx[0][3];
                            mvp[4] = posMtx[1][0]; mvp[5] = posMtx[1][1]; mvp[6] = posMtx[1][2]; mvp[7] = posMtx[1][3];
                            mvp[8] = posMtx[2][0]; mvp[9] = posMtx[2][1]; mvp[10] = posMtx[2][2]; mvp[11] = posMtx[2][3];
                            mvp[12] = 0.0f; mvp[13] = 0.0f; mvp[14] = 0.0f; mvp[15] = 1.0f;
                            
                            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, mvp);
                            
                            /* Debug: Log first vertex and matrices once per frame */
                            static u32 mvLogFrame = 0;
                            if (mvLogFrame == 0 && numVertices > 0) {
                                OSReport("[GX DEBUG] 3x4 Position Matrix (first frame):\n");
                                OSReport("  Row 0: [%.3f, %.3f, %.3f, %.3f]\n", 
                                         posMtx[0][0], posMtx[0][1], posMtx[0][2], posMtx[0][3]);
                                OSReport("  Row 1: [%.3f, %.3f, %.3f, %.3f]\n", 
                                         posMtx[1][0], posMtx[1][1], posMtx[1][2], posMtx[1][3]);
                                OSReport("  Row 2: [%.3f, %.3f, %.3f, %.3f]\n", 
                                         posMtx[2][0], posMtx[2][1], posMtx[2][2], posMtx[2][3]);
                                
                                OSReport("[GX DEBUG] 4x4 ModelView Matrix (first frame):\n");
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", mvp[0], mvp[1], mvp[2], mvp[3]);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", mvp[4], mvp[5], mvp[6], mvp[7]);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", mvp[8], mvp[9], mvp[10], mvp[11]);
                                OSReport("  [%.3f, %.3f, %.3f, %.3f]\n", mvp[12], mvp[13], mvp[14], mvp[15]);
                                
                                f32 vtxPos[4] = {glVertices[0], glVertices[1], glVertices[2], 1.0f};
                                OSReport("[GX DEBUG] First vertex before transform: (%.2f, %.2f, %.2f, %.2f)\n",
                                         vtxPos[0], vtxPos[1], vtxPos[2], vtxPos[3]);
                                mvLogFrame++;
                            }
                        } else if (mvLoc >= 0) {
                            OSReport("[GX WARN] gfx_render: currentPnMtx invalid (%u), using identity modelview\n", state->currentPnMtx);
                            /* Fallback: set identity matrix if currentPnMtx is invalid */
                            f32 identity[16] = {
                                1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f
                            };
                            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, identity);
                        } else {
                            OSReport("[GX WARN] gfx_render: uModelView uniform location not found in shader\n");
                        }
                        
                        /* Create and bind VAO (required in OpenGL 3.3+ core profile) */
                        GLuint vao = 0;
                        glGenVertexArrays(1, &vao);
                        glBindVertexArray(vao);
                        
                        /* Create and bind VBO */
                        GLuint vbo = 0;
                        glGenBuffers(1, &vbo);
                        glBindBuffer(GL_ARRAY_BUFFER, vbo);
                        glBufferData(GL_ARRAY_BUFFER, numVertices * vertexStride, glVertices, GL_STATIC_DRAW);
                        
                        /* Set up vertex attributes */
                        glEnableVertexAttribArray(0);  /* Position */
                        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexStride, (void*)0);
                        glEnableVertexAttribArray(1);  /* Color */
                        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertexStride, (void*)(3 * sizeof(f32)));
                        
                        /* Map GX primitive to OpenGL and handle quad triangulation */
                        GLenum glPrimitive = GL_TRIANGLES;
                        u32 numIndices = draw->indexCount;
                        u32 actualVertexCount = numVertices;
                        f32* finalVertices = glVertices;
                        BOOL needTriangulation = FALSE;
                        
                        if (draw->primitive == GX_QUADS) {
                            /* Quads need to be converted to triangles */
                            /* Each quad (4 vertices) becomes 2 triangles (6 vertices) */
                            if (numVertices % 4 != 0) {
                                OSReport("[GX ERROR] gfx_render: Invalid quad vertex count: %u (must be multiple of 4)\n", numVertices);
                                free(glVertices);
                                if (draw->vertexData) free((void*)draw->vertexData);
                                if (draw->indices) free((void*)draw->indices);
                                break;
                            }
                            
                            u32 numQuads = numVertices / 4;
                            actualVertexCount = numQuads * 6;  /* 2 triangles per quad */
                            
                            /* Allocate new vertex buffer for triangulated quads */
                            finalVertices = (f32*)malloc(actualVertexCount * vertexStride);
                            needTriangulation = TRUE;  /* Mark that we allocated new memory */
                            if (!finalVertices) {
                                OSReport("[GX ERROR] gfx_render: Failed to allocate triangulated vertex buffer\n");
                                free(glVertices);
                                if (draw->vertexData) free((void*)draw->vertexData);
                                if (draw->indices) free((void*)draw->indices);
                                break;
                            }
                            
                            /* Triangulate: convert each quad to 2 triangles */
                            /* Quad order: v0, v1, v2, v3 -> Triangles: (v0,v1,v2) and (v0,v2,v3) */
                            for (u32 q = 0; q < numQuads; q++) {
                                f32* quadVerts[4];
                                for (u32 i = 0; i < 4; i++) {
                                    quadVerts[i] = &glVertices[(q * 4 + i) * 7];
                                }
                                
                                /* First triangle: v0, v1, v2 */
                                f32* tri0 = &finalVertices[(q * 6 + 0) * 7];
                                f32* tri1 = &finalVertices[(q * 6 + 1) * 7];
                                f32* tri2 = &finalVertices[(q * 6 + 2) * 7];
                                memcpy(tri0, quadVerts[0], vertexStride);
                                memcpy(tri1, quadVerts[1], vertexStride);
                                memcpy(tri2, quadVerts[2], vertexStride);
                                
                                /* Second triangle: v0, v2, v3 */
                                f32* tri3 = &finalVertices[(q * 6 + 3) * 7];
                                f32* tri4 = &finalVertices[(q * 6 + 4) * 7];
                                f32* tri5 = &finalVertices[(q * 6 + 5) * 7];
                                memcpy(tri3, quadVerts[0], vertexStride);
                                memcpy(tri4, quadVerts[2], vertexStride);
                                memcpy(tri5, quadVerts[3], vertexStride);
                            }
                            
                            glPrimitive = GL_TRIANGLES;
                            needTriangulation = TRUE;
                        } else if (draw->primitive == GX_TRIANGLES) {
                            glPrimitive = GL_TRIANGLES;
                        } else if (draw->primitive == GX_TRIANGLESTRIP) {
                            glPrimitive = GL_TRIANGLE_STRIP;
                        }
                        
                        /* Create and bind IBO if we have indices */
                        GLuint ibo = 0;
                        if (draw->indices && numIndices > 0) {
                            glGenBuffers(1, &ibo);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                            glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(u16), draw->indices, GL_STATIC_DRAW);
                            
                            /* Draw with indices */
                            OSReport("[GX DEBUG] Drawing with indices: primitive=%u, numIndices=%u\n", glPrimitive, numIndices);
                            glDrawElements(glPrimitive, numIndices, GL_UNSIGNED_SHORT, NULL);
                            
                            /* Check for OpenGL errors */
                            GLenum err = glGetError();
                            if (err != GL_NO_ERROR) {
                                OSReport("[GX ERROR] OpenGL error after glDrawElements: 0x%x\n", err);
                            }
                            
                            glDeleteBuffers(1, &ibo);
                        } else {
                            /* Draw without indices */
                            static u32 drawLogCount = 0;
                            if (drawLogCount == 0) {
                                OSReport("[GX DEBUG] Drawing without indices: primitive=%u, numVertices=%u (actual=%u)\n", 
                                         glPrimitive, numVertices, actualVertexCount);
                                if (actualVertexCount > 0) {
                                    OSReport("[GX DEBUG] First vertex data: pos=(%.2f, %.2f, %.2f), color=(%.2f, %.2f, %.2f, %.2f)\n",
                                             finalVertices[0], finalVertices[1], finalVertices[2],
                                             finalVertices[3], finalVertices[4], finalVertices[5], finalVertices[6]);
                                }
                                drawLogCount++;
                            }
                            
                            /* Update VBO with final vertex data (triangulated if needed) */
                            glBufferData(GL_ARRAY_BUFFER, actualVertexCount * vertexStride, finalVertices, GL_STATIC_DRAW);
                            
                            glDrawArrays(glPrimitive, 0, actualVertexCount);
                            
                            /* Check for OpenGL errors */
                            GLenum err = glGetError();
                            if (err != GL_NO_ERROR) {
                                OSReport("[GX ERROR] OpenGL error after glDrawArrays: 0x%x\n", err);
                            }
                        }
                        
                        /* Cleanup - unbind VAO first, then delete resources */
                        glBindVertexArray(0);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        if (ibo != 0) {
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                        }
                        glDeleteBuffers(1, &vbo);
                        if (ibo != 0) {
                            glDeleteBuffers(1, &ibo);
                        }
                        glDeleteVertexArrays(1, &vao);
                        
                        /* Free vertex buffers (original and triangulated if created) */
                        free(glVertices);
                        if (needTriangulation && finalVertices != glVertices) {
                            free(finalVertices);
                        }
                        
                        /* Free the draw command data and clear pointers to prevent double-free */
                        if (draw->vertexData) {
                            free((void*)draw->vertexData);
                            draw->vertexData = NULL;  /* Prevent double-free */
                        }
                        if (draw->indices) {
                            free((void*)draw->indices);
                            draw->indices = NULL;  /* Prevent double-free */
                        }
                    }
                    break;
                    
                default:
                    OSReport("[GX WARN] gfx_render: Unknown command type %d\n", cmd->type);
                    break;
            }
        }
        
        /* Aurora has resolve targets and texture copies (WebGPU specific) - not needed for OpenGL */
        /* Aurora's code:
         *   if (passInfo.resolveTarget) {
         *     cmd.CopyTextureToTexture(...);
         *   }
         */
        /* Note: OpenGL doesn't need resolve targets like WebGPU, so this is intentionally a no-op */
    }
    
    /* Aurora has render pass descriptors and WebGPU command encoder - not needed for OpenGL */
    /* Aurora's code:
     *   const wgpu::RenderPassDescriptor renderPassDescriptor{...};
     *   auto pass = cmd.BeginRenderPass(&renderPassDescriptor);
     */
    /* Note: OpenGL doesn't need render pass descriptors like WebGPU, so this is intentionally a no-op */
    
    /* Don't swap here - let DEMOSwapBuffers() handle it after VIWaitForRetrace() */
    /* This ensures the swap happens at the right time for proper frame presentation */
    
    /* Clear render passes after rendering (Aurora does this in render()) */
    /* Aurora's code: g_renderPasses.clear(); */
    /* IMPORTANT: Free all command data before freeing the command list */
    for (u32 i = 0; i < MAX_RENDER_PASSES; i++) {
        GfxCommandList* cmdList = &g_renderPasses[i].commands;
        if (cmdList->commands) {
            /* Free data in all commands (especially DrawCommand vertexData and indices) */
            /* Note: Data may already be freed during rendering, so check for NULL */
            for (u32 j = 0; j < cmdList->count; j++) {
                GfxCommand* cmd = &cmdList->commands[j];
                if (cmd->type == COMMAND_TYPE_DRAW) {
                    DrawCommand* draw = &cmd->data.draw;
                    /* Only free if not already freed during rendering */
                    if (draw->vertexData) {
                        free((void*)draw->vertexData);
                        draw->vertexData = NULL;
                    }
                    if (draw->indices) {
                        free((void*)draw->indices);
                        draw->indices = NULL;
                    }
                }
            }
            /* Now free the command list itself */
            free(cmdList->commands);
            cmdList->commands = NULL;
        }
        cmdList->count = 0;
        cmdList->capacity = 0;
    }
    g_currentRenderPass = 0xFFFFFFFF;
}

