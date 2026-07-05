// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/GroupNode.h>
#include <icl/markers/AdvancedMarkerGridDetector.h>
namespace icl::markers {
    /// local utility class (viz3d port of the old SceneObject-based indicator)
	  class GridIndicatorObject : public viz3d::GroupNode{
      struct MarkerObj;

      public:
      GridIndicatorObject(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def);
      GridIndicatorObject(const utils::Size &checkerBoardCells,
                          const utils::Size32f &bounds);
    };

  }
