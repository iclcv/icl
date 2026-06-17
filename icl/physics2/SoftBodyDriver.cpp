// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/SoftBodyDriver.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/StateBuffer.h>
#include <icl/geom2/Node.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/Primitive.h>
#include <icl/utils/Macros.h>

#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <algorithm>

namespace icl::physics2 {

  struct SoftBodyDriver::Data {
    PhysicsWorld &world;
    Vec c00, c10, c01, c11;
    int resX, resY, fixedMask;
    float mass;
    Units units;
    SoftStateBuffer buffer;
    btSoftBody *body = nullptr;
    bool added = false;
    Data(PhysicsWorld &w) : world(w) {}
  };

  SoftBodyDriver::SoftBodyDriver(PhysicsWorld &world,
                                 const Vec &c00, const Vec &c10,
                                 const Vec &c01, const Vec &c11,
                                 int resX, int resY, int fixedCornerMask,
                                 float totalMass)
    : m_data(std::make_unique<Data>(world)) {
    m_data->c00 = c00; m_data->c10 = c10; m_data->c01 = c01; m_data->c11 = c11;
    m_data->resX = resX; m_data->resY = resY;
    m_data->fixedMask = fixedCornerMask; m_data->mass = totalMass;
  }

  SoftBodyDriver::~SoftBodyDriver() { delete m_data->body; }

  static int nodeIndex(const btSoftBody *sb, const btSoftBody::Node *n) {
    return static_cast<int>(n - &sb->m_nodes[0]);
  }

  void SoftBodyDriver::onAttach() {
    auto *mesh = dynamic_cast<geom2::MeshNode *>(node());
    if (!mesh) { ERROR_LOG("SoftBodyDriver must be attached to a geom2::MeshNode"); return; }
    m_data->units = m_data->world.getUnits();

    auto *info = m_data->world.getSoftBodyWorldInfo();
    const Units &u = m_data->units;
    // Build the patch unpinned, distribute mass, THEN pin the requested corners
    // by index — setTotalMass() resets inverse-mass and would un-pin corners
    // passed to CreatePatch's `fixeds`.
    btSoftBody *sb = btSoftBodyHelpers::CreatePatch(
        *info, u.toBulletVec(m_data->c00), u.toBulletVec(m_data->c10),
        u.toBulletVec(m_data->c01), u.toBulletVec(m_data->c11),
        m_data->resX, m_data->resY, /*fixeds*/ 0, true);

    // Stable cloth config + working soft-rigid collision. Bullet's raw defaults
    // (max-stiff links, no damping, tiny margin) make an unpinned cloth tunnel
    // through rigid bodies and then explode to NaN; mirror the proven legacy
    // PhysicsPaper settings: softer links, damping, a real collision margin,
    // SDF soft-vs-rigid collision, and some bending stiffness.
    btSoftBody::Material *mat = sb->m_materials[0];
    mat->m_kLST = 0.4f;                 // linear stiffness (softer = stable)
    mat->m_kAST = 0.4f;                 // angular/area stiffness
    sb->generateBendingConstraints(2, mat);
    sb->m_cfg.piterations = 5;          // position-solver iterations (stability)
    sb->m_cfg.kDP = 0.05f;              // damping
    sb->m_cfg.kDF = 1.0f;               // dynamic friction vs rigid bodies
    sb->getCollisionShape()->setMargin(u.toBullet(2.0f));   // legacy-proven margin
    sb->m_cfg.collisions = btSoftBody::fCollision::SDF_RS;   // soft vs rigid (SDF)
    sb->randomizeConstraints();
    sb->setTotalMass(m_data->mass, true);
    // NOTE: active soft-vs-rigid collision (cloth draping on a box) is fragile at
    // this world scale — both SDF_RS and cluster (CL_RS) can crash Bullet's
    // narrowphase on flat-cloth contact. Gravity-only soft bodies (hang/sag/
    // anchor) are stable. Stabilizing the drape case belongs to the physics
    // defaults-policy work (scale-correct margins, cluster sizing, self-collision).
    const int rx = m_data->resX, ry = m_data->resY, mask = m_data->fixedMask;
    auto pin = [&](int idx) { if (idx >= 0 && idx < sb->m_nodes.size()) sb->setMass(idx, 0); };
    if (mask & 1) pin(0);                 // c00
    if (mask & 2) pin(rx - 1);            // c10
    if (mask & 4) pin(rx * (ry - 1));     // c01
    if (mask & 8) pin(rx * ry - 1);       // c11
    m_data->body = sb;

    m_data->world.addSoftBody(sb);
    m_data->world.addCapture(this, [this]() {
      const Units &u2 = m_data->units;
      btSoftBody *b = m_data->body;
      std::vector<Vec> pos(b->m_nodes.size());
      for (int i = 0; i < b->m_nodes.size(); i++) pos[i] = u2.toIclVec(b->m_nodes[i].m_x);
      m_data->buffer.publish(std::move(pos));
    });

    // build the MeshNode topology once (vertices from nodes, triangles from faces)
    const geom2::GeomColor col(0.85f, 0.85f, 0.9f, 1.0f);
    mesh->clearGeometry();
    for (int i = 0; i < sb->m_nodes.size(); i++) {
      mesh->addVertex(u.toIclVec(sb->m_nodes[i].m_x), col);
    }
    for (int f = 0; f < sb->m_faces.size(); f++) {
      const btSoftBody::Face &face = sb->m_faces[f];
      mesh->addTriangle(nodeIndex(sb, face.m_n[0]),
                        nodeIndex(sb, face.m_n[1]),
                        nodeIndex(sb, face.m_n[2]));
    }
    mesh->createAutoNormals(true);
    m_data->added = true;
  }

  void SoftBodyDriver::onDetach() {
    if (m_data->added) {
      m_data->world.removeCapture(this);
      m_data->world.removeSoftBody(m_data->body);
      m_data->added = false;
    }
    delete m_data->body;
    m_data->body = nullptr;
  }

  void SoftBodyDriver::sync(double /*dt*/, double /*alpha*/) {
    auto *mesh = dynamic_cast<geom2::MeshNode *>(node());
    if (!mesh) return;
    std::vector<Vec> pos;
    if (!m_data->buffer.sample(pos)) return;
    auto &v = mesh->getVertices();
    if (v.size() == pos.size()) std::copy(pos.begin(), pos.end(), v.begin());
    mesh->createAutoNormals(true);   // recompute from the new positions
  }

  btSoftBody *SoftBodyDriver::softBody() const { return m_data->body; }

  int SoftBodyDriver::cornerNodeIndex(int corner) const {
    const int rx = m_data->resX, ry = m_data->resY;
    switch (corner) {
      case 0: return 0;                 // c00
      case 1: return rx - 1;            // c10
      case 2: return rx * (ry - 1);     // c01
      case 3: return rx * ry - 1;       // c11
      default: return -1;
    }
  }

  void SoftBodyDriver::anchorNode(int nodeIndex, RigidBodyDriver *rigid, float influence) {
    btSoftBody *sb = m_data->body;
    btRigidBody *rb = rigid ? rigid->body() : nullptr;
    if (!sb || !rb) return;
    m_data->world.enqueue([sb, nodeIndex, rb, influence]() {
      if (nodeIndex >= 0 && nodeIndex < sb->m_nodes.size())
        sb->appendAnchor(nodeIndex, rb, true, influence);
    });
  }

} // namespace icl::physics2
