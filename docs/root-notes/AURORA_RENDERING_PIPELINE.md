# Aurora Rendering Pipeline - Function Call Tree

This document traces the complete rendering pipeline from application start to final screen display, showing exactly which functions need to be implemented.

**Based on:** Aurora's rendering architecture and `onetri_demo` example  
**Last Updated:** 2025-01-XX

---

## Phase 1: Application Initialization

```
main()
├── DEMOInit(NULL)
    ├── OSInit()                          ✅ IMPLEMENTED
    ├── DVDInit()                         ✅ IMPLEMENTED
    ├── VIInit()                          ✅ IMPLEMENTED
    │   ├── SDL_Init(SDL_INIT_VIDEO)
    │   ├── SDL_CreateWindow()
    │   ├── SDL_GL_CreateContext()
    │   └── SDL_GL_SetSwapInterval(1)     [VSync]
    │
    ├── DEMOPadInit()                     ✅ IMPLEMENTED
    │   └── PADInit()
    │
    ├── DEMOSetRenderMode(mode)
    │   └── [Sets default render mode]
    │
    ├── DEMOConfigureMem()
    │   └── [Allocates framebuffers - stub on PC]
    │
    ├── GXInit(fifoBuffer, fifoSize)      ✅ IMPLEMENTED
    │   ├── GXInitGraphics()              ✅ IMPLEMENTED
    │   │   ├── GXStateInit()             ✅ IMPLEMENTED
    │   │   ├── gfx_init_command_queue()   ✅ IMPLEMENTED
    │   │   ├── GXGLLoadFunctions()        ✅ IMPLEMENTED
    │   │   │   └── [Loads OpenGL 3.3+ functions via wglGetProcAddress]
    │   │   ├── GXShaderInit()             ✅ IMPLEMENTED
    │   │   │   ├── CompileShader(GL_VERTEX_SHADER)
    │   │   │   ├── CompileShader(GL_FRAGMENT_SHADER)
    │   │   │   └── LinkProgram()
    │   │   └── [Set default OpenGL state: depth test, blend, cull face]
    │   │
    │   └── [Returns GXFifoObj* - stub]
    │
    ├── DEMOInitGX()
    │   ├── GXSetCopyClear(color, zClear) ✅ IMPLEMENTED
    │   ├── GXSetViewport(...)            ✅ IMPLEMENTED
    │   │   └── gfx_set_viewport(...)     ✅ IMPLEMENTED
    │   │       └── gfx_push_command(COMMAND_TYPE_SET_VIEWPORT, ...)
    │   └── GXSetScissor(...)             ✅ IMPLEMENTED
    │       └── gfx_set_scissor(...)      ✅ IMPLEMENTED
    │           └── gfx_push_command(COMMAND_TYPE_SET_SCISSOR, ...)
    │
    └── DEMOStartVI()
        ├── VIConfigure(rmode)            ✅ IMPLEMENTED
        ├── VISetNextFrameBuffer(fb1)     ✅ IMPLEMENTED
        ├── VIFlush()                     ✅ IMPLEMENTED
        └── VIWaitForRetrace()            ✅ IMPLEMENTED
            └── [Waits for VSync - SDL_GL_SwapWindow with VSync]
```

---

## Phase 2: Per-Frame Setup (Camera, Vertex Format)

