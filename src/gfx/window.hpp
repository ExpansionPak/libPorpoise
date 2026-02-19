#pragma once

// Window interface for libPorpoise
// Provides access to SDL2 window information via VI module

#include <dolphin/types.h>
#include <dolphin/vi.h>

namespace porpoise::window {

struct WindowSize {
  u16 fb_width;
  u16 fb_height;
};

inline WindowSize get_window_size() {
  int width = 640;
  int height = 480;
  VIGetWindowSize(&width, &height);
  return {static_cast<u16>(width), static_cast<u16>(height)};
}

} // namespace porpoise::window

