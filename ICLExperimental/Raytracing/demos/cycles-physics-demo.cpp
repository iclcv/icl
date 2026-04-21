// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Material.h>
#include <ICLGeom/Hit.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/RigidCylinderObject.h>
#include <Raytracing/CyclesRenderer.h>
#include <ICLIO/FileWriter.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

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

// Track mouse press position to distinguish clicks from drags
static Point32f pressPos(-1, -1);
static bool pressPending = false;

static void handleMouse(const MouseEvent &evt) {
  // Forward all events to the scene's camera handler
  scene.getMouseHandler(0)->process(evt);

  // Track press/release to detect click (no drag)
  if (evt.isPressEvent() && evt.isMiddle()) {
    pressPos = evt.getPos();
    pressPending = true;
  }
  if (evt.isReleaseEvent() && evt.isMiddle() && pressPending) {
    pressPending = false;
    Point32f delta = evt.getPos() - pressPos;
    if (std::abs(delta.x) < 5 && std::abs(delta.y) < 5) {
      // Middle-click without drag → toggle emissive
      Hit hit = scene.findObject(0, evt.getX(), evt.getY());
      if (hit) {
        auto mat = hit.obj->getMaterial();
        if (!mat) {
          mat = std::make_shared<Material>();
          hit.obj->setMaterial(mat);
        }
        float emSum = mat->emissive[0] + mat->emissive[1] + mat->emissive[2];
        if (emSum > 0.01f) {
          mat->emissive = GeomColor(0, 0, 0, 1);
        } else {
          mat->emissive = GeomColor(mat->baseColor[0] * 2.0f,
                                     mat->baseColor[1] * 2.0f,
                                     mat->baseColor[2] * 2.0f, 1.0f);
        }
        if (renderer) renderer->invalidateAll();
      }
    }
  }
  if (evt.isDragEvent()) pressPending = false;
}

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

// Objects waiting to be activated (spawned frozen, activated after render catches up)
struct PendingObject {
  RigidObject *obj;
  float mass;
  Time activateTime;
};
static std::deque<PendingObject> pendingObjects;

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
  float mass = 0.2f;

  // Spawn with real mass but frozen (linearFactor=0, angularFactor=0).
  // After a few frames (enough for reset+render to show the object),
  // unfreeze so it starts falling visibly.
  RigidObject *obj = nullptr;
  switch (type) {
    case 0: obj = new RigidBoxObject(x, y, SPAWN_HEIGHT, sz, sz*aspect, sz/aspect, mass);
            setupPhysicsBody(obj, color, false); break;
    case 1: obj = new RigidSphereObject(x, y, SPAWN_HEIGHT, sz*0.5f, mass);
            setupPhysicsBody(obj, color, true); break;
    case 2: obj = new RigidCylinderObject(x, y, SPAWN_HEIGHT, sz*0.3f, sz*aspect, mass);
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

    // Freeze the body so it hovers at spawn height
    btRigidBody *body = obj->getRigidBody();
    if (body) {
      body->setLinearFactor(btVector3(0, 0, 0));
      body->setAngularFactor(btVector3(0, 0, 0));
      body->setLinearVelocity(btVector3(0, 0, 0));
      body->setAngularVelocity(btVector3(0, 0, 0));
    }

    scene.addObject(obj, true);
    dynamicObjects.push_back(obj);
    // Queue for unfreeze after 1 second, giving the renderer time
    // to show the object before it falls.
    pendingObjects.push_back({obj, mass, Time::now() + Time(1000000)});
  }
}

