/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RayCastOctreeObject.h              **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/OctreeObject.h>
#include <ICLGeom/ViewRay.h>
#include <vector>

namespace icl{
  namespace geom {
    
    /// Special Octree implemenation that supports to query points close to caste rays
    /** The RayCastOctreeObject defines all template parameters of the more general
        OctreeObject class, while preserving the super classe's interface, it adds
        the rayCast function, which can be used to query all points, that are 
        close to a given view-ray. 
        
        \section ALGO Algorithm
        
        Internally the octree structure is exploited for accelerating ray casts.
        Each node's AABB (axis aligned bounding box) defines a bounding sphere around
        it's center. The radius of the bounding sphere is pre-computed and accessible
        from the Node structure. Whenever the center of node is further away
        from the view-ray than the pre-computed node radius plus the maximum distance
        to the ray the node and all it's sub-nodes are not tested any further.
        As an additional speed-up, internally squared distances are used to avoid
        expensive square-root calculations.
        

        \section SPEED Ray Cast Run Time
        On a Core i5 @ 3.4 GHz with 8GB Ram and 64 Bit linux, an orthogonal ray cast into 
        a regular grid 2D point cloud in 3D space (containing a typical number of, 
        320x240 points) takes about 3.2 micro-secounds - that is 330 ray-casts per 
        millisecond. This is about 250 times faster than a brute force search.
        
    */
    class RayCastOctreeObject : public OctreeObject<float, 16, 1, Vec, 1024>{
      typedef OctreeObject<float,16,1,Vec,1024> Super;
      public:
      /// creates a QuadTree for the given 2D rectangle
      RayCastOctreeObject(const float &minX, const float &minY, const float &minZ,
                           const float &width, const float &height, const float &depth):
        Super(minX,minY,minZ,width,height,depth){}
      
      /// creates a QuadTree for the given 2D rectangle
      RayCastOctreeObject(const float &min, const float &len):Super(min,len){}
        
      /// destructor
      /** Deletes the root node only, all other nodes are deleted by the allocator */
      ~RayCastOctreeObject(){}
      
      
      protected:
      
      /// internal worker method for recursive ray cast
      static void ray_cast_sqr_rec(const Super::Node *n, const ViewRay &ray, 
                                   float maxSqrDist, float maxDist, 
                                   std::vector<Vec> &result);
     
      static void ray_cast_sqr_rec_debug(const RayCastOctreeObject::Super::Node *n, const ViewRay &ray, 
                                         float maxSqrDist, float maxDist, 
                                         std::vector<Vec> &result,
                                         std::vector<AABB> &boxes, std::vector<Vec> &points);
      public:
      
      /// casts a ray and returns all points that are closer than given distance to ray
      ICLGeom_API std::vector<Vec> rayCast(const ViewRay &ray, float maxDist=1) const;

      /// casts a ray and returns all points that are closer than given distance to ray
      ICLGeom_API std::vector<Vec> rayCastDebug(const ViewRay &ray, float maxDist,
                                                std::vector<AABB> &boxes, std::vector<Vec> &points) const;
      
      /// as ray cast, but sorts the points by distance to the ray-offset
      ICLGeom_API std::vector<Vec> rayCastSort(const ViewRay &ray, float maxDist = 1) const;

      /// casts a ray and returns the point closest to the ray-offset
      ICLGeom_API Vec rayCastClosest(const ViewRay &ray, float maxDist = 1) const throw (utils::ICLException);
    };
    
  }
}
