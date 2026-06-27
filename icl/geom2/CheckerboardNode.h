// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/MeshNode.h>
#include <memory>
#include <vector>

namespace icl::geom2 {

  /// A flat checkerboard calibration target as a scene node.
  /// A single textured quad in its local z=0 plane (normal +Z). The texture is
  /// exactly ONE TEXEL PER CELL, sampled nearest-neighbour, so the squares stay
  /// crisp at any scale/distance (the renderer's own anti-aliasing smooths the
  /// edges). \a cols × \a rows count the CHECKER SQUARES; a 1-cell white
  /// quiet-zone border makes the texture (cols+2)×(rows+2) texels. The
  /// detectable inner saddle corners are (cols-1)×(rows-1) — see innerCorners().
  ///
  /// The board height follows the width so the cells stay square. The material
  /// is matte (roughness 1) so a light doesn't blow a specular hot-spot across
  /// the corners. Works in both the GL and Cycles backends (both honour the
  /// material's nearest-neighbour TexFilter).
  class ICLGeom2_API CheckerboardNode : public MeshNode {
  public:
    /// \a cols × \a rows checker squares on a board \a widthMM wide (mm; the
    /// height is derived so cells are square).
    static std::shared_ptr<CheckerboardNode> create(int cols = 7, int rows = 5,
                                                     float widthMM = 280.f);

    void setCells(int cols, int rows);   ///< rebuild for a new square count
    void setWidth(float widthMM);        ///< rebuild for a new physical width

    int   getCols()   const { return m_cols; }
    int   getRows()   const { return m_rows; }
    float getWidth()  const { return m_width; }
    float getHeight() const { return m_height; }

    /// The (cols-1)×(rows-1) inner saddle corners in the node's LOCAL frame
    /// (z=0), row-major (top row first). This is the calibration ground truth —
    /// transform by the node's pose for world coordinates.
    std::vector<Vec> innerCorners() const;

    CheckerboardNode() = default;        ///< prefer create()

  private:
    void rebuild();
    int   m_cols = 7, m_rows = 5;
    float m_width = 280.f, m_height = 200.f;
  };

} // namespace icl::geom2
