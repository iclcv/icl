// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless verification harness for PlotWidget3D's coordinate frame. Builds the
// EXACT shipping frame (icl::geom2::detail::makePlotBox/makePlotAxis/placePlotAxes) plus a
// bit of sample geometry into a bare Scene2, renders it offscreen with
// GLSceneCapture (no QWidget — the on-screen widget crashes in the sandbox) and
// saves a PNG. Lets plot axis/label/orientation fixes be checked in-sandbox.
// See memory reference_headless_gl_capture.
//
//   QT_QPA_PLATFORM=cocoa builddir/bin/geom2-plot-frame-capture-demo \
//       [out.png] [posX posY posZ upX upY upZ]
//
// The optional camera args make it cheap to dial in a default view.

#include <QGuiApplication>
#include <QSurfaceFormat>

#include <icl/geom2/Scene2.h>
#include <icl/geom2/SceneCapture.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CoordinateFrameNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/detail/PlotFrame.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <cstdlib>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "plot-frame-capture.png";
  Vec pos(2.5, 2.5, 7, 1), up(0, 1, 0, 1);  // matches PlotWidget3D default
  if (argc >= 8) {
    pos = Vec(atof(argv[2]), atof(argv[3]), atof(argv[4]), 1);
    up  = Vec(atof(argv[5]), atof(argv[6]), atof(argv[7]), 1);
  }

  QSurfaceFormat fmt;
  fmt.setVersion(4, 1);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(fmt);
  QGuiApplication app(argc, argv);

  GLSceneCapture cap(/*ownContext=*/true);

  Scene2 scene;
  scene.setBounds(5);
  scene.addCamera(Camera::lookAt(pos, Vec(0, 0, 0, 1), up, Size(1280, 960), 30.0f));

  // the shipping coordinate frame, in the [-1,1]^3 box
  auto frame = std::make_shared<GroupNode>();
  frame->addChild(icl::geom2::detail::makePlotBox());
  std::shared_ptr<GroupNode> axes[3] = {
    icl::geom2::detail::makePlotAxis(Range32f(-4, 4), false, "X"),
    icl::geom2::detail::makePlotAxis(Range32f(-4, 4), false, "Y"),
    icl::geom2::detail::makePlotAxis(Range32f(-4, 4), false, "Z"),
  };
  // ticks on the edges meeting at the corner furthest from the camera
  icl::geom2::detail::placePlotAxes(axes, pos[0] > 0 ? -1 : 1,
                                    pos[1] > 0 ? -1 : 1, pos[2] > 0 ? -1 : 1);
  for (auto &a : axes) frame->addChild(a);
  scene.addNode(frame);

  // ground-truth axis triad at the origin: R=+X, G=+Y, B=+Z. Its arrow
  // directions are the REAL geometry axes — compare against the box tick labels.
  scene.addNode(CoordinateFrameNode::create(1.6f, 0.03f));

  // a little asymmetric sample geometry so orientation is unambiguous
  auto sx = SphereNode::create( 0.6f, 0, 0, 0.18f, 20, 20);
  sx->setMaterial(Material::fromColor(GeomColor(230, 60, 60, 255)));   // +X red
  auto sy = SphereNode::create(0,  0.6f, 0, 0.18f, 20, 20);
  sy->setMaterial(Material::fromColor(GeomColor(60, 200, 60, 255)));   // +Y green
  auto sz = SphereNode::create(0, 0,  0.6f, 0.18f, 20, 20);
  sz->setMaterial(Material::fromColor(GeomColor(80, 120, 255, 255)));  // +Z blue
  scene.addNode(sx); scene.addNode(sy); scene.addNode(sz);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.2f);
  light->translate(3, 3, 3);
  scene.addLight(light);

  BVH::ImageResult r = cap.capture(scene, 0);
  if (!r.image.getDim()) { std::fprintf(stderr, "capture FAILED (empty)\n"); return 1; }
  icl::io::save(r.image, out);
  std::printf("Saved %dx%d to %s  (cam pos %.2f %.2f %.2f  up %.2f %.2f %.2f)\n",
              r.image.getWidth(), r.image.getHeight(), out.c_str(),
              pos[0], pos[1], pos[2], up[0], up[1], up[2]);
  return 0;
}
