// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Node.h>
#include <icl/math/FixedMatrix.h>
#include <mutex>
#include <cmath>

namespace icl::geom2 {

  struct Node::Data {
    Mat transformation = Mat::id();
    bool hasTransformation = false;
    Node *parent = nullptr;
    bool isVisible = true;
    mutable std::recursive_mutex mutex;
    std::string name;
  };

  Node::Node() : m_data(std::make_unique<Data>()) {}

  Node::~Node() = default;

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
    // Use ICL's proven Euler angle rotation (rxyz convention, same as geom::SceneObject)
    transform(math::create_hom_4x4<float>(rx, ry, rz));
  }

  void Node::translate(float dx, float dy, float dz) {
    // T = T * Tr where Tr has translation in column 3.
    // Result: column 3 of T gets T*[dx,dy,dz,1]^T
    // i.e. T(3,r) += T(0,r)*dx + T(1,r)*dy + T(2,r)*dz for each row r
    auto &T = m_data->transformation;
    for (int r = 0; r < 4; r++) {
      T.index_yx(r, 3) += T.index_yx(r, 0)*dx + T.index_yx(r, 1)*dy + T.index_yx(r, 2)*dz;
    }
    m_data->hasTransformation = true;
  }

  void Node::scale(float sx, float sy, float sz) {
    // T = T * S: scale columns 0,1,2
    auto &T = m_data->transformation;
    for (int r = 0; r < 4; r++) {
      T.index_yx(r, 0) *= sx;
      T.index_yx(r, 1) *= sy;
      T.index_yx(r, 2) *= sz;
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

  void Node::lock() const { m_data->mutex.lock(); }
  void Node::unlock() const { m_data->mutex.unlock(); }

  void Node::setName(const std::string &name) { m_data->name = name; }
  const std::string &Node::getName() const { return m_data->name; }

  void Node::setParent(Node *parent) { m_data->parent = parent; }

} // namespace icl::geom2
