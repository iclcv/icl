// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Interactive scene viewer: load OBJ/glTF, render with Cycles + GL preview.
//
// Usage:
//   cycles-scene-viewer -scene model.obj [-scene another.glb ...]
//                       [-size 1280x960] [-samples 512]

#include <icl/qt/Common.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Loader.h>
#include <icl/geom2/CyclesRenderer.h>
#include <icl/geom2/Renderer.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>
#include <icl/utils/FPSLimiter.h>

#include <memory>
#include <vector>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;
using namespace icl::geom2;

static Scene2 scene;
static std::unique_ptr<CyclesRenderer> renderer;
static std::vector<std::shared_ptr<MeshNode>> loadedMeshes;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);
  if (renderer && (evt.isDragEvent() || evt.isWheelEvent()))
    renderer->invalidateTransforms();
}

void init() {
  int nScenes = pa("-scene").n();
  for (int i = 0; i < nScenes; i++) {
    std::string file = pa("-scene", i).as<std::string>();
    fprintf(stderr, "Loading %s...\n", file.c_str());
    auto meshes = loadFile(file);
    fprintf(stderr, "  %zu mesh(es) loaded\n", meshes.size());
    for (auto &m : meshes) {
      scene.addNode(m);
      loadedMeshes.push_back(m);
    }
  }

  if (loadedMeshes.empty()) {
    // Create a default scene with some spheres
    auto gold = std::make_shared<SphereNode>(-200, 100, 200, 100, 40, 40);
    gold->setMaterial(Material::fromColor(GeomColor(255, 194, 87, 255)));
    gold->getMaterial()->metallic = 1.0f;
    gold->getMaterial()->roughness = 0.15f;
    scene.addNode(gold);

    auto red = std::make_shared<SphereNode>(0, 100, 200, 100, 40, 40);
    red->setMaterial(Material::fromColor(GeomColor(204, 26, 26, 255)));
    red->getMaterial()->roughness = 0.3f;
    scene.addNode(red);

    auto green = std::make_shared<SphereNode>(200, 100, 200, 100, 40, 40);
    green->setMaterial(Material::fromColor(GeomColor(26, 153, 38, 255)));
    green->getMaterial()->roughness = 0.9f;
    scene.addNode(green);

    // Ground plane
    auto ground = std::make_shared<MeshNode>();
    ground->addVertex(Vec(-500, 0, -500, 1));
    ground->addVertex(Vec( 500, 0, -500, 1));
    ground->addVertex(Vec( 500, 0,  500, 1));
    ground->addVertex(Vec(-500, 0,  500, 1));
    for (int j = 0; j < 4; j++) ground->addNormal(Vec(0, 1, 0, 1));
    ground->addTriangle(0, 1, 2, 0, 1, 2);
    ground->addTriangle(0, 2, 3, 0, 2, 3);
    ground->setMaterial(Material::fromColor(GeomColor(102, 89, 77, 255)));
    ground->getMaterial()->roughness = 0.8f;
    scene.addNode(ground);
  }

  // Lights
  auto keyLight = std::make_shared<LightNode>();
  keyLight->setColor(GeomColor(255, 250, 240, 255));
  keyLight->setIntensity(1.0f);
  keyLight->translate(-300, 400, -100);
  scene.addLight(keyLight);

  auto fillLight = std::make_shared<LightNode>();
  fillLight->setColor(GeomColor(180, 200, 220, 255));
  fillLight->setIntensity(0.6f);
  fillLight->translate(400, 300, -200);
  scene.addLight(fillLight);

  // Camera
  Size viewSize = pa("-size");
  scene.addCamera(Camera::lookAt(
      Vec(0, 300, -500, 1),
      Vec(0, 50, 200, 1),
      Vec(0, 1, 0, 1),
      viewSize, 55.0f));

  // GUI: left = GL preview, right = Cycles raytrace
  gui << Canvas3D(viewSize).handle("gl").minSize(16, 12)
      << Display().handle("rt").minSize(16, 12)
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
  // GL preview
  gui["gl"].render();

  // Cycles raytrace: poll for new image
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
