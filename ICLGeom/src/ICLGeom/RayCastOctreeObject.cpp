/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RayCastOctreeObject.cpp            **
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

#include <ICLGeom/RayCastOctreeObject.h>
#include <algorithm>

namespace icl{
  using namespace utils;

  namespace geom{

    inline float sqr_ray_point_dist(const ViewRay &ray, const Vec &p){
      const Vec x = p-ray.offset;
      return sqrnorm3(x)-sqr(sprod3(x,ray.direction));
    }

    inline bool cmp_2nd(const std::pair<int,float> &a, const std::pair<int,float> &b){
      return a.second < b.second;
    }

    static std::vector<std::pair<int,float> > compute_idx_dists(const Vec &o, const std::vector<Vec> &rays){
      std::vector<std::pair<int,float> > sv(rays.size());
      for(size_t i=0;i<rays.size();++i){
        sv[i].first = i;
        sv[i].second = ( sqr(o[0]-rays[i][0]) +
                         sqr(o[1]-rays[i][1]) +
                         sqr(o[2]-rays[i][2]) );
        if(sv[i].second < 0.0001) sv[i].second = 1.e37;
      }
      return sv;
    }



    void RayCastOctreeObject::ray_cast_sqr_rec(const RayCastOctreeObject::Super::Node *n, const ViewRay &ray,
                                               float maxSqrDist, float maxDist,
                                               std::vector<Vec> &result){
      for(const Vec *p=n->points; p<n->next;++p){
        if(sqr_ray_point_dist(ray,*p) < maxSqrDist){
          result.push_back(*p);
        }
      }
      if(n->children){
        for(int i=0;i<8;++i){
            const Super::AABB &aabb = n->children[i].boundary;
            if(sqr_ray_point_dist(ray,aabb.center) <= sqr(n->children[i].radius+maxDist)){
              ray_cast_sqr_rec(n->children+i,ray,maxSqrDist, maxDist, result);
            }
        }
      }
    }

    void RayCastOctreeObject::ray_cast_sqr_rec_debug(const RayCastOctreeObject::Super::Node *n, const ViewRay &ray,
                                                     float maxSqrDist, float maxDist,
                                                     std::vector<Vec> &result,
                                                     std::vector<AABB> &boxes, std::vector<Vec> &points){
      boxes.push_back(n->boundary);
      for(const Vec *p=n->points; p<n->next;++p){
        points.push_back(*p);
        if(sqr_ray_point_dist(ray,*p) < maxSqrDist){
          result.push_back(*p);
        }
      }
      if(n->children){
        for(int i=0;i<8;++i){
          const Super::AABB &aabb = n->children[i].boundary;
          if(sqr_ray_point_dist(ray,aabb.center) <= sqr(n->children[i].radius+maxDist)){
            ray_cast_sqr_rec_debug(n->children+i,ray,maxSqrDist, maxDist, result, boxes, points);
          }
        }
      }
    }


    std::vector<Vec> RayCastOctreeObject::rayCast(const ViewRay &ray, float maxDist) const{
      const float maxSqrDist = sqr(maxDist);
      std::vector<Vec> result;
      result.reserve(4);

      ray_cast_sqr_rec(Super::root, ray, maxSqrDist, maxDist, result);

      return result;
    }

    std::vector<Vec> RayCastOctreeObject::rayCastDebug(const ViewRay &ray, float maxDist,
                                                       std::vector<AABB> &boxes, std::vector<Vec> &points) const{
      const float maxSqrDist = sqr(maxDist);
      std::vector<Vec> result;
      result.reserve(4);

      ray_cast_sqr_rec_debug(Super::root, ray, maxSqrDist, maxDist, result, boxes, points);

      return result;
    }



    std::vector<Vec> RayCastOctreeObject::rayCastSort(const ViewRay &ray, float maxDist) const{
      std::vector<Vec> rays = rayCast(ray,maxDist);
      std::vector<std::pair<int,float> > sv = compute_idx_dists(ray.offset, rays);
      std::sort(sv.begin(),sv.end(),cmp_2nd);
      std::vector<Vec> raysSorted(rays.size());
      for(size_t i=0;i<rays.size();++i){
        raysSorted[i] = rays[sv[i].first];
      }
      return raysSorted;
    }

    /// casts a ray and returns the point closest to the ray-offset
    Vec RayCastOctreeObject::rayCastClosest(const ViewRay &ray, float maxDist) const {
      std::vector<Vec> rays = rayCast(ray,maxDist);
      if(!rays.size()) throw ICLException("RayCastOctreeObject::rayCastClosest:"
                                            " no point found in radius" );
      std::vector<std::pair<int,float> > sv = compute_idx_dists(ray.offset, rays);
      return rays[std::min_element(sv.begin(),sv.end(), cmp_2nd)->first];
    }


  }
}
