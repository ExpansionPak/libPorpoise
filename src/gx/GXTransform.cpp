#include "gx.hpp"
#include "../gfx/common.hpp"

namespace {
inline void notify_transform_state() {
  porpoise::gfx::bridge::notify_state(porpoise::gfx::bridge::Action::Transform);
}

static porpoise::Mat4x4<float> to_mat4x4_col_major(const void* mtx_) {
  const auto* src = reinterpret_cast<const f32(*)[4]>(mtx_);
  porpoise::Mat4x4<float> out{};
  for (size_t row = 0; row < 4; ++row) {
    for (size_t col = 0; col < 4; ++col) {
      out(row, col) = src[row][col];
    }
  }
  return out;
}
static porpoise::Mat3x4<float> to_mat3x4_col_major(const void* mtx_) {
  const auto* src = reinterpret_cast<const f32(*)[4]>(mtx_);
  porpoise::Mat3x4<float> out{};
  for (size_t row = 0; row < 3; ++row) {
    for (size_t col = 0; col < 4; ++col) {
      out(row, col) = src[row][col];
    }
  }
  return out;
}

static porpoise::Mat2x4<float> to_mat2x4_col_major(const void* mtx_) {
  const auto* src = reinterpret_cast<const f32(*)[4]>(mtx_);
  porpoise::Mat2x4<float> out{};
  for (size_t row = 0; row < 2; ++row) {
    for (size_t col = 0; col < 4; ++col) {
      out(row, col) = src[row][col];
    }
  }
  return out;
}

} // namespace

extern "C" {

void GXSetProjection(const void* mtx_, GXProjectionType type) {
  const auto mtx = to_mat4x4_col_major(mtx_);
  g_gxState.projType = type;
  update_gx_state(g_gxState.proj, mtx);
  notify_transform_state();
}

// TODO GXSetProjectionv

void GXLoadPosMtxImm(const void* mtx_, u32 id) {
  CHECK(id >= GX_PNMTX0 && id <= GX_PNMTX9, "invalid pn mtx {}", static_cast<int>(id));
  auto& state = g_gxState.pnMtx[id / 3];
  const auto mtx = to_mat3x4_col_major(mtx_);
  update_gx_state(state.pos, mtx);
  // Set the current matrix to the one we just loaded
  update_gx_state(g_gxState.currentPnMtx, id / 3);
  notify_transform_state();
}

// TODO GXLoadPosMtxIndx

void GXLoadNrmMtxImm(const void* mtx_, u32 id) {
  CHECK(id >= GX_PNMTX0 && id <= GX_PNMTX9, "invalid pn mtx {}", static_cast<int>(id));
  auto& state = g_gxState.pnMtx[id / 3];
  const auto mtx = to_mat3x4_col_major(mtx_);
  update_gx_state(state.nrm, mtx);
  notify_transform_state();
}

// TODO GXLoadNrmMtxImm3x3
// TODO GXLoadNrmMtxIndx3x3

void GXSetCurrentMtx(u32 id) {
  CHECK(id >= GX_PNMTX0 && id <= GX_PNMTX9, "invalid pn mtx {}", id);
  update_gx_state(g_gxState.currentPnMtx, id / 3);
  notify_transform_state();
}

void GXLoadTexMtxImm(const void* mtx_, u32 id, GXTexMtxType type) {
  CHECK((id >= GX_TEXMTX0 && id <= GX_IDENTITY) || (id >= GX_PTTEXMTX0 && id <= GX_PTIDENTITY), "invalid tex mtx {}",
        id);
  if (id >= GX_PTTEXMTX0) {
    CHECK(type == GX_MTX3x4, "invalid pt mtx type {}", underlying(type));
    const auto idx = (id - GX_PTTEXMTX0) / 3;
    const auto mtx = to_mat3x4_col_major(mtx_);
    update_gx_state(g_gxState.ptTexMtxs[idx], mtx);
  } else {
    const auto idx = (id - GX_TEXMTX0) / 3;
    switch (type) {
    case GX_MTX3x4: {
      const auto mtx = to_mat3x4_col_major(mtx_);
      update_gx_state<porpoise::gfx::gx::TexMtxVariant>(g_gxState.texMtxs[idx], mtx);
      break;
    }
    case GX_MTX2x4: {
      const auto mtx = to_mat2x4_col_major(mtx_);
      update_gx_state<porpoise::gfx::gx::TexMtxVariant>(g_gxState.texMtxs[idx], mtx);
      break;
    }
    }
  }
  notify_transform_state();
}

// TODO GXLoadTexMtxIndx
// TODO GXProject

void GXSetViewport(float left, float top, float width, float height, float nearZ, float farZ) {
    g_gxState.viewportLeft = left;
    g_gxState.viewportTop = top;
    g_gxState.viewportWidth = width;
    g_gxState.viewportHeight = height;
    g_gxState.viewportNear = nearZ;
    g_gxState.viewportFar = farZ;
    porpoise::gfx::set_viewport(left, top, width, height, nearZ, farZ);
    notify_transform_state();
}

void GXSetViewportJitter(float left, float top, float width, float height, float nearZ, float farZ, u32 field) {
    (void)field;
    g_gxState.viewportLeft = left;
    g_gxState.viewportTop = top;
    g_gxState.viewportWidth = width;
    g_gxState.viewportHeight = height;
    g_gxState.viewportNear = nearZ;
    g_gxState.viewportFar = farZ;
    porpoise::gfx::set_viewport(left, top, width, height, nearZ, farZ);
    notify_transform_state();
}

// TODO GXSetZScaleOffset

void GXSetScissorBoxOffset(s32 x_off, s32 y_off) {
  (void)x_off;
  (void)y_off;
  /* Stub: offset applied to scissor box in hardware; not yet applied in render path. */
}

// TODO GXSetClipMode
}