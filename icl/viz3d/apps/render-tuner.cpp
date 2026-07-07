// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Live Filament-vs-Cycles tuning app. The SAME calibration / build-up scene is
// rendered by BOTH backends, side by side:
//   left  — Filament (real-time; updates immediately as you drag the render.* knobs)
//   right — Cycles   (path-traced reference; refines progressively)
// Pick the step with -step, then dial the Prop panel until the two views match.
// Orbit either view with the mouse (Cycles re-converges on camera moves).
//
// The scene for each step comes from the shared builder in demos/calibrate_scenes.h,
// so this tuner and the numeric harness (viz3d-render-calibrate) show identical
// scenes. Add a step there and both tools get it.
//
// Usage: viz3d-render-tuner [-step simple] [-size 640x480] [-samples 256]

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/render/CyclesRenderer.h>
#include <icl/viz3d/render/RenderBackend.h>
#include <icl/viz3d/scene/SceneMouseHandler.h>
#include "../demos/calibrate_scenes.h"

using namespace icl::utils;
using namespace icl::qt;
using namespace icl::viz3d;

static Scene scene;
static std::unique_ptr<CyclesRenderer> renderer;
static std::shared_ptr<GeometryNode> groundNode;   // tunable-roughness ground (may be null)
static float lastRoughness = -1.0f;
GUI gui;

// Orbit the shared camera; a moved camera invalidates the Cycles transforms so it
// restarts converging from the new viewpoint (the Filament view follows for free).
static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (renderer && (evt.isDragEvent() || evt.isWheelEvent()))
    renderer->invalidateTransforms();
}

void init() {
  const std::string step = pa("-step");
  const Size sz = pa("-size");

  calib::SceneOpts o = calib::buildScene(scene, step, sz);
  if (!o.ok)
    throw ICLException("unknown -step '" + step +
                       "' (try: emissive diffuse sky metal simple simple-glossy)");
  groundNode = o.ground;
  const float initRough = groundNode ? groundNode->getMaterial()->roughness : 0.5f;

  // The ground roughness is a per-material property, not a render.* knob, so it gets
  // its own slider (below the auto-generated Prop panel). Both backends pick up the
  // change: Filament re-reads material params every frame; Cycles via invalidateNode.
  gui << (HSplit()
          << (VSplit()
              << Canvas3D(sz, {.handle = "fil", .minSize = {24, 18}})
              << Display({.handle = "cyc", .minSize = {24, 18}}))
          << (VBox().minSize({17, 26})
              << Prop(&scene, {.label = "knobs — Filament (top) / Cycles (bottom)"})
              << FSlider(0.02, 1.0, initRough,
                         {.handle = "rough", .label = "ground roughness",
                          .maxSize = {99, 2}})))
      << Show();

  // Turn SSR on for the glossy step so its floor reflection shows immediately
  // (the live preview renders continuously, so TAA warms up and resolves it).
  scene.getRenderer().setSSREnabled(o.useSSR);

  gui["fil"].link(scene.getGLCallback(0).get());
  gui["fil"].install(handleMouse);

  // Cycles in poll-driven interactive mode (render(0) once per frame in run()).
  int samples = pa("-samples");
  renderer = std::make_unique<CyclesRenderer>(scene, RenderQuality::Interactive);
  renderer->setSamples(samples);
  renderer->setSceneScale(1.0f);
}

void run() {
  // Live ground-roughness edit → update the material; Filament re-reads it next
  // frame, Cycles re-syncs the shader (and restarts converging) on invalidateNode.
  if (groundNode) {
    float r = gui["rough"];
    if (r != lastRoughness) {
      lastRoughness = r;
      groundNode->getMaterial()->roughness = r;
      renderer->invalidateNode(groundNode.get());
    }
  }

  gui["fil"].render();   // live Filament preview

  renderer->render(0);   // advance the Cycles path-trace
  static int lastUpdate = 0;
  int updates = renderer->getUpdateCount();
  if (updates > lastUpdate) {
    lastUpdate = updates;
    gui["cyc"] = renderer->getImage();
  }

  static FPSLimiter fps(30);
  fps.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
      "-step(str=simple) -size(Size=640x480) -samples(int=256)",
      init, run).exec();
}
