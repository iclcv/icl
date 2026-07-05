// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/scene-graph demo: a small "solar system" that
// exercises the scene graph — nested GroupNodes with per-node spin/orbit
// Drivers, lights anchored to a moving node (added as its children, so the
// renderer's light traversal picks them up), shift+click picking that drives a
// TextNode position indicator, and offscreen capture via Scene2::renderToImage.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/LightNode.h>
#include <icl/viz3d/TextNode.h>
#include <icl/viz3d/CoordinateFrameNode.h>
#include <icl/viz3d/Driver.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

HBox gui;
Scene2 scene;
Time lastTick;

// a flat ring of line segments at the given orbital radius
static std::shared_ptr<MeshNode> makeOrbit(float orbit, const GeomColor &c) {
  auto m = std::make_shared<MeshNode>();
  int i = 0;
  for (float a = 0; a < 2*M_PI; a += 0.01f, ++i)
    m->addVertex(Vec(orbit*std::cos(a), orbit*std::sin(a), 0, 1), c);
  for (int j = 1; j < i; ++j) m->addLine(j-1, j, c);
  m->setPrimitiveVisible(PrimVertex, false);
  return m;
}

// drives a planet: orbit the origin at `speed`, faithful to the legacy
// prepareForRendering (absolute transform from elapsed time).
class OrbitDriver : public Driver {
public:
  OrbitDriver(float orbit, float speed) : m_orbit(orbit), m_speed(speed) {}
  void sync(double dt, double) override {
    m_t += dt;
    if (auto *n = node()) {
      n->removeTransformation();
      n->translate(m_orbit, 0, 0);
      n->rotate(0, 0, m_t * m_speed);
    }
  }
private:
  float m_orbit, m_speed; double m_t = 0;
};

// a sphere on an orbit; children (moons/lights) ride along
static std::shared_ptr<GroupNode> makePlanet(float radius, const GeomColor &color,
                                             float orbit, float speed) {
  auto g = std::make_shared<GroupNode>();
  auto s = SphereNode::create(0, 0, 0, radius, 30, 30);
  s->setMaterial(Material::fromColor(color));
  s->setPrimitiveVisible(PrimVertex | PrimLine, false);
  g->addChild(s);
  g->addDriver<OrbitDriver>(orbit, speed);
  return g;
}

// shift/ctrl/alt + left-click picks a 3D point; the indicator jumps there and
// shows the camera-space coordinate. Otherwise forwards to camera navigation.
struct PositionIndicator : public GroupNode {
  std::shared_ptr<TextNode> label;
  PositionIndicator() {
    addChild(CoordinateFrameNode::create(60, 3, true));
    label = TextNode::create("pos is ...", 12, GeomColor(255,255,255,255));
    label->translate(0, 0, 120);
    addChild(label);
  }
  void update(const Vec &w, const Vec &c) {
    lock();
    removeTransformation();
    translate(w[0], w[1], w[2]);
    label->setText(str(c.transp()));
    unlock();
  }
} *pos = nullptr;

Scene2MouseHandler *sceneHandler = nullptr;
struct Handler : public MouseHandler {
  MouseResult process(const MouseEvent &evt) override {
    if (evt.isModifierActive(ShiftModifier) || evt.isModifierActive(AltModifier) ||
        evt.isModifierActive(ControlModifier)) {
      if (evt.isLeft() && evt.isPressEvent()) {
        scene.lock();
        Hit2 h = scene.findObject(0, evt.getX(), evt.getY());
        if (h) {
          Mat T = scene.getCamera(0).getCSTransformationMatrix();
          pos->update(h.pos, T * h.pos);
        }
        scene.unlock();
      }
    } else {
      sceneHandler->process(evt);
    }
    return MouseResult::Forward;
  }
};

void init() {
  gui << Canvas3D({.handle="view"})
      << Canvas({.handle="image"})
      << (VBox({.maxSize={14, 99}})
          << Combo("none,rgb,depth", {.handle="capture", .label="offscreen rendering"})
          << Combo("dist. to z0,dist to cam center", {.handle="dmode", .label="depth map mode"}))
      << Show();

  scene.addCamera(Camera(Vec(567,12,215,1),
                         Vec(-0.937548,-0.0012938,-0.347854,1),
                         Vec(0.333029,0.286923,-0.898202,1)));

  scene.addNode(makePlanet(40, GeomColor(150,150,50,255), 0, 0.5));
  scene.addNode(makePlanet(20, GeomColor(255,200,200,255), 300, 1));
  scene.addNode(makeOrbit(300, GeomColor(255,200,200,255)));
  scene.addNode(makePlanet(22, GeomColor(255,200,200,255), 240, 1.23));
  scene.addNode(makeOrbit(240, GeomColor(255,200,200,255)));

  auto p = makePlanet(22, GeomColor(255,10,10,255), 200, 2.0);
  p->addChild(makePlanet(10, GeomColor(255,10,60,255), 80, 3.5));
  p->addChild(makeOrbit(80, GeomColor(255,10,60,255)));
  p->addChild(makePlanet(8, GeomColor(255,60,10,255), 60, 2.1));
  p->addChild(makeOrbit(60, GeomColor(255,60,10,255)));
  p->addChild(makePlanet(8, GeomColor(255,60,255,255), 43, 1.84));
  p->addChild(makeOrbit(43, GeomColor(255,60,255,255)));
  // two reddish lights riding along with p (anchored = added as its children)
  for (int s : {60, -60}) {
    auto l = std::make_shared<LightNode>(LightNode::Point);
    l->setColor(GeomColor(1.0f, 0.4f, 0.4f, 1.0f));
    l->setIntensity(0.8f);
    if (s < 0) l->setShadowEnabled(true);
    l->translate(0, 0, s);
    p->addChild(l);
  }
  scene.addNode(p);
  scene.addNode(makeOrbit(200, GeomColor(10,244,200,255)));

  auto indicator = std::make_shared<PositionIndicator>();
  pos = indicator.get();
  scene.addNode(indicator);

  // a fill light
  auto key = std::make_shared<LightNode>(LightNode::Point);
  key->setColor(GeomColor(0.6f, 0.6f, 1.0f, 1.0f));
  key->translate(0, 0, 600);
  scene.addLight(key);

  scene.setBounds(600);
  scene.addNode(CoordinateFrameNode::create(400, 6));

  sceneHandler = scene.getMouseHandler(0);
  gui["view"].install(new Handler);
  gui["view"].link(scene.getGLCallback(0).get());

  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  scene.sync((now - lastTick).toSecondsDouble());
  lastTick = now;

  gui["view"].render();

  int capture = gui["capture"];
  if (capture) {
    BVH::DepthMode dm = gui["dmode"].as<int>() == 0 ? BVH::DistToCamPlane
                                                    : BVH::DistToCamCenter;
    BVH::ImageResult r = scene.renderToImage(0, dm);
    gui["image"] = capture == 1 ? Image(r.image) : Image(r.depth);
    gui["image"].render();
  }
}

int main(int n, char **argv) {
  return ICLApp(n, argv, "", init, run).exec();
}
