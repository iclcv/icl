// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom/SceneObject.h>
#include <icl/markers/AdvancedMarkerGridDetector.h>
namespace icl::markers {
    /// local utility class
	  class GridIndicatorObject : public geom::SceneObject{
      struct MarkerObj;

      public:
      GridIndicatorObject(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def);
      GridIndicatorObject(const utils::Size &checkerBoardCells,
                          const utils::Size32f &bounds);
    };

  }