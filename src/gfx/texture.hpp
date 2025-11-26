#pragma once

// Simplified texture handling for libPorpoise (replacing Aurora's WebGPU-based texture system)
// This provides a minimal interface that can be adapted to OpenGL or other backends

#include "internal.hpp"
#include <dolphin/gx.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace porpoise::gfx {

// Forward declaration - actual texture handle type depends on graphics backend
// For now, using a simple pointer-based system that can be adapted
struct TextureHandle {
  void* handle = nullptr;  // Will be GLuint for OpenGL, or other backend-specific type
  bool isRenderTexture = false;
  u32 gxFormat = 0;
  u16 width = 0;
  u16 height = 0;
  u32 size = 0;  // Size in bytes
  
  bool operator==(const TextureHandle& rhs) const { return handle == rhs.handle; }
  bool operator!=(const TextureHandle& rhs) const { return !(*this == rhs); }
};

// Generic clip rectangle used by GX copy/resolve operations
struct ClipRect {
  s32 x = 0;
  s32 y = 0;
  s32 width = 0;
  s32 height = 0;
};

// Texture binding information
struct TextureBind {
  TextureHandle* ref = nullptr;
  bool operator==(const TextureBind& rhs) const { return ref == rhs.ref; }
  bool operator!=(const TextureBind& rhs) const { return !(*this == rhs); }
};

// Stub texture functions - to be implemented with actual graphics backend
TextureHandle* new_static_texture_2d(u32 width, u32 height, u32 mips, u32 format,
                                     aurora::ArrayRef<uint8_t> data, bool tlut, const char* label);
TextureHandle* new_dynamic_texture_2d(u16 width, u16 height, u32 mipLevels, u32 format, const char* name);
TextureHandle* new_render_texture(u16 width, u16 height, u32 format, const char* name);
void write_texture(TextureHandle& handle, std::string_view data);
void resolve_pass(TextureHandle& handle, const ClipRect& rect, bool clear, const aurora::Vec4<float>& clearColor);

} // namespace porpoise::gfx

// Compatibility aliases for Aurora code
namespace aurora::gfx {
using TextureHandle = porpoise::gfx::TextureHandle;
using TextureBind = porpoise::gfx::TextureBind;
using ClipRect = porpoise::gfx::ClipRect;

inline TextureHandle* new_static_texture_2d(u32 width, u32 height, u32 mips, u32 format,
                                            aurora::ArrayRef<uint8_t> data, bool tlut, const char* label) {
  return porpoise::gfx::new_static_texture_2d(width, height, mips, format, data, tlut, label);
}

inline TextureHandle* new_dynamic_texture_2d(u16 width, u16 height, u32 mipLevels, u32 format, const char* name) {
  return porpoise::gfx::new_dynamic_texture_2d(width, height, mipLevels, format, name);
}

inline void write_texture(TextureHandle& handle, std::string_view data) {
  porpoise::gfx::write_texture(handle, data);
}

inline TextureHandle* new_render_texture(u16 width, u16 height, u32 format, const char* name) {
  return porpoise::gfx::new_render_texture(width, height, format, name);
}

inline void resolve_pass(TextureHandle& handle, const ClipRect& rect, bool clear, const aurora::Vec4<float>& clearColor) {
  porpoise::gfx::resolve_pass(handle, rect, clear, clearColor);
}
}

