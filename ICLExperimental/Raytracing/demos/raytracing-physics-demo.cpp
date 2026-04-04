// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/Time.h>
#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/RigidCylinderObject.h>
#include <Raytracing/SceneRaytracer.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <random>
#include <deque>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;
using namespace icl::physics;

static PhysicsScene scene;
static std::unique_ptr<icl::rt::SceneRaytracer> raytracer;
GUI gui;

// Random generators
static std::mt19937 rng(42);
static std::uniform_real_distribution<float> randX(-180, 180);
static std::uniform_real_distribution<float> randZ(-100, 100);
static std::uniform_real_distribution<float> randSize(20, 70);
static std::uniform_real_distribution<float> randAspect(0.4f, 1.5f);
static std::uniform_int_distribution<int> randType(0, 2);
static std::uniform_int_distribution<int> randColor(60, 230);
static std::uniform_real_distribution<float> randAngle(0.0f, 6.283f);
static std::uniform_real_distribution<float> randRefl(0.0f, 0.35f);

// Table parameters
static constexpr float TABLE_Y = -30;
static constexpr float TABLE_THICKNESS = 20;
static constexpr float TABLE_BOTTOM = TABLE_Y - TABLE_THICKNESS / 2;
static constexpr float REMOVAL_Y = TABLE_BOTTOM - 20 * TABLE_THICKNESS;
static constexpr float SPAWN_HEIGHT = 500;

// Track dynamic objects
static std::deque<PhysicsObject *> dynamicObjects;

static GeomColor randomColor() {
  return GeomColor(randColor(rng), randColor(rng), randColor(rng), 255);
}

static void setupPhysicsBody(RigidObject *obj, const GeomColor &color, bool smooth) {
  obj->setVisible(Primitive::line, false);
  obj->setVisible(Primitive::vertex, false);
  obj->createAutoNormals(smooth);
  obj->setColor(Primitive::quad, color);
  obj->setColor(Primitive::triangle, color);
  obj->setReflectivity(randRefl(rng));
  obj->setFriction(0.7f);
  obj->setRestitution(0.25f);
  obj->setRollingFriction(0.1f);
  obj->setDamping(0.3f, 0.4f);  // reduce jitter on surfaces

  // Enable CCD to prevent tunneling through the table
  btRigidBody *body = obj->getRigidBody();
  if (body) {
    // Swept sphere radius: half the smallest object dimension
    body->setCcdMotionThreshold(1.0f);  // activate CCD when moving > 1 Bullet unit/step
    body->setCcdSweptSphereRadius(0.05f); // ~5mm in ICL units
  }
}

static void spawnObject() {
  float x = randX(rng);
  float z = randZ(rng);
  float sz = randSize(rng);
  float aspect = randAspect(rng);
  int type = randType(rng);
  GeomColor color = randomColor();

  // Random starting rotation
  float ax = randAngle(rng), ay = randAngle(rng), az = randAngle(rng);

  RigidObject *obj = nullptr;
  switch (type) {
    case 0: { // box with varied proportions
      float dx = sz, dy = sz * aspect, dz = sz / aspect;
      obj = new RigidBoxObject(x, SPAWN_HEIGHT, z, dx, dy, dz, 0.2f);
      setupPhysicsBody(obj, color, false);
      break;
    }
    case 1: { // sphere
      obj = new RigidSphereObject(x, SPAWN_HEIGHT, z, sz * 0.5f, 0.2f);
      setupPhysicsBody(obj, color, true);
      break;
    }
    case 2: { // cylinder with varied height
      float r = sz * 0.3f, h = sz * aspect;
      obj = new RigidCylinderObject(x, SPAWN_HEIGHT, z, r, h, 0.2f);
      setupPhysicsBody(obj, color, false);
      break;
    }
  }

  if (obj) {
    // Apply random rotation
    Mat rot = Mat::id();
    float ca = std::cos(ax), sa = std::sin(ax);
    float cb = std::cos(ay), sb = std::sin(ay);
    float cc = std::cos(az), sc = std::sin(az);
    // Euler angles XYZ
    rot(0,0) = cb*cc;           rot(1,0) = -cb*sc;          rot(2,0) = sb;
    rot(0,1) = sa*sb*cc+ca*sc;  rot(1,1) = -sa*sb*sc+ca*cc; rot(2,1) = -sa*cb;
    rot(0,2) = -ca*sb*cc+sa*sc; rot(1,2) = ca*sb*sc+sa*cc;  rot(2,2) = ca*cb;
    // Keep translation from physics
    Mat t = obj->getTransformation();
    rot(3,0) = t(3,0); rot(3,1) = t(3,1); rot(3,2) = t(3,2);
    obj->setTransformation(rot);

    scene.addObject(obj, true);
    dynamicObjects.push_back(obj);
  }
}