```
main()
└── CameraInit(v)
    ├── MTXFrustum(p, left, right, bottom, top, znear, zfar)  ✅ IMPLEMENTED
    └── GXSetProjection(p, GX_PERSPECTIVE)                    ✅ IMPLEMENTED
        └── [Stores projection matrix in GXState]

main()
└── DrawInit()
    ├── GXSetCopyClear(black, zClear)                         ✅ IMPLEMENTED
    │   └── [Sets clear color in GXState]
    │
    ├── GXClearVtxDesc()                                      ✅ IMPLEMENTED
    │   └── [Clears vertex descriptor array]
    │
    ├── GXSetVtxDesc(GX_VA_POS, GX_INDEX8)                   ✅ IMPLEMENTED
    │   └── [Sets vertex descriptor for position attribute]
    │
    ├── GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8)                  ✅ IMPLEMENTED
    │   └── [Sets vertex descriptor for color attribute]
    │
    ├── GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, ...)          ✅ IMPLEMENTED
    │   └── [Sets vertex format: 3 components, s16, no fraction]
    │
    ├── GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, ...)         ✅ IMPLEMENTED
    │   └── [Sets vertex format: 4 components, RGBA8, no fraction]
    │
    ├── GXSetArray(GX_VA_POS, Verts_s16, stride)             ✅ IMPLEMENTED
    │   └── [Stores array pointer, stride; size=0 (inferred later)]
    │
    ├── GXSetArray(GX_VA_CLR0, Colors_rgba8, stride)          ✅ IMPLEMENTED
    │   └── [Stores array pointer, stride; size=0 (inferred later)]
    │
    ├── GXSetNumChans(1)                                      ✅ IMPLEMENTED
    │   └── [Sets number of color channels]
    │
    ├── GXSetNumTexGens(0)                                    ✅ IMPLEMENTED
    │   └── [Sets number of texture coordinate generators]
    │
    ├── GXSetTevOrder(GX_TEVSTAGE0, ...)                      ✅ IMPLEMENTED
    │   └── [Sets TEV stage texture/color order]
    │
    └── GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR)                 ⚠️  STUB
        └── [Sets TEV operation - currently stub]
```

---

## Phase 3: Main Rendering Loop (Per Frame)

