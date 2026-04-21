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
HSplit gui;

// Interaction state
static int hoveredObject = -1;
static int glowingObject = -1;
static GeomColor glowingOrigColor;
static int mouseX = -1, mouseY = -1;
static bool mouseClicked = false;

// Custom mouse handler: wraps SceneMouseHandler, tracks hover position and clicks
struct RaytracerMouseHandler : public MouseHandler {
  MouseHandler *sceneHandler = nullptr;
  bool dragging = false;

  void process(const MouseEvent &e) override {
    // Track mouse position (in image coordinates)
    mouseX = e.getX();
    mouseY = e.getY();

    bool isLeft = !e.isRight() && !e.isMiddle();

    if (e.isPressEvent() && isLeft) {
      mouseClicked = true;
    }

    // Forward non-left-button events and wheel events to scene handler
    // Left button is reserved for object picking
    if (sceneHandler && (!isLeft || e.isWheelEvent())) {
      sceneHandler->process(e);
    }
  }
};

// Random generators
static std::mt19937 rng(42);
static std::uniform_real_distribution<float> randXY(-180, 180);
static std::uniform_real_distribution<float> randSize(20, 70);
static std::uniform_real_distribution<float> randAspect(0.4f, 1.5f);
static std::uniform_int_distribution<int> randType(0, 2);
static std::uniform_int_distribution<int> randColor(60, 230);
static std::uniform_real_distribution<float> randAngle(0.0f, 6.283f);
static std::uniform_real_distribution<float> randRefl(0.0f, 0.8f);
static std::uniform_real_distribution<float> randMetallic(0.0f, 1.0f);
static std::uniform_real_distribution<float> randRoughness(0.05f, 0.95f);
static std::uniform_real_distribution<float> randUnit(0.0f, 1.0f);

// Table parameters (Z-up convention, matching ICL/Bullet default)
static constexpr float TABLE_Z = -30;
static constexpr float TABLE_THICKNESS = 20;
static constexpr float TABLE_BOTTOM = TABLE_Z - TABLE_THICKNESS / 2;
static constexpr float REMOVAL_Z = TABLE_BOTTOM - 20 * TABLE_THICKNESS;
static constexpr float SPAWN_HEIGHT = 500; // Z above table

// Track dynamic objects
static std::deque<PhysicsObject *> dynamicObjects;

static GeomColor randomColor() {
  return GeomColor(randColor(rng), randColor(rng), randColor(rng), 255);
}

static void setupPhysicsBody(RigidObject *obj, const GeomColor &color, bool smooth,
                             bool skipNormals = false) {
  obj->setVisible(Primitive::line, false);
  obj->setVisible(Primitive::vertex, false);
  if (!skipNormals) obj->createAutoNormals(smooth);

  // Create a PBR material with random properties
  auto mat = std::make_shared<Material>();
  mat->baseColor = color * (1.0f / 255.0f);

  // Random material class: 40% plastic, 30% metal, 20% rough, 10% mirror
  float roll = randUnit(rng);
  if (roll < 0.4f) {
    // Plastic: dielectric, low-to-medium roughness
    mat->metallic = 0.0f;
    mat->roughness = 0.2f + randRoughness(rng) * 0.5f;
    mat->reflectivity = 0.0f;
  } else if (roll < 0.7f) {
    // Metal: metallic, variable roughness
    mat->metallic = 0.8f + randUnit(rng) * 0.2f;
    mat->roughness = 0.1f + randUnit(rng) * 0.6f;
    mat->reflectivity = 0.0f;
    // Metals tint their specular with base color (already handled by BRDF)
  } else if (roll < 0.9f) {
    // Rough/matte: high roughness, no metallic
    mat->metallic = 0.0f;
    mat->roughness = 0.7f + randUnit(rng) * 0.3f;
    mat->reflectivity = 0.0f;
  } else {
    // Mirror: explicit reflectivity
    mat->metallic = 0.0f;
    mat->roughness = 0.05f;
    mat->reflectivity = 0.5f + randUnit(rng) * 0.5f;
    mat->baseColor = GeomColor(0.9f, 0.9f, 0.92f, 1.0f); // silver tint
  }

  obj->setMaterial(mat);

  obj->setFriction(0.5f);
  obj->setRestitution(0.35f);
  obj->setRollingFriction(0.02f);
  obj->setDamping(0.2f, 0.08f); // low angular damping — let objects topple naturally

  btRigidBody *body = obj->getRigidBody();
  if (body) {
    body->setCcdMotionThreshold(1.0f);
    body->setCcdSweptSphereRadius(0.05f);
    // Give a random spin so objects tumble instead of balancing on edges
    float wx = (randUnit(rng) - 0.5f) * 4.0f;
    float wy = (randUnit(rng) - 0.5f) * 4.0f;
    float wz = (randUnit(rng) - 0.5f) * 4.0f;
    body->setAngularVelocity(btVector3(wx, wy, wz));
  }
}