static void removeObjectsBelowThreshold() {
  // Check all objects, not just front — any could have fallen
  auto it = dynamicObjects.begin();
  while (it != dynamicObjects.end()) {
    PhysicsObject *obj = *it;
    Mat t = obj->getTransformation();
    float y = t(3, 1);
    if (y < REMOVAL_Y) {
      scene.removeObject(obj);
      it = dynamicObjects.erase(it);
    } else {
      ++it;
    }
  }
}

void init() {
  Camera cam(Vec(0, 500, 900, 1),
             Vec(0, -0.5, -1, 1),
             Vec(0, 1, 0, 1));
  cam.setResolution(Size(800, 600));
  scene.addCamera(cam);

  // Gravity: Y-down
  scene.setGravity(Vec(0, -9810, 0, 0));

  // Static table (mass = 0)
  auto *table = new RigidBoxObject(0, TABLE_Y, 0, 500, TABLE_THICKNESS, 350, 0);
  table->setVisible(Primitive::line, false);
  table->setVisible(Primitive::vertex, false);
  table->createAutoNormals(false);
  table->setColor(Primitive::quad, GeomColor(140, 100, 60, 255));
  table->setReflectivity(0.1f);
  table->setFriction(0.8f);
  table->setRestitution(0.15f);
  scene.addObject(table, true);

  // Lights
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(200, 500, 200, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 245, 220, 255));
  scene.getLight(0).setAmbient(GeomColor(35, 33, 30, 255));
  scene.getLight(0).setSpecular(GeomColor(255, 245, 220, 255));
  scene.getLight(0).setSpecularEnabled(true);

  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(-300, 300, -100, 1));
  scene.getLight(1).setDiffuse(GeomColor(60, 70, 100, 255));
  scene.getLight(1).setAmbient(GeomColor(10, 10, 15, 255));

  // Initial objects
  for (int i = 0; i < 5; i++) spawnObject();

  raytracer = std::make_unique<icl::rt::SceneRaytracer>(scene);

  gui << Canvas().handle("draw").minSize(40, 30)
      << (VBox()
          << Combo("!1x (off),4x (2x2),9x (3x3),16x (4x4)").handle("aa").label("MSAA (ray-based)")
          << CheckBox("FXAA post-process", "checked").handle("fxaa")
          << Slider(10, 120, 30).handle("spawnRate").label("spawn interval (frames)")
          << CheckBox("pause physics", "unchecked").handle("pause")
          << Button("spawn 10").handle("burst")
          << Label("--").handle("info")
         )
      << Show();

  gui["draw"].install(scene.getMouseHandler(0));
}

static int frameCount = 0;

void run() {
  // Read GUI controls
  static const int aaValues[] = {1, 4, 9, 16};
  int aaIdx = gui["aa"].as<ComboHandle>().getSelectedIndex();
  raytracer->setAASamples(aaValues[aaIdx]);
  raytracer->setFXAA(gui["fxaa"].as<bool>());

  int spawnRate = gui["spawnRate"].as<int>();
  bool paused = gui["pause"].as<bool>();

  if (!paused) {
    scene.step();
  }

  // Sync physics → scene
  for (int i = 0; i < scene.getObjectCount(); i++) {
    scene.getObject(i)->prepareForRendering();
  }

  frameCount++;
  if (frameCount % spawnRate == 0) spawnObject();

  // Burst spawn
  if (gui["burst"].as<ButtonHandle>().wasTriggered()) {
    for (int i = 0; i < 10; i++) spawnObject();
  }

  removeObjectsBelowThreshold();

  raytracer->invalidateAll();

  Time t = Time::now();
  raytracer->render(0);
  float ms = (float)(Time::now() - t).toMilliSeconds();

  const Img8u &img = raytracer->getImage();
  DrawHandle draw = gui["draw"];
  draw = img;

  static char buf[128];
  snprintf(buf, sizeof(buf), "%s | %.1f ms (%.0f fps) | %d objects | %dx AA",
           raytracer->backendName(), ms, 1000.0f / ms,
           (int)dynamicObjects.size(), aaValues[aaIdx]);
  draw->text(buf, 10, 20, 10);
  draw->render();

  gui["info"] = std::string(buf);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
