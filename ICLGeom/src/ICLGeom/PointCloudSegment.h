// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Ueckermann

#pragma once

#include <ICLGeom/PointCloudObject.h>
#include <ICLUtils/Exception.h>

namespace icl::geom {
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

      Mat computeEigenVectorFrame() const;

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
      std::shared_ptr<PointCloudSegment> getSubSegment(int i);

      /// creates a flattened deep copy of the segment
      std::shared_ptr<PointCloudSegment> flatten() const;


      /// returns a vector containing this and all recursive children
      std::vector<std::shared_ptr<PointCloudSegment> > extractThisAndChildren() const;

    protected:
      void init(PointCloudObject &obj,
                const std::vector<const std::vector<int>*> &indices);
    };

    ///
    using PointCloudSegmentPtr = std::shared_ptr<PointCloudSegment>;

    inline std::ostream &operator<<(std::ostream &str, const PointCloudSegment::AABB &a){
      return str << "AABB(min: " << a.min.transp() << "  max: " << a.max.transp() << ")";
    }
  }