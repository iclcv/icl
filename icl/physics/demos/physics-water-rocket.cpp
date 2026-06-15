// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Water-air-pressure bottle rocket demo.
//
//   A 1 L PET (Coke) bottle is launched straight up by the air-pressure thrust
//   of the expelled water. During the powered phase the loose nose tip is held
//   on the head purely by contact (the accelerating bottle presses into it).
//   At burnout the bottle decelerates faster than the free-flying tip (air drag
//   on the bottle >> drag on the tip), so the tip separates by itself near the
//   zenith - no constraint, motor or spring involved. At the rocket's apogee a
//   parachute (a real Bullet soft-body cloth, anchored to the bottle by its
//   shroud lines) is released and catches the descent.
//
//   Everything is driven by gravity, contact, air drag and the thrust force only.
//
// Architecture: the Bullet simulation runs in an ICLPhysics PhysicsWorld, while
// rendering is done through the geom2 pipeline. Each physics body is mirrored by
// a geom2 node whose transformation is updated from the Bullet pose every frame
// (the soft-body parachute drives a geom2 MeshNode vertex-by-vertex).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>

#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>

#include <icl/physics/PhysicsWorld.h>
#include <icl/physics/PhysicsDefs.h>
#include <icl/physics/RigidCompoundObject.h>
#include <icl/physics/RigidCylinderObject.h>
#include <icl/physics/RigidBoxObject.h>
#include <icl/physics/RigidConvexHullObject.h>
#include <icl/physics/SoftObject.h>

#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <cmath>
#include <algorithm>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics;

// --- tunables (all in ICL units = millimeters) -----------------------------
static const float GROUND_Z      = 0;       // top surface of the launch pad
static const float EMPTY_MASS     = 0.06f;  // empty bottle (kg)
static const float TIP_MASS       = 0.012f; // nose tip cone
static const float THRUST_PER_BAR = 2000.f; // thrust force per bar of pressure
static const float COAST_DAMP     = 0.25f;  // bottle linear air-drag while flying
static const float TIP_DAMP       = 0.02f;  // tip is streamlined -> little drag
static const float CAM_DIST       = 1600;   // camera distance from the rocket

// bottle geometry (z measured from the base, total ~300 mm, ~90 mm dia)
static const float BODY_R = 45,  BODY_H = 180;          // 0   .. 180
static const float SHLD_R = 30,  SHLD_H = 40;           // 180 .. 220
static const float NECK_R = 15,  NECK_H = 30;           // 220 .. 250
static const float HEAD_Z = 250;                        // where the tip caps
static const float TIP_R  = 16,  TIP_H  = 60;           // nose cone

static std::shared_ptr<Material> mat(const GeomColor &c) {
  return Material::fromColor(c);   // already returns a shared_ptr
}

// =====================================================================
//  Parachute: a from-scratch Bullet soft-body cloth (physics only; it is
//  rendered through a geom2 MeshNode, see updateChuteMesh()).
// =====================================================================
struct BottleParachute : public SoftObject {
  int NR, NS;                                // radial rings, angular segments
  int CANOPY = 0;                            // # canopy nodes (= NR*NS), first in node array
  std::vector<int> tris;                     // canopy triangle indices (render mesh)
  std::vector<std::vector<int>> ropeChains;  // shroud lines: node-index polylines
  std::vector<int> anchorIdx;                // riser node anchored to the bottle

