// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Interactive scene viewer: load OBJ/glTF, render with Cycles + GL preview.
//
// Usage:
//   cycles-scene-viewer -scene model.obj [-scene another.glb ...]
//                       [-size 1280x960] [-samples 512]

#include <icl/qt/Common2.h>
#include <icl/geom2/DemoScene2.h>
#include <icl/geom2/CyclesRenderer.h>
#include <icl/geom2/Renderer.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/utils/FPSLimiter.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom2;

static DemoScene2 scene;
static std::unique_ptr<CyclesRenderer> renderer;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (renderer && (evt.isDragEvent() || evt.isWheelEvent()))
    renderer->invalidateTransforms();
}

void init() {
  Size viewSize = pa("-size");

  std::vector<std::string> files;
  if (pa("-scene")) {
    int n = pa("-scene").n();
    for (int i = 0; i < n; i++)
      files.push_back(pa("-scene", i).as<std::string>());
  }

  scene.setup(files, viewSize);

  // GUI: left = GL preview, right = Cycles raytrace, debug controls below
  gui << (VSplit()
       << (HSplit()
          << Canvas3D(viewSize).handle("gl").minSize(16, 12)
          << Display().handle("rt").minSize(16, 12))
       << (HBox()
          << Combo("shaded,normals,albedo,UVs,lighting,NdotL,"
                   "SSR confidence,depth,SSR only").handle("glDebug")
                   .label("GL Debug")))
      << Show();

  gui["gl"].link(scene.getGLCallback(0).get());
  gui["gl"].install(handleMouse);

  // Start Cycles
  int samples = pa("-samples");
  renderer = std::make_unique<CyclesRenderer>(scene, RenderQuality::Interactive);
  renderer->setSamples(samples);
  renderer->setSceneScale(1.0f);
  renderer->start(0);
}

void run() {
  scene.getRenderer().setDebugMode(gui["glDebug"].as<ComboHandle>().getSelectedIndex());
  gui["gl"].render();

  renderer->render(0);
  static int lastUpdate = 0;
  int updates = renderer->getUpdateCount();
  if (updates > lastUpdate) {
    lastUpdate = updates;
    gui["rt"] = renderer->getImage();
  }

  static FPSLimiter fps(30);
  fps.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
      "-scene(...) -size(Size=800x600) -samples(int=256)",
      init, run).exec();
}
