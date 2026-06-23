// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL geom2 demo: in-scene "monitors" showing offscreen renders of the scene
//
// The scene contains 4 cameras and 4 screen quads. Each frame every camera is
// rendered offscreen (Scene2::renderToImage) and the result is pushed onto the
// matching screen as a live texture (Material::setBaseColorMap → the renderer
// re-uploads only that material). Because the screens are part of the scene,
// each monitor also shows the other monitors — a one-frame "security-room"
// feedback. A combo selects which camera the mouse controls; the main view
// shows that camera, so you can fly any of the four around and watch its
// monitor update.
//
// Everything (main view + the 4 offscreen captures + texture uploads) runs in
// the canvas paint callback, where the GL context is current; the captures are
// throttled so the interaction stays responsive.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/GLCallback.h>
#include <icl/qt/DrawWidget3D.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/CoordinateFrameNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/utils/time/Time.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;

// Camera / monitor resolution (the offscreen render size = screen texture size)
static const Size CAP_RES(256, 192);
static const float HALF_PI = 1.5707963f;

GUI gui;
Scene2 scene;
std::shared_ptr<GroupNode> spinner;
std::shared_ptr<MeshNode> screens[4];
int activeCam = 0;

void buildScene() {
  // 4 cameras looking at the central content from front / right / back / left
  const float R = 320, CH = 90, CZ = 60;
  const Vec center(0, 0, CZ, 1), up(0, 0, 1, 1);
  scene.addCamera(Camera::lookAt(Vec( 0, -R, CH, 1), center, up, CAP_RES, 45.0f));
  scene.addCamera(Camera::lookAt(Vec( R,  0, CH, 1), center, up, CAP_RES, 45.0f));
  scene.addCamera(Camera::lookAt(Vec( 0,  R, CH, 1), center, up, CAP_RES, 45.0f));
  scene.addCamera(Camera::lookAt(Vec(-R,  0, CH, 1), center, up, CAP_RES, 45.0f));

  // Central spinning content (a few shapes grouped so they rotate together)
  spinner = std::make_shared<GroupNode>();
  auto sph = SphereNode::create(0, 0, 70, 45, 32, 32);
  sph->setMaterial(Material::fromColor(GeomColor(220, 70, 70, 255)));
  spinner->addChild(sph);
  auto cube = CuboidNode::create(120, 0, 45, 60, 60, 90);
  cube->setMaterial(Material::fromColor(GeomColor(70, 90, 230, 255)));
  cube->getMaterial()->metallic = 0.6f; cube->getMaterial()->roughness = 0.25f;
  spinner->addChild(cube);
  auto cone = ConeNode::create(-120, 0, 30, 45, 45, 90, 28);
  cone->setMaterial(Material::fromColor(GeomColor(240, 200, 60, 255)));
  spinner->addChild(cone);
  scene.addNode(spinner);

  // Ground + world frame
  auto ground = std::make_shared<MeshNode>();
  float gs = 340;
  ground->addVertex(Vec(-gs, -gs, 0, 1));
  ground->addVertex(Vec( gs, -gs, 0, 1));
  ground->addVertex(Vec( gs,  gs, 0, 1));
  ground->addVertex(Vec(-gs,  gs, 0, 1));
  ground->addQuad(0, 1, 2, 3);
  ground->createAutoNormals(false);
  ground->setMaterial(Material::fromColor(GeomColor(160, 160, 165, 255)));
  scene.addNode(ground);
  scene.addNode(CoordinateFrameNode::create(100, 2.5f));

  // 4 monitors in a back-wall row, standing upright facing -Y. Start with a
  // flat gray placeholder; the live camera renders replace it every frame.
  Img8u placeholder(CAP_RES, formatRGB);
  placeholder.clear(-1, 50, false);
  const float SX[4] = {-258, -86, 86, 258};
  for (int i = 0; i < 4; i++) {
    screens[i] = MeshNode::createTexturedQuad(160, 120, placeholder);
    screens[i]->translate(SX[i], 215, 105);   // position …
    screens[i]->rotate(HALF_PI, 0, 0);         // … then stand it up, facing -Y
    scene.addNode(screens[i]);
  }

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setColor(GeomColor(1.0f, 0.97f, 0.92f, 1.0f));
  light->setIntensity(1.6f);
  light->translate(250, -150, 400);
  scene.addLight(light);

  scene.setBounds(450);
}

// Main canvas callback (GUI thread, context current): spin the content, render
// every camera offscreen into its monitor, then draw the active camera's view.
struct ViewCallback : public GLCallback {
  void draw(ICLDrawWidget3D *) override {
    if (spinner) spinner->rotate(0, 0, 0.012f);   // gentle spin (race-free here)

    static Time lastCap = Time::now();
    if (lastCap.age().toMilliSeconds() >= 50) {    // ~20 Hz (4 renders+readbacks)
      lastCap = Time::now();
      for (int i = 0; i < 4; i++) {
        BVH::ImageResult r = scene.renderToImage(i, BVH::NoDepth);   // color only
        if (r.image.getDim() && screens[i] && screens[i]->getMaterial())
          screens[i]->getMaterial()->setBaseColorMap(r.image);       // live texture
      }
    }
    scene.render(activeCam);
  }
};
ViewCallback viewCB;

// Switch which camera the mouse controls (and which the main view shows).
void changeActiveCam() {
  const int newCam = ComboHandle(gui["cam"]).getSelectedIndex();
  if (newCam == activeCam) return;
  ICLDrawWidget3D *w = DrawHandle3D(gui["view"]).operator->();
  w->uninstall(scene.getMouseHandler(activeCam));
  w->install(scene.getMouseHandler(newCam));
  activeCam = newCam;
}

void init() {
  buildScene();

  gui << (HSplit()
      << Canvas3D(Size(820, 620), {.handle="view", .label="scene + live monitors", .minSize={40, 30}})
      << (VBox({.minSize={11, 1}, .maxSize={11, 100}})
          << Combo("cam 0 (front),cam 1 (right),cam 2 (back),cam 3 (left)",
                   {.handle="cam", .label="move camera"})
          << Fps({.handle="fps", .label="fps"})))
      << Show();

  gui["view"].link(&viewCB);
  gui["view"].install(scene.getMouseHandler(activeCam));
  gui["cam"].registerCallback(changeActiveCam);
}

void run() {
  gui["view"].render();
  gui["fps"].render();
  static FPSLimiter limiter(60);
  limiter.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
