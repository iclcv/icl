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

  namespace detail {
    /// Internal CRTP mixin shared between `PointT` and `SizeT`.
    /** These two types are semantically different (one is a position,
        the other a dimension — hence `x,y` vs `width,height`) but
        their arithmetic, comparison, and narrow/widen-conversion
        machinery is identical.  BiTuple collects that machinery in
        one place, accessing the two elements of its Derived through
        Derived's `operator[]`.

        Each Derived class must:
          * inherit publicly from `BiTuple<Derived, T>`
          * expose `operator[](int)` / `operator[](int) const`
            (i == 0 → first element, else → second)
          * provide a two-argument `(T, T)` constructor
          * define `template<typename U> using rebind = …;` so the
            narrowing helpers can synthesize a differently-typed result
            (e.g. `PointT<int>::rebind<float>` == `PointT<float>`).

        Defined in Point.h so that neither a separate header nor an
        extra installed file is required.  SizeT picks it up by
        including Point.h. */
    template<typename Derived, typename T>
    class BiTuple {
      protected:
      constexpr Derived &self() { return static_cast<Derived&>(*this); }
      constexpr const Derived &self() const { return static_cast<const Derived&>(*this); }

      public:
      /// true iff both elements are zero
      constexpr bool isNull() const {
        return self()[0] == T(0) && self()[1] == T(0);
      }

      constexpr bool operator==(const Derived &o) const {
        return self()[0] == o[0] && self()[1] == o[1];
      }
      constexpr bool operator!=(const Derived &o) const {
        return !(self() == o);
      }

      constexpr Derived operator+(const Derived &o) const {
        return { T(self()[0] + o[0]), T(self()[1] + o[1]) };
      }
      constexpr Derived operator-(const Derived &o) const {
        return { T(self()[0] - o[0]), T(self()[1] - o[1]) };
      }
      constexpr Derived operator*(double d) const {
        return { static_cast<T>(d * self()[0]), static_cast<T>(d * self()[1]) };
      }
      constexpr Derived operator/(double d) const { return (*this) * (1.0 / d); }

      Derived &operator+=(const Derived &o) {
        self()[0] = T(self()[0] + o[0]);
        self()[1] = T(self()[1] + o[1]);
        return self();
      }
      Derived &operator-=(const Derived &o) {
        self()[0] = T(self()[0] - o[0]);
        self()[1] = T(self()[1] - o[1]);
        return self();
      }
      Derived &operator*=(double d) {
        self()[0] = static_cast<T>(self()[0] * d);
        self()[1] = static_cast<T>(self()[1] * d);
        return self();
      }
      Derived &operator/=(double d) { return (*this) *= (1.0 / d); }

      // ---------- explicit cross-type conversions ----------
      //
      // Each returns a Derived re-bound to the target type U — e.g.
      // `PointT<float>{1.5f, 2.5f}.rounded<int>()` → `PointT<int>{2, 3}`.
      // Default U=int covers the common float → int narrowing; the
      // caller picks the policy (round / floor / ceil / trunc) at
      // the call site instead of it happening silently inside an
      // implicit conversion.

      template<typename U = int>
      constexpr auto truncated() const {
        using R = typename Derived::template rebind<U>;
        return R{ static_cast<U>(self()[0]), static_cast<U>(self()[1]) };
      }

      template<typename U = int>
      auto rounded() const {
        using R = typename Derived::template rebind<U>;
        return R{ static_cast<U>(std::round(self()[0])),
                  static_cast<U>(std::round(self()[1])) };
      }

      template<typename U = int>
      auto floored() const {
        using R = typename Derived::template rebind<U>;
        return R{ static_cast<U>(std::floor(self()[0])),
                  static_cast<U>(std::floor(self()[1])) };
      }

      template<typename U = int>
      auto ceiled() const {
        using R = typename Derived::template rebind<U>;
        return R{ static_cast<U>(std::ceil(self()[0])),
                  static_cast<U>(std::ceil(self()[1])) };
      }
    };
  } // namespace detail


  /// Templated 2D point.
  /** ICL provides two instantiations as typedefs:
      - `Point` (aka `PointT<int>`) — integer pixel coordinates
      - `Point32f` (aka `PointT<float>`) — sub-pixel coordinates

      Arithmetic, equality, and narrow/widen conversion machinery are
      inherited from `detail::BiTuple`; only point-specific methods
      (distanceTo, norm, normalize, inTriangle, toIppiPoint) live here.

      Cross-type construction is implicit int → float (lossless) and
      explicit for anything lossy (float → int, etc.).  Narrowing with
      a specific rounding policy is performed via the inherited
      `.rounded()`, `.floored()`, `.ceiled()`, or `.truncated()` at
      the call site. */
  template<typename T>
  class PointT : public detail::BiTuple<PointT<T>, T> {
    public:
    /// x position
    T x{T(0)};
    /// y position
    T y{T(0)};

    /// required by BiTuple's cross-type conversion helpers
    template<typename U> using rebind = PointT<U>;

    /// null point (x=0, y=0)
    inline static const PointT null{};

    /// default constructor
    constexpr PointT() = default;

    /// coordinate constructor
    constexpr PointT(T x_, T y_) : x(x_), y(y_) {}

    /// implicit widening from integer-coord point to floating-coord point
    /** Lossless — integer pixel coordinates fit exactly in `float`. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && std::is_integral_v<U>
                && std::is_floating_point_v<T>)
    constexpr PointT(const PointT<U> &o)
      : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

    /// explicit cross-type construction for anything lossy (truncates)
    /** For narrowing with a specific rounding policy, prefer
        `.rounded<int>()`, `.floored<int>()`, `.ceiled<int>()`, or
        `.truncated<int>()` at the call site. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && !(std::is_integral_v<U> && std::is_floating_point_v<T>))
    explicit constexpr PointT(const PointT<U> &o)
      : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

    /// BiTuple element access (i == 0 → x, else → y)
    constexpr T &operator[](int i) { return i ? y : x; }
    constexpr const T &operator[](int i) const { return i ? y : x; }

    /// explicit conversion to IppiPoint (int coordinates)
    IppiPoint toIppiPoint() const {
      return { static_cast<int>(x), static_cast<int>(y) };
    }

    // ---------- point-specific math (out-of-line) ----------

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

    // ---------- float-only ----------

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
