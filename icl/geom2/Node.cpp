// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Node.h>
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
    float cx = std::cos(rx), sx = std::sin(rx);
    float cy = std::cos(ry), sy = std::sin(ry);
    float cz = std::cos(rz), sz = std::sin(rz);
    Mat Rx = Mat::id(); Rx(1,1) = cx; Rx(2,1) = sx; Rx(1,2) = -sx; Rx(2,2) = cx;
    Mat Ry = Mat::id(); Ry(0,0) = cy; Ry(2,0) = -sy; Ry(0,2) = sy; Ry(2,2) = cy;
    Mat Rz = Mat::id(); Rz(0,0) = cz; Rz(1,0) = sz; Rz(0,1) = -sz; Rz(1,1) = cz;
    transform(Rz * Ry * Rx);
  }

  void Node::translate(float dx, float dy, float dz) {
    Mat t = Mat::id();
    t(3,0) = dx; t(3,1) = dy; t(3,2) = dz;
    transform(t);
  }

  void Node::scale(float sx, float sy, float sz) {
    Mat s = Mat::id();
    s(0,0) = sx; s(1,1) = sy; s(2,2) = sz;
    transform(s);
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
