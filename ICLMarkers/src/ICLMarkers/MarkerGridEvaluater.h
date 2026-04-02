// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLMath/FixedVector.h>

namespace icl::markers {
	  struct ICLMarkers_API MarkerGridEvaluater{
      using MarkerGrid = AdvancedMarkerGridDetector::MarkerGrid;
      using Marker = AdvancedMarkerGridDetector::Marker;

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
          using V = math::FixedColVector<float,2>;
          using M = math::FixedMatrix<float,2,2>;
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