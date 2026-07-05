// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/SoftBodyDriver.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/StateBuffer.h>
#include <icl/viz3d/nodes/Node.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/render/Primitive.h>
#include <icl/utils/Macros.h>
#include <icl/utils/prop/Constraints.h>
#include <algorithm>
#include <cmath>
#include <mutex>

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
    bool smoothNormals = true;   // render-side (UI thread): interpolate vs faceted
    Data(PhysicsWorld &w) : world(w) {}
  };

  namespace {
    // Snapshot of the live-tunable cloth parameters (read on the UI thread,
    // applied on the sim thread).
    struct ClothCfg {
      float kLST, kDF, kDP, kCHR, marginMm;
      int   piter;
      bool  selfCol, clusters;
      float ksDeform, kdDeform;   // deformable mass-spring force constants (node-mass aware)
    };

    // Deformable mass-spring stiffness must scale with NODE MASS, else a dense
    // grid (small per-node mass) explodes under explicit integration: stability
    // needs dt < ~2/sqrt(k/m), and a fixed k with shrinking m blows past it. With
    // k = w^2 * m the natural frequency w = sqrt(k/m) is resolution-INDEPENDENT,
    // so the same slider gives the same feel AND the same stability at every grid
    // density. kDeformBase sets w^2 at slider=1 (tuned for stability at >=120 Hz
    // sim; final feel wants a real-display pass).
    constexpr float kDeformBase = 8000.f;
    void deformParams(float kLST, float kDP, float nodeMass, float &ks, float &kd) {
      ks = kLST * kDeformBase * nodeMass;                  // k = (kLST*base) * m
      kd = kDP * 2.f * std::sqrt(std::max(1e-12f, ks * nodeMass));  // fraction of critical damping
    }

    void applyClothCfg(btSoftBody *sb, const ClothCfg &c, const Units &u) {
      if (!sb) return;
      sb->m_materials[0]->m_kLST = c.kLST;
      sb->m_materials[0]->m_kAST = c.kLST;
      // Links cache m_c0 = (ima+imb)*kLST at creation, so changing kLST alone
      // is inert — recompute the link constants (this does NOT reset rest
      // lengths, unlike updateConstants(), so an in-flight drape is preserved).
      sb->updateLinkConstants();
      sb->m_cfg.piterations = c.piter;
      sb->m_cfg.kDP  = c.kDP;
      sb->m_cfg.kDF  = c.kDF;
      sb->m_cfg.kCHR = c.kCHR;
      sb->getCollisionShape()->setMargin(u.toBullet(c.marginMm));

      int col;
      if (c.clusters) {
        // Cluster collision (groups of faces vs rigid) — smoother drape than
        // per-vertex SDF. Clusters are generated up front in buildBody(). Keep
        // cluster-rigid hardness well below 1 — maximal hardness overshoots and
        // pumps energy at rest until the body explodes.
        col = btSoftBody::fCollision::CL_RS;
        if (c.selfCol) col |= btSoftBody::fCollision::CL_SS;
        sb->m_cfg.kSRHR_CL = c.kCHR; sb->m_cfg.kSKHR_CL = c.kCHR; sb->m_cfg.kSSHR_CL = c.kCHR;
      } else {
        col = btSoftBody::fCollision::SDF_RS;
        if (c.selfCol) col |= btSoftBody::fCollision::VF_SS;
      }
      sb->m_cfg.collisions = col;
    }

    // Deformable-world cloth config. The deformable solver uses different
    // collision flags (SDF_RD/SDF_RDF face contacts) than the legacy SDF_RS, and
    // stiffness/damping live on the mass-spring force (swapped via the world), not
    // on the material. Clusters / piterations are legacy-only and ignored here.
    void applyDeformableCfg(btSoftBody *sb, const ClothCfg &c, const Units &u,
                            PhysicsWorld &w) {
      if (!sb) return;
      sb->getCollisionShape()->setMargin(u.toBullet(c.marginMm));
      sb->m_cfg.kDF  = c.kDF;
      sb->m_cfg.kCHR = c.kCHR;
      sb->m_cfg.kKHR = 1;
      int col = btSoftBody::fCollision::SDF_RD | btSoftBody::fCollision::SDF_RDF;
      if (c.selfCol) col |= btSoftBody::fCollision::VF_DD;
      sb->m_cfg.collisions = col;
      w.setClothStiffness(sb, c.ksDeform, c.kdDeform);
    }
  }

  SoftBodyDriver::SoftBodyDriver(PhysicsWorld &world,
                                 const Vec &c00, const Vec &c10,
                                 const Vec &c01, const Vec &c11,
                                 int resX, int resY, int fixedCornerMask,
                                 float totalMass)
    : m_data(std::make_unique<Data>(world)) {
    m_data->c00 = c00; m_data->c10 = c10; m_data->c01 = c01; m_data->c11 = c11;
    m_data->resX = resX; m_data->resY = resY;
    m_data->fixedMask = fixedCornerMask; m_data->mass = totalMass;

    // Scale-aware default margin: a fraction of the node spacing (a fixed small
    // margin lets fast nodes tunnel through rigid bodies in one step).
    auto len = [](const Vec &a, const Vec &b) {
      float dx = a[0]-b[0], dy = a[1]-b[1], dz = a[2]-b[2];
      return std::sqrt(dx*dx + dy*dy + dz*dz);
    };
    float spacing = std::min(len(c10, c00) / std::max(1, resX - 1),
                             len(c01, c00) / std::max(1, resY - 1));
    float marginMm = std::min(12.0f, std::max(3.0f, spacing * 0.3f));

    using namespace utils;
    addProperty("stiffness", prop::Range{.min=0.02f, .max=1.f, .step=0.01f}, 0.4f,
                "link stiffness (softer = drapier / stabler; high values suit heavier cloth)");
    addProperty("friction", prop::Range{.min=0.f, .max=1.f, .step=0.05f}, 0.5f,
                "friction vs rigid bodies (lower = drapes, higher = grips)");
    addProperty("damping", prop::Range{.min=0.f, .max=0.5f, .step=0.01f}, 0.12f,
                "velocity damping (higher = calmer at rest)");
    addProperty("contact hardness", prop::Range{.min=0.f, .max=1.f, .step=0.05f}, 0.6f,
                "rigid-contact hardness (1 overshoots/explodes at rest, low = sinks through)");
    addProperty("collision margin/mm", prop::Range{.min=1.f, .max=30.f, .step=0.5f}, marginMm,
                "soft-vs-rigid buffer (too small -> tunneling)");
    // SoftRigid-only knobs: the deformable solver ignores the legacy position
    // iterations and cluster-collision path (see applyDeformableCfg), so they are
    // not exposed when this driver's world runs the deformable pipeline — the
    // Prop UI then only shows what actually applies to the selected backend.
    const bool deform = world.isDeformable();
    if (!deform) {
      addProperty("position iterations", prop::Range{.min=1, .max=30}, 10,
                  "solver iterations (higher = stabler, slower)");
      addProperty("collision mode", prop::Menu{"SDF", "Clusters"}, "SDF",
                  "Clusters = face-group collision, smoother drape (fragile at large scale)");
    }
    addProperty("self collision", prop::Flag{}, false,
                "stops the cloth folding through itself");
    addProperty("smooth normals", prop::Flag{}, true,
                "interpolate vertex normals for smooth shading (off = faceted/flat)");
    addProperty("size", prop::Range{.min=1.f, .max=3.f, .step=0.05f}, 1.f,
                "scale the cloth about its centre (resets it flat at the new size)");
    addProperty("node density", prop::Range{.min=10, .max=100}, std::max(resX, resY),
                "grid resolution per axis (rebuilds the cloth; high = smooth but slow)");

    registerCallback([this](const Configurable::Property &p) {
      if (p.name == "size") {
        applySize();                     // geometry change, not a solver param
      } else if (p.name == "node density") {
        int n = (int)prop("node density").value;   // structural rebuild
        rebuildAtResolution(n, n);
      } else if (p.name == "smooth normals") {
        m_data->smoothNormals = (bool)prop(p.name).value;   // render-side only
        if (auto *mesh = dynamic_cast<viz3d::MeshNode *>(node()))
          mesh->setSmoothShading(m_data->smoothNormals);    // flat vs faceted
      } else {
        pushConfig(true);
      }
    });
  }

  SoftBodyDriver::~SoftBodyDriver() { delete m_data->body; }

  static int nodeIndex(const btSoftBody *sb, const btSoftBody::Node *n) {
    return static_cast<int>(n - &sb->m_nodes[0]);
  }

  void SoftBodyDriver::cornersAtSize(Vec &o00, Vec &o10, Vec &o01, Vec &o11) const {
    const float s = (float)prop("size").value;
    const Vec &a = m_data->c00, &b = m_data->c10, &c = m_data->c01, &d = m_data->c11;
    Vec ctr((a[0]+b[0]+c[0]+d[0])*0.25f, (a[1]+b[1]+c[1]+d[1])*0.25f,
            (a[2]+b[2]+c[2]+d[2])*0.25f, 1);
    auto sc = [&](const Vec &v) {
      return Vec(ctr[0]+s*(v[0]-ctr[0]), ctr[1]+s*(v[1]-ctr[1]), ctr[2]+s*(v[2]-ctr[2]), 1);
    };
    o00 = sc(a); o10 = sc(b); o01 = sc(c); o11 = sc(d);
  }

  void SoftBodyDriver::onAttach() { buildBody(); }

  void SoftBodyDriver::rebuildAtResolution(int resX, int resY) {
    if (!m_data->body) { m_data->resX = resX; m_data->resY = resY; return; }
    // Block the sim thread for the body swap + mesh-topology rebuild.
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    m_data->world.removeCapture(this);
    m_data->world.removeSoftBody(m_data->body);
    delete m_data->body;
    m_data->body = nullptr;
    m_data->added = false;
    m_data->resX = resX; m_data->resY = resY;
    buildBody();
  }

  void SoftBodyDriver::buildBody() {
    auto *mesh = dynamic_cast<viz3d::MeshNode *>(node());
    if (!mesh) { ERROR_LOG("SoftBodyDriver must be attached to a viz3d::MeshNode"); return; }
    m_data->units = m_data->world.getUnits();

    auto *info = m_data->world.getSoftBodyWorldInfo();
    const Units &u = m_data->units;
    // Build the patch (at the current size-scaled corners) unpinned, distribute
    // mass, THEN pin the requested corners by index — setTotalMass() resets
    // inverse-mass and would un-pin corners passed to CreatePatch's `fixeds`.
    Vec s00, s10, s01, s11;
    cornersAtSize(s00, s10, s01, s11);
    btSoftBody *sb = btSoftBodyHelpers::CreatePatch(
        *info, u.toBulletVec(s00), u.toBulletVec(s10),
        u.toBulletVec(s01), u.toBulletVec(s11),
        m_data->resX, m_data->resY, /*fixeds*/ 0, true);

    // Structural setup (not live-tunable): bending links, collision-solver
    // iterations, constraint ordering, mass distribution, corner pins. The
    // tunable params (stiffness / friction / margin / collision mode / ...) are
    // applied by pushConfig() from the Configurable properties.
    btSoftBody::Material *mat = sb->m_materials[0];
    sb->generateBendingConstraints(2, mat);
    sb->m_cfg.citerations = 6;
    sb->randomizeConstraints();
    sb->setTotalMass(m_data->mass, true);
    const int rx = m_data->resX, ry = m_data->resY, mask = m_data->fixedMask;
    auto pin = [&](int idx) { if (idx >= 0 && idx < sb->m_nodes.size()) sb->setMass(idx, 0); };
    if (mask & 1) pin(0);                 // c00
    if (mask & 2) pin(rx - 1);            // c10
    if (mask & 4) pin(rx * (ry - 1));     // c01
    if (mask & 8) pin(rx * ry - 1);       // c11

    const bool deform = m_data->world.isDeformable();
    if (!deform) {
      // Generate collision clusters up front (~one per 6x6 node patch). Doing it
      // at build time means switching to cluster-collision mode is instant —
      // generating them lazily on a live, stepping body runs a slow k-means and
      // is fragile. Clusters are inert until cluster collision is enabled.
      // (Deformable mode uses SDF/face contacts, not clusters.)
      sb->generateClusters(std::clamp((rx * ry) / 36, 16, 128));
    }

    m_data->body = sb;

    if (deform) {
      // Deformable: the body must be in the world before its forces are attached,
      // and the forces must exist before pushConfig() live-updates their stiffness.
      float nodeMass = m_data->mass / std::max(1, rx * ry);
      float ks, kd;
      deformParams((float)prop("stiffness").value, (float)prop("damping").value,
                   nodeMass, ks, kd);
      m_data->world.addSoftBody(sb);
      m_data->world.addClothForces(sb, ks, kd);
      pushConfig(false);   // collision flags + margin + final stiffness/damping
    } else {
      pushConfig(false);   // apply the tunable properties (sim not running yet)
      m_data->world.addSoftBody(sb);
    }
    m_data->world.addCapture(this, [this]() {
      const Units &u2 = m_data->units;
      btSoftBody *b = m_data->body;
      // Anti-explosion safety net: soft-body contact solvers can pump energy at
      // rest and diverge within a few steps. Cap node velocity to a value far
      // above any real drape motion — this arrests the runaway harmlessly while
      // never touching normal movement.
      const btScalar vmax = u2.toBullet(15000.f);   // ~15 m/s
      const btScalar vmax2 = vmax * vmax;
      for (int i = 0; i < b->m_nodes.size(); i++) {
        btScalar v2 = b->m_nodes[i].m_v.length2();
        if (v2 > vmax2) b->m_nodes[i].m_v *= vmax / btSqrt(v2);
      }
      std::vector<Vec> pos(b->m_nodes.size());
      for (int i = 0; i < b->m_nodes.size(); i++) pos[i] = u2.toIclVec(b->m_nodes[i].m_x);
      m_data->buffer.publish(std::move(pos));
    });

    // build the MeshNode topology once (vertices from nodes, triangles from faces)
    const viz3d::GeomColor col(0.85f, 0.85f, 0.9f, 1.0f);
    mesh->clearGeometry();
    for (int i = 0; i < sb->m_nodes.size(); i++) {
      mesh->addVertex(u.toIclVec(sb->m_nodes[i].m_x), col);
    }
    for (int f = 0; f < sb->m_faces.size(); f++) {
      const btSoftBody::Face &face = sb->m_faces[f];
      int a = nodeIndex(sb, face.m_n[0]), b = nodeIndex(sb, face.m_n[1]),
          c = nodeIndex(sb, face.m_n[2]);
      // Normal indices = vertex indices, so the per-vertex normals computed by
      // createAutoNormals() are actually used by the renderer (it indexes normals
      // by the triangle's n[]; left at -1 it falls back to flat per-face normals).
      mesh->addTriangle(a, b, c, a, b, c);
    }
    mesh->createAutoNormals(true);
    mesh->setSmoothShading(m_data->smoothNormals);   // flat vs faceted at render time
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

  void SoftBodyDriver::pushConfig(bool live) {
    ClothCfg c;
    c.kLST     = (float)prop("stiffness").value;
    c.kDF      = (float)prop("friction").value;
    c.kDP      = (float)prop("damping").value;
    c.kCHR     = (float)prop("contact hardness").value;
    c.marginMm = (float)prop("collision margin/mm").value;
    c.selfCol  = (bool)prop("self collision").value;
    const bool deform = m_data->world.isDeformable();
    // "position iterations" / "collision mode" only exist in SoftRigid mode (the
    // deformable solver ignores them); default them when absent.
    c.piter    = deform ? 10 : (int)prop("position iterations").value;
    c.clusters = deform ? false : ((std::string)prop("collision mode").value == "Clusters");
    if (deform) {
      float nodeMass = m_data->mass / std::max(1, m_data->resX * m_data->resY);
      deformParams(c.kLST, c.kDP, nodeMass, c.ksDeform, c.kdDeform);
    }
    if (live) {
      // Apply on the sim thread (the body is mid-simulation).
      btSoftBody *body = m_data->body;
      Units u = m_data->units;
      PhysicsWorld *w = &m_data->world;
      m_data->world.enqueue([=]() {
        if (deform) applyDeformableCfg(body, c, u, *w);
        else        applyClothCfg(body, c, u);
      });
    } else {
      if (deform) applyDeformableCfg(m_data->body, c, m_data->units, m_data->world);
      else        applyClothCfg(m_data->body, c, m_data->units);
    }
  }

  void SoftBodyDriver::placeFlatPatch(const Vec &c00, const Vec &c10,
                                      const Vec &c01, const Vec &c11, bool rebakeRest) {
    btSoftBody *body = m_data->body;
    if (!body) return;
    const int rx = m_data->resX, ry = m_data->resY;
    const Units u = m_data->units;
    m_data->world.enqueue([=]() {
      // Lay nodes onto the bilinear flat patch (node idx = j*rx + i), zero
      // velocity/forces; pinned corners keep their zero inverse mass.
      for (int j = 0; j < ry; j++) {
        for (int i = 0; i < rx; i++) {
          float fu = rx > 1 ? (float)i / (rx - 1) : 0.f;
          float fv = ry > 1 ? (float)j / (ry - 1) : 0.f;
          float w00 = (1-fu)*(1-fv), w10 = fu*(1-fv), w01 = (1-fu)*fv, w11 = fu*fv;
          Vec p(w00*c00[0] + w10*c10[0] + w01*c01[0] + w11*c11[0],
                w00*c00[1] + w10*c10[1] + w01*c01[1] + w11*c11[1],
                w00*c00[2] + w10*c10[2] + w01*c01[2] + w11*c11[2], 1);
          btVector3 bp = u.toBulletVec(p);
          btSoftBody::Node &n = body->m_nodes[j * rx + i];
          n.m_x = bp; n.m_q = bp;
          n.m_v = btVector3(0, 0, 0);
          n.m_f = btVector3(0, 0, 0);
        }
      }
      // rebakeRest: make THIS flat shape the rest state (rest lengths scale with
      // size) via updateConstants(). Otherwise keep the original rest lengths so
      // the cloth springs back to its built size (a pure reset).
      if (rebakeRest) body->updateConstants();
      else { body->updateNormals(); body->updateBounds(); }
    });
  }

  void SoftBodyDriver::reset() {
    Vec s00, s10, s01, s11;
    cornersAtSize(s00, s10, s01, s11);
    placeFlatPatch(s00, s10, s01, s11, /*rebakeRest*/ false);
  }

  void SoftBodyDriver::applySize() {
    Vec s00, s10, s01, s11;
    cornersAtSize(s00, s10, s01, s11);
    placeFlatPatch(s00, s10, s01, s11, /*rebakeRest*/ true);
  }

  void SoftBodyDriver::sync(double /*dt*/, double /*alpha*/) {
    auto *mesh = dynamic_cast<viz3d::MeshNode *>(node());
    if (!mesh) return;
    std::vector<Vec> pos;
    if (!m_data->buffer.sample(pos)) return;
    auto &v = mesh->getVertices();
    if (v.size() == pos.size()) std::copy(pos.begin(), pos.end(), v.begin());
    // Always compute smooth per-vertex normals; the node's smooth-shading flag
    // (set from the "smooth normals" property) decides flat vs smooth at render.
    mesh->createAutoNormals(true);
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
    const bool deform = m_data->world.isDeformable();
    m_data->world.enqueue([sb, nodeIndex, rb, influence, deform]() {
      if (nodeIndex < 0 || nodeIndex >= sb->m_nodes.size()) return;
      // The deformable solver ignores legacy m_anchors — it has its own
      // node-anchor constraint (btDeformableNodeAnchorConstraint).
      if (deform) sb->appendDeformableAnchor(nodeIndex, rb);
      else        sb->appendAnchor(nodeIndex, rb, true, influence);
    });
  }

} // namespace icl::physics2
