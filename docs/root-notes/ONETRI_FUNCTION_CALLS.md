# onetri_test.c Function Call Tree

## main()
**Location:** `examples/onetri_test.c:22`

**Calls:**
- OSReport()
- OSInit()
- VIInit()
- GXInit()
- SDL_PollEvent()
- VIWaitForRetrace()
- gfx_begin_frame()
- GXSetCopyClear()
- GXStateApply()
- glGetError()
- glClear()
- GXFlush()
- GXDrawDone()
- gfx_end_frame()
- gfx_render()
- GXCopyDisp()
- SDL_GL_SwapWindow()
- SDL_GL_GetCurrentWindow()
- GXSetViewport()
- GXSetScissor()
- GXSetCullMode()
- GXSetBlendMode()
- GXSetProjection()
- GXSetVtxDesc()
- GXSetFog()
- GXSetTevOp()

---

## OSReport() //// DONE ////
**Location:** `src/os/OS.c`

**Calls:**
- (OS internal - printf wrapper)

---

## OSInit() //// DONE ////
**Location:** `src/os/OS.c`

**Calls:**
- (OS internal initialization)

---

## VIInit() //// DONE? ////
**Location:** `src/vi/VI.c`

**Calls:**
- SDL_Init()
- SDL_CreateWindow()
- SDL_GL_CreateContext()
- GXInitGraphics() (after OpenGL context is created)
- (VI internal setup)

---

## GXInit() //// DONE ////
**Location:** `src/gx/GXManage.c:36`