  // Round canopy (annulus with a 10%-radius central vent) PLUS flexible shroud
  // lines, all as ONE soft body: the canopy rim drops rope chains of soft-linked
  // nodes down to a single riser node, which is the only thing anchored to the
  // bottle — so the ropes swing and bend like real cord. All nodes are created
  // before any link (appending nodes later would dangle the links' node ptrs).
  BottleParachute(PhysicsWorld *world, const Vec &center, float R,
                  int rings, int segs, const Vec &attach)
    : SoftObject(), NR(rings), NS(segs), CANOPY(rings*segs) {
    const float rin = 0.05f * R;       // small central vent (less pressure bleed)
    std::vector<btVector3> X;
    for (int r = 0; r < NR; ++r) {
      float rad = rin + (R - rin) * (float(r) / float(NR-1));
      for (int s = 0; s < NS; ++s) {
        float a = 2.f*M_PI*s/NS;
        X.push_back(btVector3(icl2bullet(center[0] + rad*std::cos(a)),
                              icl2bullet(center[1] + rad*std::sin(a)),
                              icl2bullet(center[2])));
      }
    }
    for (int r = 0; r < NR-1; ++r)
      for (int s = 0; s < NS; ++s) {
        int s2 = (s+1) % NS;
        int a = r*NS+s, b = r*NS+s2, c = (r+1)*NS+s2, d = (r+1)*NS+s;
        tris.insert(tris.end(), { a,b,c, a,c,d });
      }

    // riser node + shroud rope chains (outer ring -> intermediate nodes -> riser)
    const btVector3 conv(icl2bullet(attach[0]), icl2bullet(attach[1]), icl2bullet(attach[2]));
    const int riser = (int)X.size();
    X.push_back(conv);
    const int outer = (NR-1)*NS, step = std::max(1, NS/12), L = 5;
    for (int sg = 0; sg < NS; sg += step) {
      int top = outer + sg;
      btVector3 tp = X[top];
      std::vector<int> chain{ top };
      for (int k = 1; k < L; ++k) {
        int idx = (int)X.size();
        X.push_back(tp.lerp(conv, float(k)/L));
        chain.push_back(idx);
      }
      chain.push_back(riser);
      ropeChains.push_back(chain);
    }

    // per-node masses (canopy spreads ~0.10kg; ropes light; riser a bit heavier)
    const int total = (int)X.size();
    std::vector<btScalar> M(total, 0.0008f);
    for (int i = 0; i < CANOPY; ++i) M[i] = 0.10f / CANOPY;
    M[riser] = 0.003f;

    btSoftBody *s = new btSoftBody(world->getWorldInfo(), total, X.data(), M.data());
    setPhysicalObject(s);

    btSoftBody::Material *mc = s->m_materials[0];   // canopy: soft -> billows
    mc->m_kLST = 0.12f; mc->m_kAST = 0.10f;
    btSoftBody::Material *mr = s->appendMaterial(); // rope: stiff stretch, free bend
    mr->m_kLST = 0.9f;  mr->m_kAST = 0.0f;

    for (size_t t = 0; t < tris.size(); t += 3) {
      int a = tris[t], b = tris[t+1], c = tris[t+2];
      s->appendLink(a, b, mc, true);
      s->appendLink(b, c, mc, true);
      s->appendLink(c, a, mc, true);
      s->appendFace(a, b, c, mc);                  // faces drive the aerodynamic drag
    }
    for (auto &ch : ropeChains)
      for (size_t i = 0; i+1 < ch.size(); ++i)
        s->appendLink(ch[i], ch[i+1], mr, true);

    s->getCollisionShape()->setMargin(icl2bullet(3));
    s->m_cfg.kDP  = 0.10f;
    s->m_cfg.kDG  = 1.5f;                           // aerodynamic drag; live-tuned
    s->m_cfg.kLF  = 0.0f;                           // no lift -> no gliding upward
    s->m_cfg.kAHR = 1.0f;
    s->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSided;
    s->forceActivationState(DISABLE_DEACTIVATION);  // never sleep / freeze mid-air

    // Deploy already open: the canopy is created at its full (round) shape and
    // Bullet does what it's good at — billowing/swaying under aerodynamic drag
    // as it descends. (Self-opening from a packed bundle would need soft-body
    // self-collision / FSI, which Bullet's crude aero can't do reliably.)
    anchorIdx = { riser };
  }

  int nodeCount() const { return CANOPY; }

  /// anchor the riser node to the bottle (the single hard attachment point)
  void anchorTo(RigidObject *r) {
    btSoftBody *s = getSoftBody();
    btRigidBody *rb = r->getRigidBody();
    for (int c : anchorIdx) s->appendAnchor(c, rb, false, 1.0f);
  }
};

// =====================================================================
//  Globals
// =====================================================================
enum RocketState { ARMED, THRUST, COAST, DESCENT, LANDED };

GUI gui;
FPSLimiter fps(60);
PhysicsWorld world;                 // Bullet simulation
Scene2 scene;                       // geom2 rendering

// physics bodies
RigidCompoundObject *rocket = 0;
RigidConvexHullObject *tip = 0;
RigidBoxObject *ground = 0;
BottleParachute *chute = 0;

// geom2 render nodes mirroring the physics bodies
std::shared_ptr<GroupNode> rocketNode;
std::shared_ptr<ConeNode>  tipNode;
std::shared_ptr<MeshNode>  chuteMesh;
std::shared_ptr<MeshNode>  ropes;       // shroud lines: canopy corners -> bottle head

