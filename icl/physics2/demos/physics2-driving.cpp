// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: driving game — MILESTONE 2 (controls + chase camera).
// A raycast-vehicle car on a flat ground, driven with the keyboard, followed by
// a third-person chase camera. Runs in the default unified Deformable world.
//
//   W / S : throttle forward / reverse
//   A / D : steer left / right
//   Space : brake
//
// M3 adds a proper course (ramps, jumps); M4 adds interactive object stations.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/KeyboardHandler.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/VehicleDriver.h>
#include <cmath>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;              // default unified (Deformable) world
VehicleDriver *car = nullptr;
KeyboardHandler keys;            // WASD + space, polled each frame
Time lastTick;

// --- a simple third-person chase camera (demo-local; promote to geom2 if reused) ---
static Vec cnorm(const Vec &v) {
  float l = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  l = l > 1e-6f ? l : 1.f;
  return Vec(v[0]/l, v[1]/l, v[2]/l, 1);
}
struct ChaseCamera {
  Vec pos{0,0,0,1};
  bool seeded = false;
  void update(Camera &cam, const Mat &carPose, double dt) {
    Vec p(carPose(0,3), carPose(1,3), carPose(2,3), 1);          // car position
    Vec fwd = cnorm(Vec(carPose(0,1), carPose(1,1), carPose(2,1), 1)); // car local +Y
    const float behind = 4200, up = 1700, ahead = 900, targetUp = 200;
    Vec desired(p[0] - fwd[0]*behind, p[1] - fwd[1]*behind, p[2] - fwd[2]*behind + up, 1);
    if (!seeded) { pos = desired; seeded = true; }
    float a = 1.f - std::exp(-4.0f * (float)dt);                 // exponential smoothing
    pos = Vec(pos[0] + (desired[0]-pos[0])*a, pos[1] + (desired[1]-pos[1])*a,
              pos[2] + (desired[2]-pos[2])*a, 1);
    Vec target(p[0] + fwd[0]*ahead, p[1] + fwd[1]*ahead, p[2] + targetUp, 1);
    cam.setPosition(pos);
    cam.setNorm(cnorm(Vec(target[0]-pos[0], target[1]-pos[1], target[2]-pos[2], 1)), false);
    // ICL's m_up points toward +image-Y (the image BOTTOM), so the visual up
    // (+Z) must be negated — same convention Camera::lookAt applies internally.
    cam.setUp(Vec(0,0,-1,1), true);
  }
} chase;

// rotations for tilted course pieces (pitch about X, roll about Y)
static Mat rotX(float t){ Mat m=Mat::id(); m(1,1)=cosf(t);m(1,2)=-sinf(t);m(2,1)=sinf(t);m(2,2)=cosf(t); return m; }
static Mat rotY(float t){ Mat m=Mat::id(); m(0,0)=cosf(t);m(0,2)=sinf(t);m(2,0)=-sinf(t);m(2,2)=cosf(t); return m; }

void init() {
  const float EXT = 4000.f;
  scene.setupDefault(DefaultScene::SceneType::Studio, EXT);
  const float G = -EXT * 0.52f;   // ground top (where things rest)

  // --- the playground course: static geometry the car drives over ---
  auto addStatic = [&](float cx, float cy, float cz, float sx, float sy, float sz,
                       GeomColor col, const Mat &rot = Mat::id()) {
    auto n = CuboidNode::create(0, 0, 0, sx, sy, sz);
    n->setMaterial(Material::fromColor(col));
    Mat m = rot; m(0,3) = cx; m(1,3) = cy; m(2,3) = cz;   // world = R with translation
    n->setTransformation(m);
    scene.add(std::static_pointer_cast<Node>(n), 0.0f);   // static
  };
  const GeomColor gray(150,150,150,255), orange(230,140,40,255),
                  blue(70,110,200,255), red(210,70,70,255), green(80,170,90,255);

  // boundary walls (16000 x 16000 arena)
  addStatic(0, 8000, G+500, 16000, 300, 1000, gray);
  addStatic(0,-8000, G+500, 16000, 300, 1000, gray);
  addStatic( 8000, 0, G+500, 300, 16000, 1000, gray);
  addStatic(-8000, 0, G+500, 300, 16000, 1000, gray);

  // climb ramp ahead (+Y): low end BURIED in the ground so the top surface
  // emerges at ground level (no floating edge), rising to a raised platform.
  addStatic(0, 2800, G+150, 3000, 4000, 200, orange, rotX(0.14f));  // far top ~G+528
  addStatic(0, 6000, G+428, 3000, 2400, 200, orange);               // platform, top ~G+528
  // jump kicker behind (-Y): near end buried, far end a launch lip
  addStatic(0,-3500, G+150, 3000, 2400, 200, red,   rotX(-0.26f));
  // banked turn on the +X side (rolled about Y); near (+X) edge buried
  addStatic(5500, 0, G+150, 2200, 7000, 260, blue,  rotY(0.30f));
  // blocks scattered across the arena to weave through
  addStatic(-3500, 2800, G+350, 700,700,700, green);
  addStatic(-4800,-2200, G+350, 700,700,700, green);
  addStatic( 3000,-3800, G+350, 700,700,700, green);
  addStatic(-1800,-5200, G+350, 700,700,700, green);
  addStatic( 4200, 3500, G+350, 700,700,700, green);

  auto body = CuboidNode::create(0, 0, 0, 600, 1200, 300);
  body->setMaterial(Material::fromColor(GeomColor(200, 60, 60, 255)));
  body->translate(0, 0, G + 320);   // just above the ground -> settles on its wheels
  car = scene.addVehicle(std::static_pointer_cast<Node>(body));
  car->setCcd(300.f, 60.f);         // don't tunnel through ramps/walls at speed

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(12, 99).minSize(12, 1)
              << Label("W/S or Up/Down", {.handle = "h1"})
              << Label("A/D or Left/Right", {.handle = "h2"})
              << Label("Space brake", {.handle = "h3"})
              << CheckBox("collision debug", {.handle = "dbg"}))
          << Canvas3D(Size(900, 650), {.handle = "draw"}))
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(&keys);       // keyboard drives the car
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  const float ENGINE = 20000.f, STEER = 0.4f, BRAKE = 3000.f;
  bool fwd = keys.held(Qt::Key_W) || keys.held(Qt::Key_Up);
  bool rev = keys.held(Qt::Key_S) || keys.held(Qt::Key_Down);
  bool left = keys.held(Qt::Key_A) || keys.held(Qt::Key_Left);
  bool right = keys.held(Qt::Key_D) || keys.held(Qt::Key_Right);
  float throttle = fwd ? ENGINE : rev ? -ENGINE * 0.6f : 0.f;
  float steer = left ? STEER : right ? -STEER : 0.f;
  float brake = keys.held(Qt::Key_Space) ? BRAKE : 0.f;
  car->setEngineForce(throttle);
  car->setSteering(steer);
  car->setBrake(brake);
  scene.setDebugDrawEnabled(gui["dbg"]);

  scene.sync(dt);
  chase.update(scene.scene().getCamera(0), car->getChassisPose(), dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
