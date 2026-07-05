// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: water-air-pressure bottle rocket — the physics2 port of the
// legacy physics-water-rocket.
//
// ⚠️ STATUS: MOSTLY BROKEN (real display, Session 78). The flight *mechanics* are
//    validated headless (test physics2.rocket_compound_thrust_to_apogee: the
//    compound bottle launches, sheds mass, reaches apogee), but the live demo does
//    not behave acceptably yet — needs real-display work on the thrust/mass unit
//    scale, the follow camera, the contact-only tip separation, and the (here only
//    faked) parachute. UNDECIDED whether to keep this demo at all; do not rely on
//    it. See next.md.
//
//   A 1 L PET bottle (a COMPOUND body: body + shoulder + neck + crossed fins) is
//   launched straight up by thrust. During the powered phase the loose nose tip
//   is held on the head purely by CONTACT (the accelerating bottle presses into
//   it). At burnout the bottle decelerates (air drag) faster than the free tip,
//   so the tip separates by itself near the zenith — no constraint involved. At
//   apogee (detected via getLinearVelocity) a parachute deploys: a drag spike on
//   the bottle plus a visual canopy, and the rocket drifts back down.
//
// Exercises: compound rigid bodies, dynamic mass change (water drains), a thrust
// force over the burn, apogee detection, and contact-only part separation.
//
//   Launch / Reset, pressure + water-fill sliders.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/CylinderNode.h>
#include <icl/viz3d/ConeNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/viz3d/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <cmath>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

// --- bottle geometry (mm, z from the base; total ~300, ~90 dia) -------------
static const float BODY_R=45, BODY_H=180, SHLD_R=30, SHLD_H=40,
                   NECK_R=15, NECK_H=30, HEAD_Z=250, TIP_R=16, TIP_H=60;
static const float EMPTY_MASS=0.06f, TIP_MASS=0.012f, THRUST_PER_BAR=2000.f;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;
RigidBodyDriver *rocket=nullptr, *tip=nullptr;
std::shared_ptr<ConeNode> canopy;          // parachute visual (shown at apogee)
Mat rocketHome, tipHome;
float groundZ = 0;
Time lastTick, launchTime;

enum RocketState { ARMED, THRUST, COAST, DESCENT, LANDED };
RocketState state = ARMED;
float burnTime=0.5f, thrust=12000.f, waterMass=0.2f;

static const char *stateName(RocketState s){
  switch(s){case ARMED:return "ARMED";case THRUST:return "THRUST";case COAST:return "COAST";
            case DESCENT:return "DESCENT";default:return "LANDED";} }

void init() {
  const float EXT = 3000.f;
  scene.setupDefault(DefaultScene::SceneType::Studio, EXT);
  groundZ = -EXT * 0.52f;                   // the launch-pad surface (DefaultScene ground)

  auto green = Material::fromColor(GeomColor(40,170,60,255));
  auto white = Material::fromColor(GeomColor(230,230,230,255));
  auto red   = Material::fromColor(GeomColor(180,40,40,255));

  // --- the bottle: ONE compound body (body + shoulder + neck + crossed fins) ---
  auto bottle = std::make_shared<GroupNode>();
  auto add = [&](NodePtr n){ bottle->addChild(n); };
  { auto b=CylinderNode::create(0,0, BODY_H/2,               BODY_R,BODY_R, BODY_H, 24); b->setMaterial(green); add(b); }
  { auto s=CylinderNode::create(0,0, BODY_H+SHLD_H/2,        SHLD_R,SHLD_R, SHLD_H, 24); s->setMaterial(green); add(s); }
  { auto k=CylinderNode::create(0,0, BODY_H+SHLD_H+NECK_H/2, NECK_R,NECK_R, NECK_H, 16); k->setMaterial(white); add(k); }
  { auto f=CuboidNode::create(0,0, 45, 2*BODY_R+30, 3, 80); f->setMaterial(red); add(f); }
  { auto f=CuboidNode::create(0,0, 45, 3, 2*BODY_R+30, 80); f->setMaterial(red); add(f); }
  bottle->translate(0,0,groundZ);
  rocketHome = bottle->getTransformation(true);
  rocket = scene.add(std::static_pointer_cast<Node>(bottle), EMPTY_MASS);
  rocket->setFriction(0.6f); rocket->setRestitution(0.1f);
  rocket->setDamping(0.f, 0.6f);            // angular damping keeps it upright

  // --- the loose nose tip, resting on the neck (held by contact, no joint) ---
  auto tn = ConeNode::create(0,0, TIP_H/2, TIP_R, TIP_R, TIP_H, 16);
  tn->setMaterial(white);
  tn->translate(0,0, groundZ + HEAD_Z);
  tipHome = tn->getTransformation(true);
  tip = scene.add(std::static_pointer_cast<Node>(tn), TIP_MASS);
  tip->setFriction(0.6f); tip->setRestitution(0.05f); tip->setDamping(0.02f, 0.3f);

  // --- parachute canopy (a wide shallow cone, hidden until deploy) ---
  canopy = ConeNode::create(0,0,0, 360,360, 200, 24);
  canopy->setMaterial(Material::fromColor(GeomColor(255,140,0,255)));
  canopy->setVisible(false);
  scene.add(std::static_pointer_cast<Node>(canopy), 0.0f);   // static node, posed by hand

  scene.start(120);

  gui << (HBox()
          << Canvas3D(Size(900,650), {.handle="draw"})
          << (VBox().maxSize(16,99).minSize(16,1)
              << Button("Launch", {.handle="launch"})
              << Button("Reset",  {.handle="reset"})
              << Slider(1, 10, 6,   {.handle="pressure", .label="pressure [bar]"})
              << Slider(0, 100, 60, {.handle="water",    .label="water fill [%]"})
              << CheckBox("free look", {.handle="free"})
              << Label("ARMED",     {.handle="state", .label="state"})))
      << Show();
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
  lastTick = Time::now();
}

