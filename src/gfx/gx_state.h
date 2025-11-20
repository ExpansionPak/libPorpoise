/*---------------------------------------------------------------------------*
  gx_state.h - GX Graphics State Management
  
  Similar to Aurora's g_gxState, this manages the global GX rendering state
 *---------------------------------------------------------------------------*/

#ifndef GX_STATE_H
#define GX_STATE_H

#include <dolphin/types.h>
#include <dolphin/gx/GXEnum.h>
#include <dolphin/mtx/GeoTypes.h>
#include "gx_state_helpers.h"  // For GXColorF32

#define GX_PNMTX_COUNT 10
#define GX_TEXMTX_COUNT 10

/* TEV structures (similar to Aurora's TevPass, TevOp, TevStage) */
/* TevPass - stores 4 arguments (a, b, c, d) for color or alpha pass */
typedef struct {
    u32 a;  /* GXTevColorArg or GXTevAlphaArg */
    u32 b;  /* GXTevColorArg or GXTevAlphaArg */
    u32 c;  /* GXTevColorArg or GXTevAlphaArg */
    u32 d;  /* GXTevColorArg or GXTevAlphaArg */
} TevPass;

/* TevOp - stores TEV operation parameters */
typedef struct {
    u32 op;      /* GXTevOp */
    u32 bias;    /* GXTevBias */
    u32 scale;   /* GXTevScale */
    u32 outReg;  /* GXTevRegID */
    BOOL clamp;  /* GXBool */
    u8 _p1;      /* Padding */
    u8 _p2;      /* Padding */
    u8 _p3;      /* Padding */
} TevOp;

/* TevStage - stores complete TEV stage configuration */
typedef struct {
    TevPass colorPass;  /* Color pass arguments */
    TevPass alphaPass;  /* Alpha pass arguments */
    TevOp colorOp;      /* Color operation */
    TevOp alphaOp;      /* Alpha operation */
    u32 texCoordId;     /* GXTexCoordID */
    u32 texMapId;       /* GXTexMapID */
    u32 channelId;      /* GXChannelID */
    /* TODO: Add other TEV stage fields (kcSel, kaSel, etc.) as needed */
} TevStage;

/* VtxAttrFmt - vertex attribute format (similar to Aurora's VtxAttrFmt) */
typedef struct {
    u32 cnt;   /* GXCompCnt */
    u32 type;  /* GXCompType */
    u8 frac;   /* Fractional bits */
} VtxAttrFmt;

/* VtxFmt - vertex format (similar to Aurora's VtxFmt) */
typedef struct {
    VtxAttrFmt attrs[GX_VA_MAX_ATTR];  /* Format for each attribute */
} VtxFmt;

/* AttrArray - vertex array (similar to Aurora's AttrArray) */
typedef struct {
    const void* data;  /* Pointer to array data */
    u32 size;          /* Size in bytes */
    u8 stride;         /* Stride in bytes */
} AttrArray;

/* GX State structure - similar to Aurora's GXState
 * This is exposed here so update_gx_state can access fields directly
 */
struct GXState {
    BOOL initialized;
    BOOL stateDirty;  // True when state needs to be updated
    
    /* OpenGL state */
    u32 currentProgram;
    u32 currentVAO;
    u32 currentVBO;
    u32 currentEBO;
    
    /* Rendering state */
    BOOL depthTest;
    BOOL blendEnabled;
    BOOL cullFace;
    
    /* Viewport */
    s32 viewportX;
    s32 viewportY;
    u32 viewportWidth;
    u32 viewportHeight;
    
    /* Scissor */
    s32 scissorX;
    s32 scissorY;
    u32 scissorWidth;
    u32 scissorHeight;
    
    /* Clear color (Vec4<float> in Aurora) - matches GXColorF32 */
    GXColorF32 clearColor;
    
    /* Cull mode (similar to Aurora's cullMode) */
    u32 cullMode;  /* GXCullMode */
    
    /* Blend mode (similar to Aurora's blendMode, blendFacSrc, blendFacDst, blendOp) */
    u32 blendMode;      /* GXBlendMode */
    u32 blendFacSrc;   /* GXBlendFactor */
    u32 blendFacDst;   /* GXBlendFactor */
    u32 blendOp;       /* GXLogicOp */
    
    /* Projection matrix (similar to Aurora's proj, projType) */
    f32 proj[4][4];     /* 4x4 projection matrix */
    u32 projType;       /* GXProjectionType */
    
