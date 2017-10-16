/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/StraightLine2D.cpp                 **
** Module : ICLMath                                                **
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

#include <ICLMath/StraightLine2D.h>

using namespace icl::utils;

namespace icl{
  namespace math{

    StraightLine2D::StraightLine2D(float angle, float distance){
      o = Pos(cos(angle),sin(angle))*distance;
      v = Pos(o[1],-o[0]);
      v = v/v.length();
    }

    StraightLine2D::StraightLine2D(const Pos &o, const Pos &v):
      o(o),v(v){
      float l = v.length();
      if(l){
        this->v = this->v/l;
      }
    }

    /// creates a straight line from given point32f
    StraightLine2D::StraightLine2D(const Point32f &o, const Point32f &v):
      o(o.x,o.y),v(v.x,v.y){
      float l = this->v.length();
      if(l){
        this->v = this->v/l;
      }
    }


    StraightLine2D::PointPolar StraightLine2D::getAngleAndDistance() const{
      Pos p(v[1],-v[0]);
      return PointPolar(distance(Pos(0,0)),atan2(-v[0],v[1]));
    }

    float StraightLine2D::signedDistance(const Pos &p2) const{
      Pos p = p2-o;
      Pos v = this->v/this->v.length();
      float fac = (p[0]*v[0] + p[1]*v[1]);
      float dist = ((v*fac)-p).length();
      return (p[0] * v[1] - p[1] * v[0] > 0) ? dist : -dist;
    }

    float StraightLine2D::distance(const Pos &p2) const{
      Pos p = p2-o;
      Pos v = this->v/this->v.length();
      float fac = (p[0]*v[0] + p[1]*v[1]); /*/(v[0]*v[0] + v[1]*v[1]);  = 1!!*/
      return ((v*fac)-p).length();
    }

    StraightLine2D::Pos StraightLine2D::intersect(const StraightLine2D &other) const throw(ICLException){
      float A1,B1,C1,A2,B2,C2;
      {
        Pos a=o;
        Pos b=o+v;
        A1 = b[1]-a[1];
        B1 = a[0]-b[0];
        C1 = A1*a[0] + B1*a[1];
      }
      {
        Pos a=other.o;
        Pos b=other.o+other.v;
        A2 = b[1]-a[1];
        B2 = a[0]-b[0];
        C2 = A2*a[0] + B2*a[1];
      }
      float det = A1*B2 - A2*B1;
      if(det == 0){
        throw ICLException("lines are parallel");
      }
      return Pos((B2*C1 - B1*C2)/det,(A1*C2 - A2*C1)/det);
    }
    StraightLine2D::Pos StraightLine2D::getClosestPoint(const Pos &p) const{
      Pos x = p - o;
      return o + v*(v[0]*x[0]+v[1]*x[1]);
    }

  } // namespace math
}

