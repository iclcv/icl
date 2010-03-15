/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/src/Rect.cpp                                  **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>
#include <cmath>
const icl::Rect icl::Rect::null(0,0,0,0);

namespace icl{
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
}