btTransform rocketHome, tipHome;
Vec tripodPos(-900,-600,350,1);     // fixed camera spot used while the chute descends
RocketState state = ARMED;
Time launchTime;
float burnTime = 0.5f, thrust = 12000.f, waterMass = 0.2f;

static Vec sbNode(const btSoftBody *s, int i) {
  const btVector3 &p = s->m_nodes[i].m_x;
  return Vec(bullet2icl(p[0]), bullet2icl(p[1]), bullet2icl(p[2]), 1);
}

static std::vector<Vec> makeCone(float r, float h, int seg) {
  std::vector<Vec> v;
  v.push_back(Vec(0,0,h,1));                                 // apex
  for (int i = 0; i < seg; ++i) {
    float a = 2*M_PI*i/seg;
    v.push_back(Vec(r*std::cos(a), r*std::sin(a), 0, 1));    // base ring
  }
  return v;
}

// point the camera at a target from a fixed 3/4 angle (lookAt handles the
// world-up -> internal up convention; we pass world +Z as the up that should
// appear at the top of the image)
static void framePoint(const Vec &target) {
  Vec camPos = target + Vec(-CAM_DIST*0.80f, -CAM_DIST*0.55f, CAM_DIST*0.32f, 0);
  camPos[3] = 1;
  scene.getCamera(0) = Camera::lookAt(camPos, target, Vec(0,0,1,1), Size::VGA, 50.f);
}

// =====================================================================
//  Build
// =====================================================================
static void buildRocket() {
  // ---- physics: bottle = compound of body + shoulder + neck + crossed fins ----
  rocket = new RigidCompoundObject(0, 0, GROUND_Z, EMPTY_MASS);
  rocket->addChild(new RigidCylinderObject(0,0, BODY_H/2,                 BODY_R, BODY_H), true);
  rocket->addChild(new RigidCylinderObject(0,0, BODY_H+SHLD_H/2,          SHLD_R, SHLD_H), true);
  rocket->addChild(new RigidCylinderObject(0,0, BODY_H+SHLD_H+NECK_H/2,   NECK_R, NECK_H), true);
  rocket->addChild(new RigidBoxObject(0,0, 45, 2*BODY_R+30, 3, 80), true);
  rocket->addChild(new RigidBoxObject(0,0, 45, 3, 2*BODY_R+30, 80), true);
  rocket->setFriction(0.6f);
  rocket->setRestitution(0.1f);
  rocket->setActivationMode(RigidObject::ACTIVE_FOREVER);
  world.addObject(rocket);

  // ---- physics: loose nose tip cone, just resting on the neck (no constraint) ----
  tip = new RigidConvexHullObject(0, 0, HEAD_Z, makeCone(TIP_R, TIP_H, 16),
                                  Vec(0,0,0,0), TIP_MASS);
  tip->setFriction(0.6f);
  tip->setRestitution(0.05f);
  tip->setDamping(TIP_DAMP, 0.3f);
  tip->setActivationMode(RigidObject::ACTIVE_FOREVER);
  world.addObject(tip);

  rocketHome = rocket->getRigidBody()->getWorldTransform();
  tipHome    = tip->getRigidBody()->getWorldTransform();

  // ---- geom2: visual mirror of the bottle (local coords match the compound) ----
  auto green = mat(GeomColor(40,170,60,255));
  auto white = mat(GeomColor(230,230,230,255));
  auto red   = mat(GeomColor(180,40,40,255));

  rocketNode = std::make_shared<GroupNode>();
  auto body  = CylinderNode::create(0,0, BODY_H/2,               BODY_R, BODY_R, BODY_H, 24);
  auto shld  = CylinderNode::create(0,0, BODY_H+SHLD_H/2,        SHLD_R, SHLD_R, SHLD_H, 24);
  auto neck  = CylinderNode::create(0,0, BODY_H+SHLD_H+NECK_H/2, NECK_R, NECK_R, NECK_H, 16);
  auto finA  = CuboidNode::create(0,0, 45, 2*BODY_R+30, 3, 80);
  auto finB  = CuboidNode::create(0,0, 45, 3, 2*BODY_R+30, 80);
  body->setMaterial(green); shld->setMaterial(green); neck->setMaterial(white);
  finA->setMaterial(red);   finB->setMaterial(red);
  rocketNode->addChild(body); rocketNode->addChild(shld); rocketNode->addChild(neck);
  rocketNode->addChild(finA); rocketNode->addChild(finB);
  scene.addNode(rocketNode);

  // geom2 nose cone: local cone (base z=0, apex z=TIP_H) -> centered at TIP_H/2
  tipNode = ConeNode::create(0,0, TIP_H/2, TIP_R, TIP_R, TIP_H, 16);
  tipNode->setMaterial(white);
  scene.addNode(tipNode);
}

