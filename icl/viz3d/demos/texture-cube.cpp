// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

// viz3d port of the legacy geom/texture-cube demo: a cube textured with the
// same image on all six faces, lit by three orbiting colored point lights,
// inside a starry background sphere. Shows native viz3d texturing (a single
// baseColorMap + per-face UVs) and LightNodes animated by Drivers.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/LightNode.h>
#include <icl/viz3d/Driver.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;
Time lastTick;

// orbit a light/node: absolute transform recomputed each frame from elapsed
// time (faithful to the legacy prepareForRendering motion).
class OrbitDriver : public Driver {
public:
  OrbitDriver(int idx) : m_idx(idx) {}
  void sync(double dt, double) override {
    m_t += dt;
    if (auto *n = node()) {
      n->removeTransformation();
      n->translate((m_idx == 0) * 8.f, (m_idx == 1) * 8.f, (m_idx == 2) * 8.f);
      n->rotate((m_idx == 2) * m_t, (m_idx == 0) * m_t, (m_idx == 1) * m_t);
    }
  }
private:
  int m_idx; double m_t = 0;
};

// a cube of half-extent h, every face UV-mapped to the full texture
static std::shared_ptr<MeshNode> makeTexturedCube(float h, const Image &tex) {
  auto m = std::make_shared<MeshNode>();
  const float c[8][3] = {{-h,-h,-h},{ h,-h,-h},{ h, h,-h},{-h, h,-h},
                         {-h,-h, h},{ h,-h, h},{ h, h, h},{-h, h, h}};
  for (auto &p : c) m->addVertex(Vec(p[0], p[1], p[2], 1));
  const float nrm[6][3] = {{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}};
  for (auto &nn : nrm) m->addNormal(Vec(nn[0], nn[1], nn[2], 1));
  m->addTexCoord(0, 0); m->addTexCoord(1, 0);    // uv corners: 0..3
  m->addTexCoord(1, 1); m->addTexCoord(0, 1);
  const int face[6][4] = {{4,5,6,7},{1,0,3,2},{5,1,2,6},
                          {0,4,7,3},{7,6,2,3},{0,1,5,4}};
  for (int f = 0; f < 6; ++f) {
    const int *q = face[f];
    m->addQuad(q[0], q[1], q[2], q[3], f,f,f,f, 0,1,2,3);
  }
  auto mat = Material::fromColor(GeomColor(255, 255, 255, 255));
  mat->setBaseColorMap(tex);
  m->setMaterial(mat);
  m->setPrimitiveVisible(PrimLine | PrimVertex, false);
  return m;
}

void init() {
  gui << Canvas3D({.handle="draw", .label="scene view", .minSize={16, 12}}) << Show();

  scene.addCamera(Camera(Vec(0,-10,-10), Vec(0,0.707,0.707), Vec(1,0,0)));

  Image tex = icl::qt::scale(create("lena"), 300, 300);
  scene.addNode(makeTexturedCube(7, tex));

  const GeomColor lc[3] = {GeomColor(1,0,0,1), GeomColor(0,1,0,1), GeomColor(0,0,1,1)};
  for (int i = 0; i < 3; ++i) {
    auto l = std::make_shared<LightNode>(LightNode::Point);
    l->setColor(lc[i]);
    l->setIntensity(1.0f);
    l->addDriver<OrbitDriver>(i);
    scene.addLight(l);
  }

  // starry background sphere: blue surface speckled with white points
  auto bg = std::make_shared<SphereNode>(0, 0, 0, 40, 40, 40, 40, 100);
  auto bgMat = Material::fromColor(GeomColor(0, 0, 100, 255));
  bgMat->pointColor = GeomColor(1, 1, 1, 1);
  bg->setMaterial(bgMat);
  bg->setPointSize(3);
  bg->setPrimitiveVisible(PrimVertex, true);
  bg->setPrimitiveVisible(PrimLine, false);
  scene.addNode(bg);

  scene.setBounds(80);
  gui["draw"].install(scene.getMouseHandler(0));
  gui["draw"].link(scene.getGLCallback(0).get());

  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  scene.sync((now - lastTick).toSecondsDouble());
  lastTick = now;
  Thread::msleep(20);
  gui["draw"].render();
}

int main(int n, char **ppc) {
  return ICLApplication(n, ppc, "", init, run).exec();
}
