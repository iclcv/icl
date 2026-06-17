// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PhysicsScene.h>
#include <icl/geom2/Node.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/Primitive.h>

namespace icl::physics2 {

  PhysicsScene::PhysicsScene() = default;

  PhysicsScene::~PhysicsScene() {
    // Stop the sim thread before the scene (and its driver-bearing nodes) tear
    // down, so onDetach() doesn't race a step.
    m_world.stop();
  }

  RigidBodyDriver *PhysicsScene::add(std::shared_ptr<geom2::Node> node, float mass) {
    m_scene.addNode(node);
    return m_world.addRigidBody(std::move(node), mass);
  }

  SoftBodyDriver *PhysicsScene::addCloth(const Vec &c00, const Vec &c10,
                                         const Vec &c01, const Vec &c11,
                                         int resX, int resY, int fixedCornerMask,
                                         float totalMass) {
    auto mesh = std::make_shared<geom2::MeshNode>();
    m_scene.addNode(std::static_pointer_cast<geom2::Node>(mesh));
    return m_world.addDriver<SoftBodyDriver>(std::static_pointer_cast<geom2::Node>(mesh),
                                             c00, c10, c01, c11, resX, resY,
                                             fixedCornerMask, totalMass);
  }

  SensorDriver *PhysicsScene::addSensor(std::shared_ptr<geom2::Node> node) {
    m_scene.addNode(node);
    return m_world.addDriver<SensorDriver>(std::move(node));
  }

  void PhysicsScene::setupDefault(geom2::DefaultScene::SceneType type, float extent) {
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
    auto collider = geom2::CuboidNode::create(0, 0, 0, 2 * gs, 2 * gs, thick);
    collider->translate(0, 0, groundLevel - thick * 0.5f);
    collider->setVisible(false);
    add(std::static_pointer_cast<geom2::Node>(collider), 0.0f);  // static; scene keeps it alive
  }

  void PhysicsScene::addNode(std::shared_ptr<geom2::Node> node) {
    m_scene.addNode(std::move(node));
  }

  void PhysicsScene::addLight(std::shared_ptr<geom2::LightNode> light) {
    m_scene.addLight(std::move(light));
  }

  void PhysicsScene::addCamera(const geom::Camera &cam) { m_scene.addCamera(cam); }
  void PhysicsScene::setBounds(float maxDim) { m_scene.setBounds(maxDim); }

  void PhysicsScene::start(float hz) { m_world.start(hz); }
  void PhysicsScene::stop() { m_world.stop(); }
  void PhysicsScene::stepOnce(float dt, int maxSubSteps, float fixedTimeStep) {
    m_world.stepOnce(dt, maxSubSteps, fixedTimeStep);
  }

  void PhysicsScene::sync(double dt, double alpha) {
    m_scene.sync(dt, alpha);   // drivers pull body poses into their nodes
    if (m_debugEnabled && m_debugNode) {
      auto lines = m_world.getDebugLines();
      m_debugNode->clearGeometry();
      const geom2::GeomColor green(0, 1, 0, 1);
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
      m_debugNode = std::make_shared<geom2::MeshNode>();
      m_debugNode->setName("physics2-debug-overlay");
      m_debugNode->setRenderOnTop(true);   // draw over the solids it traces
      m_debugNode->setLineWidth(2.0f);
      m_scene.addNode(std::static_pointer_cast<geom2::Node>(m_debugNode));
    }
    if (m_debugNode) m_debugNode->setVisible(on);
  }

  bool PhysicsScene::getDebugDrawEnabled() const { return m_debugEnabled; }

  std::shared_ptr<qt::GLCallback> PhysicsScene::getGLCallback(int cameraIndex) {
    return m_scene.getGLCallback(cameraIndex);
  }
  geom2::Scene2MouseHandler *PhysicsScene::getMouseHandler(int cameraIndex) {
    return m_scene.getMouseHandler(cameraIndex);
  }

  geom2::Scene2 &PhysicsScene::scene() { return m_scene; }
  PhysicsWorld &PhysicsScene::world() { return m_world; }

} // namespace icl::physics2
