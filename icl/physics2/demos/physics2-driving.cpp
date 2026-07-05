// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: driving game — MILESTONE 4 (interactive stations).
// A raycast-vehicle car you drive around an open playground, poking at one of
// every physics2 subsystem. The car *drives into* each station instead of each
// being checked in isolation — this is the live, end-to-end integration test.
//
//   W / S or Up / Down   : throttle forward / reverse
//   A / D or Left / Right : steer left / right
//   Space                : brake
//
// Stations (all in the one unified Deformable world):
//   - RIGID       : a box pyramid to smash, a cluster of barrels to scatter
//   - CONSTRAINTS : a swing gate (hinge), a see-saw (hinge + angular limits),
//                   a wrecking ball (ball-socket pendulum), spring bollards
//   - a drive-through gateway arch (was a soft-body cloth banner — see the note
//     at station 7 for why the cloth is temporarily out)
// Plus the M3 course furniture: a climb ramp, a jump kicker and a banked turn.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/KeyboardHandler.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/CylinderNode.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/Constraint.h>
#include <icl/physics2/VehicleDriver.h>
#include <cmath>

using namespace icl::viz3d;
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
Mat carSpawn = Mat::id();        // where the reset button respawns the car

// --- a simple third-person chase camera (demo-local; promote to viz3d if reused) ---
static Vec cnorm(const Vec &v) {
  float l = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  l = l > 1e-6f ? l : 1.f;
  return Vec(v[0]/l, v[1]/l, v[2]/l, 1);
}
struct ChaseCamera {
  Vec pos{0,0,0,1};
  bool seeded = false;
  float distance = 6000;   // behind-the-car distance (live via the slider)
  void update(Camera &cam, const Mat &carPose, double dt) {
    Vec p(carPose(0,3), carPose(1,3), carPose(2,3), 1);          // car position
    Vec fwd = cnorm(Vec(carPose(0,1), carPose(1,1), carPose(2,1), 1)); // car local +Y
    // height / look-ahead scale with the follow distance (keeps the framing)
    const float behind = distance, up = distance*0.40f,
                ahead = distance*0.21f, targetUp = distance*0.05f;
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
  const float S = 3.f;            // course scale: a roomy, well-spaced arena
  const float EXT = 4000.f * S;
  scene.setupDefault(DefaultScene::SceneType::Studio, EXT);
  const float G = -EXT * 0.52f;   // ground top (where things rest)

  const GeomColor gray(150,150,150,255), orange(230,140,40,255),
                  blue(70,110,200,255), red(210,70,70,255), green(80,170,90,255),
                  yellow(240,200,40,255), purple(170,80,200,255), brown(150,100,60,255);

  // --- builders ---------------------------------------------------------------
  // node factories (geom is origin-centred; translate into place)
  auto mkBox = [&](float cx,float cy,float cz, float sx,float sy,float sz, GeomColor col){
    auto n = CuboidNode::create(0,0,0, sx,sy,sz);
    n->setMaterial(Material::fromColor(col)); n->translate(cx,cy,cz); return n; };
  auto mkCyl = [&](float cx,float cy,float cz, float r,float h, GeomColor col){
    auto n = CylinderNode::create(0,0,0, r,r,h); // axis is Z -> stands upright
    n->setMaterial(Material::fromColor(col)); n->translate(cx,cy,cz); return n; };
  auto mkSph = [&](float cx,float cy,float cz, float r, GeomColor col){
    auto n = SphereNode::create(0,0,0, r);
    n->setMaterial(Material::fromColor(col)); n->translate(cx,cy,cz); return n; };
  // add helpers (static = mass 0; dynamic returns its driver for tuning)
  auto addStat = [&](NodePtr n){ scene.add(n, 0.0f); };
  auto addDyn  = [&](NodePtr n, float mass, float dl, float da){
    auto d = scene.add(n, mass); d->setDamping(dl, da); return d; };

  // --- M3 course furniture (static). Sizes stay car-tuned; positions scale by S. -
  // boundary walls (the arena edge)
  addStat(mkBox(0, 8000*S, G+500, 16000*S, 300, 1000, gray));
  addStat(mkBox(0,-8000*S, G+500, 16000*S, 300, 1000, gray));
  addStat(mkBox( 8000*S, 0, G+500, 300, 16000*S, 1000, gray));
  addStat(mkBox(-8000*S, 0, G+500, 300, 16000*S, 1000, gray));
  // climb ramp ahead (+Y): low end buried so the surface emerges at ground level
  { auto n = mkBox(0, 2800*S, G+150, 3000, 4000, 200, orange); n->setTransformation(
      [&]{ Mat m=rotX(0.14f); m(0,3)=0; m(1,3)=2800*S; m(2,3)=G+150; return m; }()); addStat(n); }
  addStat(mkBox(0, 6000*S, G+428, 3000, 2400, 200, orange));   // raised platform
  // jump kicker behind (-Y)
  { auto n = mkBox(0,-3500*S, G+150, 3000, 2400, 200, red); n->setTransformation(
      [&]{ Mat m=rotX(-0.26f); m(0,3)=0; m(1,3)=-3500*S; m(2,3)=G+150; return m; }()); addStat(n); }
  // banked turn on the +X side (rolled about Y)
  { auto n = mkBox(5500*S, 0, G+150, 2200, 7000, 260, blue); n->setTransformation(
      [&]{ Mat m=rotY(0.30f); m(0,3)=5500*S; m(1,3)=0; m(2,3)=G+150; return m; }()); addStat(n); }

  // === STATION 1 — RIGID: a box pyramid to smash (NW) =========================
  // Heavy boxes (mass 25): hitting them visibly slows the car instead of being
  // brushed aside.
  {
    const float px = -3000*S, py = 4500*S, s = 480;
    auto layer = [&](float z, std::initializer_list<float> xs){
      for (float x : xs) addDyn(mkBox(px+x, py, z, s,s,s, brown), 25.f, 0.05f, 0.05f); };
    layer(G + 0.5f*s,      {-720,-240, 240, 720});
    layer(G + 1.5f*s,      {-480,   0, 480});
    layer(G + 2.5f*s,      {-240, 240});
    layer(G + 3.5f*s,      {0});
  }

  // === STATION 2 — RIGID: barrels to scatter (SE) =============================
  {
    const float px = 3000*S, py = -2000*S;
    for (int i = 0; i < 5; i++) {
      float ang = i * 1.2566f;   // 2*pi/5, a loose ring
      addDyn(mkCyl(px + 500*std::cos(ang), py + 500*std::sin(ang), G+350,
                   250, 700, yellow), 15.f, 0.05f, 0.05f);
    }
  }

  // === STATION 3 — CONSTRAINTS: a swing gate (hinge about Z, W of spawn) ======
  {
    const float gx = -2000*S, gy = 0;
    auto post  = mkBox(gx,        gy, G+475, 150,150, 950, gray);   // hinge post
    auto post2 = mkBox(gx + 1300, gy, G+475, 150,150, 950, gray);   // frame post
    addStat(post); addStat(post2);
    auto door = mkBox(gx + 650, gy, G+475, 1200, 80, 900, blue);
    addDyn(door, 2.0f, 0.05f, 0.3f);
    // hinge axis Z (vertical) -> swings horizontally when the car pushes through.
    // pivot world (gx+50,gy,*) : post-local (50,0,0) == door-local (-600,0,0).
    scene.addHinge(post, door, Vec(50,0,0,1), Vec(-600,0,0,1), 2);
  }

  // === STATION 4 — CONSTRAINTS: a see-saw (hinge about X + angular limits) =====
  {
    const float px = -5500*S, py = 2500*S;
    auto fulcrum = mkBox(px, py, G+200, 500,500,400, gray);
    addStat(fulcrum);
    auto plank = mkBox(px, py, G+440, 500, 3000, 80, green);   // long in Y, tips in YZ
    addDyn(plank, 3.0f, 0.1f, 0.3f);
    // hinge about X (axis 0) at the plank centre; pivot world (px,py,G+440):
    // fulcrum-local (0,0,240) == plank-local (0,0,0).
    auto c = scene.addHinge(fulcrum, plank, Vec(0,0,240,1), Vec(0,0,0,1), 0);
    c->setAngularLimits(Vec(-0.25f,0,0,1), Vec(0.25f,0,0,1));  // teeter stops
  }

  // === STATION 5 — CONSTRAINTS: a wrecking ball (ball-socket pendulum, SW) =====
  // The ball hangs low (centre ~G+700, bottom ~G+350) so the car can actually
  // strike it — a long tether off a high anchor gives it a big, slow swing.
  {
    const float px = -6000*S, py = -2500*S;
    auto pillar = mkBox(px, py, G+1300, 200,200,2600, gray);    // ground -> G+2600
    auto arm    = mkBox(px, py-500, G+2500, 150,1100,150, gray);// decorative jib
    addStat(pillar); addStat(arm);
    auto ball = mkSph(px, py-1000, G+700, 350, red);
    addDyn(ball, 8.0f, 0.02f, 0.05f);
    // anchor world (px,py-1000,G+2500): pillar-local (0,-1000,1200) == ball-local (0,0,1800)
    scene.addBallSocket(pillar, ball, Vec(0,-1000,1200,1), Vec(0,0,1800,1));
  }

  // === STATION 6 — CONSTRAINTS: spring bollards (spring back when nudged, NE) ==
  {
    const float px = 2500*S, py = 2500*S;
    for (int i = 0; i < 3; i++) {
      float bx = px + (i-1)*700, by = py;
      auto bollard = mkCyl(bx, by, G+300, 120, 600, purple);
      addDyn(bollard, 1.2f, 0.2f, 0.4f);
      scene.addSpring(bollard, Vec(0,0,0,1), Vec(bx,by,G+300,1), 3000.f, 0.7f);
    }
  }

  // === STATION 7 — a gateway arch to drive through (S) ========================
  // NOTE: this was a soft-body cloth banner. The deformable cloth is stable
  // headless (6000 stepOnce steps, exact threaded params) and in the simple
  // physics2-cloth demo, but goes NaN / explodes in *this* threaded scene
  // (many rigid bodies + constraints + vehicle in the one Deformable world) —
  // a threaded-only soft-body instability that no deterministic repro triggers.
  // Replaced with a static arch until that's root-caused; see next.md.
  {
    const float px = -4500*S, py = -5000*S, halfW = 800;
    addStat(mkBox(px-halfW, py, G+800,  150,150,1600, gray));   // left post
    addStat(mkBox(px+halfW, py, G+800,  150,150,1600, gray));   // right post
    addStat(mkBox(px,       py, G+1525, 2*halfW+150,150,150, orange)); // cross beam
  }

  // --- the car (spawned last, at the origin, facing +Y) -----------------------
  auto body = mkBox(0, 0, G + 320, 600, 1200, 300, GeomColor(200,60,60,255));
  carSpawn = body->getTransformation(true);   // remember it for the reset button
  car = scene.addVehicle(std::static_pointer_cast<Node>(body));
  car->setCcd(300.f, 60.f);         // don't tunnel through ramps/walls at speed

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(12, 99).minSize(12, 1)
              << Label("W/S or Up/Down", {.handle = "h1"})
              << Label("A/D or Left/Right", {.handle = "h2"})
              << Label("Space brake", {.handle = "h3"})
              << Button("reset car", {.handle = "reset"})
              << FSlider(3000, 24000, 6000, {.handle = "camdist", .label = "cam distance"})
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

  static ButtonHandle reset = gui["reset"];
  if (reset.wasTriggered()) car->reset(carSpawn);
  chase.distance = gui["camdist"];
  scene.setDebugDrawEnabled(gui["dbg"]);

  scene.sync(dt);
  chase.update(scene.scene().getCamera(0), car->getChassisPose(), dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
