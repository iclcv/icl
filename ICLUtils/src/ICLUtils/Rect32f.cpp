/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Rect32f.cpp                      **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Rect.h>



namespace icl{
  namespace utils{

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


  } // namespace utils
}
