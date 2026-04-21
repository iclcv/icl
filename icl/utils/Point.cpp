// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Point.h>
#include <cmath>
#include <istream>
#include <ostream>

namespace icl::utils {

  template<typename T>
  PointT<T> PointT<T>::transform(double xfac, double yfac) const {
    return PointT<T>{ static_cast<T>(xfac * x), static_cast<T>(yfac * y) };
  }

  template<typename T>
  float PointT<T>::distanceTo(const PointT<T> &p) const {
    const float dx = static_cast<float>(p.x - x);
    const float dy = static_cast<float>(p.y - y);
    return std::sqrt(dx*dx + dy*dy);
  }

  template<typename T>
  float PointT<T>::norm(float p) const {
    const float fx = static_cast<float>(x);
    const float fy = static_cast<float>(y);
    return std::pow(std::pow(std::abs(fx), p) + std::pow(std::abs(fy), p), 1.0f / p);
  }

  namespace {
    template<typename T>
    float triangle_sign(const PointT<T> &p1, const PointT<T> &p2, const PointT<T> &p3) {
      return static_cast<float>((p1.x - p3.x) * (p2.y - p3.y) -
                                (p2.x - p3.x) * (p1.y - p3.y));
    }
  }

  template<typename T>
  bool PointT<T>::inTriangle(const PointT<T> &v1,
                             const PointT<T> &v2,
                             const PointT<T> &v3) const {
    const bool b1 = triangle_sign(*this, v1, v2) <= 0;
    const bool b2 = triangle_sign(*this, v2, v3) <= 0;
    const bool b3 = triangle_sign(*this, v3, v1) <= 0;
    return (b1 == b2) && (b2 == b3);
  }

  template<typename T>
  PointT<T> &PointT<T>::normalize() requires std::is_floating_point_v<T> {
    const float l = norm();
    x = static_cast<T>(x / l);
    y = static_cast<T>(y / l);
    return *this;
  }

  template<typename T>
  PointT<T> PointT<T>::normalized() const requires std::is_floating_point_v<T> {
    return PointT<T>{*this}.normalize();
  }

  template<typename T>
  std::ostream &operator<<(std::ostream &s, const PointT<T> &p) {
    return s << "(" << p.x << "," << p.y << ")";
  }

  template<typename T>
  std::istream &operator>>(std::istream &s, PointT<T> &p) {
    char c, d;
    s >> c;
    if (((c >= '0') && (c <= '9')) || c == '-') {
      s.unget();
    }
    s >> p.x;
    s >> d; // any delimiter
    s >> p.y;
    if (!(((c >= '0') && (c <= '9')) || c == '-')) {
      s >> d;
      if (c == '|' && d != '|') s.unget();
      if (c == '(' && d != ')') s.unget();
      if (c == '[' && d != ']') s.unget();
      if (c == '{' && d != '}') s.unget();
    }
    return s;
  }

  // ---------- explicit instantiations ----------

  template class PointT<int>;
  template class PointT<float>;

  template std::ostream &operator<<(std::ostream &, const PointT<int> &);
  template std::ostream &operator<<(std::ostream &, const PointT<float> &);
  template std::istream &operator>>(std::istream &, PointT<int> &);
  template std::istream &operator>>(std::istream &, PointT<float> &);

  } // namespace icl::utils