```
main()
└── while(!quit)
    ├── DEMOBeforeRender()
    │   ├── gfx_begin_frame()                                 ✅ IMPLEMENTED
    │   │   ├── [Clears render passes]
    │   │   ├── [Creates new render pass (pass 0)]
    │   │   └── [Sets clear color from GXState]
    │   │
    │   ├── GXSetViewport(...)                                ✅ IMPLEMENTED
    │   │   └── gfx_set_viewport(...)
    │   │       └── gfx_push_command(COMMAND_TYPE_SET_VIEWPORT, ...)
    │   │
    │   ├── GXInvalidateVtxCache()                            ✅ IMPLEMENTED (no-op)
    │   └── GXInvalidateTexAll()                              ✅ IMPLEMENTED (no-op)
    │
    ├── DrawTick(v)
    │   ├── AnimTick(v)
    │   │   └── MTXLookAt(v, camLoc, up, objPt)               ✅ IMPLEMENTED
    │   │
    │   ├── GXLoadPosMtxImm(mv, GX_PNMTX0)                    ✅ IMPLEMENTED
    │   │   └── [Stores position matrix in GXState->pnMtx[0].pos]
    │   │
    │   ├── GXBegin(GX_QUADS, GX_VTXFMT0, 36)                 ✅ IMPLEMENTED
    │   │   ├── CalculateVertexSize()                         ✅ IMPLEMENTED
    │   │   │   └── [Calculates vertex size from vtxDesc]
    │   │   ├── [Allocates vertex buffer]
    │   │   ├── [Allocates index buffer]
    │   │   └── [Initializes StreamState]
    │   │
    │   ├── [Vertex Commands - Multiple calls]
    │   │   ├── GXPosition1x8(v)                              ✅ IMPLEMENTED
    │   │   │   └── EmGXPosition1x8(v)                        ✅ IMPLEMENTED
    │   │   │       └── [Appends index to stream buffer]
    │   │   │
    │   │   └── GXColor1x8(c)                                 ✅ IMPLEMENTED
    │   │       └── EmGXColor1x8(c)                            ✅ IMPLEMENTED
    │   │           └── [Appends index to stream buffer]
    │   │
    │   └── GXEnd()                                            ✅ IMPLEMENTED
    │       ├── [Resolves indexed attributes to vertex data]
    │       ├── [Queues COMMAND_TYPE_DRAW command]
    │       │   └── gfx_push_command(COMMAND_TYPE_DRAW, drawCmd)
    │       └── [Copies vertex/index data to command]
    │
    └── DEMODoneRender()
        ├── GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE)           ✅ IMPLEMENTED
        │   └── [Sets depth compare, func, update in GXState]
        │
        ├── GXSetColorUpdate(GX_TRUE)                         ✅ IMPLEMENTED
        │   └── [Sets colorUpdate in GXState]
        │
        ├── GXCopyDisp(buffer, GX_TRUE)                       ⚠️  STUB
        │   └── [Display copy - stub for now]
        │
        ├── GXDrawDone()                                      ✅ IMPLEMENTED (no-op)
        │   └── [Ensures all commands are queued]
        │
        ├── gfx_end_frame()                                   ✅ IMPLEMENTED
        │   └── [Marks frame as ended, sets currentRenderPass to invalid]
        │
        ├── gfx_render()                                      ✅ IMPLEMENTED
        │   ├── [Sets up OpenGL state: depth test, blend, cull face]
        │   ├── [Iterates through render passes]
        │   │   ├── glClearColor(...)
        │   │   ├── glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        │   │   └── [Processes commands for this pass]
        │   │       ├── COMMAND_TYPE_SET_VIEWPORT
        │   │       │   └── glViewport(...)
        │   │       │
        │   │       ├── COMMAND_TYPE_SET_SCISSOR
        │   │       │   ├── glEnable(GL_SCISSOR_TEST)
        │   │       │   └── glScissor(...)
        │   │       │
        │   │       └── COMMAND_TYPE_DRAW
        │   │           ├── [First pass: Find max index per attribute]
        │   │           ├── [Calculate effective size: (maxIndex + 1) * stride]
        │   │           ├── [Second pass: Resolve indices to vertex data]
        │   │           ├── [Build interleaved vertex buffer for OpenGL]
        │   │           ├── glUseProgram(shaderProgram)
        │   │           ├── glUniformMatrix4fv(uProjection, ...)
        │   │           ├── glUniformMatrix4fv(uModelView, ...)
        │   │           ├── glGenVertexArrays(1, &vao)
        │   │           ├── glBindVertexArray(vao)
        │   │           ├── glGenBuffers(1, &vbo)
        │   │           ├── glBindBuffer(GL_ARRAY_BUFFER, vbo)
        │   │           ├── glBufferData(GL_ARRAY_BUFFER, ...)
        │   │           ├── glEnableVertexAttribArray(0)  [Position]
        │   │           ├── glVertexAttribPointer(0, ...)
        │   │           ├── glEnableVertexAttribArray(1)  [Color]
        │   │           ├── glVertexAttribPointer(1, ...)
        │   │           ├── glDrawArrays(primitive, 0, numVertices)
        │   │           ├── glDeleteBuffers(1, &vbo)
        │   │           └── glDeleteVertexArrays(1, &vao)
        │   │
        │   └── SDL_GL_SwapWindow(window)                      ✅ IMPLEMENTED
        │       └── [Presents frame to screen - FINAL STEP]
        │
        └── DEMOSwapBuffers()
            └── [Swaps framebuffer pointers - stub on PC]
```

---

## Function Implementation Status

### ✅ Fully Implemented

**Initialization:**
- `OSInit()`, `DVDInit()`, `VIInit()`
- `GXInit()`, `GXInitGraphics()`
- `GXStateInit()`, `gfx_init_command_queue()`
- `GXGLLoadFunctions()`, `GXShaderInit()`

