/*---------------------------------------------------------------------------*
  gx_state.c - GX Graphics State Management
  
  Manages global GX rendering state, similar to Aurora's g_gxState
 *---------------------------------------------------------------------------*/

#include "gx_state.h"
#include "gx_gl.h"  // For OpenGL calls
#include <dolphin/os.h>
#include <stdlib.h>
#include <string.h>

/* GX State structure is now defined in gx_state.h */

/* Global GX state - similar to Aurora's g_gxState */
static GXState s_gxState;
static BOOL s_gxStateInitialized = FALSE;

/* Expose g_gxState pointer for Aurora's update_gx_state pattern */
GXState* g_gxState = &s_gxState;

static void SetMtxIdentity(Mtx m)
{
    memset(m, 0, sizeof(Mtx));
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
}

static void SetMtx44Identity(Mtx44 m)
{
    memset(m, 0, sizeof(Mtx44));
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
}

/*---------------------------------------------------------------------------*
  Name:         GXGetState
  
  Description:  Returns pointer to global GX state
  
  Returns:      Pointer to GXState
 *---------------------------------------------------------------------------*/
GXState* GXGetState(void) {
    if (!s_gxStateInitialized) {
        GXStateInit();
    }
    return &s_gxState;
}

/*---------------------------------------------------------------------------*
  Name:         GXStateInit
  
  Description:  Initializes the global GX state structure
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXStateInit(void) {
    if (s_gxStateInitialized) {
        return;
    }
    
    memset(&s_gxState, 0, sizeof(s_gxState));
    
    s_gxState.initialized = TRUE;
    s_gxState.stateDirty = TRUE;
    
    /* Initialize default OpenGL state */
    s_gxState.currentProgram = 0;
    s_gxState.currentVAO = 0;
    s_gxState.currentVBO = 0;
    s_gxState.currentEBO = 0;
    
    /* Default rendering state */
    s_gxState.depthTest = FALSE;
    s_gxState.blendEnabled = FALSE;
    s_gxState.cullFace = FALSE;
    
    /* Default viewport */
    s_gxState.viewportX = 0;
    s_gxState.viewportY = 0;
    s_gxState.viewportWidth = 640;
    s_gxState.viewportHeight = 480;
    
    /* Default scissor (disabled) */
    s_gxState.scissorX = 0;
    s_gxState.scissorY = 0;
    s_gxState.scissorWidth = 0;
    s_gxState.scissorHeight = 0;
    
    /* Default clear color (black) */
    s_gxState.clearColor.r = 0.0f;
    s_gxState.clearColor.g = 0.0f;
    s_gxState.clearColor.b = 0.0f;
    s_gxState.clearColor.a = 1.0f;
    
    /* Initialize cull mode (default: GX_CULL_BACK, matching Aurora) */
    s_gxState.cullMode = 2; /* GX_CULL_BACK */
    
    /* Initialize blend mode (default: GX_BM_NONE, matching Aurora) */
    s_gxState.blendMode = 0;      /* GX_BM_NONE */
    s_gxState.blendFacSrc = 4;    /* GX_BL_SRCALPHA */
    s_gxState.blendFacDst = 5;    /* GX_BL_INVSRCALPHA */
    s_gxState.blendOp = 0;        /* GX_LO_CLEAR */
    
    /* Initialize matrix storage (matching Aurora's structure) */
    for (int i = 0; i < GX_PNMTX_COUNT; ++i) {
        /* Initialize pos matrix to identity */
        s_gxState.pnMtx[i].pos[0][0] = 1.0f; s_gxState.pnMtx[i].pos[0][1] = 0.0f; s_gxState.pnMtx[i].pos[0][2] = 0.0f; s_gxState.pnMtx[i].pos[0][3] = 0.0f;
        s_gxState.pnMtx[i].pos[1][0] = 0.0f; s_gxState.pnMtx[i].pos[1][1] = 1.0f; s_gxState.pnMtx[i].pos[1][2] = 0.0f; s_gxState.pnMtx[i].pos[1][3] = 0.0f;
        s_gxState.pnMtx[i].pos[2][0] = 0.0f; s_gxState.pnMtx[i].pos[2][1] = 0.0f; s_gxState.pnMtx[i].pos[2][2] = 1.0f; s_gxState.pnMtx[i].pos[2][3] = 0.0f;
        /* Initialize nrm matrix to identity */
        s_gxState.pnMtx[i].nrm[0][0] = 1.0f; s_gxState.pnMtx[i].nrm[0][1] = 0.0f; s_gxState.pnMtx[i].nrm[0][2] = 0.0f; s_gxState.pnMtx[i].nrm[0][3] = 0.0f;
        s_gxState.pnMtx[i].nrm[1][0] = 0.0f; s_gxState.pnMtx[i].nrm[1][1] = 1.0f; s_gxState.pnMtx[i].nrm[1][2] = 0.0f; s_gxState.pnMtx[i].nrm[1][3] = 0.0f;
        s_gxState.pnMtx[i].nrm[2][0] = 0.0f; s_gxState.pnMtx[i].nrm[2][1] = 0.0f; s_gxState.pnMtx[i].nrm[2][2] = 1.0f; s_gxState.pnMtx[i].nrm[2][3] = 0.0f;
    }
    s_gxState.currentPnMtx = 0;  /* Default to slot 0 (GX_PNMTX0) */
    for (int i = 0; i < GX_TEXMTX_COUNT; ++i) {
        /* Initialize texture matrix to identity (4x4) */
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                s_gxState.texMtx[i][r][c] = (r == c) ? 1.0f : 0.0f;
            }
        }
        s_gxState.texMtxType[i] = GX_MTX3x4;
    }
    /* Initialize identity texture matrix */
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            s_gxState.texIdentityMtx[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }
    
    /* Initialize TEV stages (default: all zeros, matching Aurora) */
    s_gxState.numTevStages = 0;
    /* memset already zeroed the tevStages array */
    
    /* Initialize texture coordinate generation (default: 0, matching Aurora) */
    s_gxState.numTexGens = 0;
    
    /* Initialize pixel operations (defaults matching Aurora) */
    s_gxState.depthCompare = TRUE;   /* true in Aurora */
    s_gxState.depthFunc = 3;         /* GX_LEQUAL in Aurora */
    s_gxState.depthUpdate = TRUE;    /* true in Aurora */
    s_gxState.colorUpdate = TRUE;     /* true in Aurora */
    s_gxState.alphaUpdate = TRUE;     /* true in Aurora */
    
    /* Initialize color channels (default: 0, matching Aurora) */
    s_gxState.numChans = 0;
    
    /* Initialize vertex formats and arrays (already zeroed by memset) */
    /* No explicit initialization needed - memset already cleared them */
    
    s_gxStateInitialized = TRUE;
    
    OSReport("GXState: Initialized global GX state\n");
}

