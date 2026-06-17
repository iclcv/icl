// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Phase 1 of the physics2 plan: driver-based Bullet physics on geom2. These
// tests use the deterministic single-threaded stepOnce() path (no render
// thread, no GL), so they are fully verifiable headless.

#include "harness/Test.h"
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/Units.h>
#include <icl/physics2/SoftBodyDriver.h>
#include <icl/physics2/SensorDriver.h>
#include <icl/geom2/Node.h>
#include <icl/geom2/Driver.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/physics2/SoftBodyDriver.h>
#include <BulletSoftBody/btSoftBody.h>

#include <algorithm>
#include <cmath>

using namespace icl::physics2;
using namespace icl::geom2;

static float xOf(const Mat &m) { return m(0, 3); }   // translation x (row 0, col 3)
static float zOf(const Mat &m) { return m(2, 3); }   // translation z (row 2, col 3)

// rotation about Y by theta (so a tilted surface makes a ball roll in +/- x)
static Mat rotY(float th) {
  Mat m = Mat::id();
  m(0,0) =  std::cos(th); m(0,2) = std::sin(th);
  m(2,0) = -std::sin(th); m(2,2) = std::cos(th);
  return m;
}

ICL_REGISTER_TEST("physics2.units.roundtrip", "ICL<->Bullet transform round-trips")
{
  Units u;                                  // default 0.01
  Mat m = Mat::id();
  m(0,3) = 100; m(1,3) = -50; m(2,3) = 250;
  btTransform T = u.toBullet(m);
  ICL_TEST_NEAR(T.getOrigin().z(), 2.5f, 1e-4);   // 250 mm * 0.01
  Mat back = u.toIcl(T);
  ICL_TEST_NEAR(back(0,3), 100.0f, 1e-3);
  ICL_TEST_NEAR(back(2,3), 250.0f, 1e-3);
}

ICL_REGISTER_TEST("physics2.box_falls_and_rests", "a dynamic box drops onto a static ground and settles")
{
  PhysicsScene scene;

  // ground: 20-thick slab centred at origin -> top surface at z = +10
  auto ground = CuboidNode::create(0,0,0, 1000,1000,20);
  auto gd = scene.add(std::static_pointer_cast<Node>(ground), 0.0f);   // static

  // box: 100^3, dropped from z = 500 (half-height 50)
  auto box = CuboidNode::create(0,0,0, 100,100,100);
  box->translate(0, 0, 500);
  auto bd = scene.add(std::static_pointer_cast<Node>(box), 1.0f);

  ICL_TEST_NEAR(zOf(bd->getPose()), 500.0f, 1e-2);   // starts high

  for (int i = 0; i < 600; i++) scene.stepOnce(1.f/120.f);   // 5 s

  float z = zOf(bd->getPose());
  // expected resting centre ~ ground-top(10) + box-half(50) = 60 (+/- collision margins)
  ICL_TEST_TRUE(z > 45.0f && z < 80.0f);
  // it actually fell a long way
  ICL_TEST_TRUE(z < 150.0f);
  // the static ground never moved
  ICL_TEST_NEAR(zOf(gd->getPose()), 0.0f, 1.0f);
}

ICL_REGISTER_TEST("physics2.setup_default_ground", "setupDefault's collider is coincident with the drawn ground")
{
  PhysicsScene scene;
  scene.setupDefault(DefaultScene::SceneType::Studio, 1000.f);
  // ext=1000 -> visual + collider ground top at z = -500 - 2% = -520

  auto ball = SphereNode::create(0,0,0, 50, 16, 16);
  ball->translate(0, 0, 200);
  auto bd = scene.add(std::static_pointer_cast<Node>(ball), 1.0f);
  ICL_TEST_NEAR(zOf(bd->getPose()), 200.0f, 1e-2);   // starts above the ground

  for (int i = 0; i < 700; i++) scene.stepOnce(1.f/120.f);

  // rests at groundLevel(-520) + radius(50) = -470, within collision margins —
  // proves the invisible collider lines up with DefaultScene's checkerboard.
  float z = zOf(bd->getPose());
  ICL_TEST_TRUE(z > -490.0f && z < -445.0f);
}

