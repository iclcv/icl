// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: the tilt-maze — the physics2 port of the legacy
// physics-maze. A labyrinth board (many walls + a floor) is built as ONE
// kinematic COMPOUND body; tilt it with the sliders and the red ball rolls
// across it under gravity. Nine green holes are no-contact-response SENSORS
// that follow the board's tilt and flash yellow when the ball passes over them.
//
// Exercises, in one scene: compound bodies (btCompoundShape from a GroupNode),
// a kinematic body driven each frame, rolling rigid dynamics, and ghost sensors
// that move with a kinematic body (SensorDriver::setTransform).
//
//   left-drag    : tilt the board about X / Y (synced to the sliders, and back)
//   tilt sliders : tilt the board about X / Y
//   reset        : drop the ball back at the start + level the board

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/core/Img.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/CylinderNode.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/SensorDriver.h>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;
RigidBodyDriver *maze = nullptr;          // the kinematic compound board
RigidBodyDriver *ball = nullptr;
Time lastTick;

// --- maze geometry (legacy physics-maze coordinates) ------------------------
static const float SC = 4.f;              // scale the ~165mm legacy board up
static const float CX = 82, CY = 72;      // legacy centring offset
static const float P[39][2] = {
  {0,145}, {165,145}, {0,0}, {165,0},
  {25,122.5}, {42.5,122.5}, {60,122.5}, {85,122.5}, {102.5,122.5}, {122.5,122.5},
  {142.5,120}, {122.5,145}, {122.5,102.5}, {142.5,102.5}, {82.5,92.5}, {102.5,92.5},
  {22.5,80}, {145,82.5}, {22.5,70}, {42.5,70}, {65,72.5}, {82.5,72.5},
  {42.5,65}, {102.5,62.5}, {122.5,62.5}, {142.5,60}, {165,82.5}, {22.5,42.5},
  {102.5,42.5}, {142.5,42.5}, {0,22.5}, {40,22.5}, {62.5,22.5}, {102.5,25},
  {125,22.5}, {165,22.5}, {62.5,0}, {42.5,102.5}, {60,102.5}
};
static const int L[26][2] = {
  {0,1},{1,3},{3,2},{2,0},                                   // outer frame (0..3)
  {4,6},{5,22},{16,27},{18,19},{27,29},{11,9},{7,9},{8,15},{14,15},{14,21},
  {20,21},{30,31},{32,36},{10,13},{13,12},{12,24},{23,24},{23,33},{17,26},
  {25,29},{34,35},{37,38}
};
static const float HOLE[9][2] = {         // centred (x,y); z = -11 (board plane)
  {-74,-42},{-52,-21},{-27,-38},{-18,38},{2,-63},{13,29},{53,-21},{75,-42},{75,65}
};

// A 4-colour patchwork texture so the ball's rotation is easy to read.
static core::Image makePatchTexture() {
  const int S = 256, tiles = 6, ppt = S / tiles;
  static const int pal[4][3] = {{220,60,60},{245,245,245},{60,100,210},{250,205,40}};
  core::Img8u tex(Size(S,S), 4);
  core::Channel8u r=tex[0], g=tex[1], b=tex[2], a=tex[3];
  for (int y=0;y<S;y++) for (int x=0;x<S;x++) {
    const int *c = pal[((x/ppt)&1) + 2*((y/ppt)&1)];
    r(x,y)=c[0]; g(x,y)=c[1]; b(x,y)=c[2]; a(x,y)=255;
  }
  return core::Image(tex);
}

static Mat rotX(float t){ Mat m=Mat::id(); m(1,1)=cosf(t);m(1,2)=-sinf(t);m(2,1)=sinf(t);m(2,2)=cosf(t); return m; }
static Mat rotY(float t){ Mat m=Mat::id(); m(0,0)=cosf(t);m(0,2)=sinf(t);m(2,0)=-sinf(t);m(2,2)=cosf(t); return m; }
static Mat transAt(float x,float y,float z){ Mat m=Mat::id(); m(0,3)=x;m(1,3)=y;m(2,3)=z; return m; }

// the hole sensors + their visual disc nodes + base (untilted) poses + lit state
struct Hole { SensorDriver *sensor; std::shared_ptr<CylinderNode> disc; Mat base; bool lit=false; };
std::vector<Hole> holes;
Mat ballSpawn = Mat::id();

// --- drag-to-tilt interactor -----------------------------------------------
// Left-drag the board to tilt it (up to MAX_TILT away from the camera, about X
// and Y). The handler WRITES the tilt sliders, and run() reads the tilt back
// from them — so the mouse and the sliders stay in sync (and the sliders work
// on their own too).
static const float MAX_TILT = 0.35f;   // ~20 deg
static const float DRAG_GAIN = 1.4f;   // full board drag -> full tilt
Point32f pressPos; float pressTX = 0, pressTY = 0; bool dragging = false;

void onMouse(const MouseEvent &e) {
  if (e.isPressEvent() && e.isLeft()) {
    pressPos = e.getRelPos(); pressTX = gui["tx"]; pressTY = gui["ty"]; dragging = true;
  } else if (e.isReleaseEvent()) {
    dragging = false;
  } else if (e.isDragEvent() && dragging) {
    Point32f d = e.getRelPos() - pressPos;                  // normalized drag delta
    gui["ty"] = std::clamp(pressTY + d.x * DRAG_GAIN, -MAX_TILT, MAX_TILT);
    gui["tx"] = std::clamp(pressTX + d.y * DRAG_GAIN, -MAX_TILT, MAX_TILT);
  }
}

