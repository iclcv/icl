// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Material.h>
#include <ICLUtils/Time.h>
#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/RigidCylinderObject.h>
#include <Raytracing/CyclesRenderer.h>
#include <ICLIO/FileWriter.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <random>
#include <deque>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;
using namespace icl::physics;

static PhysicsScene scene;
static std::unique_ptr<icl::rt::CyclesRenderer> renderer;
HSplit gui;

// Random generators
static std::mt19937 rng(42);
static std::uniform_real_distribution<float> randXY(-180, 180);
static std::uniform_real_distribution<float> randSize(20, 70);
static std::uniform_real_distribution<float> randAspect(0.4f, 1.5f);
static std::uniform_int_distribution<int> randType(0, 2);
static std::uniform_int_distribution<int> randColor(60, 230);
static std::uniform_real_distribution<float> randAngle(0.0f, 6.283f);
static std::uniform_real_distribution<float> randUnit(0.0f, 1.0f);

// Table parameters
static constexpr float TABLE_Z = -30;
static constexpr float TABLE_THICKNESS = 20;
static constexpr float REMOVAL_Z = TABLE_Z - TABLE_THICKNESS / 2 - 20 * TABLE_THICKNESS;
static constexpr float SPAWN_HEIGHT = 500;

static std::deque<PhysicsObject *> dynamicObjects;

static GeomColor randomColor() {
  return GeomColor(randColor(rng), randColor(rng), randColor(rng), 255);
}

static void setupPhysicsBody(RigidObject *obj, const GeomColor &color, bool smooth,
                             bool skipNormals = false) {
  obj->setVisible(Primitive::line, false);
  obj->setVisible(Primitive::vertex, false);
  if (!skipNormals) obj->createAutoNormals(smooth);

  auto mat = std::make_shared<Material>();
  mat->baseColor = color * (1.0f / 255.0f);

  float roll = randUnit(rng);
  if (roll < 0.4f) {
    mat->metallic = 0.0f;
    mat->roughness = 0.2f + randUnit(rng) * 0.5f;
  } else if (roll < 0.7f) {
    mat->metallic = 0.8f + randUnit(rng) * 0.2f;
    mat->roughness = 0.1f + randUnit(rng) * 0.6f;
  } else if (roll < 0.9f) {
    mat->metallic = 0.0f;
    mat->roughness = 0.7f + randUnit(rng) * 0.3f;
  } else {
    mat->metallic = 1.0f;
    mat->roughness = 0.02f;
    mat->baseColor = GeomColor(0.95f, 0.95f, 0.97f, 1.0f);
  }

  obj->setMaterial(mat);
  obj->setFriction(0.5f);
  obj->setRestitution(0.35f);
  obj->setRollingFriction(0.02f);
  obj->setDamping(0.2f, 0.08f);

  btRigidBody *body = obj->getRigidBody();
  if (body) {
    body->setCcdMotionThreshold(1.0f);
    body->setCcdSweptSphereRadius(0.05f);
    float w = (randUnit(rng) - 0.5f) * 4.0f;
    body->setAngularVelocity(btVector3(w, w, w));
  }
}

