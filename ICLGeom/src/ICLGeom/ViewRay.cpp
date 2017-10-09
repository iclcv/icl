/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ViewRay.cpp                        **
** Module : ICLGeom                                                **
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

#include <ICLGeom/ViewRay.h>
#include <ICLGeom/Camera.h>
#include <ICLMath/HomogeneousMath.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace geom{

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

    float ViewRay::closestSqrDistanceTo(const Vec &p) const{
      const Vec x = p-offset;
	  //return l3(x - icl::math::sprod3(x,direction)*direction);
      return sqrnorm3(x)-sqr(sprod3(x,direction));
    }

    float ViewRay::closestDistanceTo(const ViewRay &vr) const{
      Vec c = cross(direction,vr.direction);
      return fabs(sprod3(offset - vr.offset, c) / norm3(c));
    }


    std::ostream &operator<<(std::ostream &s, const ViewRay &vr){
      return s << "ViewRay(" << vr.offset.transp() << " + lambda * " << vr.direction.transp() << ")";
    }

    inline float dot(const Vec &a, const Vec &b){
      return a[0]*b[0] +a[1]*b[1] +a[2]*b[2];
    }

    ViewRay::TriangleIntersection ViewRay::getIntersectionWithTriangle(const Vec &ta, const Vec &tb, const Vec &tc,
                                                                       Vec *intersectionPoint, Point32f *parametricCoords) const {
      //Vector    u, v, n;             // triangle vectors
      //Vector    dir, w0, w;          // ray vectors
      //float     r, a, b;             // params to calc ray-plane intersect
      static const float EPSILON = 0.00000001;
      // get triangle edge vectors and plane normal
      Vec u = tb - ta;
      Vec v = tc - ta;
      Vec n = cross(v,u);
      if (fabs(n[0]) < EPSILON && fabs(n[1]) < EPSILON && fabs(n[2]) < EPSILON){
        return degenerateTriangle;
      }

      const Vec dir = this->direction;
      Vec w0 =  this->offset - ta;

      float a = -dot(n,w0);
      float b = dot(n,dir);
      if (fabs(b) < EPSILON) {     // ray is parallel to triangle plane
        return a<EPSILON ? rayIsCollinearWithTriangle : noIntersection;
      }

      // get intersect point of ray with triangle plane
      float rr = a / b;
      if (rr < 0) {
        return wrongDirection;
      }

      Vec intersection = this->offset + dir * rr;


      // is I inside T?
      float uu = dot(u,u);
      float uv = dot(u,v);
      float vv = dot(v,v);
      Vec w = intersection - ta;
      float wu = dot(w,u);
      float wv = dot(w,v);
      float D = uv * uv - uu * vv;

      // get and test parametric coords
      float s = (uv * wv - vv * wu) / D;
      if (s < 0.0 || s > 1.0){
        return noIntersection;
      }
      float tt = (uv * wu - uu * wv) / D;
      if (tt < 0.0 || (s + tt) > 1.0){
        return noIntersection;
      }

      if(parametricCoords) *parametricCoords = Point32f(s,tt);
      if(intersectionPoint) {
        *intersectionPoint = intersection;
        (*intersectionPoint)[3] = 1;
      }

      return foundIntersection;

    }

  } // namespace geom
}
