/*---------------------------------------------------------------------------*
  gx_helpers.h - GX Helper Functions (Aurora-style)
  
  These functions mirror Aurora's internal helper functions.
  Functions we don't have yet will show alert stubs.
 *---------------------------------------------------------------------------*/

#ifndef GX_HELPERS_H
#define GX_HELPERS_H

#include <dolphin/types.h>
#include "gx_state_helpers.h"  // For GXColorF32

#ifdef __cplusplus
extern "C" {
#endif

/* Viewport and Scissor commands (similar to Aurora's SetViewportCommand, SetScissorCommand) */
typedef struct {
    f32 left;
    f32 top;
    f32 width;
    f32 height;
    f32 znear;
    f32 zfar;
} SetViewportCommand;

typedef struct {
    u32 x;
    u32 y;
    u32 w;
    u32 h;
} SetScissorCommand;

/* Command types (similar to Aurora's CommandType enum) */
typedef enum {
    COMMAND_TYPE_SET_VIEWPORT,
    COMMAND_TYPE_SET_SCISSOR,
    COMMAND_TYPE_DRAW
} CommandType;

/* Draw command data (simplified for now) */
typedef struct {
    u8* vertexData;      /* Vertex buffer data */
    u32 vertexDataSize;  /* Size of vertex data in bytes */
    u16* indices;        /* Index buffer data */
    u32 indexCount;      /* Number of indices */
    GXPrimitive primitive; /* Primitive type */
} DrawCommand;

/* Command structure (similar to Aurora's Command) */
typedef struct {
    CommandType type;
    union {
        SetViewportCommand setViewport;
        SetScissorCommand setScissor;
        DrawCommand draw;
    } data;
} GfxCommand;

/* Command list (similar to Aurora's CommandList = std::vector<Command>) */
typedef struct {
    GfxCommand* commands;
    u32 count;
    u32 capacity;
} GfxCommandList;

/* Render pass (similar to Aurora's RenderPass) */
typedef struct {
    GfxCommandList commands;
    GXColorF32 clearColor;  /* Clear color for this render pass (from gx_state_helpers.h) */
    /* TODO: Add resolveTarget, resolveRect, etc. when needed */
} GfxRenderPass;

/* Helper functions matching Aurora's API */
void gfx_set_viewport(f32 left, f32 top, f32 width, f32 height, f32 znear, f32 zfar);
void gfx_set_scissor(u32 x, u32 y, u32 w, u32 h);

/* Internal functions that Aurora uses */
void gfx_push_command(CommandType type, const void* data);

/* Command queue management (similar to Aurora's g_renderPasses, g_currentRenderPass) */
void gfx_init_command_queue(void);
void gfx_shutdown_command_queue(void);
void gfx_clear_commands(void);

/* Frame rendering (similar to Aurora's begin_frame, end_frame, render) */
void gfx_begin_frame(void);
void gfx_end_frame(void);
void gfx_render(void);

#ifdef __cplusplus
}
#endif

#endif /* GX_HELPERS_H */

