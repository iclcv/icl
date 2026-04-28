// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point.h>
#include <icl/utils/Rect.h>
#include <iosfwd>
#include <type_traits>
#include <vector>

namespace icl::core {

  /// Templated 2D line described by a start and end PointT<T>.
  /** ICL provides two instantiations as typedefs:
      - `Line`    (aka `LineT<int>`)   — integer pixel-coordinate lines,
                                          Bresenham-sampled without floating-point
      - `Line32f` (aka `LineT<float>`) — sub-pixel float-coordinate lines

      The arithmetic, comparison, and method bodies are shared; methods that
      only make sense for one specialization are constrained via
      `requires std::is_integral_v<T>` / `std::is_floating_point_v<T>`. */
  template<typename T>
  class ICLCore_API LineT {
    public:
    /// element point type (PointT<T>)
    using point_type = utils::PointT<T>;

    /// rebind helper for cross-type construction
    template<typename U> using rebind = LineT<U>;

    /// null line (start == end == PointT<T>::null)
    inline static const LineT null{};

    /// start point
    point_type start{};
    /// end point
    point_type end{};

    /// default ctor (null line)
    constexpr LineT() = default;

    /// ctor from start and end point
    constexpr LineT(point_type start, point_type end) : start(start), end(end) {}

    /// ctor from start, angle (radians), and length
    LineT(point_type start, float angle, float length);

    /// implicit widening from integer-coord line to float-coord line (lossless)
    template<typename U>
      requires (!std::is_same_v<U, T>
                && std::is_integral_v<U>
                && std::is_floating_point_v<T>)
    constexpr LineT(const LineT<U> &o) : start(o.start), end(o.end) {}

    /// explicit cross-type construction for anything lossy (truncates)
    template<typename U>
      requires (!std::is_same_v<U, T>
                && !(std::is_integral_v<U> && std::is_floating_point_v<T>))
    explicit constexpr LineT(const LineT<U> &o) : start(o.start), end(o.end) {}

    /// translate this line by a vector
    constexpr LineT operator+(const point_type &p) const { return {start + p, end + p}; }
    constexpr LineT operator-(const point_type &p) const { return {start - p, end - p}; }

    /// euclidean length
    float length() const;

    /// swap start and end in place
    void swap() { auto tmp = start; start = end; end = tmp; }

    /// sample this line as a sequence of integer-pixel Points (Bresenham)
    /** @param limits bounding rect; line points outside are clipped.
                      A null/empty Rect means no clipping.

        For integer lines this is the most efficient sampling path; for float
        lines the endpoints are rounded to int before Bresenham. */
    std::vector<utils::Point> sample(const utils::Rect &limits = utils::Rect::null) const;

    /// returns true iff this line intersects `other`
    /** Optionally writes the float intersection point into `*p` and the
        interpolation factors into `*dstr` / `*dsts`. */
    bool intersects(const LineT &other, utils::Point32f *p = nullptr,
                    float *dstr = nullptr, float *dsts = nullptr) const;

    // ---------- integer-only ----------

    /// closest point on this line to `p` (integer-pixel result)
    utils::Point findClosestPoint(const utils::Point &p) const
      requires std::is_integral_v<T>;

    /// minimum distance from this line to `p`
    inline float getMinDist(const utils::Point &p) const
      requires std::is_integral_v<T> {
      return findClosestPoint(p).distanceTo(p);
    }

    // ---------- float-only ----------

    /// line angle in radians: atan2(start.y - end.y, start.x - end.x)
    float getAngle() const requires std::is_floating_point_v<T>;

    /// center point: (start + end) / 2
    utils::Point32f getCenter() const requires std::is_floating_point_v<T>;

    /// sample appending into pre-allocated x/y vectors (no allocation)
    void sample(std::vector<int> &xs, std::vector<int> &ys,
                const utils::Rect &limits = utils::Rect::null) const
      requires std::is_floating_point_v<T>;
  };

  /// ostream operator: formats as "(start)(end)"
  template<typename T>
  std::ostream &operator<<(std::ostream &s, const LineT<T> &l);

  /// istream operator
  template<typename T>
  std::istream &operator>>(std::istream &s, LineT<T> &l);

  // ---------- established ICL type aliases ----------

  /// integer-coordinate 2D line (pixel coordinates, Bresenham-friendly)
  using Line = LineT<int>;

  /// single-precision floating-point 2D line (sub-pixel coordinates)
  using Line32f = LineT<float>;

  } // namespace icl::core