    /* Position/Normal matrices (similar to Aurora's PnMtx struct) */
    struct {
        Mtx pos;  /* Position matrix (3x4) */
        Mtx nrm;  /* Normal matrix (3x4) */
    } pnMtx[GX_PNMTX_COUNT];  /* Array of 10 PnMtx structures */
    u32 currentPnMtx;         /* Current PN matrix slot (0-9, not GX_PNMTX enum) */
    
    /* Texture matrices (similar to Aurora's texMtxs array) */
    Mtx44 texMtx[GX_TEXMTX_COUNT];
    GXTexMtxType texMtxType[GX_TEXMTX_COUNT];
    Mtx44 texIdentityMtx;
    
    /* Vertex descriptor (similar to Aurora's vtxDesc) */
    u32 vtxDesc[GX_VA_MAX_ATTR];  /* GXAttrType for each attribute */
    
    /* Vertex formats (similar to Aurora's vtxFmts array) */
    VtxFmt vtxFmts[GX_MAX_VTXFMT];  /* Array of vertex formats */
    
    /* Vertex arrays (similar to Aurora's arrays) */
    AttrArray arrays[GX_VA_MAX_ATTR];  /* Vertex arrays for indexed attributes */
    
    /* Color channels (similar to Aurora's numChans) */
    u8 numChans;                        /* Number of color channels */
    
    /* Fog state (similar to Aurora's FogState) */
    u32 fogType;        /* GXFogType */
    f32 fogStartZ;      /* Fog start Z */
    f32 fogEndZ;        /* Fog end Z */
    f32 fogNearZ;       /* Fog near Z */
    f32 fogFarZ;        /* Fog far Z */
    GXColorF32 fogColor; /* Fog color */
    
    /* TEV stages (similar to Aurora's tevStages array) */
    TevStage tevStages[GX_MAX_TEVSTAGE];  /* Array of TEV stages */
    u8 numTevStages;                      /* Number of active TEV stages */
    
    /* Texture coordinate generation (similar to Aurora's numTexGens) */
    u8 numTexGens;                        /* Number of texture coordinate generators */
    
    /* Pixel operations (similar to Aurora's depthCompare, depthFunc, depthUpdate, colorUpdate, alphaUpdate) */
    BOOL depthCompare;                    /* Enable depth comparison */
    u32 depthFunc;                        /* GXCompare function for depth */
    BOOL depthUpdate;                     /* Enable depth buffer updates */
    BOOL colorUpdate;                     /* Enable color buffer updates */
    BOOL alphaUpdate;                     /* Enable alpha buffer updates */
    
    /* TODO: Add more state as needed:
     * - Textures
     * - Vertex formats
     * - Lighting
     * etc.
     */
};

typedef struct GXState GXState;

/* Get the global GX state */
GXState* GXGetState(void);

/* Aurora's g_gxState - exposed for update_gx_state pattern */
extern GXState* g_gxState;

/* Initialize GX state */
void GXStateInit(void);

/* Shutdown GX state */
void GXStateShutdown(void);

/* Set clear color (normalized 0.0-1.0) */
void GXStateSetClearColor(f32 r, f32 g, f32 b, f32 a);

/* Get clear color */
void GXStateGetClearColor(f32* r, f32* g, f32* b, f32* a);

/* Mark state as dirty (for update_gx_state helpers) */
void GXStateMarkDirty(void);

/* Apply GX state to OpenGL (called when state is dirty) */
void GXStateApply(void);

/* Get direct pointers to GXState fields (for update_gx_state pattern) */
/* Returns NULL if field doesn't exist - this will trigger alerts */
u32* GXStateGetCullModePtr(void);
u32* GXStateGetBlendModePtr(void);
u32* GXStateGetBlendFacSrcPtr(void);
u32* GXStateGetBlendFacDstPtr(void);
u32* GXStateGetBlendOpPtr(void);

/* Access GX state fields (for update_gx_state helpers) */
/* These will show alerts if fields don't exist yet */
void GXStateSetCullMode(u32 mode);
u32 GXStateGetCullMode(void);
void GXStateSetBlendMode(u32 mode);
u32 GXStateGetBlendMode(void);
void GXStateSetBlendFacSrc(u32 src);
u32 GXStateGetBlendFacSrc(void);
void GXStateSetBlendFacDst(u32 dst);
u32 GXStateGetBlendFacDst(void);
void GXStateSetBlendOp(u32 op);
u32 GXStateGetBlendOp(void);

#endif

