/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/CoPlanarityFeatureExtractor.h      **
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

#include <ICLMath/DynMatrix.h>
#include <ICLGeom/SurfaceFeatureExtractor.h>

namespace icl{
  namespace geom{

    /// class for coplanarity feature.
    /** This class implements the coplanarity feature for feature-graph based segmentation.*/

    class CoPlanarityFeatureExtractor{
      	
  	  public:
  	      
        /// Calculates the coplanarity feature matrix.
      /** @param initialMatrix the initial boolean test matrix (0 test if both planar, 1 dont test, preferably an adjacency matrix)
          @param features the surface feature for the surfaces
          @param depthImage the input depthImage
          @param surfaces the vector of surface id vectors
          @param maxAngle the maximum angle for surface orientation similarity test
          @param distanceTolerance the maximum distance in mm for occlusion check
          @param outlierTolerance the maximum number of outlier points in percent for occlusion check
          @param triangles number of combined surface tests
          @param scanlines number of occlusion tests
          @return the boolean coplanarity matrix */
        static math::DynMatrix<bool> apply(math::DynMatrix<bool> &initialMatrix, std::vector<SurfaceFeatureExtractor::SurfaceFeature> features,
                          const core::Img32f &depthImage, std::vector<std::vector<int> > &surfaces, float maxAngle=30,
                          float distanceTolerance=3, float outlierTolerance=5, int triangles=50, int scanlines=9);
        
      private:
      
        static float getAngle(Vec n1, Vec n2);
        static utils::Point getRandomPoint(std::vector<int> surface, int imgWidth);
        static Vec getNormal(Vec p1, Vec p2, Vec p3);
        static bool criterion1(Vec n1, Vec n2, float maxAngle);
        static bool criterion2(const core::Img32f &depthImage, std::vector<int> &surface1, std::vector<int> &surface2, 
                              Vec n1, Vec n2, float maxAngle, int triangles);
        static bool criterion3(const core::Img32f &depthImage, std::vector<int> &surface1, std::vector<int> &surface2, 
                              float distanceTolerance, float outlierTolerance, int scanlines);
    };
     
  } // namespace geom
}
