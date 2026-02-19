#pragma once

// Compatibility header to replace Aurora's internal.hpp
// Minimal implementation for libPorpoise

#include "math.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <type_traits>
#include <vector>
#include <array>

namespace porpoise {

struct Range {
  uint32_t offset = 0;
  uint32_t size = 0;

  bool operator==(const Range& rhs) const = default;
};

class ByteBuffer {
public:
  ByteBuffer() = default;
  ByteBuffer(uint8_t* data, size_t size) { append(data, size); }

  void reserve_extra(size_t extra) { m_data.reserve(m_data.size() + extra); }
  size_t size() const { return m_data.size(); }

  uint8_t* data() { return m_data.data(); }
  const uint8_t* data() const { return m_data.data(); }

  void append_zeroes(size_t count) { m_data.insert(m_data.end(), count, 0); }

  template <typename T>
  void append(const T& value) {
    const auto* bytes = reinterpret_cast<const uint8_t*>(&value);
    m_data.insert(m_data.end(), bytes, bytes + sizeof(T));
  }

  void append(const void* ptr, size_t length) {
    if (length == 0) {
      return;
    }
    if (ptr == nullptr) {
      append_zeroes(length);
      return;
    }
    const auto* bytes = static_cast<const uint8_t*>(ptr);
    m_data.insert(m_data.end(), bytes, bytes + length);
  }

private:
  std::vector<uint8_t> m_data;
};

} // namespace porpoise

// Byte order swapping
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef SBIG
#define SBIG(q) \
  (((q) & 0x000000FF) << 24 | ((q) & 0x0000FF00) << 8 | ((q) & 0x00FF0000) >> 8 | ((q) & 0xFF000000) >> 24)
#endif
#else
#ifndef SBIG
#define SBIG(q) (q)
#endif
#endif

template <typename T>
  requires(sizeof(T) == sizeof(uint16_t) && std::is_arithmetic_v<T>)
constexpr T bswap(T val) noexcept {
  union {
    uint16_t u;
    T t;
  } v{.t = val};
#if __GNUC__
  v.u = __builtin_bswap16(v.u);
#elif _WIN32
  v.u = _byteswap_ushort(v.u);
#else
  v.u = (v.u << 8) | ((v.u >> 8) & 0xFF);
#endif
  return v.t;
}

template <typename T>
  requires(sizeof(T) == sizeof(uint32_t) && std::is_arithmetic_v<T>)
constexpr T bswap(T val) noexcept {
  union {
    uint32_t u;
    T t;
  } v{.t = val};
#if __GNUC__
  v.u = __builtin_bswap32(v.u);
#elif _WIN32
  v.u = _byteswap_ulong(v.u);
#else
  v.u = ((v.u & 0x0000FFFF) << 16) | ((v.u & 0xFFFF0000) >> 16) | ((v.u & 0x00FF00FF) << 8) | ((v.u & 0xFF00FF00) >> 8);
#endif
  return v.t;
}

template <typename T>
  requires(std::is_enum_v<T>)
auto underlying(T value) -> std::underlying_type_t<T> {
  return static_cast<std::underlying_type_t<T>>(value);
}

// Simple logging replacement
namespace porpoise {
class Log {
  const char* m_name;
public:
  explicit Log(const char* name = nullptr) : m_name(name) {}
  
  void fatal(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[FATAL] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
  }
  
  void error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
  }
  
  void warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[WARN] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
  }
  
  void info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
  }
};

// Global log instance
extern Log g_log;
} // namespace porpoise

// Compatibility aliases
namespace porpoise {
using Module = porpoise::Log;
using Range = porpoise::Range;
using ByteBuffer = porpoise::ByteBuffer;
}

#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(name) 0
#endif
#if __has_cpp_attribute(unlikely)
#define UNLIKELY [[unlikely]]
#else
#define UNLIKELY
#endif

#define FATAL(msg, ...) porpoise::g_log.fatal(msg, ##__VA_ARGS__);
#define ASSERT(cond, msg, ...) \
  if (!(cond)) \
  UNLIKELY FATAL(msg, ##__VA_ARGS__)
#ifdef NDEBUG
#define CHECK(cond, msg, ...)
#else
#define CHECK(cond, msg, ...) ASSERT(cond, msg, ##__VA_ARGS__)
#endif
#define DEFAULT_FATAL(msg, ...) UNLIKELY default : FATAL(msg, ##__VA_ARGS__)
#define TRY(cond, msg, ...) \
  if (!(cond)) \
    UNLIKELY { \
      porpoise::g_log.error(msg, ##__VA_ARGS__); \
      return false; \
    }
#define TRY_WARN(cond, msg, ...) \
  if (!(cond)) \
    UNLIKELY { porpoise::g_log.warn(msg, ##__VA_ARGS__); }

// ArrayRef compatibility
namespace porpoise {
template <typename T>
class ArrayRef {
public:
  using value_type = std::remove_cvref_t<T>;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;
  using iterator = const_pointer;
  using const_iterator = const_pointer;
  using size_type = std::size_t;

  ArrayRef() = default;
  explicit ArrayRef(const T& one) : ptr(&one), length(1) {}
  ArrayRef(const T* data, size_t length) : ptr(data), length(length) {}
  ArrayRef(const T* begin, const T* end) : ptr(begin), length(end - begin) {}
  template <size_t N>
  constexpr ArrayRef(const T (&arr)[N]) : ptr(arr), length(N) {}
  template <size_t N>
  constexpr ArrayRef(const std::array<T, N>& arr) : ptr(arr.data()), length(arr.size()) {}
  ArrayRef(const std::vector<T>& vec) : ptr(vec.data()), length(vec.size()) {}

  const T* data() const { return ptr; }
  size_t size() const { return length; }
  bool empty() const { return length == 0; }

  const T& front() const {
    assert(!empty());
    return ptr[0];
  }
  const T& back() const {
    assert(!empty());
    return ptr[length - 1];
  }
  const T& operator[](size_t i) const {
    assert(i < length && "Invalid index!");
    return ptr[i];
  }

  iterator begin() const { return ptr; }
  iterator end() const { return ptr + length; }

private:
  const T* ptr = nullptr;
  size_t length = 0;
};
} // namespace aurora