ICL_REGISTER_TEST("physics2.sync_writes_node", "scene.sync() pushes the simulated pose into the node")
{
  PhysicsScene scene;
  auto ground = CuboidNode::create(0,0,0, 1000,1000,20);
  scene.add(std::static_pointer_cast<Node>(ground), 0.0f);

  auto sphere = SphereNode::create(0,0,0, 40, 24, 24);
  sphere->translate(0, 0, 400);
  auto sd = scene.add(std::static_pointer_cast<Node>(sphere), 1.0f);

  for (int i = 0; i < 600; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);     // alpha defaults to 1.0 -> latest pose into node

  // node transform now matches the driver's sampled pose
  float nodeZ = zOf(sphere->getTransformation());
  float poseZ = zOf(sd->getPose());
  ICL_TEST_NEAR(nodeZ, poseZ, 1e-3);
  // sphere radius 40 resting on ground top 10 -> centre ~ 50
  ICL_TEST_TRUE(nodeZ > 35.0f && nodeZ < 70.0f);
}

ICL_REGISTER_TEST("physics2.kinematic_tilt_rolls_ball", "a kinematic tilted ramp makes a ball roll (the maze fix)")
{
  PhysicsScene scene;

  // kinematic ramp tilted about Y -> its collision surface really tilts
  auto ramp = CuboidNode::create(0,0,0, 4000,4000,40);
  auto rd = scene.add(std::static_pointer_cast<Node>(ramp), 0.0f);
  rd->setKinematic(true);
  rd->setKinematicTransform(rotY(0.2f));   // ~11 deg, queued

  auto ball = SphereNode::create(0,0,0, 50, 24, 24);
  ball->translate(0, 0, 150);
  auto bd = scene.add(std::static_pointer_cast<Node>(ball), 1.0f);
  bd->setRollingFriction(0.0f);

  float x0 = xOf(bd->getPose());
  for (int i = 0; i < 120; i++) scene.stepOnce(1.f/120.f);   // 1 s (stays on the ramp)
  float x1 = xOf(bd->getPose());

  ICL_TEST_TRUE(std::fabs(x1 - x0) > 80.0f);    // it rolled downhill
  ICL_TEST_TRUE(zOf(bd->getPose()) > -1500.0f); // didn't tunnel / fall through
}

ICL_REGISTER_TEST("physics2.kinematic_flat_holds_ball", "a flat kinematic ramp keeps the ball put (control)")
{
  PhysicsScene scene;

  auto ramp = CuboidNode::create(0,0,0, 4000,4000,40);
  auto rd = scene.add(std::static_pointer_cast<Node>(ramp), 0.0f);
  rd->setKinematic(true);
  rd->setKinematicTransform(Mat::id());      // flat

  auto ball = SphereNode::create(0,0,0, 50, 24, 24);
  ball->translate(0, 0, 150);
  auto bd = scene.add(std::static_pointer_cast<Node>(ball), 1.0f);
  bd->setRollingFriction(0.0f);

  float x0 = xOf(bd->getPose());
  for (int i = 0; i < 300; i++) scene.stepOnce(1.f/120.f);
  float x1 = xOf(bd->getPose());

  ICL_TEST_TRUE(std::fabs(x1 - x0) < 30.0f);  // no roll on the flat
}

