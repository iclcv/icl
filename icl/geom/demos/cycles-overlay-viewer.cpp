// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Overlay viewer: Cycles path tracing as background, GL scene rendered on top.
// Allows visual comparison of both renderers in a single pane.
//
// Usage:
//   cycles-overlay-viewer -scene model.glb [-size 800x600] [-background gradient]

#include <icl/qt/Common.h>
#include <icl/geom/DemoScene.h>
#include <icl/geom/GLRenderer.h>
#include <icl/geom/Raytracer.h>
#include <icl/utils/FPSLimiter.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static DemoScene scene;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (evt.isDragEvent() || evt.isReleaseEvent()) {
    scene.getRaytracer().invalidateTransforms();
  }
}


static void init() {
  Size size = pa("-size").as<Size>();

  scene.setup(pa("-scene").subargs<std::string>(), size,
              pa("-background").as<std::string>(),
              pa("-backlight").as<bool>(),
              pa("-decimate") ? pa("-decimate").as<int>() : 0,
              pa("-rotate") ? pa("-rotate").as<std::string>() : "");

  // Scene owns both renderers
  scene.getGLRenderer().setHideSky(true);

  // Set initial raytracer params from GUI defaults BEFORE start(),
  // so the first render isn't interrupted by setter mismatches in run().
  auto &rt = scene.getRaytracer();
  rt.setMaxBounces(4);        // match slider default
  rt.setExposure(1.0f);       // match slider default (100/100)
  rt.setBrightness(scene.getSky().intensity * 100.0f);
  rt.setDenoising(false);     // off for interactive preview (fast noisy intermediates)

  // GUI: canvas on left, controls on right
  gui << (HSplit()
       << Canvas3D(size).handle("canvas").minSize(32, 24)
       << (VBox().minSize(12, 0)
          << Slider(0, 100, 50).handle("alpha").label("GL Overlay %")
          << Slider(1, 16, 4).handle("bounces").label("Bounces")
          << Slider(10, 500, 100).handle("exposure").label("Exposure %")
          << Combo(std::string("!") + Scene::getMaterialPresetNames()).handle("material").label("Material")
          << Label("--").handle("info")))
     << Show();

  DrawHandle3D canvas = gui["canvas"];
  canvas->install(new MouseHandler(handleMouse));
  canvas->link(scene.getGLCallback(0));

  // Start autonomous rendering
  scene.getRaytracer().start(0);
}

static void run() {
  float alpha = gui["alpha"].as<float>() / 100.0f;
  float exposure = gui["exposure"].as<float>() / 100.0f;
  int bounces = gui["bounces"].as<int>();

  auto &rt = scene.getRaytracer();
  rt.setMaxBounces(bounces);
  rt.setExposure(exposure);
  rt.setBrightness(scene.getSky().intensity * 100.0f);

  scene.getGLRenderer().setExposure(exposure);
  scene.getGLRenderer().setOverlayAlpha(alpha);
  scene.setMaterialPreset(gui["material"].as<ComboHandle>().getSelectedIndex());

  // Feed latest Cycles image to widget so the image info indicator works.
  gui["canvas"] = rt.getImage();
  gui["canvas"].render();

  static FPSLimiter fps(30);
  gui["info"] = fps.formatted("%d spp | #fps fps | GL %.0f%%",
                               rt.getUpdateCount(), alpha * 100);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
    "-size(Size=800x600) -scene(...) -background(string=gradient) "
    "-backlight -decimate(int) -rotate(string)",
    init, run).exec();
}
