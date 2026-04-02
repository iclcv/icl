// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>
#include <cmath>


namespace icl::utils {
    const Rect Rect::null(0,0,0,0);

    /// ostream operator (x,y)wxy
    std::ostream &operator<<(std::ostream &s, const Rect &r){
      return s << r.ul() << r.getSize();
    }

    /// istream operator
    std::istream &operator>>(std::istream &s, Rect &r){
      Point offs;
      Size size;
      s >> offs >> size;
      r = Rect(offs,size);
      return s;
    }
    Rect::Rect(const Rect32f &other){
      x = round(other.x);
      y = round(other.y);
      width = round(other.width);
      height = round(other.height);
    }

    Rect Rect::operator&(const Rect &r) const {
      Point ul_pt(iclMax(x, r.x), iclMax(y, r.y));
      Point lr_pt(iclMin(right(), r.right()), iclMin(bottom(), r.bottom()));
      Rect result(ul_pt.x, ul_pt.y, lr_pt.x - ul_pt.x, lr_pt.y - ul_pt.y);
      if (result.width > 0 && result.height > 0) return result;
      else return null;
    }

    Rect &Rect::operator&=(const Rect &r){
      (*this) = (*this) & r;
      return *this;
    }

    Rect Rect::operator|(const Rect &r) const {
      Point ul_pt(iclMin(x, r.x), iclMin(y, r.y));
      Point lr_pt(iclMax(right(), r.right()), iclMax(bottom(), r.bottom()));
      return Rect(ul_pt.x, ul_pt.y, lr_pt.x - ul_pt.x, lr_pt.y - ul_pt.y);
    }

    Rect &Rect::operator|=(const Rect &r){
      (*this) = (*this) | r;
      return *this;
    }

    Rect Rect::normalized() const {
      Rect r(*this);
      if (r.width < 0) { r.x += r.width; r.width = -r.width; }
      if (r.height < 0) { r.y += r.height; r.height = -r.height; }
      return r;
    }

    bool Rect::contains(const Rect &r) const {
      return x <= r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
    }

    bool Rect::contains(int px, int py) const {
      return this->x <= px && right() > px && this->y <= py && bottom() > py;
    }

    Rect &Rect::enlarge(int k){
      x -= k; y -= k; width += 2 * k; height += 2 * k;
      return *this;
    }

    Rect Rect::enlarged(int k) const {
      return Rect(*this).enlarge(k);
    }

    Rect Rect::transform(double xfac, double yfac) const {
      return Rect(static_cast<int>(xfac * x), static_cast<int>(yfac * y),
                  static_cast<int>(xfac * width), static_cast<int>(yfac * height));
    }

  } // namespace icl::utils