/*---------------------------------------------------------------------------*
  Name:         GXStateShutdown
  
  Description:  Shuts down the GX state (cleanup)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXStateShutdown(void) {
    if (!s_gxStateInitialized) {
        return;
    }
    
    /* TODO: Cleanup OpenGL resources if needed */
    
    s_gxStateInitialized = FALSE;
    OSReport("GXState: Shutdown\n");
}

/*---------------------------------------------------------------------------*
  Name:         GXStateSetClearColor
  
  Description:  Sets the clear color in GX state
  
  Arguments:    r, g, b, a - Color components (normalized 0.0-1.0)
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXStateSetClearColor(f32 r, f32 g, f32 b, f32 a) {
    GXState* state = GXGetState();
    if (!state) {
        return;
    }
    
    /* Update clear color if changed */
    if (state->clearColor.r != r || state->clearColor.g != g ||
        state->clearColor.b != b || state->clearColor.a != a) {
        state->clearColor.r = r;
        state->clearColor.g = g;
        state->clearColor.b = b;
        state->clearColor.a = a;
        state->stateDirty = TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GXStateGetClearColor
  
  Description:  Gets the clear color from GX state
  
  Arguments:    r, g, b, a - Pointers to store color components
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXStateGetClearColor(f32* r, f32* g, f32* b, f32* a) {
    GXState* state = GXGetState();
    if (!state || !r || !g || !b || !a) {
        return;
    }
    
    *r = state->clearColor.r;
    *g = state->clearColor.g;
    *b = state->clearColor.b;
    *a = state->clearColor.a;
}

/*---------------------------------------------------------------------------*
  Name:         GXStateMarkDirty
  
  Description:  Marks the GX state as dirty (needs update)
                Used by update_gx_state helpers
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXStateMarkDirty(void) {
    GXState* state = GXGetState();
    if (state) {
        state->stateDirty = TRUE;
    }
}

/*---------------------------------------------------------------------------*
  GX State field accessors - for update_gx_state helpers
  These show alerts if fields don't exist in GXState yet
 *---------------------------------------------------------------------------*/

/* Get direct pointers to state fields (for update_gx_state pattern) */
u32* GXStateGetCullModePtr(void) {
    GXState* state = GXGetState();
    if (state) {
        return &state->cullMode;
    }
    return NULL; /* Will trigger alert in update_gx_state */
}

u32* GXStateGetBlendModePtr(void) {
    GXState* state = GXGetState();
    if (state) {
        return &state->blendMode;
    }
    return NULL;
}

u32* GXStateGetBlendFacSrcPtr(void) {
    GXState* state = GXGetState();
    if (state) {
        return &state->blendFacSrc;
    }
    return NULL;
}

u32* GXStateGetBlendFacDstPtr(void) {
    GXState* state = GXGetState();
    if (state) {
        return &state->blendFacDst;
    }
    return NULL;
}

u32* GXStateGetBlendOpPtr(void) {
    GXState* state = GXGetState();
    if (state) {
        return &state->blendOp;
    }
    return NULL;
}

void GXStateSetCullMode(u32 mode) {
    GXState* state = GXGetState();
    if (state && state->cullMode != mode) {
        state->cullMode = mode;
        state->stateDirty = TRUE;
    }
}

u32 GXStateGetCullMode(void) {
    GXState* state = GXGetState();
    if (state) {
        return state->cullMode;
    }
    return 2; /* GX_CULL_BACK default */
}

void GXStateSetBlendMode(u32 mode) {
    GXState* state = GXGetState();
    if (state && state->blendMode != mode) {
        state->blendMode = mode;
        state->stateDirty = TRUE;
    }
}

u32 GXStateGetBlendMode(void) {
    GXState* state = GXGetState();
    if (state) {
        return state->blendMode;
    }
    return 0; /* GX_BM_NONE default */
}

void GXStateSetBlendFacSrc(u32 src) {
    GXState* state = GXGetState();
    if (state && state->blendFacSrc != src) {
        state->blendFacSrc = src;
        state->stateDirty = TRUE;
    }
}

u32 GXStateGetBlendFacSrc(void) {
    GXState* state = GXGetState();
    if (state) {
        return state->blendFacSrc;
    }
    return 4; /* GX_BL_SRCALPHA default */
}

void GXStateSetBlendFacDst(u32 dst) {
    GXState* state = GXGetState();
    if (state && state->blendFacDst != dst) {
        state->blendFacDst = dst;
        state->stateDirty = TRUE;
    }
}

u32 GXStateGetBlendFacDst(void) {
    GXState* state = GXGetState();
    if (state) {
        return state->blendFacDst;
    }
    return 5; /* GX_BL_INVSRCALPHA default */
}

void GXStateSetBlendOp(u32 op) {
    GXState* state = GXGetState();
    if (state && state->blendOp != op) {
        state->blendOp = op;
        state->stateDirty = TRUE;
    }
}

u32 GXStateGetBlendOp(void) {
    GXState* state = GXGetState();
    if (state) {
        return state->blendOp;
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         GXStateApply
  
  Description:  Applies the current GX state to OpenGL
                Called when state is dirty or at frame start
  
  Arguments:    None
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void GXStateApply(void) {
    GXState* state = GXGetState();
    if (!state || !state->initialized) {
        OSReport("[GX ERROR] GXStateApply: State not initialized\n");
        return;
    }
    
    /* Apply clear color */
    glClearColor(state->clearColor.r, state->clearColor.g, 
                 state->clearColor.b, state->clearColor.a);
    
    /* Check for OpenGL errors */
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        OSReport("[GX ERROR] GXStateApply: OpenGL error after glClearColor: 0x%04X\n", err);
    }
    
    /* Apply viewport */
    glViewport(state->viewportX, state->viewportY, 
               state->viewportWidth, state->viewportHeight);
    
    /* Check for OpenGL errors */
    err = glGetError();
    if (err != GL_NO_ERROR) {
        OSReport("[GX ERROR] GXStateApply: OpenGL error after glViewport: 0x%04X\n", err);
    }
    
    /* Apply scissor */
    if (state->scissorWidth > 0 && state->scissorHeight > 0) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(state->scissorX, state->scissorY, 
                  state->scissorWidth, state->scissorHeight);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
    
    /* Apply cull mode */
    switch (state->cullMode) {
        case 0: /* GX_CULL_NONE */
            glDisable(GL_CULL_FACE);
            break;
        case 1: /* GX_CULL_FRONT */
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case 2: /* GX_CULL_BACK */
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case 3: /* GX_CULL_ALL */
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
        default:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
    }
    
    /* Apply blend mode */
    if (state->blendMode == 0) { /* GX_BM_NONE */
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        /* TODO: Map GXBlendFactor to OpenGL blend functions */
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    /* Clear state dirty flag */
    state->stateDirty = FALSE;
}

