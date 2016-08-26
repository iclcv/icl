/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PCLPointCloudObject.h              **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#ifndef ICL_HAVE_PCL
#warning "this header must not be included without ICL_HAVE_PCL defined"
#endif

#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLGeom/PCLIncludes.h>

/** \cond */
namespace pcl{
  template<class PCLPointType> class PointCloud;
}
/** \endcond */

namespace icl{
  namespace geom{
  
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
      typedef PCLPointType Entry;
  
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
      pcl::PointCloud<PCLPointType> &pcl() throw (utils::ICLException);
  
      /// grants access to the underlying pcl-point-cloud (const)
      const pcl::PointCloud<PCLPointType> &pcl() const throw (utils::ICLException);
      
      /// sets wrapped PCL point cloud (const, always deeply copied)
      void setPCL(const pcl::PointCloud<PCLPointType> &pcl);
  
      /// sets wrapped PCL point cloud (const, always deeply copied)
      void setPCL(pcl::PointCloud<PCLPointType> &pcl, bool deepCopy = true);
  
      /// generic version, that trys to get an offset (can be optimized with specialization)
      virtual bool supports(FeatureType t) const;
  
      /// returns whether pointcloud is 2D organized
      virtual bool isOrganized() const;
  
      /// returns the 2D size of the pointcloud (throws exception if not ordered)
      virtual utils::Size getSize() const throw (utils::ICLException);
      
      /// return the linearily ordered number of point in the point cloud
      virtual int getDim() const;
  
      /// adapts the point cloud size
      /** if the sizes height is smaller than 1, the cloud becomes un-organized*/
      virtual void setSize(const utils::Size &size);
  
      bool isNull() const;
  
      // well known fields
      virtual core::DataSegment<float,1> selectIntensity();
      virtual core::DataSegment<icl32s,1> selectLabel();
      virtual core::DataSegment<icl8u,3> selectBGR();
      virtual core::DataSegment<icl8u,4> selectBGRA();
      virtual core::DataSegment<icl32s,1> selectBGRA32s();
  
      virtual core::DataSegment<float,3> selectXYZ();
      virtual core::DataSegment<float,4> selectXYZH();
      virtual core::DataSegment<float,4> selectNormal();
      virtual core::DataSegment<float,4> selectRGBA32f();
  
      /// selects a dynamic feature
      /** some pcl types support special features that can be selected by a string ID.
          By default, the featureName 'all' can be used to create a DataSegment that
          give access to all data entries as floats */
      virtual core::DataSegmentBase select(const std::string &featureName);
  
      /// deep copy interface
      virtual PCLPointCloudObject<PCLPointType> *copy() const;
  
    };
  
  } // namespace geom
}
