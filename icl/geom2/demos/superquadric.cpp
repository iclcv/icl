// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// geom2 port of the legacy geom/superquadric demo, now driven by the new
// parametric SuperquadricNode. Sliders morph the shape (size, the two
// squareness exponents, tessellation) and orient it (the node transform).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/SuperquadricNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/cv3d/Camera.h>
#include <icl/math/la/FixedMatrix.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

HSplit gui;
Scene2 scene;
std::shared_ptr<SuperquadricNode> sq;

void init() {
  gui << Canvas3D(Size::VGA, {.handle="draw", .minSize={32, 24}})
      << ( VBox({.minSize={15, 1}, .maxSize={18, 99}})
           << FSlider(-7, 7, 0, {.handle="rx", .label="x-rotation"})
           << FSlider(-7, 7, 0, {.handle="ry", .label="y-rotation"})
           << FSlider(-7, 7, 0, {.handle="rz", .label="z-rotation"})
           << FSlider(0.1, 10, 1, {.handle="dx", .label="x-size"})
           << FSlider(0.1, 10, 1, {.handle="dy", .label="y-size"})
           << FSlider(0.1, 10, 1, {.handle="dz", .label="z-size"})
           << ( HBox({.label="e1"}) << CheckBox("1/x", {.handle="e1x"}) << FSlider(1, 10, 1, {.handle="e1"}) )
           << ( HBox({.label="e2"}) << CheckBox("1/x", {.handle="e2x"}) << FSlider(1, 10, 1, {.handle="e2"}) )
           << Slider(5, 100, 30, {.handle="step1", .label="x-Steps"})
           << Slider(5, 100, 30, {.handle="step2", .label="y-Steps"})
           << CheckBox("grid", {.handle="grid"}) )
      << Show();

  scene.addCamera(Camera(Vec(0,0,-10), Vec(0,0,1), Vec(1,0,0)));
  sq = SuperquadricNode::create(0,0,0, 1,1,1, 1,1, 30,30);
  scene.addNode(sq);

  auto l = std::make_shared<LightNode>(LightNode::Point);
  l->setColor(GeomColor(0, 1, 0, 1));
  l->translate(10, 10, 10);
  scene.addLight(l);

  gui["draw"].install(scene.getMouseHandler(0));
  gui["draw"].link(scene.getGLCallback(0).get());
}

void run() {
  float e1 = gui["e1"], e2 = gui["e2"];
  if (gui["e1x"]) e1 = 1.f / e1;
  if (gui["e2x"]) e2 = 1.f / e2;
  const float dx = gui["dx"], dy = gui["dy"], dz = gui["dz"];
  const int s1 = gui["step1"], s2 = gui["step2"];
  const float rx = gui["rx"], ry = gui["ry"], rz = gui["rz"];

  scene.lock();
  // size / exponents / tessellation only regenerate the mesh when they change
  static float pe1=-1,pe2=-1,pdx=-1,pdy=-1,pdz=-1; static int ps1=-1,ps2=-1;
  if (e1!=pe1||e2!=pe2)        { sq->setExponents(e1,e2);   pe1=e1; pe2=e2; }
  if (dx!=pdx||dy!=pdy||dz!=pdz){ sq->setSize(dx,dy,dz);     pdx=dx; pdy=dy; pdz=dz; }
  if (s1!=ps1||s2!=ps2)        { sq->retessellate(s1,s2);    ps1=s1; ps2=s2; }
  // rotation rides on the node transform
  sq->setTransformation(create_hom_4x4<float>(rx, ry, rz));
  const bool grid = gui["grid"];
  sq->setPrimitiveVisible(PrimLine, grid);
  sq->setPrimitiveVisible(PrimQuad, !grid);
  scene.unlock();

  gui["draw"].render();
  static FPSLimiter limiter(25);
  limiter.wait();
}

int main(int n, char **ppc) {
  return ICLApplication(n, ppc, "", init, run).exec();
}