static void spawnObject() {
  float x = randXY(rng);
  float y = randXY(rng);
  float sz = randSize(rng);
  float aspect = randAspect(rng);
  int type = randType(rng);
  GeomColor color = randomColor();
  float ax = randAngle(rng), ay = randAngle(rng), az = randAngle(rng);

  RigidObject *obj = nullptr;
  switch (type) {
    case 0: {
      float dx = sz, dy = sz * aspect, dz = sz / aspect;
      obj = new RigidBoxObject(x, y, SPAWN_HEIGHT, dx, dy, dz, 0.2f);
      setupPhysicsBody(obj, color, false);
      break;
    }
    case 1: {
      obj = new RigidSphereObject(x, y, SPAWN_HEIGHT, sz * 0.5f, 0.2f);
      setupPhysicsBody(obj, color, true);
      break;
    }
    case 2: {
      float r = sz * 0.3f, h = sz * aspect;
      obj = new RigidCylinderObject(x, y, SPAWN_HEIGHT, r, h, 0.2f);
      // Cylinder now has proper radial normals for sides + flat for caps.
      // Don't call createAutoNormals — the constructor already sets them.
      setupPhysicsBody(obj, color, false, true);
      break;
    }
  }

  if (obj) {
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
    float z = t(3, 2); // translation Z
    if (z < REMOVAL_Z) {
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
  // Z-up camera: position above and to the side, looking at origin
  Camera cam(Vec(800, -600, 500, 1),    // position
             Vec(-0.7, 0.5, -0.4, 1),   // view direction (toward origin)
             Vec(0, 0, -1, 1));          // up = -Z (ICL convention for Z-up scenes)
  cam.setResolution(pa("-size"));
  scene.addCamera(cam);

  // Default gravity is already (0, 0, -9810) — no need to set

  // Static table (mass = 0) — flat in XY plane
  auto *table = new RigidBoxObject(0, 0, TABLE_Z, 500, 350, TABLE_THICKNESS, 0);
  table->setVisible(Primitive::line, false);
  table->setVisible(Primitive::vertex, false);
  table->createAutoNormals(false);
  {
    auto mat = std::make_shared<Material>();
    mat->baseColor = GeomColor(0.55f, 0.39f, 0.24f, 1.0f); // wood brown
    mat->metallic = 0.0f;
    mat->roughness = 0.7f;
    mat->reflectivity = 0.05f;
    table->setMaterial(mat);
  }
  table->setFriction(0.8f);
  table->setRestitution(0.15f);
  scene.addObject(table, true);

  // Lights
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(200, -200, 500, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 245, 220, 255));
  scene.getLight(0).setAmbient(GeomColor(35, 33, 30, 255));
  scene.getLight(0).setSpecular(GeomColor(255, 245, 220, 255));
  scene.getLight(0).setSpecularEnabled(true);

  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(-300, 200, 300, 1));
  scene.getLight(1).setDiffuse(GeomColor(60, 70, 100, 255));
  scene.getLight(1).setAmbient(GeomColor(10, 10, 15, 255));

  // ---- Material archetype display ----
  // Helper to create a static sphere with a given material
  auto addArchetype = [&](float x, float y, float radius,
                          std::shared_ptr<Material> mat) {
    auto *obj = new RigidSphereObject(x, y, TABLE_Z + radius + 5, radius, 0);
    obj->setVisible(Primitive::line, false);
    obj->setVisible(Primitive::vertex, false);
    obj->createAutoNormals(true);
    obj->setMaterial(mat);
    scene.addObject(obj, true);
  };

  float sphereR = 25;
  float rowSpacing = 70;

  // Row 1 (back, y=100): Roughness gradient — same red dielectric, roughness 0.05 to 0.95
  {
    float y = 100;
    const char *labels[] = {"r=0.05", "r=0.2", "r=0.4", "r=0.6", "r=0.8", "r=0.95"};
    float roughVals[] = {0.05f, 0.2f, 0.4f, 0.6f, 0.8f, 0.95f};
    int n = 6;
    float startX = -(n - 1) * rowSpacing / 2.0f;
    for (int i = 0; i < n; i++) {
      auto mat = std::make_shared<Material>();
      mat->name = labels[i];
      mat->baseColor = GeomColor(0.8f, 0.15f, 0.15f, 1.0f);
      mat->metallic = 0.0f;
      mat->roughness = roughVals[i];
      addArchetype(startX + i * rowSpacing, y, sphereR, mat);
    }
  }

  // Row 2 (front, y=-30): Material archetypes
  {
    float y = -30;
    float startX = -2.5f * rowSpacing;

    // Gold metal
    auto gold = std::make_shared<Material>();
    gold->name = "gold";
    gold->baseColor = GeomColor(1.0f, 0.76f, 0.34f, 1.0f);
    gold->metallic = 1.0f;
    gold->roughness = 0.2f;
    addArchetype(startX, y, sphereR, gold);

    // Copper metal
    auto copper = std::make_shared<Material>();
    copper->name = "copper";
    copper->baseColor = GeomColor(0.95f, 0.64f, 0.54f, 1.0f);
    copper->metallic = 1.0f;
    copper->roughness = 0.4f;
    addArchetype(startX + rowSpacing, y, sphereR, copper);

    // White plastic
    auto plastic = std::make_shared<Material>();
    plastic->name = "plastic";
    plastic->baseColor = GeomColor(0.9f, 0.9f, 0.9f, 1.0f);
    plastic->metallic = 0.0f;
    plastic->roughness = 0.3f;
    addArchetype(startX + 2 * rowSpacing, y, sphereR, plastic);

    // Green rubber
    auto rubber = std::make_shared<Material>();
    rubber->name = "rubber";
    rubber->baseColor = GeomColor(0.2f, 0.6f, 0.15f, 1.0f);
    rubber->metallic = 0.0f;
    rubber->roughness = 0.95f;
    addArchetype(startX + 3 * rowSpacing, y, sphereR, rubber);

    // Mirror
    auto mirror = std::make_shared<Material>();
    mirror->name = "mirror";
    mirror->baseColor = GeomColor(0.95f, 0.95f, 0.97f, 1.0f);
    mirror->metallic = 0.0f;
    mirror->roughness = 0.02f;
    mirror->reflectivity = 0.9f;
    addArchetype(startX + 4 * rowSpacing, y, sphereR, mirror);

    // Emissive
    auto emissive = std::make_shared<Material>();
    emissive->name = "emissive";
    emissive->baseColor = GeomColor(1.0f, 0.9f, 0.6f, 1.0f);
    emissive->emissive = GeomColor(1.5f, 1.2f, 0.7f, 1.0f);
    emissive->roughness = 0.8f;
    addArchetype(startX + 5 * rowSpacing, y, sphereR, emissive);
  }

  std::string backend = pa("-backend");
  if (backend == "auto") backend = "";
  raytracer = std::make_unique<icl::rt::SceneRaytracer>(scene, backend);

  // Default to path tracing + SVGF + ACES for best visual quality
  raytracer->setPathTracing(true);
  raytracer->setDenoising(icl::rt::DenoisingMethod::SVGF);
  raytracer->setToneMapping(icl::rt::ToneMapMethod::ACES);
  raytracer->setExposure(0.7f);

  float initialScale = pa("-scale");
  if (initialScale < 1.0f) {
    raytracer->setRenderScale(initialScale);
    raytracer->setUpsampling(icl::rt::UpsamplingMethod::Bilinear);
  }

  gui  << Canvas().handle("draw").minSize(64, 48)
       << (VBox().minSize(20, 24)
          << CheckBox("pause physics", "checked").handle("pause")
          << Slider(10, 120, 30).handle("spawnRate").label("spawn interval")
          << Button("spawn 10").handle("burst")
          << CheckBox("Path tracing GI", "checked").handle("pathTracing")
          << Combo("!1x off,4x 2x2,9x 3x3,16x 4x4").handle("aa").label("MSAA")
          << CheckBox("FXAA", "checked").handle("fxaa")
          << CheckBox("Adaptive AA", "unchecked").handle("adaptiveAA")
          << Combo("!Bilinear,Edge-Aware,MetalFX Spatial,MetalFX Temporal").handle("upsampling").label("Upsampling")
          << Slider(25, 100, 100).handle("renderScale").label("Render Resolution %")
          << Combo("None,Bilateral,A-Trous Wavelet,!SVGF").handle("denoising").label("Denoising")
          << Slider(0, 100, 50).handle("denoiseStrength").label("Denoise Strength %")
          << Slider(0, 100, 100).handle("lightDim").label("Scene Lights %")
          << Combo("None,Reinhard,!ACES Filmic,Hable").handle("toneMap").label("Tone Mapping")
          << Slider(10, 500, 70).handle("exposure").label("Exposure %")
          << Label("--").handle("info")
        )<< Show();

  static RaytracerMouseHandler mouseHandler;
  mouseHandler.sceneHandler = scene.getMouseHandler(0);
  gui["draw"].install(&mouseHandler);
}

