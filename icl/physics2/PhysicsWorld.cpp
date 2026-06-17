// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PhysicsWorld.h>

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>
#include <BulletCollision/NarrowPhaseCollision/btPersistentManifold.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace icl::physics2 {
  namespace {
    /// Collects Bullet's debug-draw line segments (Bullet units).
    struct DebugDrawCollector : public btIDebugDraw {
      std::vector<std::pair<btVector3, btVector3>> lines;
      int mode = DBG_DrawWireframe;
      void drawLine(const btVector3 &from, const btVector3 &to,
                    const btVector3 & /*color*/) override {
        lines.push_back({from, to});
      }
      void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int,
                            const btVector3 &) override {}
      void reportErrorWarning(const char *) override {}
      void draw3dText(const btVector3 &, const char *) override {}
      void setDebugMode(int m) override { mode = m; }
      int getDebugMode() const override { return mode; }
    };
  }
}

namespace icl::physics2 {

  struct PhysicsWorld::Data {
    btSoftBodyRigidBodyCollisionConfiguration *config = nullptr;
    btCollisionDispatcher *dispatcher = nullptr;
    btBroadphaseInterface *broadphase = nullptr;
    btSequentialImpulseConstraintSolver *solver = nullptr;
    btDefaultSoftBodySolver *softSolver = nullptr;
    btSoftRigidDynamicsWorld *world = nullptr;
    btSoftBodyWorldInfo *worldInfo = nullptr;
    btGhostPairCallback *ghostCb = nullptr;   // enables ghost-object overlap tracking

    Units units;
    DebugDrawCollector debugDrawer;

    std::recursive_mutex mutex;
    std::thread thread;
    std::atomic<bool> running{false};

    std::vector<std::function<void()>> commands;
    std::mutex cmdMutex;

    // post-step capture hooks (soft bodies snapshot their nodes here)
    std::vector<std::pair<void *, std::function<void()>>> captures;

    ContactCallback contactCb;
    std::vector<std::pair<int, ForceField>> forceFields;
    int nextFieldId = 1;
  };

  PhysicsWorld::PhysicsWorld() : m_data(std::make_unique<Data>()) {
    m_data->config = new btSoftBodyRigidBodyCollisionConfiguration();
    m_data->dispatcher = new btCollisionDispatcher(m_data->config);
    m_data->broadphase = new btDbvtBroadphase();
    m_data->solver = new btSequentialImpulseConstraintSolver();
    m_data->softSolver = new btDefaultSoftBodySolver();
    m_data->world = new btSoftRigidDynamicsWorld(m_data->dispatcher, m_data->broadphase,
                                                 m_data->solver, m_data->config,
                                                 m_data->softSolver);
    m_data->world->setDebugDrawer(&m_data->debugDrawer);

    // ghost-pair callback so btGhostObject sensors accumulate overlapping pairs
    m_data->ghostCb = new btGhostPairCallback();
    m_data->broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(m_data->ghostCb);

    // soft-body world info (gravity + air density for aerodynamics later).
    // Air density 1.2 kg/m^3 scaled to Bullet length units (= mass/length^3, so
    // scales by meterToBullet^3); without it soft-body drag is ~1000x too strong.
    const float meterToBullet = 0.001f / m_data->units.iclToBullet;
    m_data->worldInfo = new btSoftBodyWorldInfo();
    m_data->worldInfo->air_density =
        static_cast<btScalar>(1.2 * meterToBullet * meterToBullet * meterToBullet);
    m_data->worldInfo->water_density = 0;
    m_data->worldInfo->water_offset = 0;
    m_data->worldInfo->water_normal = btVector3(0, 0, 0);
    m_data->worldInfo->m_broadphase = m_data->broadphase;
    m_data->worldInfo->m_dispatcher = m_data->dispatcher;
    m_data->worldInfo->m_sparsesdf.Initialize();

    setGravity(Vec(0, 0, -9810, 1));   // 9.81 m/s^2 in ICL mm/s^2 (world + info)
  }

