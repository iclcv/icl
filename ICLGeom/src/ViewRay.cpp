/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ViewRay.cpp                                **
** Module : ICLGeom                                                **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLGeom/ViewRay.h>
#include <ICLGeom/Camera.h>

namespace icl{
  
  ViewRay::ViewRay(const Vec &offset, const Vec &direction,bool autoNormalizeDirection):
    offset(offset),direction(direction){
    if(autoNormalizeDirection){
      this->direction *= 1.0f/norm3(direction);
    }
    this->offset[3]=this->direction[3]=1;
  }
  
  Vec ViewRay::getIntersection(const PlaneEquation &plane) const throw (ICLException){
    return Camera::getIntersection(*this,plane);
  }
  
  Vec ViewRay::operator()(float lambda) const { 
    Vec r = offset + direction*lambda; 
    r[3] = 1;
    return r;
  }

  float ViewRay::closestDistanceTo(const Vec &p) const{
    const Vec x = p-offset;
    return ::sqrt(sqrnorm3(x)-sqr(sprod3(x,direction)));
  }
  
  float ViewRay::closestDistanceTo(const ViewRay &vr) const{
    Vec c = cross(direction,vr.direction);
    return fabs(sprod3(offset - vr.offset, c) / norm3(c));
  }


  std::ostream &operator<<(std::ostream &s, const ViewRay &vr){
    return s << "ViewRay(" << vr.offset.transp() << " + lambda * " << vr.direction.transp() << ")";
  }

}