**Calls:**
- (No function calls - just returns NULL, matching Aurora's stub)


---

## SDL_PollEvent() //// DONE ////
**Location:** SDL2 library
**Note:** Only used for window close events (SDL_QUIT). Input should be handled via PAD API.

**Calls:**
- (SDL internal)

---

## VIWaitForRetrace() //// DONE ////
**Location:** `src/vi/VI.c`

**Calls:**
- OSSleepTicks() (waits for retrace count to increment)

---

## gfx_begin_frame() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:256`

**Calls:**
- GXGetState() (to get clear color from GX state)
- free() (to clear old render pass command lists)
- OSReport() - "[GX STUB] gfx_begin_frame: Buffer mapping (WebGPU) not implemented for OpenGL backend"

---

## ensure_render_pass() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:29` (static)
**Note:** Called from gfx_push_command() to ensure render pass exists

**Calls:**
- OSReport() - "[GX WARN] ensure_render_pass: No active render pass..." (warning if called without begin_frame)
- GXGetState() (to get clear color if creating default render pass)

---

## GXSetCopyClear() //// DONE ////
**Location:** `src/gx/GXFrameBuffer.c:46`

**Calls:**
- gx_from_gx_color()
- update_gx_state()

---

## gx_from_gx_color() //// DONE ////
**Location:** `src/gfx/gx_state_helpers.c:104`

**Calls:**
- (No function calls - just converts GXColor to GXColorF32)

---

## update_gx_state() //// DONE ////
**Location:** `src/gfx/gx_update_state.h` (macro)

**Calls:**
- update_gx_state_impl_colorf32() [for GXColorF32]
- update_gx_state_impl_u32() [for u32]
- update_gx_state_impl_f32() [for f32]
- update_gx_state_impl_bool() [for BOOL]
- GXStateMarkDirty()

---

## update_gx_state_impl_colorf32() //// DONE ////
**Location:** `src/gfx/gx_update_state.h:56` (static inline)

**Calls:**
- GXStateMarkDirty()

---

## GXStateMarkDirty() //// DONE ////
**Location:** `src/gfx/gx_state.c:171`

**Calls:**
- GXGetState()

---

## GXGetState() //// DONE ////
**Location:** `src/gfx/gx_state.c:29`

**Calls:**
- GXStateInit() [if not initialized]

---

## GXStateApply() //// DONE ////
**Location:** `src/gfx/gx_state.c:312`

**Calls:**
- GXGetState()
- OSReport() [error/debug messages]
- glClearColor()
- glGetError()
- glViewport()
- glEnable() [GL_SCISSOR_TEST]
- glDisable() [GL_SCISSOR_TEST]
- glScissor()
- glEnable() [GL_CULL_FACE]
- glDisable() [GL_CULL_FACE]
- glCullFace()
- glEnable() [GL_BLEND]
- glDisable() [GL_BLEND]
- glBlendFunc()

---

## glGetError()
**Location:** OpenGL library

**Calls:**
- (OpenGL internal)

---

## glClear()
**Location:** OpenGL library

**Calls:**
- (OpenGL internal)

---

## GXFlush() //// DONE ////
**Location:** `src/gx/GXManage.c:73`

**Calls:**
- (No-op - no function calls, matching Aurora's implementation)

---

## GXDrawDone() //// DONE ////
**Location:** `src/gx/GXManage.c:108`

**Calls:**
- (Calls callback if set - no other function calls, matching Aurora's implementation)

---

## gfx_end_frame() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:272`

**Calls:**
- OSReport() - "[GX STUB] gfx_end_frame: Buffer unmapping/writing (WebGPU) not implemented for OpenGL backend"
- OSReport() - "[GX STUB] gfx_end_frame: Texture uploads (WebGPU) not implemented for OpenGL backend"

---

## gfx_render() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:320`

**Calls:**
- glClearColor()
- glClear()
- glViewport()
- glEnable() [GL_SCISSOR_TEST]
- glScissor()
- OSReport() - "[GX WARN] gfx_render: Draw commands not yet implemented"
- OSReport() - "[GX WARN] gfx_render: Unknown command type..."
- OSReport() - "[GX STUB] gfx_render: Resolve targets and texture copies (WebGPU) not implemented for OpenGL backend"
- OSReport() - "[GX STUB] gfx_render: WebGPU render pass descriptors not implemented for OpenGL backend"

---

## GXCopyDisp() //// DONE ////
**Location:** `src/gx/GXFrameBuffer.c:80`

**Calls:**
- (No function calls - no-op stub matching Aurora)

---

## SDL_GL_SwapWindow() //// DONE ////
**Location:** SDL2 library
**Note:** External SDL2 function - swaps OpenGL buffers. Should be called via VIFlush() or similar.

**Calls:**
- (SDL internal)

---

## SDL_GL_GetCurrentWindow() //// DONE ////
**Location:** SDL2 library
**Note:** External SDL2 function - gets current OpenGL window. Used for buffer swapping.

**Calls:**
- (SDL internal)

---

## GXSetViewport() //// DONE ////
**Location:** `src/gx/GXTransform.c:42`

**Calls:**
- gfx_set_viewport()

---

## gfx_set_viewport() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:185`

**Calls:**
- GXGetState()
- gfx_push_command()
- glViewport()

---

## gfx_push_command() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:117`

**Calls:**
- ensure_render_pass()
- realloc() [if command list needs to grow]
- OSReport() [error/warning messages]

---

## GXSetScissor() //// DONE ////
**Location:** `src/gx/GXCull.c:7`

**Calls:**
- gfx_set_scissor()

---

## gfx_set_scissor() //// DONE ////
**Location:** `src/gfx/gx_helpers.c:221`

**Calls:**
- GXGetState()
- gfx_push_command()
- glScissor()

---

## GXSetCullMode() //// DONE ////
**Location:** `src/gx/GXCull.c:19`

**Calls:**
- update_gx_state()

---

## GXSetBlendMode() //// DONE ////
**Location:** `src/gx/GXPixel.c:25`

**Calls:**
- update_gx_state() [4 times - for blendMode, blendFacSrc, blendFacDst, blendOp]
- glDisable() [GL_BLEND]
- glEnable() [GL_BLEND]
- glBlendFunc()

---

## GXSetProjection() //// DONE ////
**Location:** `src/gx/GXTransform.c:7`

**Calls:**
- OSReport() - "[GX ERROR] GXSetProjection: NULL matrix pointer" [if error]
- update_gx_state() [for projType]
- memcmp() [to check if matrix changed]
- memcpy() [to copy matrix]
- GXStateMarkDirty() [if matrix changed]

---

## GXSetVtxDesc() //// DONE ////
**Location:** `src/gx/GXGeometry.c:5`

**Calls:**
- OSReport() - "[GX ERROR] GXSetVtxDesc: Invalid attribute..." [if error]
- update_gx_state() [for vtxDesc[attr]]

---

## GXSetFog() //// DONE ////
**Location:** `src/gx/GXPixel.c:10`

**Calls:**
- update_gx_state() [6 times - for fogType, fogStartZ, fogEndZ, fogNearZ, fogFarZ, fogColor]
- gx_from_gx_color() [to convert GXColor to GXColorF32]

---

## GXSetTevOp() //// DONE ////
**Location:** `src/gx/GXTev.c:5`

**Calls:**
- GXSetTevColorIn() [for each mode case]
- GXSetTevAlphaIn() [for each mode case]
- GXSetTevColorOp() [at end]
- GXSetTevAlphaOp() [at end]
- OSReport() - "[GX WARN] GXSetTevOp: Unknown mode..." [if unknown mode]
