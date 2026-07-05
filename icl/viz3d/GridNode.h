// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/MeshNode.h>
#include <icl/utils/Size.h>

namespace icl::viz3d {

  /// 2D lattice of nodes in 3D space, drawn as grid lines and/or quad cells.
  /** viz3d port of the old geom::GridSceneObject. A MeshNode whose vertices form
      an nx*ny row-major lattice. getNode(x,y) gives mutable access, so the lattice
      can be deformed after construction (e.g. to visualise a calibration grid).
      Being a GeometryNode, it renders through the normal Renderer path. */
  class ICLViz3d_API GridNode : public MeshNode {
    int m_nx = 0, m_ny = 0;

  public:
    GridNode() = default;

    /// Build from explicit row-major points (must hold exactly nx*ny elements).
    GridNode(int nx, int ny, const std::vector<Vec> &points,
             bool lines = true, bool quads = true);

    /// Build a regular lattice: node(x,y) = origin + x*dx + y*dy.
    GridNode(int nx, int ny, const Vec &origin, const Vec &dx, const Vec &dy,
             bool lines = true, bool quads = true);

    NodePtr deepCopy() const override;

    /// row-major flat vertex index of grid node (x,y)
    int getIdx(int x, int y) const { return x + m_nx * y; }

    /// mutable access to grid node (x,y)
    Vec &getNode(int x, int y) { return getVertices()[getIdx(x, y)]; }

    /// const access to grid node (x,y)
    const Vec &getNode(int x, int y) const {
      return const_cast<GridNode *>(this)->getVertices()[getIdx(x, y)];
    }

    /// lattice dimensions (nx, ny) in nodes
    utils::Size getSize() const { return utils::Size(m_nx, m_ny); }
    int getWidth() const { return m_nx; }
    int getHeight() const { return m_ny; }

    static std::shared_ptr<GridNode> create(int nx, int ny, const std::vector<Vec> &points,
                                            bool lines = true, bool quads = true) {
      return std::make_shared<GridNode>(nx, ny, points, lines, quads);
    }
    static std::shared_ptr<GridNode> create(int nx, int ny, const Vec &origin,
                                            const Vec &dx, const Vec &dy,
                                            bool lines = true, bool quads = true) {
      return std::make_shared<GridNode>(nx, ny, origin, dx, dy, lines, quads);
    }
  };

} // namespace icl::viz3d