static void spawnObject() {
  float x = randXY(rng);
  float y = randXY(rng);
  float sz = randSize(rng);
  float aspect = randAspect(rng);
  int type = randType(rng);
  GeomColor color = randomColor();

  RigidObject *obj = nullptr;
  switch (type) {
    case 0: obj = new RigidBoxObject(x, y, SPAWN_HEIGHT, sz, sz*aspect, sz/aspect, 0.2f);
            setupPhysicsBody(obj, color, false); break;
    case 1: obj = new RigidSphereObject(x, y, SPAWN_HEIGHT, sz*0.5f, 0.2f);
            setupPhysicsBody(obj, color, true); break;
    case 2: obj = new RigidCylinderObject(x, y, SPAWN_HEIGHT, sz*0.3f, sz*aspect, 0.2f);
            setupPhysicsBody(obj, color, false, true); break;
  }

  if (obj) {
    float ax = randAngle(rng), ay = randAngle(rng), az = randAngle(rng);
    Mat rot = Mat::id();
    float ca = std::cos(ax), sa = std::sin(ax);
    float cb = std::cos(ay), sb = std::sin(ay);
    float cc = std::cos(az), sc = std::sin(az);
    rot(0,0) = cb*cc;           rot(1,0) = -cb*sc;          rot(2,0) = sb;
    rot(0,1) = sa*sb*cc+ca*sc;  rot(1,1) = -sa*sb*sc+ca*cc; rot(2,1) = -sa*cb;
    rot(0,2) = -ca*sb*cc+sa*sc; rot(1,2) = ca*sb*sc+sa*cc;  rot(2,2) = ca*cb;
    Mat t = obj->getTransformation();
    rot(3,0) = t(3,0); rot(3,1) = t(3,1); rot(3,2) = t(3,2);
    obj->setTransformation(rot);

    scene.addObject(obj, true);
    dynamicObjects.push_back(obj);
  }
}

static bool removeObjectsBelowThreshold() {
  bool removed = false;
  auto it = dynamicObjects.begin();
  while (it != dynamicObjects.end()) {
    PhysicsObject *obj = *it;
    Mat t = obj->getTransformation();
    if (t(3, 2) < REMOVAL_Z) {
      scene.removeObject(obj);
      it = dynamicObjects.erase(it);
      removed = true;
    } else {
      ++it;
    }
  }
  return removed;
}

