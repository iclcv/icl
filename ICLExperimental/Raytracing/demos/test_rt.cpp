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
  rt.render(0);
  const Img8u &img = rt.getImage();

  printf("Image: %dx%d, %d channels\n", img.getWidth(), img.getHeight(), img.getChannels());

  int nonBg = 0;
  const auto *R = img.getData(0);
  const auto *G = img.getData(1);
  for (int i = 0; i < img.getWidth() * img.getHeight(); i++) {
    if (R[i] > 30 || G[i] > 30) nonBg++;
  }
  printf("Non-background pixels: %d (%.1f%%)\n", nonBg, 100.0*nonBg/(img.getWidth()*img.getHeight()));
  int cx = 160 + 120*320;
  printf("Center pixel: R=%d G=%d B=%d\n",
         img.getData(0)[cx], img.getData(1)[cx], img.getData(2)[cx]);

  if (nonBg > 100) {
    printf("SUCCESS: raytracer produced visible output\n");
    return 0;
  } else {
    printf("FAIL: raytracer produced mostly empty output\n");
    return 1;
  }
}
