// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Rect32f.h>
#include <icl/utils/Rect.h>



namespace icl::utils {
  const Rect32f Rect32f::null(0,0,0,0);

  Rect32f Rect32f::operator&(const Rect32f &r) const {
    Point32f ul_pt(iclMax(x, r.x), iclMax(y, r.y));
    Point32f lr_pt(iclMin(right(), r.right()), iclMin(bottom(), r.bottom()));
    Rect32f result(ul_pt.x, ul_pt.y, lr_pt.x - ul_pt.x, lr_pt.y - ul_pt.y);
    if (result.width > 0 && result.height > 0) return result;
    else return null;
  }

  Rect32f &Rect32f::operator&=(const Rect32f &r){
    (*this) = (*this) & r;
    return *this;
  }

  Rect32f Rect32f::operator|(const Rect32f &r) const {
    Point32f ul_pt(iclMin(x, r.x), iclMin(y, r.y));
    Point32f lr_pt(iclMax(right(), r.right()), iclMax(bottom(), r.bottom()));
    return Rect32f(ul_pt.x, ul_pt.y, lr_pt.x - ul_pt.x, lr_pt.y - ul_pt.y);
  }

  Rect32f &Rect32f::operator|=(const Rect32f &r){
    (*this) = (*this) | r;
    return *this;
  }

  Rect32f Rect32f::normalized() const {
    Rect32f r(*this);
    if (r.width < 0) { r.x += r.width; r.width = -r.width; }
    if (r.height < 0) { r.y += r.height; r.height = -r.height; }
    return r;
  }

  bool Rect32f::contains(const Rect32f &r) const {
    return x <= r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
  }

  bool Rect32f::contains(float px, float py) const {
    return this->x <= px && right() >= px && this->y <= py && bottom() >= py;
  }

  Rect32f &Rect32f::enlarge(float k){
    x -= k; y -= k; width += 2 * k; height += 2 * k;
    return *this;
  }

  Rect32f Rect32f::enlarged(float k) const {
    return Rect32f(*this).enlarge(k);
  }

  Rect32f Rect32f::transform(double xfac, double yfac) const {
    return Rect32f(x * xfac, y * yfac, width * xfac, height * yfac);
  }

  /// ostream operator (x,y)wxy
  std::ostream &operator<<(std::ostream &s, const Rect32f &r){
    return s << r.ul() << r.getSize();
  }

  /// istream operator
  std::istream &operator>>(std::istream &s, Rect32f &r){
    Point32f offs;
    Size32f size;
    s >> offs;
    s >> size;
    r = Rect32f(offs,size);
    return s;
  }


  } // namespace icl::utils