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
#include <icl/physics2/Constraint.h>
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
#include <icl/physics2/PaperDriver.h>
#include <icl/physics2/FoldDriver.h>
#include <icl/physics2/PaperMoverDriver.h>
#include <icl/geom/ViewRay.h>
#include <icl/utils/Point.h>
#include <icl/utils/Size.h>
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

ICL_REGISTER_TEST("physics2.cloth_stable_at_rest", "cloth on a box stays bounded over a long run (no rest-state explosion)")
{
  // The deformable world's contact projection must hold a settled cloth steady.
  // The legacy btSoftBody solver pumps energy at rest and diverges within a few
  // steps of settling — this long run is the regression guard for that.
  PhysicsScene scene;   // default = SoftBodyMode::Deformable
  auto box = CuboidNode::create(0,0,0, 500,500,400);
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);
  auto cloth = scene.addCloth(Vec(-150,-150,500,1), Vec(150,-150,500,1),
                              Vec(-150, 150,500,1), Vec(150, 150,500,1),
                              16, 16, /*pin*/ 0, 1.0f);
  auto *mesh = dynamic_cast<MeshNode*>(cloth->node());

  // settle, then run far past where the legacy solver would have blown up
  for (int i = 0; i < 2500; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  float minz = 1e9f, maxz = -1e9f, maxAbs = 0.f;
  bool allFinite = true;
  for (const auto &p : v) {
    for (int k = 0; k < 3; k++) {
      if (!std::isfinite(p[k])) allFinite = false;
      maxAbs = std::max(maxAbs, std::fabs(p[k]));
    }
    minz = std::min(minz, p[2]); maxz = std::max(maxz, p[2]);
  }
  ICL_TEST_TRUE(allFinite);        // no NaN
  ICL_TEST_TRUE(maxAbs < 2000.0f); // no vertex flew off — the explosion signature
  ICL_TEST_TRUE(minz > 100.0f);    // still resting on the box top (~200), no tunnel
  ICL_TEST_TRUE(maxz < 450.0f);    // still settled, didn't creep upward
}

ICL_REGISTER_TEST("physics2.dense_cloth_stable", "a high-resolution cloth stays stable (resolution-aware stiffness)")
{
  // Mirrors physics2-cloth: a dense 60x60 cloth, oversized, dropped on a box.
  // Mass-spring stiffness must scale with node mass — a fixed stiffness makes a
  // fine grid (tiny per-node mass) explode under explicit integration. This is
  // the regression guard for the demo's immediate blow-up.
  PhysicsScene scene;
  auto box = CuboidNode::create(0,0,0, 500,500,400);   // top at z=200
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);
  // dense 60x60 cloth (3600 nodes -> tiny per-node mass) laid just above the box
  // top (a low drop keeps impact under the fine-grid collision margin, isolating
  // the explosion from tunneling). With a fixed stiffness this resolution
  // explodes within a few steps from internal spring forces; with node-mass-
  // proportional stiffness it stays bounded and settles.
  auto cloth = scene.addCloth(Vec(-150,-150,210,1), Vec(150,-150,210,1),
                              Vec(-150, 150,210,1), Vec(150, 150,210,1),
                              60, 60, /*pin*/ 0, 1.0f);
  auto *mesh = dynamic_cast<MeshNode*>(cloth->node());

  for (int i = 0; i < 1500; i++) scene.stepOnce(1.f/240.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  float minz = 1e9f, maxAbs = 0.f; bool allFinite = true;
  for (const auto &p : v) {
    for (int k = 0; k < 3; k++) {
      if (!std::isfinite(p[k])) allFinite = false;
      maxAbs = std::max(maxAbs, std::fabs(p[k]));
    }
    minz = std::min(minz, p[2]);
  }
  ICL_TEST_EQ((int)v.size(), 3600);
  ICL_TEST_TRUE(allFinite);          // didn't explode to NaN
  ICL_TEST_TRUE(maxAbs < 1000.0f);   // no node flew off — the explosion signature
  ICL_TEST_TRUE(minz > 100.0f);      // rested on the box top (~200), didn't tunnel
}

