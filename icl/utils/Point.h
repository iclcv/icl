// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <cmath>
#include <iosfwd>
#include <type_traits>

#ifdef ICL_HAVE_IPP
#include <ipp.h>
#else
/// Fallback for the IppiPoint struct when IPP is not available.
/** Placed in the global namespace to match the real IPP struct, so
    code using IppiPoint (and the `toIppiPoint()` method) compiles
    identically with and without IPP. */
struct IppiPoint { int x, y; };
#endif

namespace icl::utils {

  /// Templated 2D point.
  /** ICL provides two instantiations as typedefs:
      - `Point` (aka `PointT<int>`) — integer pixel coordinates
      - `Point32f` (aka `PointT<float>`) — sub-pixel coordinates

      Cross-type construction is *explicit* and truncates via
      `static_cast`.  For narrowing conversions (e.g. float→int) with
      a specific rounding policy, use the named `rounded()`,
      `floored()`, `ceiled()`, or `truncated()` methods — the policy
      is chosen at the call site, not silently. */
  template<typename T>
  class PointT {
    public:
    /// x position
    T x{T(0)};
    /// y position
    T y{T(0)};

    /// null point (x=0, y=0)
    inline static const PointT null{};

    /// default constructor
    constexpr PointT() = default;

    /// coordinate constructor
    constexpr PointT(T x_, T y_) : x(x_), y(y_) {}

    /// implicit widening from an integer-coord point to a floating-coord point.
    /** Lossless — integer pixel coordinates fit exactly in `float`. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && std::is_integral_v<U>
                && std::is_floating_point_v<T>)
    constexpr PointT(const PointT<U> &o)
      : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

    /// explicit cross-type construction for anything lossy (truncates).
    /** To narrow float → int with a chosen rounding policy, prefer
        `.rounded<int>()`, `.floored<int>()`, `.ceiled<int>()`, or
        `.truncated<int>()` at the call site — the rounding choice is
        then visible where it matters. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && !(std::is_integral_v<U> && std::is_floating_point_v<T>))
    explicit constexpr PointT(const PointT<U> &o)
      : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

    /// returns whether x == 0 && y == 0
    constexpr bool isNull() const { return x == T(0) && y == T(0); }

    constexpr bool operator==(const PointT &s) const { return x == s.x && y == s.y; }
    constexpr bool operator!=(const PointT &s) const { return x != s.x || y != s.y; }

    constexpr PointT operator+(const PointT &s) const { return {T(x + s.x), T(y + s.y)}; }
    constexpr PointT operator-(const PointT &s) const { return {T(x - s.x), T(y - s.y)}; }
    constexpr PointT operator*(double d) const {
      return {static_cast<T>(d * x), static_cast<T>(d * y)};
    }

    PointT &operator+=(const PointT &s) { x += s.x; y += s.y; return *this; }
    PointT &operator-=(const PointT &s) { x -= s.x; y -= s.y; return *this; }
    PointT &operator*=(double d) {
      x = static_cast<T>(x * d); y = static_cast<T>(y * d); return *this;
    }

    /// index-based access (i==0 → x, else y)
    T &operator[](int i) { return i ? y : x; }
    const T &operator[](int i) const { return i ? y : x; }

    /// explicit conversion to IppiPoint (int coordinates)
    IppiPoint toIppiPoint() const {
      return { static_cast<int>(x), static_cast<int>(y) };
    }

    // ---------- explicit cross-type conversions ----------

    /// `static_cast` narrowing (truncates for float→int)
    template<typename U = int>
    constexpr PointT<U> truncated() const {
      return { static_cast<U>(x), static_cast<U>(y) };
    }

    /// rounds each coordinate (std::round) before casting
    template<typename U = int>
    PointT<U> rounded() const {
      return { static_cast<U>(std::round(x)), static_cast<U>(std::round(y)) };
    }

    /// floors each coordinate (std::floor) before casting
    template<typename U = int>
    PointT<U> floored() const {
      return { static_cast<U>(std::floor(x)), static_cast<U>(std::floor(y)) };
    }

    /// ceilings each coordinate (std::ceil) before casting
    template<typename U = int>
    PointT<U> ceiled() const {
      return { static_cast<U>(std::ceil(x)), static_cast<U>(std::ceil(y)) };
    }

    // ---------- heavier methods (out-of-line) ----------

    /// element-wise scale
    PointT transform(double xfac, double yfac) const;

    /// euclidean distance (always returns float)
    float distanceTo(const PointT &p) const;

    /// p-norm of the 2D vector as a float (p=2 → euclidean)
    /** - p = 0 → 2
        - p = 1 → city-block norm
        - p = 2 → euclidean norm
        - p → inf → infinity norm */
    float norm(float p = 2) const;

    /// triangle-containment test (works for any T)
    bool inTriangle(const PointT &v1, const PointT &v2, const PointT &v3) const;

    // ---------- float-only methods ----------

    /// normalize this vector to length 1 (euclidean norm)
    PointT &normalize() requires std::is_floating_point_v<T>;

    /// returns a normalized copy of this vector
    PointT normalized() const requires std::is_floating_point_v<T>;
  };

  /// ostream operator: formats as "(x,y)"
  template<typename T>
  std::ostream &operator<<(std::ostream &s, const PointT<T> &p);

  /// istream operator: accepts (x,y), |x,y|, [x,y], {x,y}, or x,y
  template<typename T>
  std::istream &operator>>(std::istream &s, PointT<T> &p);

  // ---------- established ICL type aliases ----------

  /// integer-coordinate 2D point (used for pixel coordinates, ROIs, …)
  using Point = PointT<int>;

  /// single-precision floating-point 2D point (sub-pixel coordinates, geometry)
  using Point32f = PointT<float>;

  } // namespace icl::utils
