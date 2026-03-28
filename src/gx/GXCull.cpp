#include "gx.hpp"

extern "C" {

void GXSetScissor(u32 left, u32 top, u32 wd, u32 ht) {
  update_gx_state(g_gxState().scissorLeft, left);
  update_gx_state(g_gxState().scissorTop, top);
  update_gx_state(g_gxState().scissorWd, wd);
  update_gx_state(g_gxState().scissorHt, ht);
}

void GXSetCullMode(GXCullMode mode) { update_gx_state(g_gxState().cullMode, mode); }

void GXSetCoPlanar(GXBool) { /* stub: no-op on PC */ }
}