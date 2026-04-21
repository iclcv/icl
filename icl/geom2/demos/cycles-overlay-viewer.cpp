// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Overlay viewer: Cycles path tracing as background, GL scene rendered on top.
// Allows visual comparison of both renderers in a single pane.
//
// Usage:
//   cycles-overlay-viewer -scene model.glb [-size 800x600] [-samples 512]

#include <icl/qt/Common2.h>
#include <icl/geom2/DemoScene2.h>
#include <icl/geom2/CyclesRenderer.h>
#include <icl/geom2/Renderer.h>
#include <icl/geom2/Scene2MouseHandler.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;
using namespace icl::geom2;

static DemoScene2 scene;
static std::unique_ptr<CyclesRenderer> renderer;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (renderer && (evt.isDragEvent() || evt.isWheelEvent())) {
    renderer->invalidateTransforms();
  }
}

void init() {
  Size viewSize = pa("-size");

  // Collect scene file arguments
  std::vector<std::string> files;
  if (pa("-scene")) {
    int n = pa("-scene").n();
    for (int i = 0; i < n; i++)
      files.push_back(pa("-scene", i).as<std::string>());
  }

  // DemoScene2 handles loading, auto-scaling, ground, lights, camera
  scene.setup(files, viewSize,
              pa("-rotate") ? pa("-rotate").as<std::string>() : "",
              pa("-no-checkerboard"));

  // GUI: single canvas with overlay controls
  gui << (HSplit()
       << Canvas3D(viewSize).handle("canvas").minSize(32, 24)
       << (VBox().minSize(12, 0)
          << Slider(0, 100, 50).handle("alpha").label("GL Overlay %")
          << Slider(1, 16, 4).handle("bounces").label("Bounces")
          << Slider(10, 500, 100).handle("exposure").label("Exposure %")
          << Slider(0, 20, 3).handle("shadowSoft").label("Shadow Softness")
          << Combo("shaded,normals,albedo,UVs,lighting,NdotL,"
                   "SSR confidence,depth,SSR only").handle("glDebug")
                   .label("GL Debug")
          << Label("--").handle("info")))
     << Show();

  gui["canvas"].link(scene.getGLCallback(0).get());
  gui["canvas"].install(handleMouse);

  // Start Cycles autonomous renderer
  int samples = pa("-samples");
  renderer = std::make_unique<CyclesRenderer>(scene, RenderQuality::Interactive);
  renderer->setSamples(samples);
  renderer->setMaxBounces(4);
  renderer->setExposure(1.0f);
  renderer->setDenoising(false);
  renderer->setSceneScale(1.0f);
  renderer->start(0);
}

void run() {
  float alpha = gui["alpha"].as<float>() / 100.0f;
  float exposure = gui["exposure"].as<float>() / 100.0f;
  int bounces = gui["bounces"].as<int>();

  // Update Cycles parameters
  renderer->setMaxBounces(bounces);
  renderer->setExposure(exposure);

  // Update GL renderer overlay
  scene.getRenderer().setExposure(exposure);
  scene.getRenderer().setOverlayAlpha(alpha);
  scene.getRenderer().setDebugMode(gui["glDebug"].as<ComboHandle>().getSelectedIndex());

  // Apply shadow softness to all shadow-enabled lights
  float softness = gui["shadowSoft"].as<float>();
  for (int i = 0; i < scene.getLightCount(); i++) {
    auto *l = scene.getLight(i);
    if (l && l->getShadowEnabled())
      l->setSoftShadowRadius(softness);
  }

  // Feed latest Cycles image as canvas background, then render GL on top
  gui["canvas"] = renderer->getImage();
  gui["canvas"].render();

  static FPSLimiter fps(30);
  gui["info"] = fps.formatted("%d spp | #fps fps | GL %.0f%%",
                               renderer->getUpdateCount(), alpha * 100);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
    "-scene(...) -size(Size=800x600) -samples(int=256) -rotate(string) -no-checkerboard",
    init, run).exec();
}
