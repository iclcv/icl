/* Test the geom2 CyclesRenderer API: create a Scene2 with nodes,
 * render through Cycles, and save the result to PNG.
 *
 * Usage:
 *   cycles-renderer-test [-o output.png] [-samples 256]
 */

#include <icl/geom2/CyclesRenderer.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>
#include <icl/io/FileWriter.h>

#include <cstdio>
#include <memory>

using namespace icl;
using namespace icl::geom;
using namespace icl::geom2;

int main(int argc, const char **argv) {
  std::string output = "cycles-renderer-test.png";
  int samples = 256;
  for (int i = 1; i < argc; ++i) {
    if (std::string("-o") == argv[i] && i + 1 < argc)
      output = argv[++i];
    if (std::string("-samples") == argv[i] && i + 1 < argc)
      samples = std::atoi(argv[++i]);
  }

  Scene2 scene;

  // Camera: front-above, looking at the sphere row
  scene.addCamera(Camera::lookAt(
      Vec(0, 300, -500, 1),
      Vec(0, 50, 200, 1),
      Vec(0, 1, 0, 1),
      utils::Size(800, 600),
      55.0f));

  // Lights
  auto keyLight = std::make_shared<LightNode>();
  keyLight->setColor(GeomColor(255, 250, 240, 255));
  keyLight->setIntensity(1.0f);
  keyLight->translate(-300, 400, -100);
  scene.addLight(keyLight);

  auto fillLight = std::make_shared<LightNode>();
  fillLight->setColor(GeomColor(180, 200, 220, 255));
  fillLight->setIntensity(0.6f);
  fillLight->translate(400, 300, -200);
  scene.addLight(fillLight);

  // Ground plane
  auto ground = std::make_shared<MeshNode>();
  ground->addVertex(Vec(-500, 0, -500, 1));
  ground->addVertex(Vec( 500, 0, -500, 1));
  ground->addVertex(Vec( 500, 0,  500, 1));
  ground->addVertex(Vec(-500, 0,  500, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addNormal(Vec(0, 1, 0, 1));
  ground->addTriangle(0, 1, 2, 0, 1, 2);
  ground->addTriangle(0, 2, 3, 0, 2, 3);
  ground->setMaterial(Material::fromColor(GeomColor(102, 89, 77, 255)));
  ground->getMaterial()->roughness = 0.8f;
  scene.addNode(ground);

  // Gold sphere
  auto gold = std::make_shared<SphereNode>(-200, 100, 200, 100, 40, 40);
  gold->setMaterial(Material::fromColor(GeomColor(255, 194, 87, 255)));
  gold->getMaterial()->metallic = 1.0f;
  gold->getMaterial()->roughness = 0.15f;
  scene.addNode(gold);

  // Red plastic sphere
  auto red = std::make_shared<SphereNode>(0, 100, 200, 100, 40, 40);
  red->setMaterial(Material::fromColor(GeomColor(204, 26, 26, 255)));
  red->getMaterial()->roughness = 0.3f;
  scene.addNode(red);

  // Green rubber sphere
  auto green = std::make_shared<SphereNode>(200, 100, 200, 100, 40, 40);
  green->setMaterial(Material::fromColor(GeomColor(26, 153, 38, 255)));
  green->getMaterial()->roughness = 0.9f;
  scene.addNode(green);

  // Render
  printf("Rendering with geom2::CyclesRenderer (%d samples)...\n", samples);
  CyclesRenderer renderer(scene, RenderQuality::Final);
  renderer.setSamples(samples);
  renderer.setSceneScale(1.0f);
  renderer.renderBlocking(0);

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
