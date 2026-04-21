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

#include <QSurfaceFormat>
#include <memory>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static Scene scene;
static std::unique_ptr<icl::rt::CyclesRenderer> renderer;
static std::unique_ptr<SceneRendererGL> glRenderer;
static icl::rt::SceneSetupResult setupResult;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (renderer) renderer->invalidateAll();
}

// todo: the SceneGLRenderer should live in the scene maybe or in Qt? .. what's better for the dependency chain?
// extra_cb coulb be an optional extra lambda, that we could use hiere to call invalidateAll()
//   .. then we could have gui["canvas"]->link(&scene.getGLRenderer(extra_cb)) and the renderer would automatically trigger redraws when the scene changes (and we would not need to call invalidateAll() manually in the mouse handler)
// GL callback: render GL overlay on top of background image
static struct OverlayCallback : public ICLDrawWidget3D::GLCallback {
  void draw(ICLDrawWidget3D *widget) override {
    if (glRenderer) glRenderer->render(scene, 0, widget);
  }
} overlayCB;

static void init() {
  Size size = pa("-size").as<Size>();

  // todo let's add a pa("-scene").args() or sth. to get all args with a common prefix as vector of strings, that would be much more convenient for this use case than the current way to get multiple args with the same name (which is a bit clumsy)
  // Load scene 
  std::vector<std::string> files;
  for (int i = 0; i < pa("-scene").n(); i++)
    files.push_back(pa("-scene", i).as<std::string>());

  setupResult = icl::rt::setupScene(scene, files, size,
      pa("-background").as<std::string>(),
      pa("-backlight").as<bool>(),
      pa("-decimate") ? pa("-decimate").as<int>() : 0,
      pa("-rotate") ? pa("-rotate").as<std::string>() : "");

  // Renderers
  renderer = std::make_unique<icl::rt::CyclesRenderer>(
      scene, icl::rt::RenderQuality::Preview);
  renderer->setSceneScale(1.0f);

  // todo: as above: this must live somewhere else, maybe in the scene or in Qt? .. overlay mode can be set automatically then
  glRenderer = std::make_unique<SceneRendererGL>();
  glRenderer->setOverlayMode(true);

  // GUI
  gui << (VSplit()
       << Canvas3D(size).handle("canvas").minSize(32, 24)
       << (HBox()
          << Slider(0, 100, 50).handle("alpha").label("GL %")
          << Combo("!Original,Clay,Mirror,Gold,Copper,Chrome,Red Plastic,Green Rubber,Glass,Emissive").handle("material")
          << Slider(1, 16, 4).handle("bounces")
          << Slider(10, 500, 100).handle("exposure")
          << Label("--").handle("info")))
     << Show();

  DrawHandle3D canvas = gui["canvas"];
  canvas->setViewPort(size);
  canvas->install(new MouseHandler(handleMouse));
  canvas->link(&overlayCB);
}

static void run() {
  float alpha = gui["alpha"].as<float>() / 100.0f;
  float exposure = gui["exposure"].as<float>() / 100.0f;
  int bounces = gui["bounces"].as<int>();

  // todo: future: the renderer couild become Configurable (then such features should automatically create a UI)
  renderer->setMaxBounces(bounces);
  renderer->setExposure(exposure);
  renderer->setBrightness(scene.getSky().intensity * 100.0f);

  glRenderer->setExposure(exposure);
  glRenderer->setOverlayAlpha(alpha);

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

  // todo: this and the if statement should be shrinked to 
  // gui["canvas"] = renderer->render(0);

  // Render Cycles (non-blocking progressive)
  renderer->render(0);

  // Set Cycles image as background → triggers widget redraw + GL callback
  if (!renderer->getImage().isNull()) {
    DrawHandle3D canvas = gui["canvas"];
    canvas = renderer->getImage();
    canvas.render();
  }

  // todo: use FpsEstimator for this (possibly extend it if functionality is missing`)
  // FPS info
  static Time lastFpsTime = Time::now();
  static int lastCount = 0;
  static float fps = 0;
  int count = renderer->getUpdateCount();
  Time now = Time::now();
  float dt = (float)(now - lastFpsTime).toSecondsDouble();
  if (dt >= 0.5f) {
    fps = (count - lastCount) / dt;
    lastCount = count;
    lastFpsTime = now;
  }

  char buf[128];
  snprintf(buf, sizeof(buf), "%d spp | %.1f fps | GL %.0f%%", count, fps, alpha * 100);
  gui["info"] = std::string(buf);
}

int main(int argc, char **argv) {
  // todo: this should go into ICLApp constructor 
  QSurfaceFormat fmt;
  fmt.setVersion(4, 1);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(fmt);

  return ICLApp(argc, argv,
    "-size(Size=800x600) -scene(...) -background(string=gradient) "
    "-backlight -decimate(int) -rotate(string)",
    init, run).exec();
}
