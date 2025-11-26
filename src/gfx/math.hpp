#pragma once

// Simplified math types for libPorpoise (replacing Aurora's math.hpp)
// Compatible with Aurora's API but without WebGPU dependencies

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <array>

namespace aurora {

// Vector types
template <typename T>
struct Vec2 {
  T x{};
  T y{};

  constexpr Vec2() = default;
  constexpr Vec2(T x, T y) : x(x), y(y) {}

  bool operator==(const Vec2& rhs) const { return x == rhs.x && y == rhs.y; }
  bool operator!=(const Vec2& rhs) const { return !(*this == rhs); }
};

template <typename T>
struct Vec3 {
  T x{};
  T y{};
  T z{};

  constexpr Vec3() = default;
  constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

  bool operator==(const Vec3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
  bool operator!=(const Vec3& rhs) const { return !(*this == rhs); }
};

template <typename T>
struct Vec4 {
  T m[4];

  constexpr Vec4() : m{0, 0, 0, 0} {}
  constexpr Vec4(T x, T y, T z, T w) : m{x, y, z, w} {}
  constexpr Vec4(T x, T y, T z) : m{x, y, z, 0} {}
  constexpr Vec4(Vec3<T> v, T w) : m{v.x, v.y, v.z, w} {}

  T& x() { return m[0]; }
  T x() const { return m[0]; }
  T& y() { return m[1]; }
  T y() const { return m[1]; }
  T& z() { return m[2]; }
  T z() const { return m[2]; }
  T& w() { return m[3]; }
  T w() const { return m[3]; }
  T& operator[](size_t i) { return m[i]; }
  T operator[](size_t i) const { return m[i]; }

  bool operator==(const Vec4& rhs) const {
    return m[0] == rhs.m[0] && m[1] == rhs.m[1] && m[2] == rhs.m[2] && m[3] == rhs.m[3];
  }
  bool operator!=(const Vec4& rhs) const { return !(*this == rhs); }
};

// Matrix types (column-major)
template <typename T, size_t Rows, size_t Cols>
struct Mat {
  std::array<T, Rows * Cols> m;

  constexpr Mat() : m{} {}
  
  T& operator()(size_t row, size_t col) { return m[col * Rows + row]; }
  T operator()(size_t row, size_t col) const { return m[col * Rows + row]; }

  bool operator==(const Mat& rhs) const { return m == rhs.m; }
  bool operator!=(const Mat& rhs) const { return !(*this == rhs); }
};

template <typename T>
using Mat2x4 = Mat<T, 2, 4>;

template <typename T>
using Mat3x2 = Mat<T, 3, 2>;

template <typename T>
using Mat3x4 = Mat<T, 3, 4>;

template <typename T>
using Mat4x4 = Mat<T, 4, 4>;

} // namespace aurora