**State Management:**
- `GXSetProjection()`, `GXLoadPosMtxImm()`
- `GXSetViewport()`, `GXSetScissor()`
- `GXSetCopyClear()`, `GXSetZMode()`, `GXSetColorUpdate()`
- `GXClearVtxDesc()`, `GXSetVtxDesc()`, `GXSetVtxAttrFmt()`
- `GXSetArray()` (with dynamic size inference)
- `GXSetNumChans()`, `GXSetNumTexGens()`, `GXSetTevOrder()`

**Vertex Processing:**
- `GXBegin()`, `GXEnd()`
- `GXPosition1x8()`, `GXColor1x8()`
- `EmGXPosition1x8()`, `EmGXColor1x8()`
- Stream state management

**Frame Management:**
- `gfx_begin_frame()`, `gfx_end_frame()`, `gfx_render()`
- Command queue system
- OpenGL rendering pipeline

**Matrix Functions:**
- `MTXFrustum()`, `MTXLookAt()`, `MTXIdentity()`, `MTXConcat()`, `MTXTrans()`

### ⚠️ Partially Implemented / Stubs

**Texture System:**
- `GXInitTexObj()` - STUB
- `GXLoadTexObj()` - STUB
- `GXDestroyTexObj()` - Ready (no-op until texture loading implemented)

**TEV System:**
- `GXSetTevOp()` - STUB (needs full TEV operation implementation)

**Display:**
- `GXCopyDisp()` - STUB (framebuffer copy - not needed for OpenGL)

### ❌ Not Yet Implemented

**Advanced Features:**
- Texture loading and management
- Full TEV pipeline (color/alpha operations)
- Lighting system
- Fog effects
- Display lists
- Additional vertex attribute types (normals, texcoords, etc.)

---

## Critical Path for Basic Rendering

For a simple triangle/quad to render on screen, these functions **MUST** be implemented:

1. ✅ `OSInit()` - OS initialization
2. ✅ `VIInit()` - Video interface (SDL window + OpenGL context)
3. ✅ `GXInit()` / `GXInitGraphics()` - Graphics initialization
4. ✅ `GXSetProjection()` - Projection matrix
5. ✅ `GXSetViewport()` / `GXSetScissor()` - Viewport setup
6. ✅ `GXClearVtxDesc()` / `GXSetVtxDesc()` - Vertex descriptor
7. ✅ `GXSetVtxAttrFmt()` - Vertex format
8. ✅ `GXSetArray()` - Vertex arrays
9. ✅ `GXBegin()` / `GXEnd()` - Drawing commands
10. ✅ `GXPosition1x8()` / `GXColor1x8()` - Vertex data
11. ✅ `GXLoadPosMtxImm()` - Modelview matrix
12. ✅ `gfx_begin_frame()` / `gfx_end_frame()` / `gfx_render()` - Frame management
13. ✅ `SDL_GL_SwapWindow()` - Final display

**All critical path functions are now implemented!** ✅

---

## Notes

1. **Array Size Inference**: `GXSetArray()` uses dynamic size inference - size is calculated from maximum index used during rendering, so unmodified demo code works without providing size parameter.

2. **Texture Destruction**: `GXDestroyTexObj()` is ready but currently no-op until texture loading is implemented. When `GXLoadTexObj()` is implemented, it will track GPU handles and free them here.

3. **Command Queue**: Aurora uses a deferred command queue system. Commands are queued during the frame and executed in `gfx_render()`. This matches Aurora's architecture.

4. **OpenGL State**: OpenGL state (depth test, blend, cull face) is set up in `gfx_render()` before processing commands, ensuring consistent state.

5. **Shader Program**: Basic shader program is created in `GXShaderInit()` and used for all rendering. Uniforms (projection, modelview) are set per draw command.

---

## Next Steps

1. **Implement `GXSetTevOp()`** - Full TEV operation support
2. **Implement texture loading** - `GXInitTexObj()`, `GXLoadTexObj()`
3. **Add more vertex attribute types** - Normals, texcoords, etc.
4. **Implement lighting** - GX lighting system
5. **Add display list support** - For optimized rendering

