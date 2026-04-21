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
#include <ICLUtils/FPSLimiter.h>
#include <Raytracing/CyclesRenderer.h>
#include <Raytracing/SceneSetup.h>

#include <memory>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static Scene scene;
static std::unique_ptr<icl::rt::CyclesRenderer> renderer;
static icl::rt::SceneSetupResult setupResult;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (renderer) renderer->invalidateAll();
}

static void init() {
  Size size = pa("-size").as<Size>();

  auto files = pa("-scene").subargs<std::string>();

  setupResult = icl::rt::setupScene(scene, files, size,
      pa("-background").as<std::string>(),
      pa("-backlight").as<bool>(),
      pa("-decimate") ? pa("-decimate").as<int>() : 0,
      pa("-rotate") ? pa("-rotate").as<std::string>() : "");

  // Renderers
  renderer = std::make_unique<icl::rt::CyclesRenderer>(
      scene, icl::rt::RenderQuality::Preview);
  renderer->setSceneScale(1.0f);

  // Scene owns the GL renderer — set overlay mode for compositing
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
  canvas->setViewPort(size);
  canvas->install(new MouseHandler(handleMouse));
  canvas->link(scene.getGLCallback(0));
}

static void run() {
  float alpha = gui["alpha"].as<float>() / 100.0f;
  float exposure = gui["exposure"].as<float>() / 100.0f;
  int bounces = gui["bounces"].as<int>();

  // todo: future: the renderer couild become Configurable (then such features should automatically create a UI)
  renderer->setMaxBounces(bounces);
  renderer->setExposure(exposure);
  renderer->setBrightness(scene.getSky().intensity * 100.0f);

  scene.getRendererGL().setExposure(exposure);
  scene.getRendererGL().setOverlayAlpha(alpha);

  // ok, interesting .. maybe we can move that into a debug-feature in the scene class 
  // like it has an enum of DebugMaterials (and a function to apply them) and then we can just use that here?
  // Material preset
  static int lastMat = -1;
  int matIdx = gui["material"].as<ComboHandle>().getSelectedIndex();
  if (matIdx != lastMat) {
    lastMat = matIdx;
    icl::rt::applyMaterialPreset(matIdx, setupResult, renderer.get());
    for (int i = 0; i < scene.getObjectCount(); i++)
      scene.getObject(i)->prepareForRendering();
  }

  // Render Cycles + set as background (triggers widget redraw + GL callback)
  renderer->render(0);
  DrawHandle3D canvas = gui["canvas"];
  canvas = &renderer->getImage();
  canvas.render();

  static FPSEstimator fpsEst(10);
  char buf[128];
  snprintf(buf, sizeof(buf), "%d spp | %.0f fps | GL %.0f%%",
           renderer->getUpdateCount(), fpsEst.getFPSVal(), alpha * 100);
  gui["info"] = std::string(buf);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
    "-size(Size=800x600) -scene(...) -background(string=gradient) "
    "-backlight -decimate(int) -rotate(string)",
    init, run).exec();
}
