// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// Widget-free construction of PlotWidget3D's coordinate frame (the [-1,1]^3
// wireframe box + the three tick/label axes). Extracted out of PlotWidget3D so
// the exact shipping frame can be rendered into a bare Scene2 with no QWidget —
// used both by PlotWidget3D and by the headless GL-capture verification tool.
// NOT installed (detail/): only .cpp translation units may include this.

#include <icl/utils/Range.h>
#include <memory>
#include <string>

namespace icl::geom2 {

  class GroupNode;
  class MeshNode;

  namespace detail {

    /// Wireframe of the [-1,1]^3 coordinate cube (12 edges, vertices hidden).
    std::shared_ptr<MeshNode> makePlotBox();

    /// One axis living in the local X range [-1,1]: N tick marks (each with a
    /// numeric label) plus the axis name at the far end. \a invertLabels reverses
    /// the numeric VALUES (mx->mn) while keeping the tick positions, so a
    /// back-facing axis still reads left-to-right. The caller rotates/translates
    /// the returned group onto the correct box edge via placePlotAxes().
    std::shared_ptr<GroupNode> makePlotAxis(const utils::Range32f &range,
                                            bool invertLabels,
                                            const std::string &name);

    /// Rotate + translate the three freshly-built axes (X=0,Y=1,Z=2) onto the
    /// box edges meeting at the corner (sx,sy,sz), each sign in {-1,+1}. Picking
    /// the corner FURTHEST from the camera keeps the ticks framing the data from
    /// behind. Each axis's local +x maps to its own world + direction (values
    /// read right-handed) and its tick arms point into the two adjacent faces
    /// (they "embrace" the cube) while the numeric labels stay outside.
    /// The nodes must have identity transforms on entry (call removeTransformation
    /// first when re-placing). Default (-1,-1,-1) = the classic back-bottom-left.
    void placePlotAxes(const std::shared_ptr<GroupNode> (&axes)[3],
                       int sx = -1, int sy = -1, int sz = -1);

  } // namespace detail
} // namespace icl::geom2
