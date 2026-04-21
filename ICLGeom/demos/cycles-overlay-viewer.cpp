// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Overlay viewer: Cycles path tracing as background, GL scene rendered on top.
// Allows visual comparison of both renderers in a single pane.
//
// Usage:
//   cycles-overlay-viewer -scene model.glb [-size 800x600] [-background gradient]

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Material.h>
#include <ICLGeom/SceneRendererGL.h>
#include <ICLGeom/Raytracer.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLGeom/SceneSetup.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static Scene scene;
static icl::rt::SceneSetupResult setupResult;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  scene.getRaytracer().invalidateAll();
}

static void init() {
  Size size = pa("-size").as<Size>();

  auto files = pa("-scene").subargs<std::string>();

  setupResult = icl::rt::setupScene(scene, files, size,
      pa("-background").as<std::string>(),
      pa("-backlight").as<bool>(),
      pa("-decimate") ? pa("-decimate").as<int>() : 0,
      pa("-rotate") ? pa("-rotate").as<std::string>() : "");

  // Scene owns both renderers
  scene.getRendererGL().setOverlayMode(true);

  // GUI: canvas on left, controls on right
  gui << (HSplit()
       << Canvas3D(size).handle("canvas").minSize(32, 24)
       << (VBox().minSize(12, 0)
          << Slider(0, 100, 50).handle("alpha").label("GL Overlay %")
          << Slider(1, 16, 4).handle("bounces").label("Bounces")
          << Slider(10, 500, 100).handle("exposure").label("Exposure %")
          << Combo("!Original,Clay,Mirror,Gold,Copper,Chrome,Red Plastic,Green Rubber,Glass,Emissive").handle("material").label("Material")
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

  scene.getRendererGL().setExposure(exposure);
  scene.getRendererGL().setOverlayAlpha(alpha);

  // Material preset
  static int lastMat = -1;
  int matIdx = gui["material"].as<ComboHandle>().getSelectedIndex();
  if (matIdx != lastMat) {
    lastMat = matIdx;
    icl::rt::applyMaterialPreset(matIdx, setupResult, nullptr);
    rt.invalidateAll();
    for (int i = 0; i < scene.getObjectCount(); i++)
      scene.getObject(i)->prepareForRendering();
  }

  // Pull latest Cycles image (renderer runs autonomously via start())
  DrawHandle3D canvas = gui["canvas"];
  canvas = &rt.getImage();
  canvas.render();

  static FPSEstimator fpsEst(10);
  gui["info"] = fpsEst.formatted("%d spp | #fps fps | GL %.0f%%",
                                  rt.getUpdateCount(), alpha * 100);

  static FPSLimiter fpslim(30);
  fpslim.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
    "-size(Size=800x600) -scene(...) -background(string=gradient) "
    "-backlight -decimate(int) -rotate(string)",
    init, run).exec();
}
