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
  
  ViewRay::ViewRay(const Vec &offset, const Vec &direction):
    offset(offset),direction(direction){
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

  std::ostream &operator<<(std::ostream &s, const ViewRay &vr){
    return s << "ViewRay(" << vr.offset.transp() << " + lambda * " << vr.direction.transp() << ")";
  }

}
