// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PhysicsScene.h>
#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/Node.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/render/Primitive.h>

namespace icl::physics2 {

  PhysicsScene::PhysicsScene(SoftBodyMode mode) : m_world(mode) {}

  PhysicsScene::~PhysicsScene() {
    // Stop the sim thread before the scene (and its driver-bearing nodes) tear
    // down, so onDetach() doesn't race a step.
    m_world.stop();
  }

  RigidBodyDriver *PhysicsScene::add(std::shared_ptr<viz3d::Node> node, float mass) {
    m_scene.addNode(node);
    return m_world.addRigidBody(std::move(node), mass);
  }

  SoftBodyDriver *PhysicsScene::addCloth(const Vec &c00, const Vec &c10,
                                         const Vec &c01, const Vec &c11,
                                         int resX, int resY, int fixedCornerMask,
                                         float totalMass) {
    auto mesh = std::make_shared<viz3d::MeshNode>();
    m_scene.addNode(std::static_pointer_cast<viz3d::Node>(mesh));
    return m_world.addDriver<SoftBodyDriver>(std::static_pointer_cast<viz3d::Node>(mesh),
                                             c00, c10, c01, c11, resX, resY,
                                             fixedCornerMask, totalMass);
  }

  PaperDriver *PhysicsScene::addPaper(const utils::Size &cells, const Vec *corners,
                                      bool enableSelfCollision, float initialStiffness,
                                      float maxLinkDist) {
    auto mesh = std::make_shared<viz3d::MeshNode>();
    m_scene.addNode(std::static_pointer_cast<viz3d::Node>(mesh));
    return m_world.addDriver<PaperDriver>(std::static_pointer_cast<viz3d::Node>(mesh),
                                          cells, corners, enableSelfCollision,
                                          initialStiffness, maxLinkDist);
  }

  SensorDriver *PhysicsScene::addSensor(std::shared_ptr<viz3d::Node> node) {
    m_scene.addNode(node);
    return m_world.addDriver<SensorDriver>(std::move(node));
  }

  VehicleDriver *PhysicsScene::addVehicle(std::shared_ptr<viz3d::Node> chassis,
                                          const VehicleDriver::Config &cfg) {
    m_scene.addNode(chassis);
    auto *v = m_world.addDriver<VehicleDriver>(std::move(chassis), cfg);
    for (const auto &wheel : v->getWheelNodes()) m_scene.addNode(wheel);  // render the wheels
    return v;
  }

  std::shared_ptr<Constraint> PhysicsScene::addHinge(viz3d::NodePtr a, viz3d::NodePtr b,
                                                     const Vec &pivA, const Vec &pivB, int axis) {
    return m_world.addHinge(std::move(a), std::move(b), pivA, pivB, axis);
  }
  std::shared_ptr<Constraint> PhysicsScene::addSlider(viz3d::NodePtr a, viz3d::NodePtr b,
                                                      const Vec &pivA, const Vec &pivB, int axis) {
    return m_world.addSlider(std::move(a), std::move(b), pivA, pivB, axis);
  }
  std::shared_ptr<Constraint> PhysicsScene::addBallSocket(viz3d::NodePtr a, viz3d::NodePtr b,
                                                          const Vec &pivA, const Vec &pivB) {
    return m_world.addBallSocket(std::move(a), std::move(b), pivA, pivB);
  }
  std::shared_ptr<Constraint> PhysicsScene::addSixDOF(viz3d::NodePtr a, viz3d::NodePtr b,
                                                      const Vec &pivA, const Vec &pivB) {
    return m_world.addSixDOF(std::move(a), std::move(b), pivA, pivB);
  }
  std::shared_ptr<SpringConstraint> PhysicsScene::addSpring(viz3d::NodePtr obj, const Vec &localOffset,
                                                           const Vec &worldPoint, float stiffness, float damping) {
    return m_world.addSpring(std::move(obj), localOffset, worldPoint, stiffness, damping);
  }

