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
static std::shared_ptr<GeometryNode> glassNode;    // transmissive node for the glass sliders (may be null)
static float lastRoughness = -1.0f;
static float lastGT = -1, lastGR = -1, lastGI = -1, lastGA = -1;
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
                       "' (try: emissive diffuse sky metal simple simple-glossy darkfloor glass amber)");
  groundNode = o.ground;
  glassNode = o.glass;
  const float initRough = groundNode ? groundNode->getMaterial()->roughness : 0.5f;

  // Glass material knobs (transmission / roughness / IOR / absorption) — per-material
  // properties, not render.* knobs, so they get their own sliders. Present only when
  // the step has a transmissive node (glass / amber). `absorption` maps a 0..1 slider
  // to a Beer-Lambert distance + a warm amber tint (0 = clear, 1 = dense) — the lever
  // that makes SSR glass read as a solid stone (Filament SSR can't see the interior
  // like Cycles' ray-traced refraction, so absorption fills the see-through).
  auto gmat = glassNode ? glassNode->getMaterial() : nullptr;
  const bool hasGlass = gmat && gmat->transmission;
  const float iGT = hasGlass ? gmat->transmission->transmission : 0.0f;
  const float iGR = gmat ? gmat->roughness : 0.05f;
  const float iGI = hasGlass ? gmat->transmission->ior : 1.5f;

  // The ground roughness is a per-material property, not a render.* knob, so it gets
  // its own slider (below the auto-generated Prop panel). Both backends pick up the
  // change: Filament re-reads material params every frame; Cycles via invalidateNode.
  GUI controls = VBox().minSize({17, 26});
  controls << Prop(&scene, {.label = "knobs — Filament (top) / Cycles (bottom)"})
           << FSlider(0.02, 1.0, initRough,
                      {.handle = "rough", .label = "ground roughness", .maxSize = {99, 2}});
  if (hasGlass)
    controls << FSlider(0.0, 1.0, iGT, {.handle = "gtrans", .label = "glass transmission", .maxSize = {99, 2}})
             << FSlider(0.02, 0.6, iGR, {.handle = "grough", .label = "glass roughness", .maxSize = {99, 2}})
             << FSlider(1.0, 2.0, iGI, {.handle = "gior", .label = "glass IOR", .maxSize = {99, 2}})
             << FSlider(0.0, 1.0, 0.0, {.handle = "gabsorb", .label = "glass absorption", .maxSize = {99, 2}});
  gui << (HSplit()
          << (VSplit()
              << Canvas3D(sz, {.handle = "fil", .minSize = {24, 18}})
              << Display({.handle = "cyc", .minSize = {24, 18}}))
          << controls)
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

  // Glass material sliders (only present when the step has a transmissive node).
  if (glassNode && glassNode->getMaterial() && glassNode->getMaterial()->transmission) {
    float t = gui["gtrans"], rg = gui["grough"], io = gui["gior"], ab = gui["gabsorb"];
    if (t != lastGT || rg != lastGR || io != lastGI || ab != lastGA) {
      lastGT = t; lastGR = rg; lastGI = io; lastGA = ab;
      auto m = glassNode->getMaterial();
      auto *tr = m->transmission.get();
      tr->transmission = t;
      m->roughness = rg;
      tr->ior = io;
      if (ab <= 0.001f) {
        tr->attenuationDistance = 1e30f;   // clear (no volume absorption)
      } else {
        // Denser as the slider rises; warm amber tint so the transmitted interior
        // reads brown+solid instead of see-through-to-background.
        tr->attenuationDistance = 300.0f * (1.0f - ab) + 15.0f;
        tr->attenuationColor = GeomColor(0.80f, 0.45f, 0.15f, 1.0f);
      }
      renderer->invalidateNode(glassNode.get());
    }
  }

  gui["fil"].render();   // live Filament preview — EVERY frame, so its TAA
                         // accumulates at full rate and temporal noise clears fast.

  // Advance Cycles only every Nth frame: a progressive path-trace step is far
  // slower than a Filament frame, so stepping it every frame throttled the whole
  // loop and starved the Filament preview of the frames its TAA needs to converge.
  static int tick = 0;
  if (++tick % 4 == 0) {
    renderer->render(0);
    static int lastUpdate = 0;
    int updates = renderer->getUpdateCount();
    if (updates > lastUpdate) {
      lastUpdate = updates;
      gui["cyc"] = renderer->getImage();
    }
  }

  static FPSLimiter fps(60);   // let Filament run up to 60fps for fast TAA convergence
  fps.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
      "-step(str=simple) -size(Size=640x480) -samples(int=256)",
      init, run).exec();
}
