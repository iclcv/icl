/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RemainingPointsFeatureExtractor.h  **
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
#include <ICLGeom/GeomDefs.h>
#include <ICLCore/DataSegment.h>

namespace icl{
  namespace geom{

    /// class for remaining points feature.
    /** This class implements the remaining points feature for feature-graph based segmentation.*/

    class RemainingPointsFeatureExtractor{
      	
  	  public:
  	      
        /// Calculates and assigns the remaining points segments (including local minima separation).
      /** @param xyz the input xyz pointcloud data segment
          @param depthImage the input depth image
          @param labelImage the label image (modified by the method)
          @param maskImage the mask image (modified by the method)
          @param surfaces the surfaces (modified by the method)
          @param segments the segments (modified by the method)
          @param minSize the minimum size of remaining points segments
          @param euclideanDistance the euclidean distance for clustering
          @param radius the radius for local minima separation
          @param assignEuclideanDistance the euclidean distance for assigning to neighbouring surfaces
          @param supportTolerance the max tolerance for detecting a blob as supported and therefore separate object */
        static void apply(core::DataSegment<float,4> &xyz, const core::Img32f &depthImage, core::Img32s &labelImage, core::Img8u &maskImage, 
                          std::vector<std::vector<int> > &surfaces, std::vector<std::vector<int> > &segments, int minSize=10, float euclideanDistance=5., int radius=6, float assignEuclideanDistance=5., int supportTolerance=9);
        
        /// Calculates and assigns the remaining points segments (excluding local minima separation).
      /** @param xyz the input xyz pointcloud data segment
          @param labelImage the label image (modified by the method)
          @param maskImage the mask image (modified by the method)
          @param surfaces the surfaces (modified by the method)
          @param segments the segments (modified by the method)
          @param minSize the minimum size of remaining points segments
          @param euclideanDistance the euclidean distance for clustering
          @param assignEuclideanDistance the euclidean distance for assigning to neighbouring surfaces
          @param supportTolerance the max tolerance for detecting a blob as supported and therefore separate object */                  
        static void apply(core::DataSegment<float,4> &xyz, core::Img32s &labelImage, core::Img8u &maskImage, 
                          std::vector<std::vector<int> > &surfaces, std::vector<std::vector<int> > &segments, int minSize=10, float euclideanDistance=5., float assignEuclideanDistance=5., int supportTolerance=9);
                          
        /// Calculates the remaining points segments and creates a connectivity matrix (including local minima separation).
      /** @param xyz the input xyz pointcloud data segment
          @param depthImage the input depth image
          @param labelImage the label image (modified by the method)
          @param maskImage the mask image (modified by the method)
          @param surfaces the surfaces (modified by the method)
          @param minSize the minimum size of remaining points segments
          @param euclideanDistance the euclidean distance for clustering
          @param radius the radius for local minima separation
          @param assignEuclideanDistance the euclidean distance for assigning to neighbouring surfaces 
          @return the connectivity matrix*/                        
        static math::DynMatrix<bool> apply(core::DataSegment<float,4> &xyz, const core::Img32f &depthImage, core::Img32s &labelImage, core::Img8u &maskImage, 
                          std::vector<std::vector<int> > &surfaces, int minSize=10, float euclideanDistance=5., int radius=6, float assignEuclideanDistance=5.);
                          
        /// Calculates the remaining points segments and creates a connectivity matrix (excluding local minima separation).
      /** @param xyz the input xyz pointcloud data segment
          @param labelImage the label image (modified by the method)
          @param maskImage the mask image (modified by the method)
          @param surfaces the surfaces (modified by the method)
          @param minSize the minimum size of remaining points segments
          @param euclideanDistance the euclidean distance for clustering
          @param assignEuclideanDistance the euclidean distance for assigning to neighbouring surfaces
          @return the connectivity matrix */                        
        static math::DynMatrix<bool> apply(core::DataSegment<float,4> &xyz, core::Img32s &labelImage, core::Img8u &maskImage, 
                          std::vector<std::vector<int> > &surfaces, int minSize=10, float euclideanDistance=5., float assignEuclideanDistance=5.);
        
      private:
      
        static void calculateLocalMinima(const core::Img32f &depthImage, core::Img8u &maskImage, int radius);
        
        static void clusterRemainingPoints(core::DataSegment<float,4> &xyz, std::vector<std::vector<int> > &surfaces, core::Img32s &labelImage, core::Img8u &maskImage, 
                                           int minSize, float euclideanDistance, int numCluster); 
        
        static void detectNeighbours(core::DataSegment<float,4> &xyz, std::vector<std::vector<int> > &surfaces, core::Img32s &labelImage, std::vector<std::vector<int> > &neighbours, 
                                     std::vector<std::vector<int> > &neighboursPoints, int numCluster, float assignEuclideanDistance); 
        
        static bool checkNotExist(int zw, std::vector<int> &nb, std::vector<int> &nbPoints);
        
        static void ruleBasedAssignment(core::DataSegment<float,4> &xyz, core::Img32s &labelImage, std::vector<std::vector<int> > &surfaces, std::vector<std::vector<int> > &segments, 
                                        std::vector<std::vector<int> > &neighbours, std::vector<std::vector<int> > &neighboursPoints, int numCluster, int supportTolerance);
                                        
        static std::vector<int> segmentMapping(std::vector<std::vector<int> > &segments, int numSurfaces);
        
        static int ransacAssignment(core::DataSegment<float,4> &xyz, std::vector<std::vector<int> > &surfaces, std::vector<int> &nb, int x);
        
        static bool checkSupport(core::Img32s &labelImage, std::vector<int> &surface, int neighbourID, int supportTolerance); 
    };
     
  } // namespace geom
}