  PhysicsWorld::~PhysicsWorld() {
    stop();
    // Order matters: tearing down the world + broadphase removes remaining
    // overlapping pairs, and btHashedOverlappingPairCache::removeOverlappingPair
    // dereferences the internal ghost-pair callback — so ghostCb must outlive
    // the broadphase (delete it LAST). Deleting it early = use-after-free.
    delete m_data->worldInfo;
    delete m_data->world;
    delete m_data->softSolver;
    delete m_data->solver;
    delete m_data->broadphase;
    delete m_data->dispatcher;
    delete m_data->config;
    delete m_data->ghostCb;
  }

  Units PhysicsWorld::getUnits() const { return m_data->units; }

  void PhysicsWorld::setUnitScale(float iclToBullet) {
    m_data->units.iclToBullet = iclToBullet;
  }

  void PhysicsWorld::setGravity(const Vec &g) {
    std::scoped_lock lock(m_data->mutex);
    btVector3 bg = m_data->units.toBulletVec(g);
    m_data->world->setGravity(bg);
    if (m_data->worldInfo) m_data->worldInfo->m_gravity = bg;
  }

  void PhysicsWorld::setGravityEnabled(bool on) {
    setGravity(on ? Vec(0, 0, -9810, 1) : Vec(0, 0, 0, 1));
  }

  void PhysicsWorld::addBody(btRigidBody *body) {
    if (!body) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->addRigidBody(body);
  }

  void PhysicsWorld::removeBody(btRigidBody *body) {
    if (!body) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->removeRigidBody(body);
  }

  void PhysicsWorld::addCollisionObject(btCollisionObject *obj, int group, int mask) {
    if (!obj) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->addCollisionObject(obj, (short)group, (short)mask);
  }

  void PhysicsWorld::removeCollisionObject(btCollisionObject *obj) {
    if (!obj) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->removeCollisionObject(obj);
  }

  void PhysicsWorld::setBodyFilter(btRigidBody *body, int group, int mask) {
    if (!body) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->removeRigidBody(body);
    m_data->world->addRigidBody(body, (short)group, (short)mask);
  }

  void PhysicsWorld::setContactCallback(ContactCallback cb) {
    std::scoped_lock lock(m_data->mutex);
    m_data->contactCb = std::move(cb);
  }

  int PhysicsWorld::addForceField(ForceField field) {
    std::scoped_lock lock(m_data->mutex);
    int id = m_data->nextFieldId++;
    m_data->forceFields.push_back({id, std::move(field)});
    return id;
  }

  void PhysicsWorld::removeForceField(int handle) {
    std::scoped_lock lock(m_data->mutex);
    auto &f = m_data->forceFields;
    f.erase(std::remove_if(f.begin(), f.end(),
                           [handle](const auto &p) { return p.first == handle; }),
            f.end());
  }

  void PhysicsWorld::addSoftBody(btSoftBody *body) {
    if (!body) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->addSoftBody(body);
  }

  void PhysicsWorld::removeSoftBody(btSoftBody *body) {
    if (!body) return;
    std::scoped_lock lock(m_data->mutex);
    m_data->world->removeSoftBody(body);
  }

  btSoftBodyWorldInfo *PhysicsWorld::getSoftBodyWorldInfo() { return m_data->worldInfo; }

  void PhysicsWorld::addCapture(void *token, std::function<void()> fn) {
    std::scoped_lock lock(m_data->mutex);
    m_data->captures.push_back({token, std::move(fn)});
  }

  void PhysicsWorld::removeCapture(void *token) {
    std::scoped_lock lock(m_data->mutex);
    auto &c = m_data->captures;
    c.erase(std::remove_if(c.begin(), c.end(),
                           [token](const auto &p) { return p.first == token; }),
            c.end());
  }

  void PhysicsWorld::enqueue(std::function<void()> cmd) {
    std::scoped_lock lock(m_data->cmdMutex);
    m_data->commands.push_back(std::move(cmd));
  }

