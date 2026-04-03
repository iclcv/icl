// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/Time.h>
#include <Raytracing/SceneRaytracer.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static Scene scene;
static std::unique_ptr<icl::rt::SceneRaytracer> raytracer;
GUI gui;

// Helper: configure a shape for raytracing (hide lines/vertices, create normals)
static SceneObject *prepareShape(SceneObject *obj) {
  obj->setVisible(Primitive::line, false);
  obj->setVisible(Primitive::vertex, false);
  obj->createAutoNormals(true);
  return obj;
}

void init() {
  // Camera: looking at the desk from above-front
  Camera cam(Vec(0, 400, 800, 1),      // position: above and in front
             Vec(0, -0.4, -1, 1),       // looking slightly downward
             Vec(0, 1, 0, 1));          // up
  cam.setResolution(Size(800, 600));
  scene.addCamera(cam);

  // --- Desk (flat cuboid) ---
  SceneObject *desk = prepareShape(SceneObject::cuboid(0, -30, 0, 500, 10, 350));
  desk->setColor(Primitive::quad, GeomColor(140, 100, 60, 255)); // wood brown
  desk->setReflectivity(0.08f); // subtle polished wood reflection
  scene.addObject(desk);

  // --- Objects on desk ---

  // Red sphere (left) — slightly reflective
  SceneObject *sphere1 = prepareShape(SceneObject::sphere(-180, 50, -50, 50, 24, 24));
  sphere1->setColor(Primitive::quad, GeomColor(200, 40, 40, 255));
  sphere1->setShininess(100);
  sphere1->setSpecularReflectance(GeomColor(200, 200, 200, 255));
  sphere1->setReflectivity(0.3f);
  scene.addObject(sphere1);

  // Green cube (center-left)
  SceneObject *cube = prepareShape(SceneObject::cube(-30, 40, 30, 40));
  cube->setColor(Primitive::quad, GeomColor(40, 180, 40, 255));
  cube->setReflectivity(0.1f);
  // Rotate the cube 30 degrees around Y for visual interest
  Mat rot = Mat::id();
  float a = 30.0f * M_PI / 180.0f;
  rot(0,0) = std::cos(a);  rot(2,0) = std::sin(a);
  rot(0,2) = -std::sin(a); rot(2,2) = std::cos(a);
  cube->setTransformation(rot);
  scene.addObject(cube);

  // Blue cylinder (right)
  SceneObject *cyl = prepareShape(new SceneObject("cylinder", (float[]){150, 25, -30, 30, 30, 80, 20}));
  cyl->setColor(Primitive::quad, GeomColor(50, 80, 220, 255));
  cyl->setColor(Primitive::triangle, GeomColor(50, 80, 220, 255));
  scene.addObject(cyl);

  // Small yellow sphere (front-right)
  SceneObject *sphere2 = prepareShape(SceneObject::sphere(80, 15, 100, 20, 16, 16));
  sphere2->setColor(Primitive::quad, GeomColor(230, 200, 30, 255));
  scene.addObject(sphere2);

  // --- Desk lamp ---
  // Lamp base (short cylinder)
  SceneObject *lampBase = prepareShape(new SceneObject("cylinder", (float[]){250, -15, -120, 40, 40, 10, 16}));
  lampBase->setColor(Primitive::quad, GeomColor(60, 60, 60, 255));
  lampBase->setColor(Primitive::triangle, GeomColor(60, 60, 60, 255));
  scene.addObject(lampBase);

  // Lamp pole (thin tall cylinder)
  SceneObject *lampPole = prepareShape(new SceneObject("cylinder", (float[]){250, 100, -120, 5, 5, 200, 8}));
  lampPole->setColor(Primitive::quad, GeomColor(80, 80, 80, 255));
  lampPole->setColor(Primitive::triangle, GeomColor(80, 80, 80, 255));
  scene.addObject(lampPole);

  // Lamp shade (cone, upside down = wider at top)
  SceneObject *lampShade = prepareShape(new SceneObject("cone", (float[]){250, 220, -120, 50, 50, -40, 12}));
  lampShade->setColor(Primitive::quad, GeomColor(200, 180, 140, 255));
  lampShade->setColor(Primitive::triangle, GeomColor(200, 180, 140, 255));
  scene.addObject(lampShade);

  // --- Lights ---

  // Desk lamp light (positioned at the lamp shade opening)
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(250, 200, -120, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 240, 200, 255));  // warm white
  scene.getLight(0).setAmbient(GeomColor(30, 28, 25, 255));
  scene.getLight(0).setSpecular(GeomColor(255, 240, 200, 255));
  scene.getLight(0).setSpecularEnabled(true);

  // Soft fill light from the other side
  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(-300, 300, 200, 1));
  scene.getLight(1).setDiffuse(GeomColor(80, 90, 120, 255));    // cool blue-ish fill
  scene.getLight(1).setAmbient(GeomColor(15, 15, 20, 255));
  scene.getLight(1).setSpecularEnabled(false);

  // Create raytracer
  raytracer = std::make_unique<icl::rt::SceneRaytracer>(scene);

  // GUI
  gui << Canvas().handle("draw").minSize(40, 30)
      << Show();

  // Install mouse handler for camera orbit/pan/zoom
  gui["draw"].install(scene.getMouseHandler(0));
}

void run() {
  // Invalidate on every frame since mouse may have moved the camera
  raytracer->invalidateAll();

  Time t = Time::now();
  raytracer->render(0);
  float ms = (float)(Time::now() - t).toMilliSeconds();

  const Img8u &img = raytracer->getImage();
  DrawHandle draw = gui["draw"];
  draw = img;

  static char buf[128];
  snprintf(buf, sizeof(buf), "%s | %.1f ms (%.0f fps) | %dx%d",
           raytracer->backendName(), ms, 1000.0f / ms,
           img.getWidth(), img.getHeight());
  draw->text(buf, 10, 20, 10);
  draw->render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
