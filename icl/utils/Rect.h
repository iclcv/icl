// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point.h>
#include <icl/utils/Size.h>
#include <cmath>
#include <iosfwd>
#include <type_traits>

#ifdef ICL_HAVE_IPP
#include <ipp.h>
#else
/// Fallback for the IppiRect struct when IPP is not available.
/** Placed in the global namespace to match the real IPP struct, so
    code using IppiRect (and the `toIppiRect()` method) compiles
    identically with and without IPP. */
struct IppiRect { int x, y, width, height; };
#endif

namespace icl::utils {
  /// Templated 2D rectangle.
  /** ICL provides two instantiations as typedefs:
      - `Rect` (aka `RectT<int>`) — integer-coordinate rectangle
      - `Rect32f` (aka `RectT<float>`) — sub-pixel rectangle

      Conventions: origin (x, y) is the upper-left corner; width and
      height extend to the right and down.  For integer Rects the set
      of contained pixels is half-open — Rect(2, 1, 3, 4) covers
      x∈{2,3,4}, y∈{1,2,3,4}, 3×4 = 12 points.

      Cross-type construction follows the same rule as PointT/SizeT:
      int → float is implicit (lossless); float → int (or any other
      narrowing) is explicit, and callers choose a rounding policy
      at the call site via `.rounded()`, `.floored()`, `.ceiled()`,
      or `.truncated()`. */
  template<typename T>
  class RectT {
    public:
    T x{T(0)};       ///< x position of the upper-left corner
    T y{T(0)};       ///< y position of the upper-left corner
    T width{T(0)};   ///< width
    T height{T(0)};  ///< height

    /// required by `.rounded()` / `.floored()` / … to synthesize the target type
    template<typename U> using rebind = RectT<U>;

    /// null rect (all zeros)
    inline static const RectT null{};

    /// default constructor
    constexpr RectT() = default;

    /// defined-coordinate constructor
    constexpr RectT(T x_, T y_, T w_, T h_)
      : x(x_), y(y_), width(w_), height(h_) {}

    /// construct from offset (Point) + size (Size)
    constexpr RectT(const PointT<T> &p, const SizeT<T> &s)
      : x(p.x), y(p.y), width(s.width), height(s.height) {}

    /// implicit widening from integer-coord rect to floating-coord rect
    /** Lossless — integer coordinates fit exactly in `float`. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && std::is_integral_v<U>
                && std::is_floating_point_v<T>)
    constexpr RectT(const RectT<U> &o)
      : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)),
        width(static_cast<T>(o.width)), height(static_cast<T>(o.height)) {}

    /// explicit cross-type construction for anything lossy (truncates)
    /** For narrowing with a specific rounding policy, prefer
        `.rounded()`, `.floored()`, `.ceiled()`, or `.truncated()`
        at the call site. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && !(std::is_integral_v<U> && std::is_floating_point_v<T>))
    explicit constexpr RectT(const RectT<U> &o)
      : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)),
        width(static_cast<T>(o.width)), height(static_cast<T>(o.height)) {}

    /// true iff all four components are zero
    constexpr bool isNull() const {
      return x == T(0) && y == T(0) && width == T(0) && height == T(0);
    }

    constexpr bool operator==(const RectT &r) const {
      return x == r.x && y == r.y && width == r.width && height == r.height;
    }
    constexpr bool operator!=(const RectT &r) const { return !(*this == r); }

    /// scale all four components by a scalar
    constexpr RectT operator*(double d) const {
      return { static_cast<T>(d * x), static_cast<T>(d * y),
               static_cast<T>(d * width), static_cast<T>(d * height) };
    }
    constexpr RectT operator/(double d) const { return (*this) * (1.0 / d); }

    /// scale all four components inplace
    RectT &operator*=(double d) {
      x = static_cast<T>(x * d); y = static_cast<T>(y * d);
      width = static_cast<T>(width * d); height = static_cast<T>(height * d);
      return *this;
    }
    RectT &operator/=(double d) { return (*this) *= (1.0 / d); }

    /// grow / shrink width and height by a Size (offset unchanged)
    constexpr RectT operator+(const SizeT<T> &s) const {
      return { x, y, T(width + s.width), T(height + s.height) };
    }
    constexpr RectT operator-(const SizeT<T> &s) const {
      return { x, y, T(width - s.width), T(height - s.height) };
    }
    RectT &operator+=(const SizeT<T> &s) {
      width = T(width + s.width); height = T(height + s.height); return *this;
    }
    RectT &operator-=(const SizeT<T> &s) {
      width = T(width - s.width); height = T(height - s.height); return *this;
    }

    /// translate the offset (size unchanged)
    constexpr RectT operator+(const PointT<T> &p) const {
      return { T(x + p.x), T(y + p.y), width, height };
    }
    constexpr RectT operator-(const PointT<T> &p) const {
      return { T(x - p.x), T(y - p.y), width, height };
    }
    RectT &operator+=(const PointT<T> &p) {
      x = T(x + p.x); y = T(y + p.y); return *this;
    }
    RectT &operator-=(const PointT<T> &p) {
      x = T(x - p.x); y = T(y - p.y); return *this;
    }

    /// returns width * height
    constexpr auto getDim() const { return width * height; }

    /// intersection of two rects (returns `null` if empty)
    RectT operator&(const RectT &r) const;

    /// inplace intersection (sets to `null` if empty)
    RectT &operator&=(const RectT &r) { return *this = *this & r; }

    /// union (bounding-box of the two rects)
    RectT operator|(const RectT &r) const;

    /// inplace union
    RectT &operator|=(const RectT &r) { return *this = *this | r; }

    /// normalize: flip sign of negative width/height while keeping
    /// the rectangle geometrically equivalent
    /** e.g. (5, 5, -5, -5) normalizes to (0, 0, 5, 5). */
    RectT normalized() const;

