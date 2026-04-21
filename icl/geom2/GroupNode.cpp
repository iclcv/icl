// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/GroupNode.h>
#include <algorithm>

namespace icl::geom2 {

  struct GroupNode::Data {
    std::vector<std::shared_ptr<SceneNode>> children;
  };

  GroupNode::GroupNode() : m_data(new Data) {}

  GroupNode::~GroupNode() { delete m_data; }

  GroupNode::GroupNode(const GroupNode &other) : SceneNode(other), m_data(new Data) {
    m_data->children.resize(other.m_data->children.size());
    for (size_t i = 0; i < other.m_data->children.size(); ++i) {
      m_data->children[i].reset(other.m_data->children[i]->deepCopy());
    }
  }

  GroupNode &GroupNode::operator=(const GroupNode &other) {
    if (this != &other) {
      GroupNode tmp(other);
      SceneNode::operator=(std::move(tmp));
      std::swap(m_data, tmp.m_data);
    }
    return *this;
  }

  GroupNode::GroupNode(GroupNode &&other) noexcept
      : SceneNode(std::move(other)), m_data(other.m_data) {
    other.m_data = nullptr;
  }

  GroupNode &GroupNode::operator=(GroupNode &&other) noexcept {
    if (this != &other) {
      SceneNode::operator=(std::move(other));
      delete m_data;
      m_data = other.m_data;
      other.m_data = nullptr;
    }
    return *this;
  }

  SceneNode *GroupNode::deepCopy() const { return new GroupNode(*this); }

  void GroupNode::addChild(std::shared_ptr<SceneNode> child) {
    // Set parent (friend access to SceneNode::m_data)
    child->setParent(this);
    m_data->children.push_back(std::move(child));
  }

  void GroupNode::removeChild(SceneNode *child) {
    auto &c = m_data->children;
    auto it = std::find_if(c.begin(), c.end(),
                           [child](const auto &p) { return p.get() == child; });
    if (it != c.end()) {
      (*it)->setParent(nullptr);
      c.erase(it);
    }
  }

  void GroupNode::removeAllChildren() {
    for (auto &c : m_data->children) c->setParent(nullptr);
    m_data->children.clear();
  }

  int GroupNode::getChildCount() const { return (int)m_data->children.size(); }

  SceneNode *GroupNode::getChild(int i) {
    return (i >= 0 && i < (int)m_data->children.size()) ? m_data->children[i].get() : nullptr;
  }

  const SceneNode *GroupNode::getChild(int i) const {
    return (i >= 0 && i < (int)m_data->children.size()) ? m_data->children[i].get() : nullptr;
  }

  std::shared_ptr<SceneNode> GroupNode::getChildPtr(int i) {
    return (i >= 0 && i < (int)m_data->children.size()) ? m_data->children[i] : nullptr;
  }

} // namespace icl::geom2
