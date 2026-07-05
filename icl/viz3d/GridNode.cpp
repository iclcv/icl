// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/GridNode.h>
#include <icl/utils/Macros.h>
#include <icl/utils/Exception.h>

namespace icl::viz3d {

  GridNode::GridNode(int nx, int ny, const std::vector<Vec> &points, bool lines, bool quads)
    : m_nx(nx), m_ny(ny) {
    ICLASSERT_THROW(static_cast<int>(points.size()) == nx * ny,
                    utils::ICLException("GridNode: nx*ny differs from points.size()"));
    for (int i = 0; i < nx * ny; ++i) {
      Vec p = points[i];
      p[3] = 1;
      addVertex(p);
    }
    // interior grid lines + quad cells (mirrors the old GridSceneObject winding)
    for (int x = 1; x < nx; ++x) {
      for (int y = 1; y < ny; ++y) {
        int a = getIdx(x, y), b = getIdx(x - 1, y), c = getIdx(x, y - 1), d = getIdx(x - 1, y - 1);
        if (lines) { addLine(a, b); addLine(a, c); }
        if (quads) addQuad(a, b, d, c);
      }
    }
    // border lines along the first row / column
    for (int x = 1; x < nx; ++x) if (lines) addLine(getIdx(x, 0), getIdx(x - 1, 0));
    for (int y = 1; y < ny; ++y) if (lines) addLine(getIdx(0, y), getIdx(0, y - 1));
  }

  static std::vector<Vec> regularGrid(int nx, int ny, const Vec &origin, const Vec &dx, const Vec &dy) {
    std::vector<Vec> pts;
    pts.reserve(nx * ny);
    for (int y = 0; y < ny; ++y)
      for (int x = 0; x < nx; ++x)
        pts.push_back(origin + dx * float(x) + dy * float(y));
    return pts;
  }

  GridNode::GridNode(int nx, int ny, const Vec &origin, const Vec &dx, const Vec &dy, bool lines, bool quads)
    : GridNode(nx, ny, regularGrid(nx, ny, origin, dx, dy), lines, quads) {}

  NodePtr GridNode::deepCopy() const { return std::make_shared<GridNode>(*this); }

} // namespace icl::viz3d
