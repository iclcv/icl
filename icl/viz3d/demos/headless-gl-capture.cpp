// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless offscreen GL capture: drives GLSceneCapture(ownContext=true) with NO
// GUI window. Uses a plain QGuiApplication on the cocoa platform (NOT
// QT_QPA_PLATFORM=offscreen, which has no GL backend at all) so the capturer's
// owned QOffscreenSurface + QOpenGLContext renders Scene2::renderToImage
// entirely off screen, then saves the result to PNG.
//
// This exercises ICL's real GL render path with no on-screen QOpenGLWidget — the
// only way to verify Scene2::renderToImage / the GLSceneCapture PBuffer-style
// owned context end-to-end in a headless environment. Run as:
//
//   QT_QPA_PLATFORM=cocoa builddir/bin/viz3d-headless-gl-capture-demo [out.png]

#include <QGuiApplication>
#include <QSurfaceFormat>

#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/render/SceneCapture.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/cv3d/Camera.h>
#include <icl/viz3d/render/Material.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <memory>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::core;
using namespace icl::utils;

int main(int argc, char **argv) {
  const std::string output = argc > 1 ? argv[1] : "headless-gl-capture.png";

  // GL 4.1 core profile must be the default surface format before any context is
  // created (the GUI path does this in QApplication; we do it ourselves here).
  QSurfaceFormat fmt;
  fmt.setVersion(4, 1);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(fmt);

  // A running Q*Application is required for QOffscreenSurface / QOpenGLContext.
  // QGuiApplication (no widgets) avoids the on-screen QOpenGLWidget entirely.
  QGuiApplication app(argc, argv);

  Scene2 scene;
  scene.addCamera(Camera::lookAt(Vec(0, -520, 200, 1), Vec(0, 0, 40, 1),
                                 Vec(0, 0, 1, 1), Size(640, 480), 50.0f));
  scene.setBounds(500);

  auto sphere = SphereNode::create(0, 0, 60, 50, 40, 40);
  sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
  scene.addNode(sphere);

  auto cube = CuboidNode::create(120, 80, 30, 60, 60, 60);
  cube->setMaterial(Material::fromColor(GeomColor(60, 60, 220, 255)));
  scene.addNode(cube);

  auto ground = std::make_shared<MeshNode>();
  const float gs = 250;
  ground->addVertex(Vec(-gs, -gs, 0, 1));
  ground->addVertex(Vec(gs, -gs, 0, 1));
  ground->addVertex(Vec(gs, gs, 0, 1));
  ground->addVertex(Vec(-gs, gs, 0, 1));
  ground->addQuad(0, 1, 2, 3);
  ground->createAutoNormals(false);
  ground->setMaterial(Material::fromColor(GeomColor(170, 170, 170, 255)));
  scene.addNode(ground);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.5f);
  light->translate(200, 150, 300);
  scene.addLight(light);

  // The actual headless render: owned offscreen context, no window, any thread.
  GLSceneCapture cap(/*ownContext=*/true);
  BVH::ImageResult r = cap.capture(scene, 0);

  if (!r.image.getDim()) {
    std::fprintf(stderr, "headless GL capture FAILED — empty result "
                         "(no GL context / render produced no pixels)\n");
    return 1;
  }

  icl::io::save(r.image, output);
  std::printf("Saved %dx%d headless GL render to %s\n",
              r.image.getWidth(), r.image.getHeight(), output.c_str());
  return 0;
}
