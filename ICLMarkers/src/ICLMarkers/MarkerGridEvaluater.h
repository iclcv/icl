/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerGridEvaluater.h        **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
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

#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLMath/FixedVector.h>

namespace icl{
  namespace markers{
	  struct ICLMarkers_API MarkerGridEvaluater{
      typedef AdvancedMarkerGridDetector::MarkerGrid MarkerGrid;
      typedef AdvancedMarkerGridDetector::Marker Marker;

      const MarkerGrid *grid;
      struct Line{
        utils::Point32f a,b;
        bool isNull;
        int type;
        operator bool() const { return !isNull; }
      Line():isNull(true){}
      Line(const utils::Point32f &a, const utils::Point32f &b):a(a),b(b),isNull(false){}
        Line(const std::vector<utils::Point32f> &ps, float *error=0);

        struct PCAInfo{
          typedef math::FixedColVector<float,2> V;
          typedef math::FixedMatrix<float,2,2> M;
          V c;
          M evecs;
          V evals;
          bool isNull;
          inline float getError() const { return fabs(evals[1]); }
          operator bool() const { return !isNull; }
        };
        static PCAInfo perform_pca(const std::vector<utils::Point32f> &p);

      };
      float error;
      std::vector<Line> lines;

      MarkerGridEvaluater(const MarkerGrid *grid=0):grid(grid){}

      void setGrid(const MarkerGrid *grid) { this->grid = grid; }

      float evalError(bool storeLines=true);

      utils::VisualizationDescription vis() const;

      static float compute_error(const MarkerGrid &g);

    private:
      template<bool STORE_LINES>
      void updateLines();
    };
  }

}
