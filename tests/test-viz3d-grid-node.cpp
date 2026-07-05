// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// GridNode — viz3d port of geom::GridSceneObject: an nx*ny lattice rendered as
// grid lines / quad cells, with mutable getNode(x,y) access. Headless data checks.

#include "harness/Test.h"
#include <icl/viz3d/GridNode.h>
#include <cmath>

using namespace icl;
using namespace icl::viz3d;  // brings viz3d::Vec (FixedColVector<float,4>)

// Regular lattice: dimensions, indexing, and node positions.
ICL_REGISTER_TEST("viz3d.gridnode.regular", "GridNode regular lattice: size + node positions")
{
  const Vec origin(10, 20, 30, 1), dx(5, 0, 0, 0), dy(0, 7, 0, 0);
  GridNode g(3, 4, origin, dx, dy);

  ICL_TEST_EQ(g.getWidth(), 3);
  ICL_TEST_EQ(g.getHeight(), 4);
  ICL_TEST_EQ(g.getSize() == utils::Size(3, 4), true);
  ICL_TEST_EQ((int)g.getVertices().size(), 12);
  ICL_TEST_EQ(g.getIdx(2, 3), 2 + 3 * 3);

  // node(x,y) = origin + x*dx + y*dy
  const Vec &n = g.getNode(2, 3);
  ICL_TEST_EQ(std::fabs(n[0] - (10 + 2 * 5)) < 1e-4, true);
  ICL_TEST_EQ(std::fabs(n[1] - (20 + 3 * 7)) < 1e-4, true);
  ICL_TEST_EQ(std::fabs(n[2] - 30) < 1e-4, true);
  ICL_TEST_EQ(std::fabs(n[3] - 1) < 1e-4, true);
}

// Mutable getNode lets the lattice be deformed after construction.
ICL_REGISTER_TEST("viz3d.gridnode.mutable", "GridNode getNode is mutable")
{
  GridNode g(2, 2, Vec(0, 0, 0, 1), Vec(1, 0, 0, 0), Vec(0, 1, 0, 0));
  g.getNode(1, 1) = Vec(9, 9, 9, 1);
  const Vec &n = g.getNode(1, 1);
  ICL_TEST_EQ(std::fabs(n[0] - 9) < 1e-4 && std::fabs(n[1] - 9) < 1e-4, true);
}

// Explicit row-major points constructor + wrong-size guard.
ICL_REGISTER_TEST("viz3d.gridnode.points", "GridNode from explicit points + size guard")
{
  std::vector<Vec> pts = { Vec(0,0,0,1), Vec(1,0,0,1), Vec(0,1,0,1), Vec(1,1,0,1) };
  GridNode g(2, 2, pts);
  ICL_TEST_EQ((int)g.getVertices().size(), 4);
  ICL_TEST_EQ(std::fabs(g.getNode(1, 0)[0] - 1) < 1e-4, true);

  bool threw = false;
  try { GridNode bad(3, 3, pts); } catch (...) { threw = true; }
  ICL_TEST_EQ(threw, true);
}

// deepCopy is independent (mutating the copy does not touch the original).
ICL_REGISTER_TEST("viz3d.gridnode.deepcopy", "GridNode deepCopy is independent")
{
  GridNode g(2, 2, Vec(0, 0, 0, 1), Vec(1, 0, 0, 0), Vec(0, 1, 0, 0));
  auto copy = g.deepCopy();
  auto *gc = dynamic_cast<GridNode *>(copy.get());
  ICL_TEST_EQ(gc != nullptr, true);
  ICL_TEST_EQ(gc->getWidth() == 2 && gc->getHeight() == 2, true);
  gc->getNode(0, 0) = Vec(5, 5, 5, 1);
  ICL_TEST_EQ(std::fabs(g.getNode(0, 0)[0]) < 1e-4, true);  // original unchanged
}
