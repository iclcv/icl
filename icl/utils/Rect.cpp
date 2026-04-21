// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <icl/utils/Rect.h>
#include <algorithm>
#include <istream>
#include <ostream>

namespace icl::utils {

  template<typename T>
  RectT<T> RectT<T>::operator&(const RectT<T> &r) const {
    const T l = std::max(x, r.x);
    const T t = std::max(y, r.y);
    const T rr = std::min(right(), r.right());
    const T bb = std::min(bottom(), r.bottom());
    RectT<T> result(l, t, T(rr - l), T(bb - t));
    if (result.width > T(0) && result.height > T(0)) return result;
    return null;
  }

  template<typename T>
  RectT<T> RectT<T>::operator|(const RectT<T> &r) const {
    const T l = std::min(x, r.x);
    const T t = std::min(y, r.y);
    const T rr = std::max(right(), r.right());
    const T bb = std::max(bottom(), r.bottom());
    return RectT<T>(l, t, T(rr - l), T(bb - t));
  }

  template<typename T>
  RectT<T> RectT<T>::normalized() const {
    RectT<T> r(*this);
    if (r.width  < T(0)) { r.x += r.width;  r.width  = -r.width; }
    if (r.height < T(0)) { r.y += r.height; r.height = -r.height; }
    return r;
  }

  template<typename T>
  bool RectT<T>::contains(const RectT<T> &r) const {
    return x <= r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
  }

  template<typename T>
  bool RectT<T>::contains(T px, T py) const {
    // Half-open for integer T (the historic Rect semantic), closed on
    // the lower bound for float T (the historic Rect32f semantic);
    // both are expressible with the same `< right()` / `< bottom()`
    // test — integer Rect never sees a point exactly on the right or
    // bottom edge as a separate value anyway.
    return x <= px && px < right() && y <= py && py < bottom();
  }

  template<typename T>
  RectT<T> &RectT<T>::enlarge(T k) {
    x -= k; y -= k; width += T(2) * k; height += T(2) * k;
    return *this;
  }

  template<typename T>
  RectT<T> RectT<T>::transform(double xfac, double yfac) const {
    return RectT<T>(static_cast<T>(xfac * x),     static_cast<T>(yfac * y),
                    static_cast<T>(xfac * width), static_cast<T>(yfac * height));
  }

  template<typename T>
  std::ostream &operator<<(std::ostream &s, const RectT<T> &r) {
    return s << r.ul() << r.getSize();
  }

  template<typename T>
  std::istream &operator>>(std::istream &s, RectT<T> &r) {
    PointT<T> offs;
    SizeT<T> size;
    s >> offs >> size;
    r = RectT<T>(offs, size);
    return s;
  }

  // ---------- explicit instantiations ----------

  template class RectT<int>;
  template class RectT<float>;

  template std::ostream &operator<<(std::ostream &, const RectT<int> &);
  template std::ostream &operator<<(std::ostream &, const RectT<float> &);
  template std::istream &operator>>(std::istream &, RectT<int> &);
  template std::istream &operator>>(std::istream &, RectT<float> &);

  } // namespace icl::utils