void init() {
  // Camera: Z-up scene, looking at the table from above-right
  scene.addCamera(Camera::lookAt(
      Vec(600, -500, 400, 1),     // position
      Vec(0, 0, TABLE_Z, 1),     // look at table center
      Vec(0, 0, 1, 1),            // Z-up
      pa("-size"),                 // resolution
      55.0f));                     // 55° horizontal FOV

  // Table
  auto *table = new RigidBoxObject(0, 0, TABLE_Z, 500, 350, TABLE_THICKNESS, 0);
  table->setVisible(Primitive::line, false);
  table->setVisible(Primitive::vertex, false);
  table->createAutoNormals(false);
  {
    auto mat = std::make_shared<Material>();
    mat->baseColor = GeomColor(0.55f, 0.39f, 0.24f, 1.0f);
    mat->roughness = 0.7f;
    table->setMaterial(mat);
  }
  table->setFriction(0.8f);
  table->setRestitution(0.15f);
  scene.addObject(table, true);

  // Lights
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(200, -200, 500, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 245, 220, 255));

  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(-300, 200, 300, 1));
  scene.getLight(1).setDiffuse(GeomColor(60, 70, 100, 255));

  // Material archetype spheres on the table
  auto addArchetype = [&](float x, float y, float radius, std::shared_ptr<Material> mat) {
    auto *obj = new RigidSphereObject(x, y, TABLE_Z + radius + 5, radius, 0);
    obj->setVisible(Primitive::line, false);
    obj->setVisible(Primitive::vertex, false);
    obj->createAutoNormals(true);
    obj->setMaterial(mat);
    scene.addObject(obj, true);
  };

  float sphereR = 25, spacing = 70;
  float startX = -2.5f * spacing;
  float y = -30;

  auto gold = std::make_shared<Material>();
  gold->baseColor = GeomColor(1.0f, 0.76f, 0.34f, 1.0f);
  gold->metallic = 1.0f; gold->roughness = 0.2f;
  addArchetype(startX, y, sphereR, gold);

  auto copper = std::make_shared<Material>();
  copper->baseColor = GeomColor(0.95f, 0.64f, 0.54f, 1.0f);
  copper->metallic = 1.0f; copper->roughness = 0.4f;
  addArchetype(startX + spacing, y, sphereR, copper);

  auto plastic = std::make_shared<Material>();
  plastic->baseColor = GeomColor(0.9f, 0.9f, 0.9f, 1.0f);
  plastic->roughness = 0.3f;
  addArchetype(startX + 2*spacing, y, sphereR, plastic);

  auto rubber = std::make_shared<Material>();
  rubber->baseColor = GeomColor(0.2f, 0.6f, 0.15f, 1.0f);
  rubber->roughness = 0.95f;
  addArchetype(startX + 3*spacing, y, sphereR, rubber);

  auto mirror = std::make_shared<Material>();
  mirror->baseColor = GeomColor(0.95f, 0.95f, 0.97f, 1.0f);
  mirror->metallic = 1.0f; mirror->roughness = 0.02f;
  addArchetype(startX + 4*spacing, y, sphereR, mirror);

  auto emissive = std::make_shared<Material>();
  emissive->baseColor = GeomColor(1.0f, 0.9f, 0.6f, 1.0f);
  emissive->emissive = GeomColor(1.5f, 1.2f, 0.7f, 1.0f);
  emissive->roughness = 0.8f;
  addArchetype(startX + 5*spacing, y, sphereR, emissive);

  // Create renderer
  renderer = std::make_unique<icl::rt::CyclesRenderer>(
      scene, icl::rt::RenderQuality::Preview);
  renderer->setSceneScale(1.0f);

  // GUI with Cycles-specific quality controls
  gui << Canvas().handle("draw").minSize(64, 48)
      << (VBox().minSize(20, 24)
         << CheckBox("pause physics", "unchecked").handle("pause")
         << Slider(10, 120, 30).handle("spawnRate").label("spawn interval")
         << Button("spawn 10").handle("burst")
         << Fps(30).handle("fps")
         << (HBox()
            << Combo("!Preview,Interactive,Final").handle("quality").label("Quality"))
         << Combo("!1,2,4,8,16").handle("initSamples").label("Initial Samples")
         << Combo("1,2,4,8,16,32,64,!128,256,512,1024,2048,4096").handle("maxIter").label("Max Iterations")
         << Slider(1, 16, 2).handle("bounces").label("Max Bounces")
         << CheckBox("Denoising OIDN", "unchecked").handle("denoising")
         << Slider(10, 500, 100).handle("exposure").label("Exposure %")
         << Label("--").handle("info")
       ) << Show();

  gui["draw"].install(scene.getMouseHandler(0));
}

static int frameCount = 0;

void run() {
  // Apply GUI settings to renderer
  static int lastQuality = -1;
  int qualityIdx = gui["quality"].as<ComboHandle>().getSelectedIndex();
  if (qualityIdx != lastQuality) {
    static const icl::rt::RenderQuality qualities[] = {
      icl::rt::RenderQuality::Preview,
      icl::rt::RenderQuality::Interactive,
      icl::rt::RenderQuality::Final
    };
    renderer->setQuality(qualities[qualityIdx]);
    lastQuality = qualityIdx;
  }

  int initSamples = std::atoi(gui["initSamples"].as<ComboHandle>().getSelectedItem().c_str());
  int maxIter = std::atoi(gui["maxIter"].as<ComboHandle>().getSelectedItem().c_str());
  int bounces = gui["bounces"].as<int>();
  bool denoising = gui["denoising"].as<bool>();
  float exposure = gui["exposure"].as<int>() / 100.0f;

  renderer->setInitialSamples(initSamples);
  renderer->setSamples(maxIter);
  renderer->setMaxBounces(bounces);
  renderer->setDenoising(denoising);
  renderer->setExposure(exposure);

  bool paused = gui["pause"].as<bool>();
  bool geometryChanged = false;

  if (!paused) {
    scene.step();
    frameCount++;

    int spawnRate = gui["spawnRate"].as<int>();
    if (frameCount % spawnRate == 0) {
      spawnObject();
      geometryChanged = true;
    }
    if (gui["burst"].as<ButtonHandle>().wasTriggered()) {
      for (int i = 0; i < 10; i++) spawnObject();
      geometryChanged = true;
    }
    if (removeObjectsBelowThreshold())
      geometryChanged = true;
  }

  // Prepare objects for rendering
  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();

  // Invalidate based on what changed
  if (geometryChanged) {
    renderer->invalidateAll();
  } else if (!paused) {
    renderer->invalidateTransforms();
  }

  Time renderStart = Time::now();
  renderer->render(0);
  float ms = (float)(Time::now() - renderStart).toMilliSeconds();

  const auto &img = renderer->getImage();
  DrawHandle draw = gui["draw"];
  if (img.getWidth() > 0 && img.getHeight() > 0) {
    draw = img;
  }

  char buf[256];
  int progress = (int)(renderer->getProgress() * 100);
  int updates = renderer->getUpdateCount();
  snprintf(buf, sizeof(buf), "%d obj | %d%% (%d updates) | %d max | %d bounces%s",
           (int)dynamicObjects.size(), progress, updates, maxIter, bounces,
           denoising ? " | OIDN" : "");
  draw->text(buf, 10, 20, 10);
  draw->render();

  gui["info"] = std::string(buf);
  gui["fps"].render();
}