static void deployParachute() {
  Mat pose = bullet2icl(rocket->getRigidBody()->getWorldTransform());
  Vec canopyCenter = pose * Vec(0,0,HEAD_Z+450,1);   // canopy opens well above the head
  Vec attach       = pose * Vec(0,0,HEAD_Z,1);        // shroud lines converge at the head
  chute = new BottleParachute(&world, canopyCenter, 420, 12, 30, attach);
  world.addObject(chute);
  chute->anchorTo(rocket);
  rocket->setDamping(0.05f, 0.6f);   // let the chute (not damping) govern the descent
  tripodPos = scene.getCamera(0).getPosition();   // freeze the camera here to watch it fall

  // build the geom2 canopy mesh from the soft-body canopy nodes + triangles
  const btSoftBody *s = chute->getSoftBody();
  chuteMesh->clearGeometry();
  for (int i = 0; i < chute->nodeCount(); ++i)
    chuteMesh->addVertex(sbNode(s, i), GeomColor(255,140,0,255));
  for (size_t t = 0; t+2 < chute->tris.size(); t += 3)
    chuteMesh->addTriangle(chute->tris[t], chute->tris[t+1], chute->tris[t+2]);
  chuteMesh->createAutoNormals(true);
  chuteMesh->setVisible(true);
  ropes->setVisible(true);
  state = DESCENT;
}

static void resetRocket() {
  if (chute) { world.removeObject(chute); delete chute; chute = 0; }
  chuteMesh->clearGeometry();
  chuteMesh->setVisible(false);
  ropes->clearGeometry();
  ropes->setVisible(false);

  rocket->getRigidBody()->setWorldTransform(rocketHome);
  rocket->getRigidBody()->clearForces();
  rocket->setLinearVelocity(Vec(0,0,0,1));
  rocket->setAngularVelocity(Vec(0,0,0,1));
  rocket->setMass(EMPTY_MASS);
  rocket->setDamping(0.f, 0.f);
  rocket->setActive(true);

  tip->getRigidBody()->setWorldTransform(tipHome);
  tip->getRigidBody()->clearForces();
  tip->setLinearVelocity(Vec(0,0,0,1));
  tip->setAngularVelocity(Vec(0,0,0,1));
  tip->setActive(true);

  state = ARMED;
  gui["state"] = str("ARMED");
}

static void launch() {
  if (state != ARMED) return;
  int pressure = gui["pressure"].as<int>();
  float waterFrac = gui["water"].as<int>() / 100.f;
  thrust    = pressure * THRUST_PER_BAR;
  waterMass = waterFrac * 0.25f;            // up to 250 g of water
  burnTime  = 0.2f + waterFrac * 0.5f;      // more water -> longer burn
  launchTime = Time::now();
  rocket->setMass(EMPTY_MASS + waterMass);
  rocket->setDamping(0.f, 0.6f);            // keep upright, no air drag yet
  rocket->setActive(true);
  state = THRUST;
  gui["state"] = str("THRUST");
}

// =====================================================================
//  per-frame sync (geom2 nodes <- Bullet poses)
// =====================================================================
static void syncRender() {
  rocketNode->setTransformation(bullet2icl(rocket->getRigidBody()->getWorldTransform()));
  tipNode->setTransformation(bullet2icl(tip->getRigidBody()->getWorldTransform()));
  if (chute) {
    // the renderer caches each node's geometry; the canopy mesh changes every
    // frame, so its cache must be invalidated or it draws frozen at deploy shape
    scene.getRenderer().invalidateCache();
    chute->getSoftBody()->m_cfg.kDG = gui["drag"].as<int>() / 100.f;  // live-tunable drag
    const btSoftBody *s = chute->getSoftBody();
    auto &verts = chuteMesh->getVertices();
    const int dim = chute->nodeCount();
    for (int i = 0; i < dim && i < (int)verts.size(); ++i) verts[i] = sbNode(s, i);
    chuteMesh->createAutoNormals(true);

    // shroud lines: trace each soft-body rope chain (now flexible, swinging cord)
    ropes->clearGeometry();
    int base = 0;
    for (auto &ch : chute->ropeChains) {
      for (int idx : ch) ropes->addVertex(sbNode(s, idx), GeomColor(235,235,235,255));
      for (size_t j = 0; j+1 < ch.size(); ++j) ropes->addLine(base+j, base+j+1, GeomColor(235,235,235,255));
      base += (int)ch.size();
    }
  }
}

