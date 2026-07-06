// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/simplex-3D demo: visualises a 3D Simplex
// optimization — a goal coordinate frame, a moving one, and a trail of coloured
// tetrahedra as the simplex walks downhill toward the error minimum.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/math/fit/SimplexEngine.h>
#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/CoordinateFrameNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/scene/SceneMouseHandler.h>
#include <icl/cv3d/Camera.h>
#include <icl/viz3d/render/Material.h>
#include <icl/utils/Random.h>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene scene;

typedef FixedColVector<float,3> Pos;
Pos initPos(1000,1000,1000);
float error_function(const Pos &p) {
  return ::sqrt(sqr(p[0]+3) + sqr(p[1]-5) + sqr(p[2]-4));
}

void init() {
  gui << Canvas3D({.handle="draw", .minSize={20, 20}}) << Show();
  Camera cam;
  cam.setPosition(Vec(-611.637,-332.427,-814.748,1));
  cam.setNorm(Vec(0.331055,0.486567,0.808489,1));
  cam.setUp(Vec(0.893045,0.175791,-0.414207,1));
  scene.addCamera(cam);

  { auto f = CoordinateFrameNode::create(); f->translate(-3,5,4); scene.addNode(f); }              // goal
  { auto f = CoordinateFrameNode::create(); f->translate(initPos[0],initPos[1],initPos[2]); scene.addNode(f); }

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

static std::vector<Pos> createRandomSimplex(const Pos &p) {
  std::vector<Pos> simplex(4, p);
  for (int i = 0; i < 4; ++i)
    for (int d = 0; d < 3; ++d) simplex[i][d] += gaussRandom(0, 15);
  return simplex;
}

void run() {
  static SimplexEngine<float,Pos> opt(error_function, 3, 1);
  static std::vector<Pos> curr = createRandomSimplex(initPos);
  static float err = 10000;
  if (err > 0) {
    static int step = 0; ++step;
    GeomColor col((!(step%3))*255, (!((step+1)%3))*255, (!((step+2)%3))*255, 120);

    scene.lock();
    auto o = std::make_shared<MeshNode>();                 // a tetrahedron per step (trail)
    for (int i = 0; i < 4; ++i) o->addVertex(Vec(curr[i][0], curr[i][1], curr[i][2], 1));
    o->addTriangle(0,1,2); o->addTriangle(0,1,3); o->addTriangle(1,2,3); o->addTriangle(0,2,3);
    o->createAutoNormals();
    o->setMaterial(Material::fromColor(col));
    o->setPrimitiveVisible(PrimLine | PrimVertex, false);
    scene.addNode(o);
    scene.unlock();

    gui["draw"].render();

    auto r = opt.optimize(curr);
    curr = r.vertices;
    err = r.fx;
  }
  Thread::msleep(100);
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
