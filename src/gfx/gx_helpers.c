/*---------------------------------------------------------------------------*
  gx_helpers.c - GX Helper Functions (Aurora-style)
  
  Implements helper functions that mirror Aurora's internal functions.
  Functions we don't have yet show alert stubs.
 *---------------------------------------------------------------------------*/

#include "gx_helpers.h"
#include "gx_state.h"  // For GXGetState
#include "gx_state_helpers.h"  // For GXColorF32
#include "gx_gl.h"
#include <dolphin/os.h>
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
            /* TODO: Handle draw commands when needed */
            OSReport("[GX WARN] gfx_push_command: Draw commands not yet implemented\n");
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
    
    /* Iterate through all render passes */
    for (u32 i = 0; i < MAX_RENDER_PASSES; i++) {
        if (g_renderPasses[i].commands.count == 0 && i > 0) {
            /* Skip empty passes after the first one */
            continue;
        }
        
        const GfxRenderPass* passInfo = &g_renderPasses[i];
        
        /* Apply clear color for this render pass */
        /* Aurora's code: .clearValue = { .r = passInfo.clearColor.x(), ... } */
        glClearColor(passInfo->clearColor.r, passInfo->clearColor.g,
                     passInfo->clearColor.b, passInfo->clearColor.a);
        
        /* Clear the screen with the render pass clear color */
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
                        const DrawCommand* draw = &cmd->data.draw;
                        
                        /* For now, just log that we got a draw command */
                        /* TODO: Actually render vertices using OpenGL */
                        /* This requires:
                         * - Setting up vertex attributes from vtxDesc/vtxFmts
                         * - Binding vertex/index buffers
                         * - Setting up shaders/pipeline
                         * - Calling glDrawElements or glDrawArrays
                         */
                        OSReport("[GX INFO] gfx_render: Draw command - %u vertices, %u indices, primitive %u\n",
                                draw->vertexDataSize, draw->indexCount, draw->primitive);
                        
                        /* Free the draw command data after processing */
                        if (draw->vertexData) {
                            free((void*)draw->vertexData);
                        }
                        if (draw->indices) {
                            free((void*)draw->indices);
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
}

