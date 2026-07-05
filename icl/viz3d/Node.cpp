// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/Node.h>
#include <icl/viz3d/Driver.h>
#include <icl/viz3d/Scene2.h>
#include <icl/math/la/FixedMatrix.h>
#include <algorithm>
#include <mutex>
#include <cmath>

namespace icl::viz3d {

  struct Node::Data {
    Mat transformation = Mat::id();
    bool hasTransformation = false;
    Node *parent = nullptr;
    Scene2 *scene = nullptr;       // owning scene (non-owning back-pointer)
    bool isVisible = true;
    mutable std::recursive_mutex mutex;
    std::string name;
    std::vector<std::shared_ptr<Driver>> drivers;
  };

  Node::Node() : m_data(std::make_unique<Data>()) {}

  Node::~Node() {
    // Upholds the RAII contract "destroy the node -> its drivers release their
    // external resources" (e.g. a physics RigidBodyDriver removes its body — and
    // any constraints referencing it — from the world). onDetach is idempotent
    // (guarded), so this is safe even when a driver was already removed via
    // removeDriver. Contract: onDetach must not call back into derived-node
    // virtuals here — the derived subobject is already destroyed.
    if (m_data) for (auto &d : m_data->drivers) if (d) d->onDetach();
  }

  Node::Node(const Node &other) : m_data(std::make_unique<Data>()) {
    m_data->transformation = other.m_data->transformation;
    m_data->hasTransformation = other.m_data->hasTransformation;
    m_data->isVisible = other.m_data->isVisible;
    m_data->name = other.m_data->name;
  }

  Node &Node::operator=(const Node &other) {
    if (this != &other) {
      auto *parent = m_data->parent;
      m_data->transformation = other.m_data->transformation;
      m_data->hasTransformation = other.m_data->hasTransformation;
      m_data->isVisible = other.m_data->isVisible;
      m_data->name = other.m_data->name;
      m_data->parent = parent;
    }
    return *this;
  }

  Node::Node(Node &&other) noexcept = default;
  Node &Node::operator=(Node &&other) noexcept = default;

  void Node::setTransformation(const Mat &m) {
    m_data->transformation = m;
    m_data->hasTransformation = true;
  }

  void Node::removeTransformation() {
    m_data->transformation = Mat::id();
    m_data->hasTransformation = false;
  }

  void Node::transform(const Mat &m) {
    m_data->transformation = m_data->transformation * m;
    m_data->hasTransformation = true;
  }

  void Node::rotate(float rx, float ry, float rz) {
    // Use ICL's proven Euler angle rotation (rxyz convention, same as cv3d::SceneObject)
    transform(math::create_hom_4x4<float>(rx, ry, rz));
  }

  void Node::translate(float dx, float dy, float dz) {
    // T = T * Tr where Tr has translation in column 3.
    // Result: column 3 of T gets T*[dx,dy,dz,1]^T
    // i.e. T(3,r) += T(0,r)*dx + T(1,r)*dy + T(2,r)*dz for each row r
    auto &T = m_data->transformation;
    for (int r = 0; r < 4; r++) {
      T(r, 3) += T(r, 0)*dx + T(r, 1)*dy + T(r, 2)*dz;
    }
    m_data->hasTransformation = true;
  }

  void Node::scale(float sx, float sy, float sz) {
    // T = T * S: scale columns 0,1,2
    auto &T = m_data->transformation;
    for (int r = 0; r < 4; r++) {
      T(r, 0) *= sx;
      T(r, 1) *= sy;
      T(r, 2) *= sz;
    }
    m_data->hasTransformation = true;
  }

  Mat Node::getTransformation(bool includeParent) const {
    if (includeParent && m_data->parent) {
      return m_data->parent->getTransformation(true) * m_data->transformation;
    }
    return m_data->transformation;
  }

  bool Node::hasTransformation(bool includeParent) const {
    if (m_data->hasTransformation) return true;
    if (includeParent && m_data->parent) return m_data->parent->hasTransformation(true);
    return false;
  }

  void Node::setVisible(bool visible) { m_data->isVisible = visible; }
  bool Node::isVisible() const { return m_data->isVisible; }

  Node *Node::getParent() { return m_data->parent; }
  const Node *Node::getParent() const { return m_data->parent; }

  Scene2 *Node::getScene() const { return m_data->scene; }
  void Node::setScene(Scene2 *scene) { m_data->scene = scene; }

  // RAII: lock the owning scene around an edit, mark it changed on exit. The
  // scene pointer is captured up-front so a re-parenting edit still releases the
  // lock it took. touch() runs under the lock (the renderer reads its cache
  // under the same mutex), then we unlock.
  Node::ScopedEdit::ScopedEdit(Node *node) : m_scene(node->m_data->scene) {
    if (m_scene) m_scene->lock();
  }
  Node::ScopedEdit::~ScopedEdit() {
    if (m_scene) { m_scene->touch(); m_scene->unlock(); }
  }

  void Node::lock() const { m_data->mutex.lock(); }
  void Node::unlock() const { m_data->mutex.unlock(); }

  void Node::setName(const std::string &name) { m_data->name = name; }
  const std::string &Node::getName() const { return m_data->name; }

  void Node::setParent(Node *parent) { m_data->parent = parent; }

  // --- Drivers ---
  // Note: drivers are intentionally NOT copied by the copy ctor/assignment —
  // they are node-bound and stateful, so a deep-copied node starts driverless.

  void Node::addDriver(std::shared_ptr<Driver> driver) {
    if (!driver) return;
    driver->m_node = this;
    m_data->drivers.push_back(driver);
    driver->onAttach();
  }

  const std::vector<std::shared_ptr<Driver>> &Node::getDrivers() const {
    return m_data->drivers;
  }

  void Node::removeDriver(Driver *driver) {
    auto &d = m_data->drivers;
    auto it = std::find_if(d.begin(), d.end(),
                           [driver](const auto &p) { return p.get() == driver; });
    if (it != d.end()) {
      (*it)->onDetach();
      (*it)->m_node = nullptr;
      d.erase(it);
    }
  }

} // namespace icl::viz3d
