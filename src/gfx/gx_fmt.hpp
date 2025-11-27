#pragma once

// Simplified formatting functions for libPorpoise (replacing Aurora's fmt-based gx_fmt.hpp)
// Provides string conversion for GX enums without fmt dependency

#include "../gfx/internal.hpp"
#include <dolphin/gx/GXEnum.h>
#include <string>
#include <cstdio>

// Simple string formatting without fmt library
template<typename T>
inline std::string format_enum(T value) {
  char buf[64];
  snprintf(buf, sizeof(buf), "%d", static_cast<int>(value));
  return std::string(buf);
}

// Minimal format_as implementations - just return enum names or numbers
inline std::string format_as(const GXTevOp& op) {
  switch (op) {
  case GX_TEV_ADD: return "GX_TEV_ADD";
  case GX_TEV_SUB: return "GX_TEV_SUB";
  default: return format_enum(op);
  }
}

inline std::string format_as(const GXTevColorArg& arg) {
  switch (arg) {
  case GX_CC_CPREV: return "GX_CC_CPREV";
  case GX_CC_APREV: return "GX_CC_APREV";
  case GX_CC_ZERO: return "GX_CC_ZERO";
  default: return format_enum(arg);
  }
}

inline std::string format_as(const GXTevAlphaArg& arg) {
  switch (arg) {
  case GX_CA_APREV: return "GX_CA_APREV";
  case GX_CA_ZERO: return "GX_CA_ZERO";
  default: return format_enum(arg);
  }
}

// Add more format_as functions as needed - keeping minimal for now

