// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Patrick Nobou

#pragma once

#ifndef ICL_HAVE_PCL
#warning "this header must not be included without ICL_HAVE_PCL defined"
#endif

#include <icl/geom/PointCloudObjectBase.h>
#include <icl/geom/PCLIncludes.h>

/** \cond */
namespace pcl{
  template<class PCLPointType> class PointCloud;
}
/** \endcond */

namespace icl::geom {
  ///PointCloudObject implementation for the PCLPointCloud types
  /** Right now, the following pcl-point types are supported. Due to the non-inline
      implementation of the class, all other point-types are not available. For other
      point types, one can specialize the offset()-template in the source file.
				- pcl::PointXYZ
      - pcl::PointXYZI
      - pcl::PointXYZL
      - pcl::PointXYZRGB
      - pcl::PointXYZRGBA
      - pcl::InterestPoint
      - pcl::PointXYZRGBNormal
      - pcl::PointXYZINormal
  **/
  template<class PCLPointType>
  class PCLPointCloudObject : public PointCloudObjectBase{
    pcl::PointCloud<PCLPointType> *m_pcl; //!< internal pcl::PointCloud data pointer
    bool m_ownPCL;                        //!< ownership flag

    /// internally used typedef
    using Entry = PCLPointType;

    /// this is actually specialized for the different types
    /** Please note, offset needs to return -1 for non-supported feature types */
    int offset(FeatureType) const;

    /// returns the data orignin pointer
    inline icl8u* data();

    /// returns the data orignin pointer (const)
    inline const icl8u* data() const;

    /// creates a data segment for a given feature type
    template<class T, int N, FeatureType t>
    inline core::DataSegment<T,N> createSegment();

    /// savely delets the pcl_image
    void deletePCL();

    public:

#if 0
    // this is right now deactivated due to an extra dependency to libpcl_io
    // which always depends on openni

    /// creates a PCLPointCloudObject from given filename
    /** This is basically just a convenience function to circumvent using
        a PointCloudGrabber instance */
    PCLPointCloudObject(const std::string &filename="");
#endif
    /// creates a PCLPointCloudObject with given width height and intial value
    /** If the height value is -1, the PointCloud instance is assumed to be not ordered,
        e.g. only a 1D set of points instead of a 2D array */
    PCLPointCloudObject(int width, int height = -1, const PCLPointType &init=PCLPointType());

    /// deeply copying copy constructor
    PCLPointCloudObject(const PCLPointCloudObject<PCLPointType> &other);

    /// deeply copying assignment operator
    /** for shallow copies, use

    */
    PCLPointCloudObject<PCLPointType> &operator=(const PCLPointCloudObject<PCLPointType> &other);

    /// const wrapper for given pcl-pointcloud (deep copy only)
    PCLPointCloudObject(const pcl::PointCloud<PCLPointType> &cloud);

    /// wrapps given pcl-point-cloud (optionally shallowly copied)
    PCLPointCloudObject(pcl::PointCloud<PCLPointType> &cloud, bool deepCopy=true);

    /// Destructor
    ~PCLPointCloudObject();

    /// grants access to the underlying pcl-point-cloud
    pcl::PointCloud<PCLPointType> &pcl();

    /// grants access to the underlying pcl-point-cloud (const)
    const pcl::PointCloud<PCLPointType> &pcl() const;

    /// sets wrapped PCL point cloud (const, always deeply copied)
    void setPCL(const pcl::PointCloud<PCLPointType> &pcl);

    /// sets wrapped PCL point cloud (const, always deeply copied)
    void setPCL(pcl::PointCloud<PCLPointType> &pcl, bool deepCopy = true);

    /// generic version, that trys to get an offset (can be optimized with specialization)
    bool supports(FeatureType t) const override;

    /// returns whether pointcloud is 2D organized
    bool isOrganized() const override;

    /// returns the 2D size of the pointcloud (throws exception if not ordered)
    utils::Size getSize() const override;

    /// return the linearily ordered number of point in the point cloud
    int getDim() const override;

    /// adapts the point cloud size
    /** if the sizes height is smaller than 1, the cloud becomes un-organized*/
    void setSize(const utils::Size &size) override;

    bool isNull() const;

    // well known fields
    core::DataSegment<float,1> selectIntensity() override;
    core::DataSegment<icl32s,1> selectLabel() override;
    core::DataSegment<icl8u,3> selectBGR() override;
    core::DataSegment<icl8u,4> selectBGRA() override;
    core::DataSegment<icl32s,1> selectBGRA32s() override;

    core::DataSegment<float,3> selectXYZ() override;
    core::DataSegment<float,4> selectXYZH() override;
    core::DataSegment<float,4> selectNormal() override;
    core::DataSegment<float,4> selectRGBA32f() override;

    /// selects a dynamic feature
    /** some pcl types support special features that can be selected by a string ID.
        By default, the featureName 'all' can be used to create a DataSegment that
        give access to all data entries as floats */
    core::DataSegmentBase select(const std::string &featureName) override;

    /// deep copy interface
    virtual PCLPointCloudObject<PCLPointType> *deepCopy() const override;

  };

  } // namespace icl::geom