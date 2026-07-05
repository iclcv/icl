// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// nodeFromPrimitive3D — the viz3d replacement for Primitive3D::toSceneObject:
// CUBE→CuboidNode, SPHERE→SphereNode, CYLINDER→CylinderNode, with the
// primitive pose as the node transform. Headless.

#include "harness/Test.h"
#include <icl/viz3d/Primitive3DConverter.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CylinderNode.h>
#include <icl/cv3d/Primitive3D.h>
#include <cmath>

using namespace icl;
using namespace icl::viz3d;
using P = icl::cv3d::Primitive3D;
using icl::cv3d::Vec;
using Vec3 = math::FixedColVector<float,3>;

static P make(P::PrimitiveType t, const Vec &pos, const Vec &scale) {
  return P(t, pos, P::Quaternion(Vec3(0,0,0), 1.f), scale, 0, "test");
}

// CUBE → CuboidNode with matching extents; the node transform carries the pose.
ICL_REGISTER_TEST("viz3d.primitive3d.cube", "nodeFromPrimitive3D(CUBE) → CuboidNode + pose")
{
  auto n = nodeFromPrimitive3D(make(P::CUBE, Vec(100,200,300,1), Vec(40,60,80,1)));
  auto *cube = dynamic_cast<CuboidNode*>(n.get());
  ICL_TEST_EQ(cube != nullptr, true);
  const auto e = cube->getExtents();
  ICL_TEST_EQ(std::fabs(e[0]-40)<1e-3 && std::fabs(e[1]-60)<1e-3 && std::fabs(e[2]-80)<1e-3, true);
  const Mat T = n->getTransformation();
  ICL_TEST_EQ(std::fabs(T(0,3)-100)<1e-3 && std::fabs(T(1,3)-200)<1e-3 && std::fabs(T(2,3)-300)<1e-3, true);
}

// SPHERE → SphereNode whose radii are half the scale.
ICL_REGISTER_TEST("viz3d.primitive3d.sphere", "nodeFromPrimitive3D(SPHERE) → SphereNode radii=scale/2")
{
  auto n = nodeFromPrimitive3D(make(P::SPHERE, Vec(0,0,0,1), Vec(100,100,100,1)));
  auto *s = dynamic_cast<SphereNode*>(n.get());
  ICL_TEST_EQ(s != nullptr, true);
  ICL_TEST_EQ(std::fabs(s->getRadius()-50)<1e-3, true);
}

// CYLINDER → CylinderNode (geometry generated).
ICL_REGISTER_TEST("viz3d.primitive3d.cylinder", "nodeFromPrimitive3D(CYLINDER) → CylinderNode")
{
  auto n = nodeFromPrimitive3D(make(P::CYLINDER, Vec(0,0,0,1), Vec(50,50,120,1)));
  auto *c = dynamic_cast<CylinderNode*>(n.get());
  ICL_TEST_EQ(c != nullptr, true);
  ICL_TEST_EQ(c->getVertices().size() > 0, true);
}
