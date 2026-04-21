/* Test the CyclesRenderer API: create an ICL Scene, render it through
 * Cycles, and save the result to PNG. */

#include <ICLGeom/CyclesRenderer.h>
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
  // Camera looking at the spheres from front-above
  scene.addCamera(Camera::lookAt(
      Vec(0, 300, -500, 1),               // position
      Vec(0, 50, 200, 1),                 // look at center of sphere row
      Vec(0, 1, 0, 1),                     // Y-up
      utils::Size(800, 600),
      55.0f));

  // Light
  // Key light: front-left, above
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(-300, 400, -100, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 250, 240, 255));

  // Fill light: front-right, dimmer
  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(400, 300, -200, 1));
  scene.getLight(1).setDiffuse(GeomColor(180, 200, 220, 255));

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
  ground->setMaterial(Material::fromColor(GeomColor(102, 89, 77, 255)));
  ground->getMaterial()->roughness = 0.8f;
  scene.addObject(ground.get());

  // Gold sphere — fromColor expects [0,255] range
  auto *goldRaw = SceneObject::sphere(-200, 100, 200, 100, 40, 40);
  auto goldSphere = std::shared_ptr<SceneObject>(goldRaw);
  goldSphere->setMaterial(Material::fromColor(GeomColor(255, 194, 87, 255)));
  goldSphere->getMaterial()->metallic = 1.0f;
  goldSphere->getMaterial()->roughness = 0.15f;
  scene.addObject(goldSphere.get());

  // Red plastic sphere
  auto *redRaw = SceneObject::sphere(0, 100, 200, 100, 40, 40);
  auto redSphere = std::shared_ptr<SceneObject>(redRaw);
  redSphere->setMaterial(Material::fromColor(GeomColor(204, 26, 26, 255)));
  redSphere->getMaterial()->roughness = 0.3f;
  scene.addObject(redSphere.get());

  // Green rubber sphere
  auto *greenRaw = SceneObject::sphere(200, 100, 200, 100, 40, 40);
  auto greenSphere = std::shared_ptr<SceneObject>(greenRaw);
  greenSphere->setMaterial(Material::fromColor(GeomColor(26, 153, 38, 255)));
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
