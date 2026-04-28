// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/line/Line.h>
#include <icl/core/line/LineSampler.h>
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>

namespace icl::core {

  template<typename T>
  LineT<T>::LineT(point_type start, float angle, float length) : start(start) {
    end.x = static_cast<T>(start.x + std::cos(angle) * length);
    end.y = static_cast<T>(start.y + std::sin(angle) * length);
  }

  template<typename T>
  float LineT<T>::length() const {
    const float dx = static_cast<float>(start.x - end.x);
    const float dy = static_cast<float>(start.y - end.y);
    return std::sqrt(dx * dx + dy * dy);
  }

  template<typename T>
  std::vector<utils::Point> LineT<T>::sample(const utils::Rect &limits) const {
    using utils::Point;
    Point s, e;
    if constexpr (std::is_integral_v<T>) {
      s = start;
      e = end;
    } else {
      s = start.template rounded<int>();
      e = end.template rounded<int>();
    }
    std::vector<Point> dst;
    LineSampler ls;
    if (limits != utils::Rect::null) ls.setBoundingRect(limits);
    ls.sample(s, e, dst);
    return dst;
  }

  template<typename T>
  bool LineT<T>::intersects(const LineT &other, utils::Point32f *p,
                            float *dstr, float *dsts) const {
    const point_type &a = start, &b = end, &c = other.start, &d = other.end;

    const float x1 = (a.y - c.y) * (d.x - c.x) - (a.x - c.x) * (d.y - c.y);
    const float x2 = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);
    if (x1 == 0 && x2 == 0) return false;  // collinear
    if (x2 == 0) return false;             // parallel

    const float x3 = (a.y - c.y) * (b.x - a.x) - (a.x - c.x) * (b.y - a.y);
    const float x4 = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);
    if (x4 == 0) return false;
    const float r = x1 / x2;
    const float s = x3 / x4;

    if (dstr) *dstr = 1.0f - r;
    if (dsts) *dsts = 1.0f - s;

    if (r >= 0 && r <= 1 && s >= 0 && s <= 1) {
      // Always interpolate in float to keep sub-pixel precision regardless of T;
      // the implicit int->float widening on PointT covers T=int losslessly.
      if (p) {
        const utils::Point32f af = a, bf = b;
        *p = af + (bf - af) * r;
      }
      return true;
    }
    return false;
  }

  template<typename T>
  utils::Point LineT<T>::findClosestPoint(const utils::Point &p) const
    requires std::is_integral_v<T> {
    if (start == end) return start;
    const utils::Point x = p - start;
    utils::Point32f v = utils::Point32f(end) - utils::Point32f(start);
    const float l = v.norm();
    v *= 1.0 / l;
    const float a = v[0] * x[0] + v[1] * x[1];
    if (a > l) return end;
    if (a < 0) return start;
    return (utils::Point32f(start) + v * a).template rounded<int>();
  }

  template<typename T>
  float LineT<T>::getAngle() const requires std::is_floating_point_v<T> {
    if (start == end) return 0;
    return std::atan2(start.y - end.y, start.x - end.x);
  }

  template<typename T>
  utils::Point32f LineT<T>::getCenter() const requires std::is_floating_point_v<T> {
    return (start + end) * 0.5;
  }

  template<typename T>
  void LineT<T>::sample(std::vector<int> &xs, std::vector<int> &ys,
                        const utils::Rect &limits) const
    requires std::is_floating_point_v<T> {
    const auto pts = sample(limits);
    xs.reserve(xs.size() + pts.size());
    ys.reserve(ys.size() + pts.size());
    for (const auto &p : pts) {
      xs.push_back(p.x);
      ys.push_back(p.y);
    }
  }

  template<typename T>
  std::ostream &operator<<(std::ostream &s, const LineT<T> &l) {
    return s << l.start << l.end;
  }

  template<typename T>
  std::istream &operator>>(std::istream &s, LineT<T> &l) {
    return s >> l.start >> l.end;
  }

  // ---------- explicit instantiations ----------

  template class LineT<int>;
  template class LineT<float>;

  template std::ostream &operator<<(std::ostream &, const LineT<int> &);
  template std::ostream &operator<<(std::ostream &, const LineT<float> &);
  template std::istream &operator>>(std::istream &, LineT<int> &);
  template std::istream &operator>>(std::istream &, LineT<float> &);

  } // namespace icl::core