static void offscreen_render(const std::string &output) {
  // Same scene setup as init() but without GUI
  scene.addCamera(Camera::lookAt(
      Vec(600, -500, 400, 1),
      Vec(0, 0, TABLE_Z, 1),
      Vec(0, 0, 1, 1),
      pa("-size"),
      55.0f));

  auto *table = new RigidBoxObject(0, 0, TABLE_Z, 500, 350, TABLE_THICKNESS, 0);
  table->setVisible(Primitive::line, false);
  table->setVisible(Primitive::vertex, false);
  table->createAutoNormals(false);
  {
    auto mat = std::make_shared<Material>();
    mat->baseColor = GeomColor(0.55f, 0.39f, 0.24f, 1.0f);
    mat->roughness = 0.7f;
    table->setMaterial(mat);
  }
  scene.addObject(table, true);

  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(200, -200, 500, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 245, 220, 255));

  // Spawn objects and run physics so they fall onto the table
  for (int i = 0; i < 10; i++) spawnObject();
  for (int i = 0; i < 300; i++) {
    scene.step(1.0f / 60.0f, 1);
    removeObjectsBelowThreshold();
  }
  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();

  printf("Rendering %d objects to %s...\n", scene.getObjectCount(), output.c_str());

  icl::rt::CyclesRenderer renderer(scene, icl::rt::RenderQuality::Final);
  renderer.setSceneScale(1.0f);
  renderer.setSamples(pa("-samples").as<int>());
  renderer.render(0);

  const auto &img = renderer.getImage();
  if (img.getWidth() > 0) {
    icl::io::FileWriter writer(output);
    writer.write(&img);
    printf("Saved %dx%d image to %s\n", img.getWidth(), img.getHeight(), output.c_str());
  } else {
    fprintf(stderr, "ERROR: No image produced\n");
  }
}

int main(int argc, char **argv) {
  // Check for -offscreen before pa_init, since ICLApp also calls pa_init
  // and it must not be called twice.
  bool offscreen = false;
  std::string offscreenFile;
  for (int i = 1; i < argc; ++i) {
    if (std::string("-offscreen") == argv[i] && i + 1 < argc) {
      offscreen = true;
      offscreenFile = argv[++i];
    }
  }

  if (offscreen) {
    pa_init(argc, argv, "-size(Size=800x600) -offscreen(string) -samples(int=64)");
    offscreen_render(offscreenFile);
    return 0;
  }

  return ICLApp(argc, argv, "-size(Size=800x600) -samples(int=4)", init, run).exec();
}
