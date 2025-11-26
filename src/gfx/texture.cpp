#include "texture.hpp"
#include "gx_state.hpp"
#include <cstdlib>
#include <cstring>

namespace porpoise::gfx {

// Stub implementations - to be replaced with actual graphics backend
TextureHandle* new_static_texture_2d(u32 width, u32 height, u32 mips, u32 format,
                                     aurora::ArrayRef<uint8_t> data, bool tlut, const char* label) {
  // Size is calculated from the ArrayRef parameter (data.size)
  // This function receives the size via ArrayRef, so we don't need a separate size parameter
  auto* handle = new TextureHandle();
  handle->width = static_cast<u16>(width);
  handle->height = static_cast<u16>(height);
  handle->gxFormat = format;
  handle->size = static_cast<u32>(data.size());  // Use size from ArrayRef
  handle->handle = nullptr;  // Will be set by graphics backend
  (void)mips;
  (void)tlut;
  (void)label;
  (void)data.data();  // Data will be uploaded when implementing actual backend
  return handle;
}

TextureHandle* new_dynamic_texture_2d(u16 width, u16 height, u32 mipLevels, u32 format, const char* name) {
  // TODO: Implement with actual graphics backend (OpenGL, etc.)
  auto* handle = new TextureHandle();
  handle->width = width;
  handle->height = height;
  handle->gxFormat = format;
  handle->handle = nullptr;  // Will be set by graphics backend
  (void)mipLevels;
  (void)name;
  return handle;
}

TextureHandle* new_render_texture(u16 width, u16 height, u32 format, const char* name) {
  // TODO: Implement with actual graphics backend
  auto* handle = new TextureHandle();
  handle->width = width;
  handle->height = height;
  handle->gxFormat = format;
  handle->isRenderTexture = true;
  handle->handle = nullptr;
  (void)name;
  return handle;
}

void write_texture(TextureHandle& handle, std::string_view data) {
  // TODO: Implement with actual graphics backend
  // For now, just a stub
  (void)handle;
  (void)data;
}

void resolve_pass(TextureHandle& handle, const ClipRect& rect, bool clear, const aurora::Vec4<float>& clearColor) {
  // TODO: Implement with actual graphics backend
  // For now, just a stub
  (void)handle;
  (void)rect;
  (void)clear;
  (void)clearColor;
}

} // namespace porpoise::gfx

