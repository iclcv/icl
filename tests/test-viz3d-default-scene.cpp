// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// DefaultScene structural tests. GL-free: construction, preset switching, and
// the furniture knobs only touch the scene graph + Configurable state, so they
// are verifiable headless in the sandbox (no GL context). Visual correctness
// (the sky pass, faded ground edges, lamp rig) needs a real display.

#include "harness/Test.h"
#include <icl/viz3d/scene/DefaultScene.h>
#include <icl/viz3d/nodes/GroupNode.h>
#include <icl/viz3d/nodes/MeshNode.h>

using namespace icl::viz3d;
using SceneType = DefaultScene::SceneType;

// Studio furnishes a large ground + coordinate frame + a 4-light rig (sky is
// the backdrop, no wall); every light is also a node, so
// nodeCount = ground(1) + frame(1) + 4 lights.
ICL_REGISTER_TEST("viz3d.defaultscene.studio_furniture", "Studio builds a ground, frame and a 4-light rig")
{
  DefaultScene s(SceneType::Studio);
  ICL_TEST_EQ((int)s.getSceneType() == (int)SceneType::Studio, true);
  ICL_TEST_EQ(s.getLightCount(), 4);
  ICL_TEST_EQ(s.getNodeCount(), 6);
  ICL_TEST_EQ(s.getCameraCount(), 1);
}

// Void is a camera + key/fill + coordinate frame — no ground.
ICL_REGISTER_TEST("viz3d.defaultscene.void_minimal", "Void has lights + frame + camera but no ground")
{
  DefaultScene s(SceneType::Void);
  ICL_TEST_EQ(s.getLightCount(), 2);
  ICL_TEST_EQ(s.getNodeCount(), 3);   // frame + 2 lights
  ICL_TEST_EQ(s.getCameraCount(), 1);
}

// Switching presets must fully replace furniture — and crucially must NOT leak
// lights (the Scene2::removeNode light-purge fix). Round-tripping back to Studio
// has to land on exactly 4 lights again, not accumulate.
ICL_REGISTER_TEST("viz3d.defaultscene.preset_switch_no_leak", "preset switch rebuilds without leaking lights/cameras")
{
  DefaultScene s(SceneType::Studio);
  ICL_TEST_EQ(s.getLightCount(), 4);

  s.setSceneType(SceneType::Void);
  ICL_TEST_EQ(s.getLightCount(), 2);
  ICL_TEST_EQ(s.getNodeCount(), 3);

  s.setSceneType(SceneType::Studio);
  ICL_TEST_EQ(s.getLightCount(), 4);   // not 6 — old lights were purged
  ICL_TEST_EQ(s.getNodeCount(), 6);
  ICL_TEST_EQ(s.getCameraCount(), 1);  // camera updated in place, never duplicated
}

// User content survives a furniture rebuild (furniture is tracked separately).
ICL_REGISTER_TEST("viz3d.defaultscene.user_content_survives", "rebuild keeps user-added content")
{
  DefaultScene s(SceneType::Studio);
  const int before = s.getNodeCount();
  s.addNode(std::make_shared<GroupNode>());   // stand-in user content
  ICL_TEST_EQ(s.getNodeCount(), before + 1);
  s.setSceneType(SceneType::Void);            // rebuild furniture
  // 3 furniture nodes (Void: frame + 2 lights) + the 1 user node still present
  ICL_TEST_EQ(s.getNodeCount(), 4);
}

// The "ground" knob toggles visibility, it does NOT rebuild — so the node graph
// (and the camera) are untouched. The ground node stays, just hidden.
ICL_REGISTER_TEST("viz3d.defaultscene.ground_toggle", "ground knob toggles visibility, not the scene graph")
{
  DefaultScene s(SceneType::Studio);
  auto groundVisible = [&]() -> int {
    for (int i = 0; i < s.getNodeCount(); i++)
      if (auto *m = dynamic_cast<MeshNode*>(s.getNode(i))) return m->isVisible() ? 1 : 0;
    return -1;   // no ground found
  };
  ICL_TEST_EQ(s.getNodeCount(), 6);
  ICL_TEST_EQ(groundVisible(), 1);

  s.setPropertyValue("ground", false);
  ICL_TEST_EQ(s.getNodeCount(), 6);   // not removed — still present, just hidden
  ICL_TEST_EQ(groundVisible(), 0);
  ICL_TEST_EQ(s.getLightCount(), 4);
  ICL_TEST_EQ(s.getCameraCount(), 1);

  s.setPropertyValue("ground", true);
  ICL_TEST_EQ(groundVisible(), 1);
}

// The coordinate frame is built (default hidden) and toggled by its knob —
// again a visibility flip, not a rebuild.
ICL_REGISTER_TEST("viz3d.defaultscene.frame_toggle", "coordinate frame defaults hidden, knob toggles visibility")
{
  DefaultScene s(SceneType::Studio);
  // The frame is the only top-level GroupNode in a Studio with no user content.
  auto frameVisible = [&]() -> int {
    for (int i = 0; i < s.getNodeCount(); i++)
      if (auto *f = dynamic_cast<GroupNode*>(s.getNode(i))) return f->isVisible() ? 1 : 0;
    return -1;
  };
  ICL_TEST_EQ(frameVisible(), 0);          // default off
  s.setPropertyValue("coordinate frame", true);
  ICL_TEST_EQ(frameVisible(), 1);
  ICL_TEST_EQ(s.getNodeCount(), 6);        // toggled, not added
  s.setPropertyValue("coordinate frame", false);
  ICL_TEST_EQ(frameVisible(), 0);
}

// up-axis flip rebuilds without changing the counts.
ICL_REGISTER_TEST("viz3d.defaultscene.up_axis_rebuild", "up-axis flip rebuilds cleanly")
{
  DefaultScene s(SceneType::Studio);
  s.setUpAxis('Z');
  ICL_TEST_EQ(s.getLightCount(), 4);
  ICL_TEST_EQ(s.getNodeCount(), 6);
  ICL_TEST_EQ(s.getCameraCount(), 1);
}