static void activatePendingObjects() {
  Time now = Time::now();
  for (auto &p : pendingObjects) {
    if (p.activateTime.toMicroSeconds() > 0 && now >= p.activateTime) {
      p.activateTime = Time(0);  // mark as activated
      btRigidBody *body = p.obj->getRigidBody();
      if (body) {
        // Unfreeze — restore full motion
        body->setLinearFactor(btVector3(1, 1, 1));
        body->setAngularFactor(btVector3(1, 1, 1));
        body->activate(true);
        float w = (randUnit(rng) - 0.5f) * 4.0f;
        body->setAngularVelocity(btVector3(w, w, w));
      }
    }
  }
  // Remove activated entries
  while (!pendingObjects.empty() && pendingObjects.front().activateTime.toMicroSeconds() == 0)
    pendingObjects.pop_front();
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

  // Two mirroring boxes, slightly misaligned — inter-reflections show bounce count
  {
    auto mirrorMat = std::make_shared<Material>();
    mirrorMat->baseColor = GeomColor(0.97f, 0.97f, 0.99f, 1.0f);
    mirrorMat->metallic = 1.0f;
    mirrorMat->roughness = 0.01f;

    float boxH = 120, boxW = 150, boxD = 10;
    float cx = 0, cy = 80;  // centered on table

    // Left mirror — angled slightly inward
    auto *leftBox = new RigidBoxObject(cx - 50, cy, TABLE_Z + boxH/2 + 5,
                                        boxD, boxW, boxH, 0);
    leftBox->setVisible(Primitive::line, false);
    leftBox->setVisible(Primitive::vertex, false);
    leftBox->createAutoNormals(false);
    leftBox->setMaterial(mirrorMat);
    // Rotate ~-10° around Z to angle inward
    Mat rot = Mat::id();
    float a = -0.17f; // ~-10 degrees
    rot(0,0) = std::cos(a);  rot(1,0) = -std::sin(a);
    rot(0,1) = std::sin(a);  rot(1,1) = std::cos(a);
    Mat t = leftBox->getTransformation();
    rot(3,0) = t(3,0); rot(3,1) = t(3,1); rot(3,2) = t(3,2);
    leftBox->setTransformation(rot);
    scene.addObject(leftBox, true);

    // Right mirror — angled slightly inward (opposite direction)
    auto *rightBox = new RigidBoxObject(cx + 50, cy, TABLE_Z + boxH/2 + 5,
                                         boxD, boxW, boxH, 0);
    rightBox->setVisible(Primitive::line, false);
    rightBox->setVisible(Primitive::vertex, false);
    rightBox->createAutoNormals(false);
    rightBox->setMaterial(mirrorMat);
    rot = Mat::id();
    a = 0.17f;
    rot(0,0) = std::cos(a);  rot(1,0) = -std::sin(a);
    rot(0,1) = std::sin(a);  rot(1,1) = std::cos(a);
    t = rightBox->getTransformation();
    rot(3,0) = t(3,0); rot(3,1) = t(3,1); rot(3,2) = t(3,2);
    rightBox->setTransformation(rot);
    scene.addObject(rightBox, true);
  }

  // Create renderer
  renderer = std::make_unique<icl::rt::CyclesRenderer>(
      scene, icl::rt::RenderQuality::Preview);
  renderer->setSceneScale(1.0f);

  // GUI: Cycles view + OpenGL reference view + controls
  gui << (VSplit()
         << (HSplit()
            << Canvas().handle("draw").minSize(32, 24).label("Cycles")
            << Canvas3D().handle("gl").minSize(32, 24).label("OpenGL"))
         << (HBox()
            << CheckBox("pause physics", "unchecked").handle("pause").minSize(8,2)
            << Slider(5, 500, 50).handle("spawnRate").label("spawn ms").minSize(10,2)
            << Button("spawn 10").handle("burst").minSize(6,2)
            << Label("--").handle("renderFps").minSize(12,2))
         << (HBox()
            << Combo("!Preview,Interactive,Final").handle("quality").label("Quality").minSize(8,2)
            << Combo("!1,2,4,8,16").handle("initSamples").label("Steps/Frame").minSize(8,2)
            << Combo("1,2,4,8,16,32,64,!128,256,512,1024,2048,4096").handle("maxIter").label("Max Iter").minSize(8,2)
            << Slider(1, 16, 2).handle("bounces").label("Max Bounces").minSize(10,2)
            << CheckBox("OIDN", "unchecked").handle("denoising").minSize(6,2))
         << (HBox()
            << Slider(10, 500, 100).handle("exposure").label("Exposure %").minSize(10,2)
            << Slider(0, 100, 100).handle("brightness").label("Light/BG %").minSize(10,2)
            << Slider(10, 100, 100).handle("resScale").label("Resolution %").minSize(10,2)
            << Label("--").handle("info").minSize(12,2))
       ) << Show();

  // Unified mouse handler: camera control (forwarded to scene) + middle-click picking.
  gui["draw"].install(new MouseHandler(handleMouse));
  gui["gl"].install(new MouseHandler(handleMouse));
}

