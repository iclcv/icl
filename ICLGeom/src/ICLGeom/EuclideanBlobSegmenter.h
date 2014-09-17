/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/EuclideanBlobSegmenter.h           **
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

#include <ICLCore/DataSegment.h>

namespace icl{
  namespace geom{
    /**
       This class implements blob segmentation for depth images. It uses OpenCL for hardware parallelization if a compatible GPU is found. The input is a depth image, a binarized edge image from the ObjectEdgeDetector class and the xyz DataSegment from the PointCloudObject class. The output is a color image (e.g. as input for setColorsFromImage() method of the PointCloudObject class).*/
    class ICLGeom_API EuclideanBlobSegmenter{
  	
     public:
     
      enum Mode {BEST, GPU, CPU};
      
      /// Constructor
      /** Constructs an object of this class. All default parameters are set. Use setters for desired values.
          @param mode GPU, CPU and BEST (default) */
      EuclideanBlobSegmenter(Mode mode=BEST); 
  	
      /// Destructor
      ~EuclideanBlobSegmenter();
  	      
      /// One line call for the support plane and blobs segmentation.
      /** @param xyz the xyzh DataSegment from the PointCloudObject class
          @param edgeImg the edge image from the ObjectEdgeDetector class
          @param depthImg the input depth image
          @param stabelize frame-to-frame cross-correlation to achieve the same object labels for each frame
          @param useROI true for 3D region of interest (set by setROI() )
          @return the color image of the segments */
      core::Img8u apply(core::DataSegment<float,4> xyz, const core::Img8u &edgeImg, const core::Img32f &depthImg, bool stabelize=true, bool useROI=false);
  	  		      
      /// Sets the ROI in world coordinates
      /**       @param xMin xMin in mm
                @param xMax xMax in mm
                @param yMin yMin in mm
                @param yMax yMax in mm 
                @param zMin zMin in mm
                @param zMax zMax in mm*/
      void setROI(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
  		
      /// Sets the minimum segment size (default 25)
      /**       @param size minimum segment size in points */
      void setMinClusterSize(unsigned int size);
      
      /// Sets the RANSAC parameters for the support plane extraction
      /**       @param distance maximum distance between point and plane
                @param passes number of RANSAC passes
                @param used subset of points (e.g. 2 for every second point)*/
      void setRansacParams(float distance, int passes, int subset);
        		
      /// Sets the minimum euclidean distance of blobs 
      /**       @param distance the minimum euclidean distance for blob segmentation */
      void setBLOBSeuclDistance(int distance);
            
      /// Returns the labeled segment image.
      /**        @return the labeled segment image */
      core::Img32s getLabelImage(bool stabelize);
      
      /// Returns the colored segment image. Also returned by the apply method.
      /**        @return the colored segment image */
      core::Img8u getColoredLabelImage(bool stabelize);
      
      /// Returns the surface cluster from the pre-segmentation.
      /**        @return a vector of cluster. Every entry contains a vector with the point indices */
      std::vector<std::vector<int> > getSurfaces();
      
      /// Returns the segment blobs.
      /**        @return a vector of blobs. Every entry contains a vector with the point indices */
      std::vector<std::vector<int> > getBlobs();
      
      
     private:
     
      struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer
                 
      void regionGrow(bool useROI); 
	  	  		
      void blobSegmentation(bool useROI);
  		
      void regionGrowBlobs();

    };
  } // namespace geom
}
