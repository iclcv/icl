/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/src/Point32f.cpp                              **
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

#include <ICLUtils/Point32f.h>
#include <math.h>
namespace icl{
  const Point32f Point32f::null(0.0,0.0);

  float Point32f::norm(float p){
    return pow( pow(x,p)+ pow(y,p), float(1)/p);
  }
  
  float Point32f::distanceTo(const Point32f &p) const{
    return sqrt(pow((float) (p.x-x), 2) + pow((float) (p.y-y), 2));
  }

  std::ostream &operator<<(std::ostream &s, const Point32f &p){
    return s << "(" << p.x << ',' << p.y << ")";
  }
  
  std::istream &operator>>(std::istream &s, Point32f &p){
    char c;
    return s >> c >> p.x >> c >> p.y >> c;
  }

}