    /// true iff the given rect is fully contained in this one
    bool contains(const RectT &r) const;

    /// true iff the given (px, py) lies inside (for int: half-open)
    bool contains(T px, T py) const;

    /// grow uniformly by k on each side (use negative k to shrink)
    RectT &enlarge(T k);

    /// returns a grown / shrunk copy
    RectT enlarged(T k) const { return RectT(*this).enlarge(k); }

    // ---------- corners ----------

    constexpr PointT<T> ul() const { return { x, y }; }
    constexpr PointT<T> ll() const { return { x, T(y + height) }; }
    constexpr PointT<T> ur() const { return { T(x + width), y }; }
    constexpr PointT<T> lr() const { return { T(x + width), T(y + height) }; }

    /// center of the rect
    constexpr PointT<T> center() const {
      return { T(x + width / 2), T(y + height / 2) };
    }

    constexpr T left()   const { return x; }
    constexpr T right()  const { return T(x + width); }
    constexpr T top()    const { return y; }
    constexpr T bottom() const { return T(y + height); }

    /// returns the size of the rect
    constexpr SizeT<T> getSize() const { return { width, height }; }

    /// element-wise scale (xfac scales x,width; yfac scales y,height)
    RectT transform(double xfac, double yfac) const;

    /// explicit conversion to IppiRect (int coordinates)
    IppiRect toIppiRect() const {
      return { static_cast<int>(x), static_cast<int>(y),
               static_cast<int>(width), static_cast<int>(height) };
    }

    // ---------- explicit cross-type conversions ----------

    template<typename U = int>
    constexpr RectT<U> truncated() const {
      return { static_cast<U>(x), static_cast<U>(y),
               static_cast<U>(width), static_cast<U>(height) };
    }

    template<typename U = int>
    RectT<U> rounded() const {
      return { static_cast<U>(std::round(x)), static_cast<U>(std::round(y)),
               static_cast<U>(std::round(width)), static_cast<U>(std::round(height)) };
    }

    template<typename U = int>
    RectT<U> floored() const {
      return { static_cast<U>(std::floor(x)), static_cast<U>(std::floor(y)),
               static_cast<U>(std::floor(width)), static_cast<U>(std::floor(height)) };
    }

    template<typename U = int>
    RectT<U> ceiled() const {
      return { static_cast<U>(std::ceil(x)), static_cast<U>(std::ceil(y)),
               static_cast<U>(std::ceil(width)), static_cast<U>(std::ceil(height)) };
    }
  };

  /// ostream operator: formats as "(x,y)WxH"
  template<typename T>
  std::ostream &operator<<(std::ostream &s, const RectT<T> &r);

  /// istream operator
  template<typename T>
  std::istream &operator>>(std::istream &s, RectT<T> &r);

  // ---------- established ICL type aliases ----------

  /// integer-coordinate 2D rectangle (image ROIs, pixel regions)
  using Rect = RectT<int>;

  /// single-precision 2D rectangle (sub-pixel regions)
  using Rect32f = RectT<float>;

  } // namespace icl::utils