static int frameCount = 0;

void run() {
  // Scene light intensity
  static float lastLightDim = -1;
  float lightDim = gui["lightDim"].as<int>() / 100.0f;
  if (lightDim != lastLightDim) {
    // Base colors from init()
    static const GeomColor baseDiffuse0(255, 245, 220, 255);
    static const GeomColor baseAmbient0(35, 33, 30, 255);
    static const GeomColor baseSpecular0(255, 245, 220, 255);
    static const GeomColor baseDiffuse1(60, 70, 100, 255);
    static const GeomColor baseAmbient1(10, 10, 15, 255);

    auto scale = [](const GeomColor &c, float s) {
      return GeomColor(c[0]*s, c[1]*s, c[2]*s, c[3]);
    };
    scene.getLight(0).setDiffuse(scale(baseDiffuse0, lightDim));
    scene.getLight(0).setAmbient(scale(baseAmbient0, lightDim));
    scene.getLight(0).setSpecular(scale(baseSpecular0, lightDim));
    scene.getLight(1).setDiffuse(scale(baseDiffuse1, lightDim));
    scene.getLight(1).setAmbient(scale(baseAmbient1, lightDim));
    lastLightDim = lightDim;
    raytracer->invalidateAll(); // force scene data re-upload
  }

  static const int aaValues[] = {1, 4, 9, 16};
  int aaIdx = gui["aa"].as<ComboHandle>().getSelectedIndex();
  raytracer->setAASamples(aaValues[aaIdx]);
  raytracer->setPathTracing(gui["pathTracing"].as<bool>());
  raytracer->setFXAA(gui["fxaa"].as<bool>());
  raytracer->setAdaptiveAA(gui["adaptiveAA"].as<bool>(), 4);

  // Render resolution + upsampling
  static const icl::rt::UpsamplingMethod upMethods[] = {
    icl::rt::UpsamplingMethod::Bilinear,
    icl::rt::UpsamplingMethod::EdgeAware,
    icl::rt::UpsamplingMethod::MetalFXSpatial,
    icl::rt::UpsamplingMethod::MetalFXTemporal,
  };
  int upIdx = gui["upsampling"].as<ComboHandle>().getSelectedIndex();
  if (raytracer->supportsUpsampling(upMethods[upIdx])) {
    raytracer->setUpsampling(upMethods[upIdx]);
  }
  raytracer->setRenderScale(gui["renderScale"].as<int>() / 100.0f);

  // Denoising
  static const icl::rt::DenoisingMethod dnMethods[] = {
    icl::rt::DenoisingMethod::None,
    icl::rt::DenoisingMethod::Bilateral,
    icl::rt::DenoisingMethod::ATrousWavelet,
    icl::rt::DenoisingMethod::SVGF,
  };
  int dnIdx = gui["denoising"].as<ComboHandle>().getSelectedIndex();
  raytracer->setDenoising(dnMethods[dnIdx]);
  raytracer->setDenoisingStrength(gui["denoiseStrength"].as<int>() / 100.0f);

  // Tone mapping
  static const icl::rt::ToneMapMethod tmMethods[] = {
    icl::rt::ToneMapMethod::None,
    icl::rt::ToneMapMethod::Reinhard,
    icl::rt::ToneMapMethod::ACES,
    icl::rt::ToneMapMethod::Hable,
  };
  int tmIdx = gui["toneMap"].as<ComboHandle>().getSelectedIndex();
  raytracer->setToneMapping(tmMethods[tmIdx]);
  raytracer->setExposure(gui["exposure"].as<int>() / 100.0f);

  static float targetFps = pa("-fps");
  static float targetFrameMs = targetFps > 0 ? 1000.0f / targetFps : 0;
  static bool firstFrame = true;
  if (firstFrame) {
    if (targetFps > 0) raytracer->setTargetFrameTime(targetFrameMs);
    firstFrame = false;
  }

  Time frameStart = Time::now();

  int spawnRate = gui["spawnRate"].as<int>();
  bool paused = gui["pause"].as<bool>();

  bool physicsStepOccurred = false;
  bool geometryChanged = false; // only true when actual mesh/material changes

  if (!paused) {
    scene.step(); // wall-clock dt, Bullet accumulator handles variable frame rate
    physicsStepOccurred = true;

    frameCount++;
    if (frameCount % spawnRate == 0) {
      spawnObject();
      geometryChanged = true; // new object = new geometry
    }
    if (gui["burst"].as<ButtonHandle>().wasTriggered()) {
      for (int i = 0; i < 10; i++) spawnObject();
      geometryChanged = true;
    }
    if (removeObjectsBelowThreshold())
      geometryChanged = true;
  }

  for (int i = 0; i < scene.getObjectCount(); i++) {
    scene.getObject(i)->prepareForRendering();
  }

  // Handle click-to-glow (before render so changes are visible immediately)
  if (mouseClicked) {
    mouseClicked = false;
    int clickedObj = raytracer->getObjectAtPixel(mouseX, mouseY);
    if (clickedObj >= 0 && clickedObj < scene.getObjectCount()) {
      // Reset previous glowing object
      if (glowingObject >= 0 && glowingObject < scene.getObjectCount()) {
        auto mat = scene.getObject(glowingObject)->getOrCreateMaterial();
        mat->emissive = GeomColor(0, 0, 0, 1);
      }
      if (clickedObj == glowingObject) {
        glowingObject = -1; // toggle off
      } else {
        glowingObject = clickedObj;
        auto *obj = scene.getObject(clickedObj);
        auto mat = obj->getOrCreateMaterial();
        // Glow with a brightened version of the base color
        mat->emissive = GeomColor(
          std::min(1.5f, mat->baseColor[0] + 0.4f),
          std::min(1.5f, mat->baseColor[1] + 0.4f),
          std::min(1.5f, mat->baseColor[2] + 0.4f), 1.0f);
      }
      geometryChanged = true; // emission change = material change
    }
  }

  if (geometryChanged) {
    raytracer->invalidateAll();
  } else if (physicsStepOccurred) {
    raytracer->invalidateTransforms();
  }

  Time renderStart = Time::now();
  raytracer->render(0);
  float ms = (float)(Time::now() - renderStart).toMilliSeconds();

  // Get hovered object for border highlighting
  hoveredObject = raytracer->getObjectAtPixel(mouseX, mouseY);

  // Copy rendered image and draw hover outline
  Img8u img = *raytracer->getImage().deepCopy();

  // Draw border around hovered object using object ID buffer
  if (hoveredObject >= 0) {
    int w = img.getWidth(), h = img.getHeight();
    icl8u *R = img.getData(0);
    icl8u *G = img.getData(1);
    icl8u *B = img.getData(2);

    for (int y = 1; y < h - 1; y++) {
      for (int x = 1; x < w - 1; x++) {
        int id = raytracer->getObjectAtPixel(x, y);
        if (id != hoveredObject) continue;
        // Check if any neighbor is NOT this object → edge pixel
        bool edge = (raytracer->getObjectAtPixel(x-1, y) != hoveredObject) ||
                    (raytracer->getObjectAtPixel(x+1, y) != hoveredObject) ||
                    (raytracer->getObjectAtPixel(x, y-1) != hoveredObject) ||
                    (raytracer->getObjectAtPixel(x, y+1) != hoveredObject);
        if (edge) {
          int idx = x + y * w;
          R[idx] = 255; G[idx] = 220; B[idx] = 50; // yellow outline
        }
      }
    }
  }

  DrawHandle draw = gui["draw"];
  draw = img;

  static char buf[256];
  static int lastAccum = 0;
  int accumFrames = raytracer->getAccumulatedFrames();
  int passesThisFrame = accumFrames - lastAccum;
  lastAccum = accumFrames;
  float scale = raytracer->getRenderScale();
  int rw = (int)(img.getWidth() * scale), rh = (int)(img.getHeight() * scale);
  const char *upNames[] = {"", " bilinear", " edge-aware", " MFX-spatial", " MFX-temporal"};
  const char *upLabel = (scale < 1.0f) ? upNames[(int)raytracer->getUpsamplingMethod()] : "";

  if (accumFrames > 0) {
    snprintf(buf, sizeof(buf), "%s | %.1f ms | %d obj | PT %d spp (+%d)",
             raytracer->backendName(), ms,
             (int)dynamicObjects.size(), accumFrames, passesThisFrame);
  } else {
    snprintf(buf, sizeof(buf), "%s | %.1f ms (%.0f fps) | %d obj | %dx AA",
             raytracer->backendName(), ms, 1000.0f / ms,
             (int)dynamicObjects.size(), aaValues[aaIdx]);
  }
  if (scale < 1.0f) {
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
             " | %dx%d->%dx%d%s", rw, rh, img.getWidth(), img.getHeight(), upLabel);
  }
  const char *dnNames[] = {"", " bilateral", " a-trous", " svgf"};
  auto dnMethod = raytracer->getDenoisingMethod();
  if (dnMethod != icl::rt::DenoisingMethod::None) {
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
             " | denoise:%s", dnNames[(int)dnMethod]);
  }
  // Cap frame rate — sleep for remaining frame budget.
  if (targetFrameMs > 0) {
    float preDrawElapsed = (float)(Time::now() - frameStart).toMilliSeconds();
    if (preDrawElapsed < targetFrameMs) {
      Thread::msleep((unsigned int)(targetFrameMs - preDrawElapsed));
    }
  }
  float totalMs = (float)(Time::now() - frameStart).toMilliSeconds();
  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
           " | total %.0f ms", totalMs);

  draw->text(buf, 10, 20, 10);

  // Material tooltip on hover
  if (hoveredObject >= 0 && hoveredObject < scene.getObjectCount() &&
      mouseX >= 0 && mouseY >= 0) {
    auto mat = scene.getObject(hoveredObject)->getMaterial();
    if (mat) {
      char tip[512];
      int tipY = std::max(40, mouseY - 80);
      int tipX = std::min(mouseX + 15, (int)img.getWidth() - 200);

      const char *matClass = "custom";
      if (mat->reflectivity > 0.3f) matClass = "mirror";
      else if (mat->metallic > 0.5f) matClass = "metal";
      else if (mat->roughness > 0.6f) matClass = "matte";
      else matClass = "plastic";

      snprintf(tip, sizeof(tip), "%s%s",
               mat->name.empty() ? matClass : mat->name.c_str(),
               mat->emissive[0] + mat->emissive[1] + mat->emissive[2] > 0.01f ? " [emissive]" : "");
      draw->text(tip, tipX, tipY, 9);

      snprintf(tip, sizeof(tip), "color: (%.0f%%, %.0f%%, %.0f%%)",
               mat->baseColor[0]*100, mat->baseColor[1]*100, mat->baseColor[2]*100);
      draw->text(tip, tipX, tipY + 14, 8);

      snprintf(tip, sizeof(tip), "metallic: %.0f%%  roughness: %.0f%%",
               mat->metallic*100, mat->roughness*100);
      draw->text(tip, tipX, tipY + 26, 8);

      if (mat->reflectivity > 0.01f) {
        snprintf(tip, sizeof(tip), "reflectivity: %.0f%%", mat->reflectivity*100);
        draw->text(tip, tipX, tipY + 38, 8);
      }
    }
  }

  draw->render();

  gui["info"] = std::string(buf);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "-backend(string=auto) -fps(float=30) -size(Size=800x600) -scale(float=1.0)", init, run).exec();
}