static int frameCount = 0;
static Time lastSpawnTime = Time::now();
static Time lastPhysicsTime = Time::now();

void run() {
  // FPS limiter disabled — let the render pipeline pace the loop.
  // static FPSLimiter fpsLimit(30);
  // fpsLimit.wait();

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

  float brightness = gui["brightness"].as<int>() / 100.0f;

  renderer->setInitialSamples(initSamples);
  renderer->setSamples(maxIter);
  renderer->setMaxBounces(bounces);
  renderer->setDenoising(denoising);
  renderer->setExposure(exposure);
  renderer->setBrightness(brightness);
  renderer->setResolutionScale(gui["resScale"].as<int>() / 100.0f);

  bool paused = gui["pause"].as<bool>();
  bool geometryChanged = false;

  if (!paused) {
    // Fixed physics timestep (1/60s) regardless of frame rate
    Time now = Time::now();
    float dtSec = (float)(now - lastPhysicsTime).toSecondsDouble();
    lastPhysicsTime = now;
    float fixedStep = 1.0f / 60.0f;
    scene.step(std::min(dtSec, 0.1f), 4, fixedStep);
    activatePendingObjects();
    frameCount++;

    // Time-based spawn: spawnRate slider = spawn interval in ms
    float spawnInterval = gui["spawnRate"].as<int>() / 1000.0f;  // seconds
    if ((float)(now - lastSpawnTime).toSecondsDouble() >= spawnInterval) {
      lastSpawnTime = now;
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

  // Invalidate transforms when physics is running. The sync automatically
  // detects new/removed objects — no need for invalidateAll().
  if (!paused || geometryChanged) {
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

  // Track render FPS based on tile updates
  static int lastUpdateCount = 0;
  static Time lastFpsTime = Time::now();
  static float renderFps = 0;
  int updates = renderer->getUpdateCount();
  Time now2 = Time::now();
  float dtFps = (float)(now2 - lastFpsTime).toSecondsDouble();
  if (dtFps >= 0.5f) {  // update every 0.5s
    renderFps = (updates - lastUpdateCount) / dtFps;
    lastUpdateCount = updates;
    lastFpsTime = now2;
  }

  char buf[256];
  snprintf(buf, sizeof(buf), "%d obj | %.1f render fps | A=%d B=%d",
           (int)dynamicObjects.size(), renderFps, initSamples, maxIter);
  draw->text(buf, 10, 20, 10);
  draw->render();

  gui["renderFps"] = std::string(buf);

  // OpenGL reference view — same scene, same camera
  DrawHandle3D gl = gui["gl"];
  gl->link(scene.getGLCallback(0));
  gl.render();

  gui["info"] = std::string(buf);
}

static void saveImage(icl::rt::CyclesRenderer &r, const std::string &path) {
  const auto &img = r.getImage();
  if (img.getWidth() > 0) {
    icl::io::FileWriter writer(path);
    writer.write(&img);
    fprintf(stderr, "  Saved %dx%d → %s\n", img.getWidth(), img.getHeight(), path.c_str());
  } else {
    fprintf(stderr, "  ERROR: No image produced\n");
  }
}

static void offscreen_render(const std::string &output) {
  using namespace icl::rt;
  int samples = pa("-samples").as<int>();

  scene.addCamera(Camera::lookAt(
      Vec(600, -500, 400, 1),
      Vec(0, 0, TABLE_Z, 1),
      Vec(0, 0, 1, 1),
      pa("-size"),
      55.0f));

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
  scene.addObject(table, true);

  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(200, -200, 500, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 245, 220, 255));

  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();

  // --- Test 1: Initial scene (table only) ---
  fprintf(stderr, "\n=== Test 1: Table only (%d objects, %d spp) ===\n",
          scene.getObjectCount(), samples);

  CyclesRenderer renderer(scene, RenderQuality::Final);
  renderer.setSceneScale(1.0f);
  renderer.setSamples(samples);

  Time t0 = Time::now();
  renderer.renderBlocking(0);
  float ms1 = (float)(Time::now() - t0).toMilliSeconds();
  fprintf(stderr, "  Render: %.0fms\n", ms1);
  saveImage(renderer, output + "_1_table.png");

  // --- Test 2: Add 5 objects, run physics briefly ---
  fprintf(stderr, "\n=== Test 2: Add 5 objects + 60 physics steps ===\n");
  for (int i = 0; i < 5; i++) spawnObject();
  for (int i = 0; i < 60; i++) scene.step(1.0f / 60.0f, 1);
  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();
  renderer.invalidateTransforms();

  fprintf(stderr, "  %d objects\n", scene.getObjectCount());
  Time t1 = Time::now();
  renderer.renderBlocking(0);
  float ms2 = (float)(Time::now() - t1).toMilliSeconds();
  fprintf(stderr, "  Render: %.0fms\n", ms2);
  saveImage(renderer, output + "_2_added.png");

  // --- Test 3: Transform-only update (no new objects) ---
  fprintf(stderr, "\n=== Test 3: Transform-only (60 more physics steps) ===\n");
  for (int i = 0; i < 60; i++) scene.step(1.0f / 60.0f, 1);
  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();
  renderer.invalidateTransforms();

  Time t2 = Time::now();
  renderer.renderBlocking(0);
  float ms3 = (float)(Time::now() - t2).toMilliSeconds();
  fprintf(stderr, "  Render: %.0fms\n", ms3);
  saveImage(renderer, output + "_3_moved.png");

  // --- Test 4: Add 5 more objects ---
  fprintf(stderr, "\n=== Test 4: Add 5 more objects ===\n");
  for (int i = 0; i < 5; i++) spawnObject();
  for (int i = 0; i < 60; i++) scene.step(1.0f / 60.0f, 1);
  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();
  renderer.invalidateTransforms();

  fprintf(stderr, "  %d objects\n", scene.getObjectCount());
  Time t3 = Time::now();
  renderer.renderBlocking(0);
  float ms4 = (float)(Time::now() - t3).toMilliSeconds();
  fprintf(stderr, "  Render: %.0fms\n", ms4);
  saveImage(renderer, output + "_4_more.png");

  // --- Test 5: 1-sample render (interactive speed test) ---
  fprintf(stderr, "\n=== Test 5: 1-sample speed test ===\n");
  renderer.setSamples(1);
  renderer.invalidateTransforms();

  Time t4 = Time::now();
  renderer.renderBlocking(0);
  float ms5 = (float)(Time::now() - t4).toMilliSeconds();
  fprintf(stderr, "  1-spp render: %.0fms\n", ms5);

  fprintf(stderr, "\n=== Summary ===\n");
  fprintf(stderr, "  Test 1 (initial):       %.0fms\n", ms1);
  fprintf(stderr, "  Test 2 (add objects):    %.0fms\n", ms2);
  fprintf(stderr, "  Test 3 (transform only): %.0fms\n", ms3);
  fprintf(stderr, "  Test 4 (add more):       %.0fms\n", ms4);
  fprintf(stderr, "  Test 5 (1-spp):          %.0fms\n", ms5);
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
