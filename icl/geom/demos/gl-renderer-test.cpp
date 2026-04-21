// Minimal test for GLRenderer: coordinate frame (X=red, Y=green, Z=blue)
#include <icl/qt/Common.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Scene.h>
#include <icl/geom/SceneObject.h>
#include <icl/geom/Material.h>
#include <icl/geom/GLRenderer.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static Scene scene;
static std::unique_ptr<GLRenderer> glRenderer;
static std::vector<std::shared_ptr<SceneObject>> objects;
HSplit gui;

/// Create a thin box along an axis
static std::shared_ptr<SceneObject> makeAxisBox(float length, float thickness,
                                                 int axis, const GeomColor &color) {
  float hx = (axis == 0) ? length/2 : thickness/2;
  float hy = (axis == 1) ? length/2 : thickness/2;
  float hz = (axis == 2) ? length/2 : thickness/2;
  float cx = (axis == 0) ? length/2 : 0;
  float cy = (axis == 1) ? length/2 : 0;
  float cz = (axis == 2) ? length/2 : 0;

  float params[] = { cx, cy, cz, hx*2, hy*2, hz*2 };
  auto *obj = new SceneObject("cuboid", params);
  auto sp = std::shared_ptr<SceneObject>(obj);
  sp->setVisible(Primitive::line, false);
  sp->setVisible(Primitive::vertex, false);
  sp->createAutoNormals(false);
  sp->setMaterial(Material::fromColor(color));
  sp->getMaterial()->roughness = 0.5f;
  return sp;
}

void init() {
  float len = 200, thick = 15;

  // X axis = red
  auto xAxis = makeAxisBox(len, thick, 0, GeomColor(255, 40, 40, 255));
  scene.addObject(xAxis.get()); objects.push_back(xAxis);

  // Y axis = green
  auto yAxis = makeAxisBox(len, thick, 1, GeomColor(40, 255, 40, 255));
  scene.addObject(yAxis.get()); objects.push_back(yAxis);

  // Z axis = blue
  auto zAxis = makeAxisBox(len, thick, 2, GeomColor(40, 40, 255, 255));
  scene.addObject(zAxis.get()); objects.push_back(zAxis);

  // Origin cube (white)
  float op[] = { 0, 0, 0, thick*2, thick*2, thick*2 };
  auto *oo = new SceneObject("cuboid", op);
  auto origin = std::shared_ptr<SceneObject>(oo);
  origin->setVisible(Primitive::line, false);
  origin->setVisible(Primitive::vertex, false);
  origin->createAutoNormals(false);
  origin->setMaterial(Material::fromColor(GeomColor(220, 220, 220, 255)));
  scene.addObject(origin.get()); objects.push_back(origin);

  // Camera looking at origin from upper-right-front
  scene.addCamera(Camera::lookAt(
      Vec(400, 300, -500, 1),  // position
      Vec(0, 0, 0, 1),        // target
      Vec(0, 1, 0, 1),        // up
      pa("-size"),
      55.0f));

  // Simple key light
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(300, 400, -200, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 250, 240, 255));

  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();

  glRenderer = std::make_unique<GLRenderer>();

  gui << (HSplit()
         << Canvas3D().handle("modern").minSize(32, 24).label("Modern GL")
         << Canvas3D().handle("legacy").minSize(32, 24).label("Legacy GL"))
      << Show();

  // Link modern renderer
  struct ModernCB : public ICLDrawWidget3D::GLCallback {
    void draw(ICLDrawWidget3D *) override {
      if (glRenderer) glRenderer->render(scene, 0);
    }
  };
  DrawHandle3D modern = gui["modern"];
  modern->link(new ModernCB());

  // Link legacy renderer
  DrawHandle3D legacy = gui["legacy"];
  legacy->link(scene.getGLCallback(0));
}

void run() {
  gui["modern"].render();
  gui["legacy"].render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv,
    "-size(Size=800x600)",
    init, run).exec();
}
