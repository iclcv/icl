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
  obj->setColor(Primitive::quad, color);
  obj->setColor(Primitive::triangle, color);
  obj->setReflectivity(randRefl(rng));
  obj->setFriction(0.7f);
  obj->setRestitution(0.25f);
  obj->setRollingFriction(0.1f);
  obj->setDamping(0.3f, 0.4f);

  btRigidBody *body = obj->getRigidBody();
  if (body) {
    body->setCcdMotionThreshold(1.0f);
    body->setCcdSweptSphereRadius(0.05f);
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

static void removeObjectsBelowThreshold() {
  auto it = dynamicObjects.begin();
  while (it != dynamicObjects.end()) {
    PhysicsObject *obj = *it;
    Mat t = obj->getTransformation();
    float z = t(3, 2); // translation Z
    if (z < REMOVAL_Z) {
      scene.removeObject(obj);
      it = dynamicObjects.erase(it);
    } else {
      ++it;
    }
  }
}

void init() {
  // Z-up camera: position above and to the side, looking at origin
  Camera cam(Vec(800, -600, 500, 1),    // position
             Vec(-0.7, 0.5, -0.4, 1),   // view direction (toward origin)
             Vec(0, 0, -1, 1));          // up = -Z (ICL convention for Z-up scenes)
  cam.setResolution(Size(800, 600));
  scene.addCamera(cam);

  // Default gravity is already (0, 0, -9810) — no need to set

  // Static table (mass = 0) — flat in XY plane
  auto *table = new RigidBoxObject(0, 0, TABLE_Z, 500, 350, TABLE_THICKNESS, 0);
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
  scene.getLight(0).setPosition(Vec(200, -200, 500, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 245, 220, 255));
  scene.getLight(0).setAmbient(GeomColor(35, 33, 30, 255));
  scene.getLight(0).setSpecular(GeomColor(255, 245, 220, 255));
  scene.getLight(0).setSpecularEnabled(true);

  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(-300, 200, 300, 1));
  scene.getLight(1).setDiffuse(GeomColor(60, 70, 100, 255));
  scene.getLight(1).setAmbient(GeomColor(10, 10, 15, 255));

  for (int i = 0; i < 5; i++) spawnObject();

  std::string backend = pa("-backend");
  if (backend == "auto") backend = "";
  raytracer = std::make_unique<icl::rt::SceneRaytracer>(scene, backend);

  gui << (HSplit()
          << Canvas().handle("draw").minSize(32, 24)
          << (VBox().minSize(20, 24)
              << CheckBox("pause physics", "unchecked").handle("pause")
              << Slider(10, 120, 30).handle("spawnRate").label("spawn interval")
              << Button("spawn 10").handle("burst")
              << CheckBox("Path tracing GI", "unchecked").handle("pathTracing")
              << Combo("!1x off,4x 2x2,9x 3x3,16x 4x4").handle("aa").label("MSAA")
              << CheckBox("FXAA", "checked").handle("fxaa")
              << CheckBox("Adaptive AA", "unchecked").handle("adaptiveAA")
              << Label("--").handle("info")
             )
         )
      << Show();

  static RaytracerMouseHandler mouseHandler;
  mouseHandler.sceneHandler = scene.getMouseHandler(0);
  gui["draw"].install(&mouseHandler);
}

static int frameCount = 0;

void run() {
  static const int aaValues[] = {1, 4, 9, 16};
  int aaIdx = gui["aa"].as<ComboHandle>().getSelectedIndex();
  raytracer->setAASamples(aaValues[aaIdx]);
  raytracer->setPathTracing(gui["pathTracing"].as<bool>());
  raytracer->setFXAA(gui["fxaa"].as<bool>());
  raytracer->setAdaptiveAA(gui["adaptiveAA"].as<bool>(), 4);
  float targetFps = pa("-fps");
  if (targetFps > 0) raytracer->setTargetFrameTime(1000.0f / targetFps);

  int spawnRate = gui["spawnRate"].as<int>();
  bool paused = gui["pause"].as<bool>();

  bool sceneChanged = false;

  if (!paused) {
    scene.step();
    sceneChanged = true;

    frameCount++;
    if (frameCount % spawnRate == 0) {
      spawnObject();
    }
    if (gui["burst"].as<ButtonHandle>().wasTriggered()) {
      for (int i = 0; i < 10; i++) spawnObject();
    }
    removeObjectsBelowThreshold();
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
        scene.getObject(glowingObject)->setEmission(GeomColor(0,0,0,255), 0);
      }
      if (clickedObj == glowingObject) {
        glowingObject = -1; // toggle off
      } else {
        glowingObject = clickedObj;
        auto *obj = scene.getObject(clickedObj);
        const auto &prims = obj->getPrimitives();
        GeomColor glowColor(255, 220, 100, 255);
        if (!prims.empty()) {
          const auto &c = prims[0]->color;
          glowColor = GeomColor(
            std::min(255.0f, c[0] * 255 + 100),
            std::min(255.0f, c[1] * 255 + 100),
            std::min(255.0f, c[2] * 255 + 100), 255);
        }
        obj->setEmission(glowColor, 2.0f);
      }
      sceneChanged = true;
    }
  }

  if (sceneChanged) {
    raytracer->invalidateAll();
  }

  Time t = Time::now();
  raytracer->render(0);
  float ms = (float)(Time::now() - t).toMilliSeconds();

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

  static char buf[128];
  static int lastAccum = 0;
  int accumFrames = raytracer->getAccumulatedFrames();
  int passesThisFrame = accumFrames - lastAccum;
  lastAccum = accumFrames;
  if (accumFrames > 0) {
    snprintf(buf, sizeof(buf), "%s | %.1f ms | %d obj | PT %d spp (+%d)",
             raytracer->backendName(), ms,
             (int)dynamicObjects.size(), accumFrames, passesThisFrame);
  } else {
    snprintf(buf, sizeof(buf), "%s | %.1f ms (%.0f fps) | %d obj | %dx AA",
             raytracer->backendName(), ms, 1000.0f / ms,
             (int)dynamicObjects.size(), aaValues[aaIdx]);
  }
  draw->text(buf, 10, 20, 10);
  draw->render();

  gui["info"] = std::string(buf);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "-backend(string=auto) -fps(float=30)", init, run).exec();
}