ICL_REGISTER_TEST("physics2.cloth_sags", "a corner-pinned soft cloth sags under gravity")
{
  PhysicsScene scene;
  // 10x10 horizontal cloth at z=500, pin corners c00 & c10 (mask 1+2)
  auto cloth = scene.addCloth(Vec(-500,-500,500,1), Vec(500,-500,500,1),
                              Vec(-500, 500,500,1), Vec(500, 500,500,1),
                              10, 10, 1 + 2, 1.0f);
  auto *mesh = dynamic_cast<MeshNode*>(cloth->node());
  ICL_TEST_TRUE(mesh != nullptr);

  for (int i = 0; i < 400; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  ICL_TEST_EQ((int)v.size(), 100);            // 10x10 grid
  float minz = 1e9f, maxz = -1e9f;
  for (const auto &p : v) { minz = std::min(minz, p[2]); maxz = std::max(maxz, p[2]); }
  ICL_TEST_TRUE(minz < 400.0f);               // it sagged below the start height
  ICL_TEST_TRUE(maxz > 450.0f);               // the pinned corners stayed up
}

ICL_REGISTER_TEST("physics2.pick_resolves_driver", "raycast hit resolves to its RigidBodyDriver (unified picking)")
{
  PhysicsScene scene;
  scene.addCamera(icl::geom::Camera::lookAt(
      icl::geom::Vec(0,0,1000,1), icl::geom::Vec(0,0,0,1), icl::geom::Vec(0,1,0,1),
      icl::utils::Size(640,480), 40));
  auto box = CuboidNode::create(0,0,0, 300,300,300);
  auto bd = scene.add(std::static_pointer_cast<Node>(box), 1.0f);
  scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);                       // node transform now reflects pose

  // centre pixel ray (the mouse-pick path) hits the box -> its driver
  Hit2 hit = scene.scene().findObject(0, 320, 240);
  ICL_TEST_TRUE((bool)hit.node);
  ICL_TEST_TRUE(hit.node->getDriver<RigidBodyDriver>() == bd);   // hit -> driver
}

ICL_REGISTER_TEST("physics2.cloth_rests_on_box", "cloth collides with a rigid box (no tunnel, no NaN)")
{
  PhysicsScene scene;
  // static box, top surface at z = +200, footprint +/-250
  auto box = CuboidNode::create(0,0,0, 500,500,400);
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);

  // smaller cloth dropped flat above the box -> should rest on its top face
  auto cloth = scene.addCloth(Vec(-150,-150,500,1), Vec(150,-150,500,1),
                              Vec(-150, 150,500,1), Vec(150, 150,500,1),
                              16, 16, /*pin*/ 0, 1.0f);
  auto *mesh = dynamic_cast<MeshNode*>(cloth->node());

  for (int i = 0; i < 500; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  float minz = 1e9f, maxz = -1e9f;
  bool allFinite = true;
  for (const auto &p : v) {
    if (!std::isfinite(p[0]) || !std::isfinite(p[1]) || !std::isfinite(p[2])) allFinite = false;
    minz = std::min(minz, p[2]); maxz = std::max(maxz, p[2]);
  }
  ICL_TEST_TRUE(allFinite);          // didn't explode to NaN
  ICL_TEST_TRUE(minz > 100.0f);      // rested on the box top (~200), didn't tunnel
  ICL_TEST_TRUE(maxz < 450.0f);      // settled (didn't blow up upward)
}

ICL_REGISTER_TEST("physics2.debug_lines", "debug draw returns a non-empty collision wireframe")
{
  PhysicsScene scene;
  auto box = CuboidNode::create(0,0,0, 100,100,100);
  scene.add(std::static_pointer_cast<Node>(box), 1.0f);
  scene.stepOnce(1.f/120.f);                  // register the body
  ICL_TEST_TRUE(scene.world().getDebugLines().size() > 0);
}

