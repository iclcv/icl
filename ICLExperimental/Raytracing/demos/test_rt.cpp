#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <Raytracing/SceneRaytracer.h>
#include <cstdio>

using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;

int main() {
  Scene scene;
  Camera cam(Vec(0,0,500,1), Vec(0,0,-1,1), Vec(0,1,0,1));
  cam.setResolution(Size(320, 240));
  scene.addCamera(cam);

  SceneObject *cube = SceneObject::cube(0, 0, 0, 80);
  cube->setColor(Primitive::quad, GeomColor(200, 60, 60, 255));
  cube->setVisible(Primitive::line, false);
  cube->setVisible(Primitive::vertex, false);
  cube->createAutoNormals(true);
  scene.addObject(cube);

  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(300, 300, 400, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 255, 255, 255));
  scene.getLight(0).setAmbient(GeomColor(40, 40, 40, 255));

  icl::rt::SceneRaytracer rt(scene);
  printf("Backend: %s\n", rt.backendName());

  auto checkFrame = [&](const char *label) {
    const Img8u &img = rt.getImage();
    int w = img.getWidth(), h = img.getHeight();
    int nonBg = 0, black = 0;
    const auto *R = img.getData(0);
    const auto *G = img.getData(1);
    const auto *B = img.getData(2);
    for (int i = 0; i < w * h; i++) {
      if (R[i] > 30 || G[i] > 30) nonBg++;
      if (R[i] == 0 && G[i] == 0 && B[i] == 0) black++;
    }
    int cx = w/2 + (h/2) * w;
    printf("%-20s  vis=%d (%.1f%%)  black=%d  center=(%d,%d,%d)\n",
           label, nonBg, 100.0*nonBg/(w*h), black,
           R[cx], G[cx], B[cx]);
    return nonBg;
  };

  // Frame 1: cold start (full build)
  rt.render(0);
  int vis1 = checkFrame("frame1 (cold)");

  // Frame 2: static (no changes)
  rt.render(0);
  checkFrame("frame2 (static)");

  // Frames 3-5: move the cube (transform-only)
  for (int i = 3; i <= 5; i++) {
    cube->translate(5, 0, 0);
    rt.invalidateTransforms();
    rt.render(0);
    char label[32];
    snprintf(label, sizeof(label), "frame%d (moved)", i);
    checkFrame(label);
  }

  // Frame 6: move back (transform-only)
  cube->translate(-15, 0, 0);
  rt.invalidateTransforms();
  rt.render(0);
  int vis6 = checkFrame("frame6 (back)");

  // Compare: frame 6 should look like frame 1
  if (vis1 > 100 && std::abs(vis1 - vis6) < vis1 / 10) {
    printf("SUCCESS: rendering stable across transform updates\n");
    return 0;
  } else {
    printf("FAIL: vis1=%d vis6=%d (expected similar)\n", vis1, vis6);
    return 1;
  }
}
