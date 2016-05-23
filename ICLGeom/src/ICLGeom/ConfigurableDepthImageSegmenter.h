/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ConfigurableDepthImageSegmenter.h  **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
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

#include <ICLGeom/FeatureGraphSegmenter.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/PointCloudCreator.h>
#include <ICLUtils/Configurable.h>
#include <ICLCore/DataSegment.h>
#include <ICLGeom/Camera.h>

namespace icl{
  namespace geom{
    /**
       This class computes the whole segmentation pipeline of the feature graph based segmentation. The configurable class provides the GUI with all paramters: Prop("segmentation") */
    class ICLGeom_API ConfigurableDepthImageSegmenter : public utils::Configurable{
  	
      public:
      
      enum Mode {BEST, GPU, CPU};
      
      /// Constructor
      /** Constructs an object of this class. 
          @param mode GPU, CPU and BEST (default) 
          @param depthCam the depth camera*/
			ConfigurableDepthImageSegmenter(Mode mode, Camera depthCam,
																			icl::geom::PointCloudCreator::DepthImageMode depth_mode
																			= icl::geom::PointCloudCreator::KinectRAW11Bit);
			ConfigurableDepthImageSegmenter(Mode mode, Camera depthCam, Camera colorCam,
																			icl::geom::PointCloudCreator::DepthImageMode depth_mode
																			= icl::geom::PointCloudCreator::KinectRAW11Bit);
  	  
  	  ///Destructor
      ~ConfigurableDepthImageSegmenter();
  		
  		/// One line call for the whole segmentation pipeline.
      /** @param depthImage the input depth image
          @param obj the empty pointcloud object for the results (computed in the method) */
      void apply(const core::Img32f &depthImage, PointCloudObject &obj);
      
	  std::vector<geom::SurfaceFeatureExtractor::SurfaceFeature> getSurfaceFeatures();

      /// Returns the colored normal image.
      /**        @return the colored normal image */  		
      const core::Img8u getNormalImage();
  	  
  	  /// Returns the edge image.
      /**        @return the edge image */  			
      const core::Img8u getEdgeImage();
  	  
  	  /// Returns the label image.
      /**        @return the label image */  		
  	  core::Img32s getLabelImage();	

      const core::DataSegment<float,4> getNormalSegment();
  	  
  	  /// Returns the colored label image.
      /**        @return the colored label image */  			
      core::Img8u getColoredLabelImage();

	  /// Returns the angle image of the object edge detector
	  /// /**        @return the angle image */
	  const core::Img32f getAngleImage();

      /// Returns the mapped image (the last passed depth image is used for mapping) assumed to be grabbed by the camera cam
      /**       @return the mapped image */
      core::Img8u getMappedColorImage(const core::Img8u &image);

	  void mapImageToDepth(const core::ImgBase *src, core::ImgBase **dst);
      
      /// Returns the surface cluster from the pre-segmentation.
      /**        @return a vector of surfaces. Every entry contains a vector with the point indices */
      std::vector<std::vector<int> > getSurfaces();
      
      /// Returns the segment blobs.
      /**        @return a vector of segments. Every entry contains a vector with the surface indices */
      std::vector<std::vector<int> > getSegments();
      
	  void setNormals(core::DataSegment<float,4> &normals);

		void setEdgeSegData(core::Img8u &edges, core::Img8u &normal_img);

		void setUseExternalEdges(bool use_external_edges);
            
      private:

      void initProperties();

      struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer

    };
  } // namespace geom
}