ICL_REGISTER_TEST("physics2.live_stiffness_two_cloths", "changing a cloth slider with multiple cloths present doesn't corrupt forces")
{
  // Reproduces the physics2-cloth crash: the deformable world merges forces by
  // type, so two cloths shared one mass-spring force; a per-body delete on a
  // slider change freed a force still used by the other cloth (SIGSEGV). We now
  // give each cloth its own force, so a live stiffness change is body-local.
  PhysicsScene scene;
  auto c1 = scene.addCloth(Vec(-200,-200,400,1), Vec(200,-200,400,1),
                           Vec(-200, 200,400,1), Vec(200, 200,400,1),
                           20, 20, /*pin*/ 1+2, 1.0f);
  auto c2 = scene.addCloth(Vec(-200,-200,600,1), Vec(200,-200,600,1),
                           Vec(-200, 200,600,1), Vec(200, 200,600,1),
                           20, 20, /*pin*/ 1+2, 1.0f);

  for (int i = 0; i < 60; i++) scene.stepOnce(1.f/120.f);
  // change a slider on c1 (enqueues a force swap, drained on the next step)
  c1->setPropertyValue("stiffness", 0.8f);
  c1->setPropertyValue("friction", 0.3f);
  for (int i = 0; i < 200; i++) scene.stepOnce(1.f/120.f);   // would crash here pre-fix
  // and swap c2's force too, to exercise both bodies' independent forces
  c2->setPropertyValue("stiffness", 0.2f);
  for (int i = 0; i < 200; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  auto *m1 = dynamic_cast<MeshNode*>(c1->node());
  auto *m2 = dynamic_cast<MeshNode*>(c2->node());
  bool allFinite = true;
  for (auto *m : {m1, m2})
    for (const auto &p : m->getVertices())
      for (int k = 0; k < 3; k++) if (!std::isfinite(p[k])) allFinite = false;
  ICL_TEST_TRUE(allFinite);   // no corruption, no NaN — and the run didn't crash
}

ICL_REGISTER_TEST("physics2.legacy_softrigid_mode", "the legacy SoftRigid world still builds + simulates a cloth")
{
  // Keeps the non-default pipeline compiled + exercised (it's retained for A/B
  // comparison against the Deformable default).
  PhysicsScene scene(SoftBodyMode::SoftRigid);
  auto box = CuboidNode::create(0,0,0, 500,500,400);
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);
  auto cloth = scene.addCloth(Vec(-150,-150,500,1), Vec(150,-150,500,1),
                              Vec(-150, 150,500,1), Vec(150, 150,500,1),
                              12, 12, /*pin*/ 0, 1.0f);
  auto *mesh = dynamic_cast<MeshNode*>(cloth->node());

  for (int i = 0; i < 300; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  bool allFinite = true;
  for (const auto &p : v)
    for (int k = 0; k < 3; k++) if (!std::isfinite(p[k])) allFinite = false;
  ICL_TEST_TRUE(allFinite);
  ICL_TEST_EQ((int)v.size(), 144);
}

ICL_REGISTER_TEST("physics2.cloth_props_are_backend_aware", "the cloth driver only exposes properties applicable to its world backend")
{
  // The deformable solver ignores the legacy position-iteration + cluster-collision
  // path, so a Deformable-world cloth must not advertise those knobs (a SoftRigid one
  // does). Same knobs that apply to both stay in both.
  {
    PhysicsScene def(SoftBodyMode::Deformable);
    auto c = def.addCloth(Vec(-100,-100,300,1), Vec(100,-100,300,1),
                          Vec(-100,100,300,1), Vec(100,100,300,1), 8, 8, 0, 1.0f);
    ICL_TEST_TRUE(!c->supportsProperty("position iterations"));
    ICL_TEST_TRUE(!c->supportsProperty("collision mode"));
    ICL_TEST_TRUE(c->supportsProperty("stiffness"));
    ICL_TEST_TRUE(c->supportsProperty("self collision"));
  }
  {
    PhysicsScene sr(SoftBodyMode::SoftRigid);
    auto c = sr.addCloth(Vec(-100,-100,300,1), Vec(100,-100,300,1),
                         Vec(-100,100,300,1), Vec(100,100,300,1), 8, 8, 0, 1.0f);
    ICL_TEST_TRUE(c->supportsProperty("position iterations"));
    ICL_TEST_TRUE(c->supportsProperty("collision mode"));
    ICL_TEST_TRUE(c->supportsProperty("stiffness"));
  }
}

ICL_REGISTER_TEST("physics2.paper_builds_and_drapes", "the fold-aware paper substrate builds + falls under gravity, staying finite")
{
  // The crown-jewel transplant: a manually-built dual-mesh btSoftBody (corner grid
  // + per-cell centre vertices) in the legacy SoftRigid world.
  PhysicsScene scene(SoftBodyMode::SoftRigid);
  auto paper = scene.addPaper(icl::utils::Size(10, 10));   // default A4-ish sheet at z=40
  auto *mesh = dynamic_cast<MeshNode*>(paper->node());
  ICL_TEST_TRUE(mesh != nullptr);
  ICL_TEST_EQ(paper->getNumNodes(), 181);                  // 10*10 corners + 9*9 centres

  for (int i = 0; i < 200; i++) scene.stepOnce(1.f/120.f);
  scene.sync(1.0/60.0);

  const auto &v = mesh->getVertices();
  ICL_TEST_EQ((int)v.size(), 181);
  float minz = 1e9f; bool finite = true;
  for (const auto &p : v) {
    for (int k = 0; k < 3; k++) if (!std::isfinite(p[k])) finite = false;
    minz = std::min(minz, p[2]);
  }
  ICL_TEST_TRUE(finite);          // it simulated without NaN
  ICL_TEST_TRUE(minz < 40.0f);    // it fell below its start height (z=40)
}

ICL_REGISTER_TEST("physics2.paper_fold_grows_topology", "folding splits triangles -> more nodes, and the mesh rebuilds to match")
{
  PhysicsScene scene(SoftBodyMode::SoftRigid);
  auto paper = scene.addPaper(icl::utils::Size(8, 8));
  auto *mesh = dynamic_cast<MeshNode*>(paper->node());
  const int n0 = paper->getNumNodes();
  const size_t v0 = mesh->getVertices().size();

  // a diagonal crease across the sheet (paper coords [0,1]^2). Avoid exact grid
  // lines / cell-centre rows (those are the degenerate fold-through-a-vertex case).
  paper->foldAlongLine(icl::utils::Point32f(0.17f, 0.23f), icl::utils::Point32f(0.81f, 0.74f), true);
  for (int i = 0; i < 5; i++) scene.stepOnce(1.f/120.f);   // drain the enqueued fold + step
  scene.sync(1.0/60.0);                                    // UI rebuilds the topology

  ICL_TEST_TRUE(paper->getNumNodes() > n0);                // the fold inserted vertices
  ICL_TEST_TRUE(mesh->getVertices().size() > v0);          // the mesh grew with it
  ICL_TEST_EQ((int)mesh->getVertices().size(), paper->getNumNodes());  // and stays consistent
}

ICL_REGISTER_TEST("physics2.paper_fold_reduces_bending", "a crease reduces crossing bending links to the crease stiffness; the 2nd-order overlay hides them")
{
  PhysicsScene scene(SoftBodyMode::SoftRigid);
  auto paper = scene.addPaper(icl::utils::Size(12, 12));    // default fold softness 1e-5
  for (int i = 0; i < 30; i++) scene.stepOnce(1.f/120.f);

  const float creaseThresh = 1e-5f;   // default "fold softness" == crease stiffness
  // count 2nd-order (bending) links that are visible vs reduced-to-the-crease.
  auto bendingStats = [&](int &visible, int &reduced) {
    visible = reduced = 0;
    btSoftBody *s = paper->softBody();
    for (int i = 0; i < s->m_links.size(); ++i) {
      const btSoftBody::Link &l = s->m_links[i];
      if (!l.m_bbending) continue;                          // only 2nd-order links
      if (l.m_material && l.m_material->m_kLST <= creaseThresh) reduced++;
      else visible++;
    }
  };

  int vis0, red0; bendingStats(vis0, red0);
  ICL_TEST_EQ(red0, 0);                                     // no crease yet -> nothing reduced
  ICL_TEST_EQ((int)paper->getDebugGeometry().secondOrder.size(), vis0);

  // a diagonal crease (avoid grid lines / cell-centre rows) — recreates the
  // bending graph from the fold-map, reducing links that cross the crease.
  paper->foldAlongLine(icl::utils::Point32f(0.17f, 0.23f), icl::utils::Point32f(0.81f, 0.74f), true);
  for (int i = 0; i < 5; i++) scene.stepOnce(1.f/120.f);

  int vis1, red1; bendingStats(vis1, red1);
  ICL_TEST_TRUE(red1 > 0);                                  // crossing links were reduced
  ICL_TEST_EQ((int)paper->getCreases().size(), 1);         // one crease primitive recorded
  ICL_TEST_TRUE(paper->getDebugGeometry().creases.size() > 0);
  // the 2nd-order overlay shows exactly the non-reduced bending links
  ICL_TEST_EQ((int)paper->getDebugGeometry().secondOrder.size(), vis1);
}

ICL_REGISTER_TEST("physics2.paper_hit_and_interpolate", "paper-space picking: a ray hits the sheet, interpolatePosition round-trips")
{
  PhysicsScene scene(SoftBodyMode::SoftRigid);
  auto paper = scene.addPaper(icl::utils::Size(10, 10));   // flat sheet at z=40

  // centre of the sheet (paper 0.5,0.5) is world ~ (0,0,40)
  Vec c = paper->interpolatePosition(icl::utils::Point32f(0.5f, 0.5f));
  ICL_TEST_NEAR(c[0], 0.0f, 5.0f);
  ICL_TEST_NEAR(c[1], 0.0f, 5.0f);
  ICL_TEST_NEAR(c[2], 40.0f, 1.0f);

  // a ray straight down through the centre hits paper coord ~ (0.5,0.5)
  icl::geom::ViewRay ray(Vec(0, 0, 400, 1), Vec(0, 0, -1, 1));
  icl::utils::Point32f p = paper->hit(ray);
  ICL_TEST_TRUE(p.x >= 0.f);          // a hit
  ICL_TEST_NEAR(p.x, 0.5f, 0.1f);
  ICL_TEST_NEAR(p.y, 0.5f, 0.1f);
}

ICL_REGISTER_TEST("physics2.paper_composed_drivers_dispatch", "substrate + behaviour drivers compose on one node and resolve by type")
{
  // The composition the driver model enables: one node carries the PaperDriver
  // substrate plus two behaviour drivers, each resolved by type (the dispatch a
  // mouse handler uses), each driving the substrate.
  PhysicsScene scene(SoftBodyMode::SoftRigid);
  auto paper = scene.addPaper(icl::utils::Size(10, 10));
  auto *node = paper->node();
  auto *fold  = node->addDriver<FoldDriver>().get();
  auto *mover = node->addDriver<PaperMoverDriver>().get();

  // all three resolve from the single node, by type
  ICL_TEST_TRUE(node->getDriver<PaperDriver>() == paper);
  ICL_TEST_TRUE(node->getDriver<FoldDriver>() == fold);
  ICL_TEST_TRUE(node->getDriver<PaperMoverDriver>() == mover);
  ICL_TEST_TRUE(fold->paper() == paper);     // each behaviour resolved its substrate
  ICL_TEST_TRUE(mover->paper() == paper);

  // (1) fold THROUGH the FoldDriver -> the substrate's topology grows
  const int n0 = paper->getNumNodes();
  fold->foldAlongLine(icl::utils::Point32f(0.17f, 0.23f), icl::utils::Point32f(0.81f, 0.74f));
  for (int i = 0; i < 5; i++) scene.stepOnce(1.f/120.f);
  ICL_TEST_TRUE(paper->getNumNodes() > n0);

  // (2) grab THROUGH the PaperMoverDriver -> the grabbed centre lifts AND HOLDS
  // (the kinematic grab keeps it up even with no further input — unlike a one-shot
  // velocity nudge, which would let gravity pull it back).
  const float z0 = paper->interpolatePosition(icl::utils::Point32f(0.5f, 0.5f))[2];
  mover->beginGrab(icl::utils::Point32f(0.5f, 0.5f));
  mover->updateGrab(Vec(0, 0, z0 + 300, 1));        // lift the centre once
  for (int i = 0; i < 60; i++) scene.stepOnce(1.f/120.f);   // then DON'T move it
  const float zHold = paper->interpolatePosition(icl::utils::Point32f(0.5f, 0.5f))[2];
  ICL_TEST_TRUE(std::isfinite(zHold));
  ICL_TEST_TRUE(zHold > z0 + 200.f);                // held up near the target, no fall-back
  mover->endGrab();
  for (int i = 0; i < 60; i++) scene.stepOnce(1.f/120.f);   // released -> it drops
  const float zDrop = paper->interpolatePosition(icl::utils::Point32f(0.5f, 0.5f))[2];
  ICL_TEST_TRUE(zDrop < zHold - 20.f);              // let go -> falls under gravity
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

// ---------------------------------------------------------------------------
// Phase 4c — constraints (joints). All on the deterministic stepOnce() path.
// A static (mass 0) anchor body stands in for the fixed world frame.
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("physics2.hinge_swings", "a hinged door swings down about its axis, pivot held")
{
  PhysicsScene scene;   // default unified (Deformable) world handles joints fine
  // Hinge about X (axis 0). NB: btGeneric6DofConstraint limits the *middle*
  // angular axis (Y, index 1) to +/-90deg (Euler gimbal), so a free Y hinge
  // misbehaves — use X or Z. Door extends +Y from the pivot and swings down in
  // the YZ plane. Anchor is offset in +X, clear of the swing.
  auto anchor = CuboidNode::create(0,0,0, 40,40,40);
  anchor->translate(500, 0, 500);
  scene.add(std::static_pointer_cast<Node>(anchor), 0.0f);          // static

  auto door = CuboidNode::create(0,0,0, 100,400,20);
  door->translate(0, 200, 500);                                     // -Y edge at the pivot
  auto dd = scene.add(std::static_pointer_cast<Node>(door), 1.0f);
  dd->setDamping(0.1f, 0.3f);                                       // settle the swing

  // hinge about X at world (0,0,500): door free to rotate about X only
  scene.addHinge(anchor, door, Vec(-500,0,0,1), Vec(0,-200,0,1), 0);
  ICL_TEST_EQ(scene.world().getConstraintCount(), 1);

  for (int i = 0; i < 1500; i++) scene.stepOnce(1.f/120.f);

  // settles hanging straight down: centre swings from (0,200,500) to ~(0,0,300)
  ICL_TEST_TRUE(std::fabs(dd->getPose()(1,3)) < 90.0f);           // Y swung in (rotated about X)
  float z = zOf(dd->getPose());
  ICL_TEST_TRUE(z > 260.0f && z < 360.0f);                         // hung down, pivot held (no free fall)
}

ICL_REGISTER_TEST("physics2.slider_translates", "a slider frees one translation axis and locks the rest")
{
  PhysicsScene scene;
  auto anchor = CuboidNode::create(0,0,0, 40,40,40);
  anchor->translate(0, 0, 500);
  scene.add(std::static_pointer_cast<Node>(anchor), 0.0f);

  auto body = CuboidNode::create(0,0,0, 100,100,100);
  body->translate(0, 0, 400);
  auto bd = scene.add(std::static_pointer_cast<Node>(body), 1.0f);

  // free to slide along Z only; X/Y translation + all rotation locked
  scene.addSlider(anchor, body, Vec(0,0,0,1), Vec(0,0,0,1), 2);
  bd->setLinearVelocity(Vec(1000,0,0,1));                          // sideways kick — must be absorbed

  for (int i = 0; i < 240; i++) scene.stepOnce(1.f/120.f);         // 2 s

  ICL_TEST_TRUE(std::fabs(xOf(bd->getPose())) < 30.0f);           // X stayed locked despite the kick
  ICL_TEST_TRUE(zOf(bd->getPose()) < 200.0f);                    // slid down the free Z axis
}

ICL_REGISTER_TEST("physics2.ballsocket_swings", "a ball-socket joint swings to hang below the pivot")
{
  PhysicsScene scene;
  auto anchor = CuboidNode::create(0,0,0, 40,40,40);
  anchor->translate(0, 500, 500);                                  // out of the swing plane
  scene.add(std::static_pointer_cast<Node>(anchor), 0.0f);

  auto body = CuboidNode::create(0,0,0, 400,100,20);
  body->translate(200, 0, 500);
  auto bd = scene.add(std::static_pointer_cast<Node>(body), 1.0f);
  bd->setDamping(0.1f, 0.3f);                                      // settle the pendulum

  scene.addBallSocket(anchor, body, Vec(0,-500,0,1), Vec(-200,0,0,1));

  for (int i = 0; i < 1500; i++) scene.stepOnce(1.f/120.f);

  // hangs ~200 below the pivot; rotation is unconstrained, so it may settle
  // slightly off the start plane (wider X tolerance than the hinge).
  float z = zOf(bd->getPose());
  ICL_TEST_TRUE(z > 260.0f && z < 360.0f);                       // pivot held, hung ~200 below
  ICL_TEST_TRUE(std::fabs(xOf(bd->getPose())) < 90.0f);
}

ICL_REGISTER_TEST("physics2.sixdof_locks_body", "a fully-locked 6DOF welds the body to the anchor")
{
  PhysicsScene scene(SoftBodyMode::SoftRigid);   // also exercises a joint in the legacy world
  auto anchor = CuboidNode::create(0,0,0, 40,40,40);
  anchor->translate(0, 0, 500);
  scene.add(std::static_pointer_cast<Node>(anchor), 0.0f);

  auto body = CuboidNode::create(0,0,0, 100,100,100);
  body->translate(0, 0, 500);                                     // coincident frames
  auto bd = scene.add(std::static_pointer_cast<Node>(body), 1.0f);

  scene.addSixDOF(anchor, body, Vec(0,0,0,1), Vec(0,0,0,1));      // all 6 axes locked

  for (int i = 0; i < 600; i++) scene.stepOnce(1.f/120.f);        // 5 s of gravity

  // welded to the static anchor — it does not fall
  ICL_TEST_NEAR(zOf(bd->getPose()), 500.0f, 8.0f);
  ICL_TEST_TRUE(std::fabs(xOf(bd->getPose())) < 8.0f);
}

ICL_REGISTER_TEST("physics2.spring_pulls_to_point", "a spring constraint pulls a body toward a world point")
{
  PhysicsScene scene;
  scene.world().setGravityEnabled(false);                        // isolate the spring

  auto body = CuboidNode::create(0,0,0, 60,60,60);
  auto bd = scene.add(std::static_pointer_cast<Node>(body), 1.0f);
  bd->setDamping(0.8f, 0.0f);                                     // bleed oscillation -> settles on target

  const Vec target(500, 0, 300, 1);
  // Bullet spring damping is a 0..1 fraction (near-critical here -> no overshoot).
  auto spring = scene.addSpring(body, Vec(0,0,0,1), target, 2000.f, 0.9f);
  ICL_TEST_EQ(scene.world().getConstraintCount(), 1);

  auto distToTarget = [&] {
    Mat p = bd->getPose();
    float dx = p(0,3)-target[0], dy = p(1,3)-target[1], dz = p(2,3)-target[2];
    return std::sqrt(dx*dx + dy*dy + dz*dz);
  };
  float d0 = distToTarget();
  for (int i = 0; i < 2000; i++) scene.stepOnce(1.f/120.f);
  float d1 = distToTarget();

  ICL_TEST_TRUE(d1 < d0 * 0.3f);                                 // pulled most of the way in
  ICL_TEST_TRUE(d1 < 150.0f);
}

ICL_REGISTER_TEST("physics2.constraint_removed_with_body", "removing a body auto-drops its constraints (no crash)")
{
  PhysicsScene scene;
  auto anchor = CuboidNode::create(0,0,0, 40,40,40);
  anchor->translate(0, 0, 500);
  scene.add(std::static_pointer_cast<Node>(anchor), 0.0f);

  auto door = CuboidNode::create(0,0,0, 400,100,20);
  door->translate(200, 0, 500);
  auto dd = scene.add(std::static_pointer_cast<Node>(door), 1.0f);

  scene.addHinge(anchor, door, Vec(0,0,0,1), Vec(-200,0,0,1), 1);
  ICL_TEST_EQ(scene.world().getConstraintCount(), 1);

  // detach the door's rigid body — the hinge must leave the world FIRST (Bullet
  // requires it) so this neither crashes nor leaves a dangling joint.
  door->removeDriver(dd);
  ICL_TEST_EQ(scene.world().getConstraintCount(), 0);

  for (int i = 0; i < 120; i++) scene.stepOnce(1.f/120.f);       // still steps cleanly
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("physics2.stale_handle_inert", "a constraint handle goes inert once a body leaves")
{
  PhysicsScene scene;
  auto anchor = CuboidNode::create(0,0,0, 40,40,40);
  anchor->translate(0, 0, 500);
  scene.add(std::static_pointer_cast<Node>(anchor), 0.0f);

  auto door = CuboidNode::create(0,0,0, 400,100,20);
  door->translate(200, 0, 500);
  auto dd = scene.add(std::static_pointer_cast<Node>(door), 1.0f);

  auto h = scene.addHinge(anchor, door, Vec(0,0,0,1), Vec(-200,0,0,1), 1);
  ICL_TEST_TRUE(h->isActive());

  door->removeDriver(dd);                                        // body gone
  ICL_TEST_TRUE(!h->isActive());
  h->setAngularLimits(Vec(-1,0,0,1), Vec(1,0,0,1));            // inert: no-op, must not crash
  ICL_TEST_TRUE(true);
}
