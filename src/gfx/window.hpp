#pragma once

// Stub window interface for libPorpoise
// To be implemented with actual windowing system

namespace aurora::window {

struct WindowSize {
  u16 fb_width = 640;
  u16 fb_height = 480;
};

inline WindowSize get_window_size() {
  // TODO: Implement with actual windowing system
  return WindowSize{640, 480};
}

} // namespace aurora::window

