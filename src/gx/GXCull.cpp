#include "gx.hpp"

extern "C" {

void GXSetScissor(u32, u32, u32, u32) {}

void GXSetCullMode(GXCullMode mode) { update_gx_state(g_gxState.cullMode, mode); }

// TODO GXSetCoPlanar
}