static void launch() {
  if (state != ARMED) return;
  float frac = (int)gui["water"] / 100.f;
  thrust    = (int)gui["pressure"] * THRUST_PER_BAR;
  waterMass = frac * 0.25f;
  burnTime  = 0.2f + frac * 0.5f;
  rocket->setMass(EMPTY_MASS + waterMass);
  rocket->setDamping(0.f, 0.6f);
  canopy->setVisible(false);
  launchTime = Time::now();
  state = THRUST;
}

static void resetRocket() {
  rocket->setMass(EMPTY_MASS);
  rocket->setTransform(rocketHome);
  rocket->setDamping(0.f, 0.6f);
  tip->setTransform(tipHome);
  canopy->setVisible(false);
  state = ARMED;
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  static ButtonHandle lb = gui["launch"], rb = gui["reset"];
  if (lb.wasTriggered()) launch();
  if (rb.wasTriggered()) resetRocket();

  switch (state) {
    case THRUST: {
      float t = (now - launchTime).toSecondsDouble();
      if (t >= burnTime) {
        rocket->setMass(EMPTY_MASS);
        rocket->setDamping(0.25f, 0.6f);          // now feels the air -> tip lets go
        state = COAST;
      } else {
        float frac = 1.f - t / burnTime;          // thrust + water mass decay over the burn
        rocket->setMass(EMPTY_MASS + waterMass * frac);
        rocket->applyCentralForce(Vec(0, 0, thrust * frac, 1));
      }
      break;
    }
    case COAST:
      if (rocket->getLinearVelocity()[2] <= 0) {  // apogee -> deploy the chute
        rocket->setDamping(0.92f, 0.8f);          // parachute drag (slows the descent)
        canopy->setVisible(true);
        state = DESCENT;
      }
      break;
    case DESCENT:
      if (rocket->getPose()(2,3) <= groundZ + BODY_H &&
          std::fabs(rocket->getLinearVelocity()[2]) < 150)
        state = LANDED;
      break;
    default: break;
  }

  // keep the canopy just above the bottle's nose while it's out
  if (canopy->isVisible()) {
    Mat m = rocket->getPose();
    Mat c = m; c(0,3)=m(0,3)+m(0,2)*(HEAD_Z+250); c(1,3)=m(1,3)+m(1,2)*(HEAD_Z+250);
    c(2,3)=m(2,3)+m(2,2)*(HEAD_Z+250);
    canopy->setTransformation(c);
  }
  gui["state"] = std::string(stateName(state));

  scene.sync(dt);
  if (!(bool)gui["free"]) {                        // follow the rocket up and down
    Vec tgt = rocket->getPose() * Vec(0,0,HEAD_Z*0.7f,1);
    Vec cam = tgt + Vec(-2400, -1700, 1000, 0); cam[3]=1;
    scene.scene().getCamera(0) = Camera::lookAt(cam, tgt, Vec(0,0,1,1), Size::VGA, 50.f);
  }
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
