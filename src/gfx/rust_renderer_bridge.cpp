#include "rust_renderer_bridge.hpp"

#include "window.hpp"

#include <array>
#include <cstring>

#include <SDL.h>
#include <SDL_syswm.h>
#include <dolphin/vi.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace porpoise::gfx::bridge {
namespace {

struct RustRange {
  u32 offset;
  u32 size;
};

struct RustDrawCommand {
  RustRange vert_range;
  RustRange idx_range;
  u32 index_count;
  u32 primitive;
  u32 vtx_fmt;
  u32 dst_alpha;
  u32 indexed_stride;
  u32 pos_index_offset;
  u32 color_index_offset;
  u32 pos_index_size;
  u32 color_index_size;
  u32 pos_array_stride;
  u32 color_array_stride;
  float model_view[16];
  float proj[16];
  float viewport_left, viewport_top, viewport_width, viewport_height;
  float viewport_near, viewport_far;
  u32 scissor_left, scissor_top, scissor_wd, scissor_ht;
  float mat_color[4];
  std::array<RustRange, GX_VA_MAX_ATTR> array_ranges;
};

struct RustFrameState {
  u16 fb_width;
  u16 fb_height;
  float clear_color[4];
  float proj[16];
  float model_view[16];
  u32 depth_compare;
  u32 depth_update;
  u32 color_update;
  u32 alpha_update;
  u32 cull_mode;
  u32 blend_mode;
  u32 blend_src;
  u32 blend_dst;
  u32 blend_op;
  u32 depth_func;
  u32 num_tev_stages;
  u32 num_tex_gens;
  u32 num_chans;
  u32 scissor_left;
  u32 scissor_top;
  u32 scissor_wd;
  u32 scissor_ht;
};

using InitFn = int (*)();
using ShutdownFn = void (*)();
using NotifyStateFn = void (*)(u32);
using BeginFrameFn = int (*)(const RustFrameState*);
using PushDrawFn = void (*)(const RustDrawCommand*);
using RenderFn = int (*)(const u8*, u32, const u8*, u32, const RustFrameState*);
using CopyDispFn = int (*)(void*, u16, u16, u16, u16, u8, const RustFrameState*);
using CopyTexFn = int (*)(void*, u16, u16, u16, u16, u32, u8, const RustFrameState*);
using SetWindowFn = void (*)(u64, u64, u32, u32);
using LastStatusFn = const char* (*)();

struct Api {
#ifdef _WIN32
  HMODULE handle = nullptr;
#else
  void* handle = nullptr;
#endif
  InitFn init = nullptr;
  ShutdownFn shutdown = nullptr;
  NotifyStateFn notify_state = nullptr;
  BeginFrameFn begin_frame = nullptr;
  PushDrawFn push_draw = nullptr;
  RenderFn render = nullptr;
  CopyDispFn copy_disp = nullptr;
  CopyTexFn copy_tex = nullptr;
  SetWindowFn set_window = nullptr;
  LastStatusFn last_status = nullptr;
  bool initialized = false;
  bool attempted_load = false;
  bool window_sent = false;
};

Api g_api{};
const char* g_lastStatus = "bridge_uninitialized";

RustFrameState to_rust_frame_state(const gx::GXState& state) {
  RustFrameState out{};
  const auto size = window::get_window_size();
  out.fb_width = size.fb_width;
  out.fb_height = size.fb_height;
  out.clear_color[0] = state.clearColor.x();
  out.clear_color[1] = state.clearColor.y();
  out.clear_color[2] = state.clearColor.z();
  out.clear_color[3] = state.clearColor.w();
  for (int i = 0; i < 16; ++i) {
    out.proj[i] = state.proj.m[i];
  }
  const auto& pnMtx = state.pnMtx[state.currentPnMtx];
  for (int i = 0; i < 16; ++i) {
    out.model_view[i] = 0.0f;
  }
  out.model_view[15] = 1.0f;
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 4; ++col) {
      out.model_view[col * 4 + row] = pnMtx.pos(row, col);
    }
  }
  out.depth_compare = state.depthCompare ? 1u : 0u;
  out.depth_update = state.depthUpdate ? 1u : 0u;
  out.color_update = state.colorUpdate ? 1u : 0u;
  out.alpha_update = state.alphaUpdate ? 1u : 0u;
  out.cull_mode = static_cast<u32>(state.cullMode);
  out.blend_mode = static_cast<u32>(state.blendMode);
  out.blend_src = static_cast<u32>(state.blendFacSrc);
  out.blend_dst = static_cast<u32>(state.blendFacDst);
  out.blend_op = static_cast<u32>(state.blendOp);
  out.depth_func = static_cast<u32>(state.depthFunc);
  out.num_tev_stages = state.numTevStages;
  out.num_tex_gens = state.numTexGens;
  out.num_chans = state.numChans;
  out.scissor_left = state.scissorLeft;
  out.scissor_top = state.scissorTop;
  out.scissor_wd = state.scissorWd;
  out.scissor_ht = state.scissorHt;
  return out;
}