void init() {
  gui << ( ui::HBox()
           << ui::Canvas3D(Size(800,600), {.handle="draw", .minSize={32,24}})
           << ( ui::VBox({.maxSize={16,99}})
                << ui::Button("Launch", {.handle="launch"})
                << ui::Button("Reset",  {.handle="reset"})
                << ui::Slider(1, 10, 6,   {.handle="pressure", .label="pressure [bar]"})
                << ui::Slider(0, 100, 60, {.handle="water",    .label="water fill [%]"})
                << ui::Slider(0, 400, 150, {.handle="drag",    .label="chute drag x100"})
                << ui::CheckBox("free look", {.handle="free"})
                << ui::Label("ARMED",     {.handle="state",    .label="state"})
              )
         )
      << ui::Show();

  scene.addCamera(Camera::lookAt(Vec(-900,-600,350,1), Vec(0,0,HEAD_Z*0.7f,1),
                                 Vec(0,0,1,1), Size::VGA, 50.f));
  scene.setBounds(6000);

  world.setGravity(Vec(0,0,-9810,1));

  // ---- launch pad (static) ----
  ground = new RigidBoxObject(0,0, GROUND_Z-30, 1000, 1000, 60, 0);
  ground->setFriction(0.8f);
  world.addObject(ground);
  auto groundNode = CuboidNode::create(0,0, GROUND_Z-30, 1000, 1000, 60);
  groundNode->setMaterial(mat(GeomColor(110,110,120,255)));
  scene.addNode(groundNode);

  // ---- parachute render node (empty/invisible until deployed) ----
  chuteMesh = std::make_shared<MeshNode>();
  chuteMesh->setVisible(false);
  scene.addNode(chuteMesh);

  ropes = std::make_shared<MeshNode>();
  ropes->setVisible(false);
  ropes->setPrimitiveVisible(PrimLine, true);   // geom2 gates lines behind this
  ropes->setLineWidth(2.f);
  scene.addNode(ropes);

  buildRocket();

  // ---- light ----
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setColor(GeomColor(1.0f, 0.97f, 0.92f, 1.0f));
  light->setIntensity(1.0f);
  light->translate(800, 600, 1500);
  scene.addLight(light);

  gui["launch"].registerCallback(launch);
  gui["reset"].registerCallback(resetRocket);

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

void run() {
  switch (state) {
    case THRUST: {
      float t = (Time::now() - launchTime).toMicroSeconds() / 1.0e6f;
      if (t >= burnTime) {
        rocket->setMass(EMPTY_MASS);
        rocket->setDamping(COAST_DAMP, 0.6f);   // bottle now feels the air -> tip lets go
        state = COAST;
        gui["state"] = str("COAST");
      } else {
        float frac = 1.f - t / burnTime;         // thrust + water mass decay over the burn
        rocket->setMass(EMPTY_MASS + waterMass * frac);
        rocket->applyCentralForce(Vec(0, 0, thrust * frac, 1));
      }
      break;
    }
    case COAST:
      if (rocket->getLinearVelocity()[2] <= 0) {  // apogee -> release the chute
        deployParachute();
        gui["state"] = str("DESCENT");
      }
      break;
    case DESCENT:
      if (bullet2icl(rocket->getRigidBody()->getWorldTransform())[11] <= GROUND_Z + BODY_H/2 + 5 &&
          std::abs(rocket->getLinearVelocity()[2]) < 200) {
        state = LANDED;
        gui["state"] = str("LANDED");
      }
      break;
    default: break;
  }

  world.step();
  syncRender();
  if (!gui["free"].as<bool>()) {                   // follow by default; tick "free look" to orbit
    Vec target = bullet2icl(rocket->getRigidBody()->getWorldTransform()) * Vec(0,0,HEAD_Z*0.7f,1);
    if (state == DESCENT || state == LANDED)       // tripod: camera stays put, watches it fall
      scene.getCamera(0) = Camera::lookAt(tripodPos, target, Vec(0,0,1,1), Size::VGA, 50.f);
    else
      framePoint(target);                          // follow during ascent
  }

  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApplication(n, ppc, "", init, run).exec();
}
