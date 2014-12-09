/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SurfaceFeatureExtractor.h          **
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

#include <ICLGeom/GeomDefs.h>
#include <ICLCore/DataSegment.h>
#include <ICLCore/Img.h>

namespace icl{
  namespace geom{

    /// class for extraction of surface features.
    /** The SurfaceFeatureExtractor class computes normalHistograms, meanNormals, meanPosition, and curvatureFactor(planar, 1D curved, 2D curved) for sets of points. */

    class SurfaceFeatureExtractor{
      	
  	  public:
  	    enum Mode {
  	      NORMAL_HISTOGRAM = 1,
  	      CURVATURE_FACTOR = 2,
  	      MEAN_NORMAL = 4,
  	      MEAN_POSITION = 8,
  	      BOUNDING_BOX_3D = 16,
  	      BOUNDING_BOX_2D = 32,
  	      ALL = 63
  	    };
  	  
        
        typedef struct{
          int numPoints;//number of surface points
          core::Img32f normalHistogram;//normal histogram (11x11 bins representing x and y component (each from -1 to 1 in 0.2 steps)
          core::Channel32f normalHistogramChannel;
          Vec meanNormal;//mean normal
          Vec meanPosition;//mean position
          int curvatureFactor;//curvature Factor from enum CurvatureFactor
          std::pair<Vec,Vec> boundingBox3D;
          std::pair<utils::Point,utils::Point> boundingBox2D;
          float volume;//volume of the 3D bounding box
        }SurfaceFeature;
        
        
        enum CurvatureFactor {
          UNDEFINED=0,
          PLANAR=1,
          CURVED_1D=2,
          CURVED_2D=3
        };
        
          
        /// Applies the surface feature calculation for one single surface (please note: no BoundingBox2D)
        /** @param points the points xyz
            @param normals the corresponding point normals
            @param mode the mode from Mode enum (e.g. A | B)
            @return the SurfaceFeature struct.
        */              
        static SurfaceFeature apply(std::vector<Vec> &points, std::vector<Vec> &normals, int mode=ALL);
        
        /// Applies the surface feature calculation for all segments in the label image
        /** @param labelImage the label image of the segmentation
            @param xyzh the xyz pointcloud
            @param normals the corresponding point normals
            @param mode the mode from Mode enum (e.g. A | B)
            @return a vector of SurfaceFeature struct for each segment.
        */              
        static std::vector<SurfaceFeature> apply(core::Img32s labelImage, core::DataSegment<float,4> &xyzh, core::DataSegment<float,4> &normals, int mode=ALL);
        
        /// Calculates the matching score between 0 and 1 for two normal histograms.
        /** @param a the first normal histogram
            @param b the second normal histogram
            @return the matching score between 0 (no matching) and 1 (perfect matching).
        */
        static float matchNormalHistograms(core::Img32f &a, core::Img32f &b);
        
      private:
      
        static SurfaceFeature getInitializedStruct();
        static void update(Vec &normal, Vec &point, SurfaceFeature &feature, int mode, int x=0, int y=0);
        static void finish(SurfaceFeature &feature, int mode);      
    };
     
  } // namespace geom
}