  void PhysicsScene::setupDefault(viz3d::DefaultScene::SceneType type, float extent) {
    // Z-up to match the default gravity (0,0,-9810); preset furnishes camera,
    // lamp rig and the checkerboard ground (scaled to extent).
    m_scene.setUpAxis('Z');
    m_scene.setExtent(extent);
    m_scene.setSceneType(type);

    // Static ground collider, top coincident with DefaultScene's visual ground
    // (which sits at z = -extent/2 - 2% along the up axis). Created at the origin
    // then translated (the static body reads its pose from the node), invisible
    // so only the checkerboard is drawn.
    const float half = extent * 0.5f;
    const float groundLevel = -half - extent * 0.02f;
    const float gs = extent * 6.0f;       // match the visual ground half-size
    const float thick = extent * 0.5f;
    auto collider = viz3d::CuboidNode::create(0, 0, 0, 2 * gs, 2 * gs, thick);
    collider->translate(0, 0, groundLevel - thick * 0.5f);
    collider->setVisible(false);
    add(std::static_pointer_cast<viz3d::Node>(collider), 0.0f);  // static; scene keeps it alive
  }

  void PhysicsScene::addNode(std::shared_ptr<viz3d::Node> node) {
    m_scene.addNode(std::move(node));
  }

  void PhysicsScene::addLight(std::shared_ptr<viz3d::LightNode> light) {
    m_scene.addLight(std::move(light));
  }

  void PhysicsScene::addCamera(const cv3d::Camera &cam) { m_scene.addCamera(cam); }
  void PhysicsScene::setBounds(float maxDim) { m_scene.setBounds(maxDim); }

  void PhysicsScene::start(float hz) { m_world.start(hz); }
  void PhysicsScene::stop() { m_world.stop(); }
  void PhysicsScene::stepOnce(float dt, int maxSubSteps, float fixedTimeStep) {
    m_world.stepOnce(dt, maxSubSteps, fixedTimeStep);
  }

  void PhysicsScene::sync(double dt, double alpha) {
    m_scene.sync(dt, alpha);   // drivers pull body poses into their nodes
    if (m_debugEnabled && m_debugNode) {
      auto lines = m_world.getDebugLines();   // (locks the world internally)
      // Mutate the scene node under the SCENE lock — Scene::render() holds it on
      // the GL thread, so an unguarded mutation here would race it (crash).
      std::scoped_lock<viz3d::Scene> lk(m_scene);
      m_debugNode->clearGeometry();
      const viz3d::GeomColor green(0, 255, 0, 255);   // MeshNode colors are 0..255
      int i = 0;
      for (const auto &l : lines) {
        m_debugNode->addVertex(l.a, green);
        m_debugNode->addVertex(l.b, green);
        m_debugNode->addLine(i, i + 1, green);
        i += 2;
      }
    }
  }

  void PhysicsScene::setDebugDrawEnabled(bool on) {
    m_debugEnabled = on;
    if (on && !m_debugNode) {
      m_debugNode = std::make_shared<viz3d::MeshNode>();
      m_debugNode->setName("physics2-debug-overlay");
      m_debugNode->setRenderOnTop(true);   // draw over the solids it traces
      m_debugNode->setLineWidth(2.0f);
      m_scene.addNode(std::static_pointer_cast<viz3d::Node>(m_debugNode));
    }
    if (m_debugNode) m_debugNode->setVisible(on);
  }

  bool PhysicsScene::getDebugDrawEnabled() const { return m_debugEnabled; }

  std::shared_ptr<qt::GLCallback> PhysicsScene::getGLCallback(int cameraIndex) {
    return m_scene.getGLCallback(cameraIndex);
  }
  viz3d::SceneMouseHandler *PhysicsScene::getMouseHandler(int cameraIndex) {
    return m_scene.getMouseHandler(cameraIndex);
  }

  viz3d::Scene &PhysicsScene::scene() { return m_scene; }
  PhysicsWorld &PhysicsScene::world() { return m_world; }

} // namespace icl::physics2