ICL_REGISTER_TEST("physics2.ccd_no_tunnel", "CCD stops a fast small ball tunneling a thin floor")
{
  PhysicsScene scene;
  auto floor = CuboidNode::create(0,0,0, 2000,2000,10);   // thin static floor, top at +5
  scene.add(std::static_pointer_cast<Node>(floor), 0.0f);
  auto ball = SphereNode::create(0,0,0, 30, 16, 16);
  ball->translate(0,0,300);
  auto bd = scene.add(std::static_pointer_cast<Node>(ball), 1.0f);
  bd->setCcd(30, 10);                          // motion threshold + swept radius
  bd->setLinearVelocity(Vec(0,0,-30000,1));    // 30 m/s downward (250mm/step >> floor)

  for (int i = 0; i < 60; i++) scene.stepOnce(1.f/120.f);
  ICL_TEST_TRUE(zOf(bd->getPose()) > 0.0f);    // caught above the floor, didn't tunnel
}

ICL_REGISTER_TEST("physics2.collision_filter", "filtered bodies don't collide; default ones do")
{
  // control: top box rests on the static bottom box
  {
    PhysicsScene scene;
    auto bottom = CuboidNode::create(0,0,0, 400,400,400);   // static, top at +200
    scene.add(std::static_pointer_cast<Node>(bottom), 0.0f);
    auto top = CuboidNode::create(0,0,0, 100,100,100);
    top->translate(0,0,500);
    auto td = scene.add(std::static_pointer_cast<Node>(top), 1.0f);
    for (int i = 0; i < 400; i++) scene.stepOnce(1.f/120.f);
    ICL_TEST_TRUE(zOf(td->getPose()) > 150.0f);   // rests on the bottom (~250)
  }
  // filtered: groups that don't collide -> top falls through bottom
  {
    PhysicsScene scene;
    auto bottom = CuboidNode::create(0,0,0, 400,400,400);
    scene.add(std::static_pointer_cast<Node>(bottom), 0.0f)->setCollisionFilter(1, 1);
    auto top = CuboidNode::create(0,0,0, 100,100,100);
    top->translate(0,0,500);
    auto td = scene.add(std::static_pointer_cast<Node>(top), 1.0f);
    td->setCollisionFilter(2, 2);
    for (int i = 0; i < 400; i++) scene.stepOnce(1.f/120.f);
    ICL_TEST_TRUE(zOf(td->getPose()) < 100.0f);   // passed through
  }
}

ICL_REGISTER_TEST("physics2.contact_events", "contact callback fires when a body lands")
{
  PhysicsScene scene;
  auto ground = CuboidNode::create(0,0,0, 1000,1000,20);
  scene.add(std::static_pointer_cast<Node>(ground), 0.0f);
  auto box = CuboidNode::create(0,0,0, 100,100,100);
  box->translate(0,0,300);
  auto bd = scene.add(std::static_pointer_cast<Node>(box), 1.0f);

  int contacts = 0;
  bool sawBox = false;
  scene.world().setContactCallback(
      [&](Driver *a, Driver *b, const Vec &) {
        contacts++;
        if (a == bd || b == bd) sawBox = true;
      });

  for (int i = 0; i < 400; i++) scene.stepOnce(1.f/120.f);
  ICL_TEST_TRUE(contacts > 0);
  ICL_TEST_TRUE(sawBox);
}

ICL_REGISTER_TEST("physics2.force_field", "a force field drifts a body")
{
  PhysicsScene scene;
  scene.world().setGravityEnabled(false);      // isolate the field
  auto ball = SphereNode::create(0,0,0, 50, 16, 16);
  auto bd = scene.add(std::static_pointer_cast<Node>(ball), 1.0f);
  scene.world().addForceField([](const Vec &) { return Vec(8000,0,0,1); });  // const +x

  float x0 = bd->getPose()(0,3);
  for (int i = 0; i < 120; i++) scene.stepOnce(1.f/120.f);
  ICL_TEST_TRUE(bd->getPose()(0,3) > x0 + 50.0f);   // drifted +x
}

