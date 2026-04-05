#include "gx.hpp"

#include "../gfx/render.hpp"

extern "C" {
static GXDrawSyncCallback DrawSyncCB = nullptr;
static GXDrawDoneCallback DrawDoneCB = nullptr;
static GXVerifyCallback VerifyCB = nullptr;
static GXVerifyLevel VerifyLevel = GX_WARN_NONE;
static u16 DrawSyncToken = 0;
static BOOL DrawDonePending = FALSE;

GXFifoObj* GXInit(void* base, u32 size) {
  (void)base;
  (void)size;
  DrawSyncCB = nullptr;
  DrawDoneCB = nullptr;
  VerifyCB = nullptr;
  VerifyLevel = GX_WARN_ALL;
  DrawSyncToken = 0;
  DrawDonePending = FALSE;
  return NULL;
}

void GXAbortFrame() { DrawDonePending = FALSE; }

void GXSetDrawSync(u16 token) {
  DrawSyncToken = token;
  if (DrawSyncCB != nullptr) DrawSyncCB(token);
}

u16 GXReadDrawSync(void) { return DrawSyncToken; }

GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback cb) {
  GXDrawSyncCallback old = DrawSyncCB;
  DrawSyncCB = cb;
  return old;
}

void GXDrawDone() {
  GXSetDrawDone();
  GXWaitDrawDone();
}

void GXSetDrawDone() {
  porpoise::gfx::flush_render_if_pending();
  DrawDonePending = TRUE;
}

void GXWaitDrawDone() {
  if (!DrawDonePending) return;
  porpoise::gfx::flush_render_if_pending();
  DrawDonePending = FALSE;
  if (DrawDoneCB != nullptr) DrawDoneCB();
}

GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback cb) {
  GXDrawDoneCallback old = DrawDoneCB;
  DrawDoneCB = cb;
  return old;
}

void GXFlush() { porpoise::gfx::flush_render_if_pending(); }

void GXResetWriteGatherPipe() {}

void GXPixModeSync() { porpoise::gfx::flush_render_if_pending(); }

void GXTexModeSync() { porpoise::gfx::flush_render_if_pending(); }

void GXSetVerifyLevel(GXVerifyLevel level) { VerifyLevel = level; }

GXVerifyCallback GXSetVerifyCallback(GXVerifyCallback cb) {
  GXVerifyCallback old = VerifyCB;
  VerifyCB = cb;
  return old;
}

volatile void* GXRedirectWriteGatherPipe(void* ptr) { return (volatile void*)ptr; }

void GXRestoreWriteGatherPipe(void) {}

BOOL IsWriteGatherBufferEmpty(void) { return TRUE; }

void GXSetMisc(u32 token, u32 val) {
  (void)token;
  (void)val;
}
}