RustDrawCommand to_rust_draw(const DrawData& data) {
  RustDrawCommand out{};
  out.vert_range = {data.vertRange.offset, data.vertRange.size};
  out.idx_range = {data.idxRange.offset, data.idxRange.size};
  out.index_count = data.indexCount;
  out.primitive = static_cast<u32>(data.primitive);
  out.vtx_fmt = static_cast<u32>(data.vtxFmt);
  out.dst_alpha = data.dstAlpha;
  out.indexed_stride = data.indexedStride;
  out.pos_index_offset = data.posIndexOffset;
  out.color_index_offset = data.colorIndexOffset;
  out.pos_index_size = data.posIndexSize;
  out.color_index_size = data.colorIndexSize;
  out.pos_array_stride = data.posArrayStride;
  out.color_array_stride = data.colorArrayStride;
  for (int i = 0; i < 16; ++i) {
    out.model_view[i] = data.modelView[i];
    out.proj[i] = data.proj[i];
  }
  out.viewport_left = data.viewportLeft;
  out.viewport_top = data.viewportTop;
  out.viewport_width = data.viewportWidth;
  out.viewport_height = data.viewportHeight;
  out.viewport_near = data.viewportNear;
  out.viewport_far = data.viewportFar;
  out.scissor_left = data.scissorLeft;
  out.scissor_top = data.scissorTop;
  out.scissor_wd = data.scissorWd;
  out.scissor_ht = data.scissorHt;
  for (int i = 0; i < 4; ++i) {
    out.mat_color[i] = data.matColor[i];
  }
  for (u32 i = 0; i < GX_VA_MAX_ATTR; ++i) {
    out.array_ranges[i] = {data.arrayRanges[i].offset, data.arrayRanges[i].size};
  }
  return out;
}

template <typename FnType>
FnType load_symbol(const char* name) {
#ifdef _WIN32
  return reinterpret_cast<FnType>(GetProcAddress(g_api.handle, name));
#else
  return reinterpret_cast<FnType>(dlsym(g_api.handle, name));
#endif
}

void try_load_api() {
  if (g_api.attempted_load) {
    return;
  }
  g_api.attempted_load = true;

#ifdef _WIN32
  g_api.handle = LoadLibraryA("porpoise_rust_renderer.dll");
#else
  g_api.handle = dlopen("libporpoise_rust_renderer.so", RTLD_NOW);
  if (!g_api.handle) {
    g_api.handle = dlopen("libporpoise_rust_renderer.dylib", RTLD_NOW);
  }
#endif
  if (!g_api.handle) {
    return;
  }

  g_api.init = load_symbol<InitFn>("porpoise_rust_renderer_init");
  g_api.shutdown = load_symbol<ShutdownFn>("porpoise_rust_renderer_shutdown");
  g_api.notify_state = load_symbol<NotifyStateFn>("porpoise_rust_renderer_notify_state");
  g_api.begin_frame = load_symbol<BeginFrameFn>("porpoise_rust_renderer_begin_frame");
  g_api.push_draw = load_symbol<PushDrawFn>("porpoise_rust_renderer_push_draw");
  g_api.render = load_symbol<RenderFn>("porpoise_rust_renderer_render");
  g_api.copy_disp = load_symbol<CopyDispFn>("porpoise_rust_renderer_copy_disp");
  g_api.copy_tex = load_symbol<CopyTexFn>("porpoise_rust_renderer_copy_tex");
  g_api.set_window = load_symbol<SetWindowFn>("porpoise_rust_renderer_set_window_info");
  g_api.last_status = load_symbol<LastStatusFn>("porpoise_rust_renderer_last_status");
  if (!g_api.init || !g_api.shutdown || !g_api.notify_state || !g_api.begin_frame || !g_api.push_draw ||
      !g_api.render || !g_api.copy_disp || !g_api.copy_tex || !g_api.set_window || !g_api.last_status) {
    g_api.handle = nullptr;
    return;
  }

  g_api.initialized = g_api.init() != 0;
}

