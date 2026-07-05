// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/generic-texture-coords demo: a flat disc with
// per-vertex texture coordinates, textured with a live image-source frame, in
// front of a wireframe cube. Shows native viz3d texcoords + a live
// baseColorMap (updated every frame via Material::setBaseColorMap).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

ImageSource grabber;
Scene2 scene;
GUI gui;
Image image;
std::shared_ptr<MeshNode> disc;

// a flat textured disc (triangle fan) in the local z=-2 plane, UVs mapping the
// full image onto the circle.
static std::shared_ptr<MeshNode> makeDisc(float ar) {
  auto m = std::make_shared<MeshNode>();
  m->addVertex(Vec(0, 0, -2, 1));        // centre
  m->addNormal(Vec(0, 0, 1, 1));
  m->addTexCoord(0.5f, 0.5f);
  const int N = 100;
  for (int i = 0; i < N; ++i) {
    float a = (float(i) / N) * 2 * M_PI;
    m->addVertex(Vec(std::cos(a) * 5 * ar, std::sin(a) * 5, -2, 1));
    m->addNormal(Vec(0, 0, 1, 1));
    m->addTexCoord(std::cos(a) / 2 + 0.5f, std::sin(a) / 2 + 0.5f);
  }
  for (int i = 0; i < N; ++i) {          // fan: centre, rim[i], rim[i+1]
    int b = 1 + i, c = 1 + (i + 1) % N;
    m->addTriangle(0, b, c, 0, b, c, 0, b, c);
  }
  return m;
}

void init() {
  grabber.init(pa("-i"));
  image = grabber.grab();
  scene.addCamera(Camera());

  disc = makeDisc(float(image.getWidth()) / float(image.getHeight()));
  auto mat = Material::fromColor(GeomColor(255, 255, 255, 255));
  mat->setBaseColorMap(image);
  disc->setMaterial(mat);
  scene.addNode(disc);

  auto cube = CuboidNode::createCube(0, 0, 0, 100);
  cube->setPrimitiveVisible(PrimLine, true);
  cube->setPrimitiveVisible(PrimTriangle | PrimQuad, false);
  scene.addNode(cube);

  gui << Canvas3D({.handle="draw"}) << Show();
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

void run() {
  image = grabber.grab();
  disc->getMaterial()->setBaseColorMap(image);   // live texture update
  gui["draw"].render();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "-input|-i(2)", init, run).exec();
}
