#pragma once

#include "../gfx/internal.hpp"
#include "../gfx/math.hpp"
#include "../gfx/gx_state.hpp"
#include <dolphin/gx.h>
#include <unordered_map>
#include <cstring>
#include <bitset>
#include <memory>
#include <array>
#include <cfloat>
#include <variant>
#include <optional>

// Expose internal structures for GX implementation files
using porpoise::gfx::gx::GXTexObj_;
using porpoise::gfx::gx::GXTlutObj_;
using porpoise::gfx::gx::GXLightObj_;

// Compatibility aliases
namespace aurora::gfx::gx {
using porpoise::gfx::gx::g_gxState;
using porpoise::gfx::gx::GXTexObj_;
using porpoise::gfx::gx::GXTlutObj_;
using porpoise::gfx::gx::GXState;
using porpoise::gfx::gx::GXLightObj_;
using porpoise::gfx::gx::Light;
using porpoise::gfx::gx::VtxAttrFmt;
}

// Make g_gxState available in global scope for convenience
using aurora::gfx::gx::g_gxState;

static porpoise::Log Log("aurora::gx");

template <typename T>
static inline void update_gx_state(T& val, T newVal) {
  val = std::move(newVal);
  porpoise::gfx::gx::g_gxState.stateDirty = true;
}

static inline aurora::Vec4<float> from_gx_color(GXColor color) {
  return {
      static_cast<float>(color.r) / 255.f,
      static_cast<float>(color.g) / 255.f,
      static_cast<float>(color.b) / 255.f,
      static_cast<float>(color.a) / 255.f,
  };
}
