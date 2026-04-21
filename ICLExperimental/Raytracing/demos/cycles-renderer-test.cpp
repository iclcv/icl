/* Test the CyclesRenderer API: create an ICL Scene, render it through
 * Cycles, and save the result to PNG. */

#include <Raytracing/CyclesRenderer.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/SceneLight.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Material.h>
#include <ICLIO/FileWriter.h>

#include <cstdio>
#include <memory>

using namespace icl;
using namespace icl::geom;

int main(int argc, const char **argv) {
  std::string output = "cycles-renderer-test.png";
  for (int i = 1; i < argc; ++i) {
    if (std::string("-o") == argv[i] && i + 1 < argc)
      output = argv[++i];
  }

  // Create ICL scene
  Scene scene;

  // Camera at (0, 200, -600), looking toward origin, Y up
  // Use simple camera params that worked before
  Camera cam(Vec(0, 200, -600, 1),        // position
             Vec(0, -0.3, 1, 1),           // norm (view dir, will be normalized)
             Vec(0, 1, 0, 1),              // up
             24,                            // focal length
             utils::Point32f(0, 0),         // principal point offset
             1.0, 1.0,                      // sampling res x, y
             0,                             // skew
             Camera::RenderParams(utils::Size(800, 600), 1.0f, 100000.0f));
  scene.addCamera(cam);

  // Light
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(300, 500, -200, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 250, 240, 255));

  // Ground plane
  auto ground = std::make_shared<SceneObject>();
  ground->addVertex(Vec(-500, 0, -500, 1));
  ground->addVertex(Vec(500, 0, -500, 1));
  ground->addVertex(Vec(500, 0, 500, 1));
  ground->addVertex(Vec(-500, 0, 500, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addTriangle(0, 1, 2, 0, 1, 2);
  ground->addTriangle(0, 2, 3, 0, 2, 3);
  ground->setMaterial(Material::fromColor(GeomColor(0.4f, 0.35f, 0.3f, 1.0f)));
  ground->getMaterial()->roughness = 0.8f;
  scene.addObject(ground.get());

  // Gold sphere
  auto *goldRaw = SceneObject::sphere(-200, 100, 200, 100, 20, 20);
  auto goldSphere = std::shared_ptr<SceneObject>(goldRaw);
  goldSphere->setMaterial(Material::fromColor(GeomColor(1.0f, 0.76f, 0.34f, 1.0f)));
  goldSphere->getMaterial()->metallic = 1.0f;
  goldSphere->getMaterial()->roughness = 0.15f;
  scene.addObject(goldSphere.get());

  // Red plastic sphere
  auto *redRaw = SceneObject::sphere(0, 100, 200, 100, 20, 20);
  auto redSphere = std::shared_ptr<SceneObject>(redRaw);
  redSphere->setMaterial(Material::fromColor(GeomColor(0.8f, 0.1f, 0.1f, 1.0f)));
  redSphere->getMaterial()->roughness = 0.3f;
  scene.addObject(redSphere.get());

  // Green rubber sphere
  auto *greenRaw = SceneObject::sphere(200, 100, 200, 100, 20, 20);
  auto greenSphere = std::shared_ptr<SceneObject>(greenRaw);
  greenSphere->setMaterial(Material::fromColor(GeomColor(0.1f, 0.6f, 0.15f, 1.0f)));
  greenSphere->getMaterial()->roughness = 0.9f;
  scene.addObject(greenSphere.get());

  // Render with Cycles (scene is in mm, sceneScale converts to Cycles units)
  printf("Rendering with CyclesRenderer...\n");
  rt::CyclesRenderer renderer(scene, rt::RenderQuality::Final);
  renderer.setSamples(256);
  renderer.setSceneScale(1.0f);  // Keep mm as scene units for now
  renderer.render(0);

  // Save
  const auto &image = renderer.getImage();
  if (image.getWidth() > 0) {
    io::FileWriter writer(output);
    writer.write(&image);
    printf("Saved %dx%d image to %s\n", image.getWidth(), image.getHeight(), output.c_str());
  } else {
    fprintf(stderr, "ERROR: No image received!\n");
    return 1;
  }

  return 0;
}
