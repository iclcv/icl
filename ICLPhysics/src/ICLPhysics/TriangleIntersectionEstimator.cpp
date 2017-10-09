/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/TriangleIntersectionEstimator.cpp**
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#include <ICLPhysics/TriangleIntersectionEstimator.h>
#include<ICLPhysics/SoftObject.h>

namespace icl{

  using namespace utils;
  using namespace geom;
namespace physics{
  static inline float dot(const Vec &a, const Vec &b){
    return a[0]*b[0] +a[1]*b[1] +a[2]*b[2];
  }

  TriangleIntersectionEstimator::Intersection
  TriangleIntersectionEstimator::find(const TriangleIntersectionEstimator::Triangle &t, const ViewRay &r){
    //Vector    u, v, n;             // triangle vectors
    //Vector    dir, w0, w;          // ray/line vectors
    //float     r, a, b;             // parameters to estimate ray-plane intersect
    static const float EPSILON = 0.00000001;
    // get triangle edge vectors and plane normal
    Vec u = t.b - t.a;
    Vec v = t.c - t.a;
    Vec n = cross(v,u);  // TEST maybe v,u ??
    if (fabs(n[0]) < EPSILON && fabs(n[1]) < EPSILON && fabs(n[2]) < EPSILON){
      return degenerateTriangle;
    }

    const Vec dir = r.direction;
    Vec w0 =  r.offset - t.a;

    float a = -dot(n,w0);
    float b = dot(n,dir);
    if (fabs(b) < EPSILON) {     // ray is parallel to triangle plane
      return a<EPSILON ? Intersection(rayIsCollinearWithTriangle) : Intersection(noIntersection);
    }

    // get intersect point of ray with triangle plane
    float rr = a / b;
    if (rr < 0) {
      return Intersection(wrongDirection);
    }

    Vec intersection = r.offset + dir * rr;

    // is I inside T?
    float uu = dot(u,u);
    float uv = dot(u,v);
    float vv = dot(v,v);
    Vec w = intersection - t.a;
    float wu = dot(w,u);
    float wv = dot(w,v);
    float D = uv * uv - uu * vv;

    // get and test parametric coords
    float s = (uv * wv - vv * wu) / D;
    float tt = (uv * wu - uu * wv) / D;

    if (s < 0.0 || s > 1.0){
      return Intersection(noIntersection,intersection,Point32f(s,tt));
    }
    if (tt < 0.0 || (s + tt) > 1.0){
      return Intersection(noIntersection,intersection,Point32f(s,tt));
    }
    intersection[3] = 1;
    return Intersection(foundIntersection,intersection,Point32f(s,tt));
  }

}
}
