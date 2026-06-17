// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Phase 0 of the physics2 plan: the geom2 Driver mechanism. These tests are
// GL-free — they exercise attach/detach, typed lookup, and the Scene2::sync()
// pre-order traversal, all verifiable headless in the sandbox.

#include "harness/Test.h"
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Driver.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/CuboidNode.h>

using namespace icl::geom2;

namespace {
  /// Counts sync() calls and records the last (dt, alpha) seen.
  struct CountingDriver : public Driver {
    int syncs = 0, attaches = 0, detaches = 0;
    double lastDt = -1, lastAlpha = -1;
    void onAttach() override { attaches++; }
    void onDetach() override { detaches++; }
    void sync(double dt, double alpha) override {
      syncs++; lastDt = dt; lastAlpha = alpha;
    }
  };

  /// Translates its node along +x by rate*dt each sync (time-driven).
  struct ShiftDriver : public Driver {
    float rate;
    explicit ShiftDriver(float r) : rate(r) {}
    void sync(double dt, double) override {
      if (auto *n = node()) n->translate(rate * (float)dt, 0, 0);
    }
  };
}

ICL_REGISTER_TEST("geom2.driver.attach_sets_node", "addDriver wires node() + calls onAttach")
{
  auto cube = CuboidNode::createCube(0, 0, 0, 10);
  auto d = cube->addDriver<CountingDriver>();
  ICL_TEST_TRUE(d->node() == cube.get());
  ICL_TEST_EQ(d->attaches, 1);
  ICL_TEST_EQ(d->detaches, 0);
}

ICL_REGISTER_TEST("geom2.driver.typed_lookup", "getDriver<T> finds the attached driver")
{
  auto cube = CuboidNode::createCube(0, 0, 0, 10);
  ICL_TEST_TRUE(cube->getDriver<CountingDriver>() == nullptr);
  auto d = cube->addDriver<CountingDriver>();
  ICL_TEST_TRUE(cube->getDriver<CountingDriver>() == d.get());
  ICL_TEST_TRUE(cube->getDriver<ShiftDriver>() == nullptr);
  ICL_TEST_EQ((int)cube->getDrivers().size(), 1);
}

ICL_REGISTER_TEST("geom2.driver.remove_detaches", "removeDriver clears node() + calls onDetach")
{
  auto cube = CuboidNode::createCube(0, 0, 0, 10);
  auto d = cube->addDriver<CountingDriver>();
  CountingDriver *raw = d.get();
  cube->removeDriver(raw);
  ICL_TEST_EQ(raw->detaches, 1);
  ICL_TEST_TRUE(raw->node() == nullptr);
  ICL_TEST_EQ((int)cube->getDrivers().size(), 0);
}

ICL_REGISTER_TEST("geom2.driver.scene_sync_forwards", "Scene2::sync forwards dt/alpha to drivers")
{
  Scene2 scene;
  auto cube = CuboidNode::createCube(0, 0, 0, 10);
  auto d = cube->addDriver<CountingDriver>();
  scene.addNode(std::static_pointer_cast<Node>(cube));

  scene.sync(0.25, 0.5);
  ICL_TEST_EQ(d->syncs, 1);
  ICL_TEST_NEAR(d->lastDt, 0.25, 1e-9);
  ICL_TEST_NEAR(d->lastAlpha, 0.5, 1e-9);

  scene.sync(0.1);                 // alpha defaults to 1.0
  ICL_TEST_EQ(d->syncs, 2);
  ICL_TEST_NEAR(d->lastAlpha, 1.0, 1e-9);
}

ICL_REGISTER_TEST("geom2.driver.drives_transform", "a driver mutating the node transform takes effect")
{
  Scene2 scene;
  auto cube = CuboidNode::createCube(0, 0, 0, 10);
  cube->addDriver<ShiftDriver>(100.0f);   // +100 units/sec along x
  scene.addNode(std::static_pointer_cast<Node>(cube));

  scene.sync(0.1);                          // expect +10 along x
  Mat t = cube->getTransformation();
  ICL_TEST_NEAR(t(0, 3), 10.0f, 1e-3);      // translation x is element (row 0, col 3)

  scene.sync(0.1);                          // accumulates to +20
  t = cube->getTransformation();
  ICL_TEST_NEAR(t(0, 3), 20.0f, 1e-3);
}

ICL_REGISTER_TEST("geom2.driver.preorder_children", "sync recurses into GroupNode children")
{
  Scene2 scene;
  auto group = std::make_shared<GroupNode>();
  auto child = CuboidNode::createCube(0, 0, 0, 10);
  auto d = child->addDriver<CountingDriver>();
  group->addChild(std::static_pointer_cast<Node>(child));
  scene.addNode(std::static_pointer_cast<Node>(group));

  scene.sync(0.05);
  ICL_TEST_EQ(d->syncs, 1);                 // child driver reached via traversal
}

ICL_REGISTER_TEST("geom2.driver.not_copied", "deep-copied node starts driverless")
{
  auto cube = CuboidNode::createCube(0, 0, 0, 10);
  cube->addDriver<CountingDriver>();
  std::unique_ptr<Node> copy(cube->deepCopy());
  ICL_TEST_EQ((int)copy->getDrivers().size(), 0);
}
