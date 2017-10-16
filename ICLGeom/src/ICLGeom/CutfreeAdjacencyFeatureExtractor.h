/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/CutfreeAdjacencyFeatureExtractor.h **
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
#include <ICLCore/Img.h>
#include <ICLGeom/SurfaceFeatureExtractor.h>

namespace icl{
  namespace geom{
    /**
       This class implements the cutfree adjacency feature for feature-graph based segmentation.*/
    class CutfreeAdjacencyFeatureExtractor{

     public:

      enum Mode {BEST, GPU, CPU};

      /// Constructor
      /** Constructs an object of this class.
          @param mode GPU, CPU and BEST (default) */
      CutfreeAdjacencyFeatureExtractor(Mode mode=BEST);

      /// Destructor
      ~CutfreeAdjacencyFeatureExtractor();

      /// Calculates the cutfree adjacency feature matrix.
      /** @param xyzh the xyzh DataSegment from the PointCloudObject class
          @param surfaces the vector of surface id vectors
          @param testMatrix the initial boolean test matrix (1 test, 0 dont test, preferably an adjacency matrix)
          @param euclideanDistance the maximum euclidean distance for RANSAC in mm
          @param passes the RANSAC passes
          @param tolerance the RANSAC tolerance in number of points (outlier)
          @param labelImage the label image
          @return the boolean cutfree adjacency matrix */
       math::DynMatrix<bool> apply(core::DataSegment<float,4> &xyzh,
                std::vector<std::vector<int> > &surfaces, math::DynMatrix<bool> &testMatrix, float euclideanDistance,
                int passes, int tolerance, core::Img32s labelImage);

       /// Calculates the cutfree adjacency feature matrix with minimum angle constraint.
      /** @param xyzh the xyzh DataSegment from the PointCloudObject class
          @param surfaces the vector of surface id vectors
          @param testMatrix the initial boolean test matrix (1 test, 0 dont test, preferably an adjacency matrix)
          @param euclideanDistance the maximum euclidean distance for RANSAC in mm
          @param passes the RANSAC passes
          @param tolerance the RANSAC tolerance in number of points (outlier)
          @param labelImage the label image
          @param feature the surface feature for the surfaces
          @param minAngle the minimum angle for combination
          @return the boolean cutfree adjacency matrix */
       math::DynMatrix<bool> apply(core::DataSegment<float,4> &xyzh,
                std::vector<std::vector<int> > &surfaces, math::DynMatrix<bool> &testMatrix, float euclideanDistance,
                int passes, int tolerance, core::Img32s labelImage,
                std::vector<SurfaceFeatureExtractor::SurfaceFeature> feature, float minAngle);

     private:

      struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer

    };
  } // namespace geom
}