ICL_REGISTER_TEST("physics2.sensor_detects_passthrough", "a ghost sensor detects a body passing through without deflecting it")
{
  PhysicsScene scene;
  auto zone = CuboidNode::create(0,0,0, 400,400,400);   // zone z in [-200,200]
  auto sensor = scene.addSensor(std::static_pointer_cast<Node>(zone));
  auto ball = SphereNode::create(0,0,0, 50, 16, 16);
  ball->translate(0,0,500);
  auto bd = scene.add(std::static_pointer_cast<Node>(ball), 1.0f);

  bool detected = false;
  for (int i = 0; i < 400; i++) {
    scene.stepOnce(1.f/120.f);
    for (auto *d : sensor->getOverlappingDrivers()) if (d == bd) detected = true;
  }
  ICL_TEST_TRUE(detected);                       // sensor saw the ball inside
  ICL_TEST_TRUE(zOf(bd->getPose()) < -200.0f);   // ball passed straight through
}

ICL_REGISTER_TEST("physics2.anchor_holds_corner", "a soft-rigid anchor pins a cloth corner to a body")
{
  PhysicsScene scene;
  // static anchor body sitting at the cloth's c00 corner location
  auto anchorBox = CuboidNode::create(0,0,0, 40,40,40);
  anchorBox->translate(-500,-500,500);
  auto ad = scene.add(std::static_pointer_cast<Node>(anchorBox), 0.0f);

  // unpinned cloth; anchor its c00 node to the static body
  auto cloth = scene.addCloth(Vec(-500,-500,500,1), Vec(500,-500,500,1),
                              Vec(-500, 500,500,1), Vec(500, 500,500,1),
                              10, 10, /*pin*/ 0, 1.0f);
  cloth->anchorNode(cloth->cornerNodeIndex(0), ad, 1.0f);
  auto *mesh = dynamic_cast<MeshNode*>(cloth->node());

  for (int i = 0; i < 400; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  float anchoredZ = v[0][2];                      // node 0 = c00 (anchored)
  float minz = 1e9f;
  for (const auto &p : v) minz = std::min(minz, p[2]);
  ICL_TEST_TRUE(anchoredZ > 400.0f);              // anchored corner stayed up at the body
  ICL_TEST_TRUE(minz < 350.0f);                   // the rest sagged
}

ICL_REGISTER_TEST("physics2.static_unit_scale", "a custom unit scale still lets bodies rest sanely")
{
  PhysicsScene scene;
  scene.world().setUnitScale(0.02f);    // different scale
  scene.world().setGravityEnabled(true);

  auto ground = CuboidNode::create(0,0,0, 1000,1000,20);
  scene.add(std::static_pointer_cast<Node>(ground), 0.0f);
  auto box = CuboidNode::create(0,0,0, 100,100,100);
  box->translate(0,0,400);
  auto bd = scene.add(std::static_pointer_cast<Node>(box), 1.0f);

  for (int i = 0; i < 600; i++) scene.stepOnce(1.f/120.f);
  float z = zOf(bd->getPose());
  ICL_TEST_TRUE(z > 45.0f && z < 85.0f);   // still rests near 60 regardless of scale
}

ICL_REGISTER_TEST("physics2.cloth_resolution_rebuild", "changing node density rebuilds the soft body live")
{
  PhysicsScene scene;
  auto *cloth = scene.addCloth(Vec(-100,-100,200,1), Vec(100,-100,200,1),
                               Vec(-100, 100,200,1), Vec(100, 100,200,1),
                               12, 12, /*pin*/ 0, 1.0f);
  ICL_TEST_TRUE(cloth->softBody() != nullptr);
  ICL_TEST_EQ(cloth->softBody()->m_nodes.size(), 12 * 12);

  // Live density change → structural rebuild at the new resolution.
  cloth->setPropertyValue("node density", 20);
  ICL_TEST_EQ(cloth->softBody()->m_nodes.size(), 20 * 20);

  // Still simulates after the rebuild (no dangling body / capture).
  for (int i = 0; i < 120; i++) scene.stepOnce(1.f/120.f);
  ICL_TEST_TRUE(cloth->softBody() != nullptr);
  ICL_TEST_EQ(cloth->softBody()->m_nodes.size(), 20 * 20);
}