  void PhysicsWorld::stepOnce(float dt, int maxSubSteps, float fixedTimeStep) {
    std::scoped_lock lock(m_data->mutex);
    // Drain UI->sim commands before stepping (kinematic targets, mouse grab).
    std::vector<std::function<void()>> cmds;
    { std::scoped_lock cl(m_data->cmdMutex); cmds.swap(m_data->commands); }
    for (auto &c : cmds) c();

    // pre-step: apply force fields to each dynamic rigid body at its position
    if (!m_data->forceFields.empty()) {
      const btCollisionObjectArray &objs = m_data->world->getCollisionObjectArray();
      for (int i = 0; i < objs.size(); i++) {
        btRigidBody *rb = btRigidBody::upcast(objs[i]);
        if (!rb || rb->getInvMass() == 0.f) continue;
        Vec pos = m_data->units.toIclVec(rb->getCenterOfMassPosition());
        Vec f(0, 0, 0, 1);
        for (auto &ff : m_data->forceFields) {
          Vec a = ff.second(pos);
          f[0] += a[0]; f[1] += a[1]; f[2] += a[2];
        }
        rb->applyCentralForce(m_data->units.toBulletVec(f));
        rb->activate();
      }
    }

    m_data->world->stepSimulation(dt, maxSubSteps, fixedTimeStep);

    // post-step: soft bodies snapshot their node positions for the render side
    for (auto &cap : m_data->captures) cap.second();

    // post-step: contact events (per contacting pair this step)
    if (m_data->contactCb) {
      int n = m_data->dispatcher->getNumManifolds();
      for (int i = 0; i < n; i++) {
        btPersistentManifold *m = m_data->dispatcher->getManifoldByIndexInternal(i);
        if (m->getNumContacts() == 0) continue;
        auto *a = static_cast<geom2::Driver *>(m->getBody0()->getUserPointer());
        auto *b = static_cast<geom2::Driver *>(m->getBody1()->getUserPointer());
        Vec p = m_data->units.toIclVec(m->getContactPoint(0).getPositionWorldOnA());
        m_data->contactCb(a, b, p);
      }
    }
  }

  std::vector<DebugLine> PhysicsWorld::getDebugLines() {
    std::scoped_lock lock(m_data->mutex);
    m_data->debugDrawer.lines.clear();
    m_data->world->debugDrawWorld();
    std::vector<DebugLine> out;
    out.reserve(m_data->debugDrawer.lines.size());
    for (const auto &l : m_data->debugDrawer.lines) {
      out.push_back({m_data->units.toIclVec(l.first), m_data->units.toIclVec(l.second)});
    }
    return out;
  }

  void PhysicsWorld::start(float hz) {
    if (m_data->running.exchange(true)) return;   // already running
    const double fixedDt = 1.0 / hz;
    m_data->thread = std::thread([this, fixedDt]() {
      auto prev = std::chrono::steady_clock::now();
      double acc = 0.0;
      while (m_data->running.load()) {
        auto now = std::chrono::steady_clock::now();
        acc += std::chrono::duration<double>(now - prev).count();
        prev = now;
        if (acc > 0.25) acc = 0.25;                // avoid spiral of death
        int steps = 0;
        while (acc >= fixedDt && steps < 10) {
          stepOnce((float)fixedDt, 1, (float)fixedDt);
          acc -= fixedDt;
          steps++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
  }

  void PhysicsWorld::stop() {
    if (!m_data->running.exchange(false)) return;
    if (m_data->thread.joinable()) m_data->thread.join();
  }

  bool PhysicsWorld::isRunning() const { return m_data->running.load(); }

  btDynamicsWorld *PhysicsWorld::getDynamicsWorld() { return m_data->world; }

  void PhysicsWorld::lock() { m_data->mutex.lock(); }
  void PhysicsWorld::unlock() { m_data->mutex.unlock(); }

} // namespace icl::physics2
