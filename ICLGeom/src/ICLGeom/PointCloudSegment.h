/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudSegment.h                **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Andre Ueckermann                  **
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

#include <ICLGeom/PointCloudObject.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace geom{
    /// PointCloudSegment class used to describe data segments in the scene
    /** Extends the PointCloudObject class (which stores the points in unordered fashion)
		  with some meta information about the point cloud, 
		  i.e. eigenvalues, eigenvectors, bounding box, centroid
    */
    struct PointCloudSegment : public PointCloudObject{
      bool featuresComputed;
      math::FixedMatrix<float,3,3> covariance;
      math::Vec3 mean;
      math::Vec3 eigenvectors[3];
      math::Vec3 eigenvalues;
      size_t     numPoints;
      float cutCost;

      struct AABB{
        math::Vec3 min, max;
      } aabb;

      
      PointCloudSegment() 
        : PointCloudObject(0,false,true),
          featuresComputed(false),
          numPoints(0) {}
      PointCloudSegment(int dim, bool withColors=true);
      PointCloudSegment(PointCloudObject &obj);
      PointCloudSegment(PointCloudObject &obj, 
                        const std::vector<int> &indices);
      PointCloudSegment(PointCloudObject &obj, 
                        const std::vector<const std::vector<int>*> &indices);
      
      PointCloudSegment *copy() const;
      
      Mat computeEigenVectorFrame() const throw (utils::ICLException);
      
      /// updates the instances features
      /** parent instances update their features from their children's featuers */
      void updateFeatures();

      /// parent objects have children and a dimension of 0
      /** i.e. parent objects have no own points */
      bool isParent() { return getChildCount (); }
      
      /// returns number of sub-segements (i.e. the number of children)
      int getNumSubSegments(){
        return getChildCount();
      }
     
      /// returns week pointer to the i-th child (already casted to PointCloudSegment type)
      utils::SmartPtr<PointCloudSegment> getSubSegment(int i) 
        throw (utils::ICLException);

      /// creates a flattened deep copy of the segment
      utils::SmartPtr<PointCloudSegment> flatten() const;
      
      
      /// returns a vector containing this and all recursive children
      std::vector<utils::SmartPtr<PointCloudSegment> > extractThisAndChildren() const;
      
    protected:
      void init(PointCloudObject &obj, 
                const std::vector<const std::vector<int>*> &indices);
    };
    
    /// 
    typedef utils::SmartPtr<PointCloudSegment> PointCloudSegmentPtr;
    
    inline std::ostream &operator<<(std::ostream &str, const PointCloudSegment::AABB &a){
      return str << "AABB(min: " << a.min.transp() << "  max: " << a.max.transp() << ")";
    }
  }
}