void init() {
  scene.setupDefault(DefaultScene::SceneType::Studio, 1000.f);

  // look straight down at the board (a labyrinth is played top-down). The board
  // is ~660 wide and sits near z=0; sit the camera well above it looking -Z, with
  // world +Y at the top of the image. (Real-display tweak: distance/focal framing.)
  scene.scene().getCamera(0) =
      Camera::lookAt(Vec(0,0,950,1), Vec(0,0,-30,1), Vec(0,1,0,1), Size::VGA, 50.f);

  auto wallMat = Material::fromColor(GeomColor(150,150,160,255));
  auto floorMat = Material::fromColor(GeomColor(225,225,225,255));

  // --- the maze board: one COMPOUND kinematic body (walls + floor) ----------
  auto board = std::make_shared<GroupNode>();
  auto addWall = [&](float cx,float cy,float cz,float sx,float sy,float sz){
    auto n = CuboidNode::create(0,0,0, sx*SC,sy*SC,sz*SC);
    n->setMaterial(wallMat); n->translate(cx*SC,cy*SC,cz*SC);
    board->addChild(std::static_pointer_cast<Node>(n));
  };
  const float h = 6;                       // half wall height
  for (int i = 0; i < 26; i++) {
    const float *a = P[L[i][0]], *b = P[L[i][1]];
    const float m = (i < 4) ? 2.5f : 2.f;  // outer frame is a touch thicker
    if (a[0] == b[0])  // vertical wall
      addWall(a[0]-CX, (a[1]+b[1])/2-CY, -h, 2*m, std::fabs(a[1]-b[1])+2*m, 2*h);
    else               // horizontal wall
      addWall((a[0]+b[0])/2-CX, a[1]-CY, -h, std::fabs(a[0]-b[0])+2*m, 2*m, 2*h);
  }
  { auto fl = CuboidNode::create(0,0,0, 170*SC,150*SC,3*SC);   // the floor
    fl->setMaterial(floorMat); fl->translate(0.5f*SC,0.5f*SC,-13.5f*SC);
    board->addChild(std::static_pointer_cast<Node>(fl)); }
  maze = scene.add(std::static_pointer_cast<Node>(board), 0.0f);
  maze->setKinematic(true);

  // --- the ball (patch-textured so its rolling reads clearly) ---------------
  auto bn = SphereNode::create(0,0,0, 7*SC, 32, 32);
  auto ballMat = std::make_shared<Material>();
  ballMat->baseColor = GeomColor(1,1,1,1);     // white * texture = texture
  ballMat->roughness = 0.5f;
  ballMat->textures = std::make_shared<Material::TextureMaps>();
  ballMat->textures->baseColorMap = makePatchTexture();
  bn->setMaterial(ballMat);
  bn->translate(15*SC, 0, -3*SC);
  ballSpawn = bn->getTransformation(true);
  ball = scene.add(std::static_pointer_cast<Node>(bn), 1.0f);
  ball->setRollingFriction(0.0f); ball->setFriction(0.5f); ball->setRestitution(0.1f);

  // --- the nine holes: no-response SENSORS that follow the board ------------
  for (int i = 0; i < 9; i++) {
    auto disc = CylinderNode::create(0,0,0, 11*SC,11*SC, 6*SC, 24);
    disc->setMaterial(Material::fromColor(GeomColor(60,190,90,255)));
    Mat base = transAt(HOLE[i][0]*SC, HOLE[i][1]*SC, -11*SC);
    disc->setTransformation(base);
    auto *sensor = scene.addSensor(std::static_pointer_cast<Node>(disc));
    holes.push_back({sensor, disc, base, false});
  }

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(14,99).minSize(14,1)
              << Label("drag the board to tilt", {.handle="hint"})
              << FSlider(-MAX_TILT, MAX_TILT, 0.0f, {.handle="tx", .label="tilt about X"})
              << FSlider(-MAX_TILT, MAX_TILT, 0.0f, {.handle="ty", .label="tilt about Y"})
              << Button("reset ball", {.handle="reset"})
              << CheckBox("collision debug", {.handle="dbg"}))
          << Canvas3D(Size(900,650), {.handle="draw"}))
      << Show();
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(&onMouse);            // left-drag tilts the board
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  // tilt the kinematic board; the holes (sensors + discs) follow the same tilt
  Mat tilt = rotX((float)gui["tx"]) * rotY((float)gui["ty"]);
  maze->setKinematicTransform(tilt);
  for (auto &hole : holes) {
    Mat pose = tilt * hole.base;
    hole.disc->setTransformation(pose);   // render
    hole.sensor->setTransform(pose);      // ghost zone
  }

  static ButtonHandle reset = gui["reset"];
  if (reset.wasTriggered()) {                                // respawn + level the board
    ball->setTransform(ballSpawn);
    gui["tx"] = 0.f; gui["ty"] = 0.f;
  }

  // light a hole green->yellow while the ball is over it
  auto *ballDrv = static_cast<Driver*>(ball);
  for (auto &hole : holes) {
    bool over = false;
    for (auto *d : hole.sensor->getOverlappingDrivers()) if (d == ballDrv) over = true;
    if (over != hole.lit) {
      hole.lit = over;
      hole.disc->setMaterial(Material::fromColor(over ? GeomColor(240,220,40,255)
                                                      : GeomColor(60,190,90,255)));
    }
  }

  scene.setDebugDrawEnabled(gui["dbg"]);
  scene.sync(dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