bool ensure_loaded() {
  try_load_api();
  g_lastStatus = g_api.last_status ? g_api.last_status() : "bridge_no_status";
  return g_api.handle && g_api.initialized;
}

void ensure_window_info_sent(const gx::GXState& state) {
  if (!g_api.initialized || !g_api.set_window) {
    return;
  }
  SDL_Window* sdlWindow = VIGetSDLWindow();
  if (!sdlWindow) {
    return;
  }
  SDL_SysWMinfo wmInfo{};
  SDL_VERSION(&wmInfo.version);
  if (!SDL_GetWindowWMInfo(sdlWindow, &wmInfo)) {
    return;
  }

  u64 nativeWindow = 0;
  u64 nativeDisplay = 0;
#ifdef _WIN32
  nativeWindow = reinterpret_cast<u64>(wmInfo.info.win.window);
  nativeDisplay = reinterpret_cast<u64>(wmInfo.info.win.hinstance ? wmInfo.info.win.hinstance : GetModuleHandleA(nullptr));
#endif
  const auto size = window::get_window_size();
  g_api.set_window(nativeWindow, nativeDisplay, size.fb_width, size.fb_height);
  g_api.window_sent = nativeWindow != 0;
  (void)state;
}

} // namespace

void notify_state(Action action) {
  if (!ensure_loaded()) {
    return;
  }
  g_api.notify_state(static_cast<u32>(action));
}

bool begin_frame(const gx::GXState& state) {
  if (!ensure_loaded()) {
    g_lastStatus = "bridge_not_loaded";
    return false;
  }
  ensure_window_info_sent(state);
  const auto frame_state = to_rust_frame_state(state);
  const bool handled = g_api.begin_frame(&frame_state) != 0;
  g_lastStatus = g_api.last_status();
  return handled;
}

void push_draw_command(const DrawData& data) {
  if (!ensure_loaded()) {
    return;
  }
  const auto draw = to_rust_draw(data);
  g_api.push_draw(&draw);
}

bool render(const std::vector<u8>& vertex_buffer, const std::vector<u8>& index_buffer, const gx::GXState& state) {
  if (!ensure_loaded()) {
    g_lastStatus = "bridge_not_loaded";
    return false;
  }
  ensure_window_info_sent(state);
  const auto frame_state = to_rust_frame_state(state);
  const u8* vb = vertex_buffer.empty() ? nullptr : vertex_buffer.data();
  const u8* ib = index_buffer.empty() ? nullptr : index_buffer.data();
  const bool handled =
      g_api.render(vb, static_cast<u32>(vertex_buffer.size()), ib, static_cast<u32>(index_buffer.size()), &frame_state) != 0;
  g_lastStatus = g_api.last_status();
  return handled;
}

bool copy_disp(void* dest, u16 left, u16 top, u16 width, u16 height, GXBool clear, const gx::GXState& state) {
  if (!ensure_loaded()) {
    g_lastStatus = "bridge_not_loaded";
    return false;
  }
  const auto frame_state = to_rust_frame_state(state);
  const bool handled = g_api.copy_disp(dest, left, top, width, height, clear == GX_TRUE ? 1u : 0u, &frame_state) != 0;
  g_lastStatus = g_api.last_status();
  return handled;
}

bool copy_tex(void* dest, const ClipRect& rect, GXTexFmt fmt, GXBool clear, const gx::GXState& state) {
  if (!ensure_loaded()) {
    g_lastStatus = "bridge_not_loaded";
    return false;
  }
  const auto frame_state = to_rust_frame_state(state);
  const bool handled =
      g_api.copy_tex(dest, static_cast<u16>(rect.x), static_cast<u16>(rect.y), static_cast<u16>(rect.width),
                     static_cast<u16>(rect.height), static_cast<u32>(fmt),
                     clear == GX_TRUE ? 1u : 0u, &frame_state) != 0;
  g_lastStatus = g_api.last_status();
  return handled;
}

const char* last_status() { return g_lastStatus ? g_lastStatus : "bridge_unknown"; }

} // namespace porpoise::gfx::bridge
